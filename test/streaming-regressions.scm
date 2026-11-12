(use-modules (curl) (ice-9 ftw) (ice-9 popen) (ice-9 rdelim)
             (ice-9 threads) (rnrs bytevectors) (rnrs io ports)
             (srfi srfi-1) (srfi srfi-13) (srfi srfi-64))

(test-begin "thread errors, fd streaming, progress and cancel")

;; The runner may set corporate proxies without local-address exclusions.
(setenv "no_proxy" "127.0.0.1,localhost")
(setenv "NO_PROXY" "127.0.0.1,localhost")

(define temp-dir (mkdtemp "/tmp/guile-curl-regressions.XXXXXX"))
(define port-number 18081)
(define (path name) (string-append temp-dir "/" name))
(define (generate-file name size)
  (let* ((cmd (open-pipe* OPEN_READ "nice" "-n" "19" "ionice" "-c3"
                          "head" "-c" (number->string size) "/dev/urandom"))
         (out (open-file (path name) "wb")))
    (let loop ((remaining size))
      (when (> remaining 0)
        (let ((bv (get-bytevector-n cmd (min remaining (* 1024 1024)))))
          (put-bytevector out bv)
          (loop (- remaining (bytevector-length bv))))))
    (close-port out)
    (close-pipe cmd)))
(define (sha256 file)
  (let* ((p (open-pipe* OPEN_READ "sha256sum" file)) (line (read-line p)))
    (close-pipe p)
    (substring line 0 64)))
(define (start-server rate)
  (let ((pid (primitive-fork)))
    (if (= pid 0)
        (apply execlp "guile" "guile" "test/support/range-server.scm"
               "--root" temp-dir "--port" (number->string port-number)
               (if rate (list "--rate" (number->string rate)) '()))
        (let wait-ready ((tries 0))
          (cond ((zero? (system* "curl" "--noproxy" "*" "-fsS" "-o"
                                "/dev/null" (url "valid.bin"))) pid)
                ((>= tries 900)
                 (error "Range test server failed to start" port-number))
                (else (usleep 100000) (wait-ready (+ tries 1))))))))
(define (stop-server pid)
  (when pid (kill pid SIGTERM) (waitpid pid)))
(define (new-handle url)
  (let ((h (curl-easy-init)))
    (curl-easy-setopt h 'url url)
    h))
(define (url name) (format #f "http://127.0.0.1:~a/~a" port-number name))
(define (hash-equal? a b) (string=? (sha256 a) (sha256 b)))
(define (read-rss)
  (call-with-input-file "/proc/self/status"
    (lambda (p)
      (let loop ((line (read-line p)))
        (cond ((eof-object? line) 0)
              ((string-prefix? "VmRSS:" line)
               (let ((n (string->number (string-trim-both
                                         (substring line 6
                                           (string-index line #\k))))))
                 (* n 1024)))
              (else (loop (read-line p))))))))
(define (make-destination name size)
  (let ((p (open (path name) (logior O_RDWR O_CREAT O_TRUNC) #o644)))
    (truncate-file p size)
    p))

(generate-file "valid.bin" 4096)
(generate-file "one-gib.bin" (* 1024 1024 1024))
(generate-file "quarter-source.bin" (* 256 1024 1024))
(generate-file "half-gib.bin" (* 512 1024 1024))
(define server-pid (start-server #f))

;; A2: each thread's error state must remain private across repeated calls.
(let ((failed? #f) (lock (make-mutex)))
  (define refused
    (call-with-new-thread
     (lambda ()
       (let ((h (new-handle "http://127.0.0.1:1/")))
         (do ((i 0 (+ i 1))) ((= i 50))
           (curl-easy-perform h)
           (unless (and (= (curl-error-code) CURLE_COULDNT_CONNECT)
                        (not (string-null? (curl-error-string))))
             (format (current-error-port) "A2 refused mismatch: ~a ~s~%"
                     (curl-error-code) (curl-error-string))
             (with-mutex lock (set! failed? #t))))
         (curl-easy-cleanup h)))))
  (define valid
    (call-with-new-thread
     (lambda ()
       (let ((h (new-handle (url "valid.bin"))))
         (do ((i 0 (+ i 1))) ((= i 50))
           (curl-easy-perform h)
           (unless (= (curl-error-code) 0)
             (format (current-error-port) "A2 valid mismatch: ~a ~s~%"
                     (curl-error-code) (curl-error-string))
             (with-mutex lock (set! failed? #t))))
         (curl-easy-cleanup h)))))
  (join-thread refused)
  (join-thread valid)
  (test-assert "A2 errors are thread-local across 50 concurrent requests"
    (not failed?)))

;; A3(a): stream 1 GiB to disk with bounded RSS.
(let* ((h (new-handle (url "one-gib.bin")))
       (out (open-file (path "one-gib.out") "wb"))
       (baseline (read-rss))
       (peak baseline)
       (finished? #f)
       (monitor (call-with-new-thread
                 (lambda ()
                   (let loop ()
                     (unless finished?
                       (set! peak (max peak (read-rss)))
                       (usleep 500000)
                       (loop)))))))
  (let ((n (curl-easy-perform-to-fd h (port->fdes out))))
    (set! finished? #t)
    (join-thread monitor)
    (close-port out)
    (test-equal "A3(a) 1 GiB byte count" (* 1024 1024 1024) n)
    (test-assert "A3(a) RSS growth under 64 MiB" (< (- peak baseline) (* 64 1024 1024)))
    (test-assert "A3(a) file digest" (hash-equal? (path "one-gib.bin") (path "one-gib.out"))))
  (curl-easy-cleanup h))

;; A3(b): four range workers pwrite their quarter into one preallocated file.
(let* ((size (* 256 1024 1024)) (quarter (/ size 4))
       (out (make-destination "quarter.out" size))
       (fd (port->fdes out))
       (threads
        (map
         (lambda (i)
           (call-with-new-thread
            (lambda ()
              (let* ((start (* i quarter))
                     (end (- (+ start quarter) 1))
                     (h (new-handle (url "quarter-source.bin"))))
                (curl-easy-setopt h 'range (format #f "~a-~a" start end))
                (let ((n (curl-easy-perform-to-fd h fd start)))
                  (unless (and (number? n) (= n quarter))
                    (format (current-error-port) "A3(b) range ~a-~a: bytes ~s code ~a error ~s~%"
                            start end n (curl-error-code) (curl-error-string)))
                  (curl-easy-cleanup h)
                  n)))))
         '(0 1 2 3))))
  (test-assert "A3(b) four workers wrote exact quarters"
    (every (lambda (thread) (= (join-thread thread) quarter)) threads))
  (force-output out)
  (close-port out)
  (test-assert "A3(b) merged file digest"
    (hash-equal? (path "quarter-source.bin") (path "quarter.out"))))

;; A3(c): a read-only descriptor must fail with errno details.
(let* ((h (new-handle (url "valid.bin")))
       (in (open-file (path "valid.bin") "rb"))
       (result (curl-easy-perform-to-fd h (port->fdes in))))
  (test-assert "A3(c) read-only fd fails" (not result))
  (test-equal "A3(c) write error code" CURLE_WRITE_ERROR (curl-error-code))
  (test-assert "A3(c) reports errno" (string-contains (curl-error-string) "Bad file descriptor"))
  (close-port in)
  (curl-easy-cleanup h))

(stop-server server-pid)
(set! server-pid #f)
(set! server-pid (start-server (* 64 1024 1024)))

;; A4(a): progress is monotonic and reaches the known total.
(let* ((size (* 512 1024 1024))
       (h (new-handle (url "half-gib.bin")))
       (out (open-file (path "progress.out") "wb"))
       (finished? #f) (monotonic? #t) (previous 0) (seen-total #f)
       (poller
        (call-with-new-thread
         (lambda ()
           (let loop ()
             (unless finished?
               (let ((progress (curl-easy-progress h)))
                 (when (< (car progress) previous) (set! monotonic? #f))
                 (set! previous (car progress))
                 (when (and (> (cdr progress) 0) (= (cdr progress) size))
                   (set! seen-total #t)))
               (usleep 100000)
               (loop)))))))
  (let ((n (curl-easy-perform-to-fd h (port->fdes out))))
    (set! finished? #t)
    (join-thread poller)
    (close-port out)
    (test-equal "A4(a) result bytes" size n)
    (test-assert "A4(a) progress monotonic and total known" (and monotonic? seen-total))
    (test-equal "A4(a) final progress pair" (cons size size) (curl-easy-progress h)))
  (curl-easy-cleanup h))

;; A4(b): cancel after approximately one quarter and require prompt abort.
(let* ((h (new-handle (url "half-gib.bin")))
       (out (open-file (path "cancel.out") "wb"))
       (result #t) (worker-code #f)
       (worker (call-with-new-thread
                (lambda ()
                  (set! result (curl-easy-perform-to-fd h (port->fdes out)))
                  (set! worker-code (curl-error-code))))))
  (let wait-until-quarter ((tries 0))
    (cond ((>= (car (curl-easy-progress h)) (/ (* 512 1024 1024) 4)) #t)
          ((or worker-code (>= tries 300)) #f)
          (else (usleep 50000) (wait-until-quarter (+ tries 1)))))
  (let ((cancel-time (get-internal-real-time)))
    (curl-easy-cancel! h)
    (join-thread worker)
    (test-assert "A4(b) cancel returns within one second"
      (< (/ (- (get-internal-real-time) cancel-time) internal-time-units-per-second) 1))
    (test-assert "A4(b) cancelled result is false" (not result))
    (test-equal "A4(b) abort callback code" CURLE_ABORTED_BY_CALLBACK worker-code))
  (close-port out)
  (curl-easy-cleanup h))

(stop-server server-pid)
(for-each delete-file
          (map (lambda (name) (string-append temp-dir "/" name))
               (scandir temp-dir (lambda (name) (not (member name '("." "..")))))))
(rmdir temp-dir)
(test-end "thread errors, fd streaming, progress and cancel")
