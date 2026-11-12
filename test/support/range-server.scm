#!/usr/bin/env guile
!#
;; Test helper copied from khala-private/tests/support/range-server.scm.
;; Minimal threaded HTTP Range server for Khala and guile-curl tests.
(use-modules (ice-9 ftw) (ice-9 match) (ice-9 rdelim) (ice-9 threads)
             (rnrs bytevectors) (rnrs io ports) (srfi srfi-1)
             (srfi srfi-13) (ice-9 popen) (ice-9 format))

(define root #f)
(define port-number 18080)
(define rate #f)
(define fail-offset #f)
(define fail-count 0)
(define log-file #f)
(define lock (make-mutex))
(define failures-left 0)
(define etags (make-hash-table))

(define (option args name default)
  (let ((tail (member name args)))
    (if (and tail (pair? (cdr tail))) (cadr tail) default)))
(let* ((args (cdr (command-line)))
       (fail (option args "--fail-chunk" #f)))
  (set! root (option args "--root" #f))
  (set! port-number (string->number (option args "--port" "18080")))
  (set! rate (and (option args "--rate" #f)
                  (string->number (option args "--rate" "0"))))
  (set! log-file (option args "--log" #f))
  (when fail
    (let ((parts (string-split fail #\:)))
      (set! fail-offset (string->number (car parts)))
      (set! fail-count (string->number (cadr parts)))))
  (unless root (error "--root is required")))
(set! failures-left fail-count)

(define (sha256 path)
  (let* ((p (open-pipe* OPEN_READ "sha256sum" path))
         (line (read-line p)))
    (close-pipe p)
    (substring line 0 64)))

(define (scan-files dir)
  (for-each
   (lambda (name)
     (let ((path (string-append dir "/" name)))
       (cond ((file-is-directory? path) (scan-files path))
             ((file-exists? path)
              (hash-set! etags (string-drop path (+ (string-length root) 1))
                         (sha256 path))))))
   (scandir dir (lambda (name) (not (member name '("." "..")))))))

(define (header-value headers key)
  (let ((entry (find (lambda (pair) (string-ci=? (car pair) key)) headers)))
    (and entry (cdr entry))))

(define (parse-request port)
  (let ((request (read-line port)))
    (if (eof-object? request) #f
        (let ((parts (string-split (string-trim-right request) #\space)))
          (let loop ((headers '()))
            (let* ((raw (read-line port))
                   (line (if (eof-object? raw) raw (string-trim-right raw))))
              (if (or (eof-object? line) (string-null? line))
                  (list (car parts) (cadr parts) headers)
                  (let ((i (string-index line #\:)))
                    (loop (if i
                              (acons (substring line 0 i)
                                     (string-trim-both (substring line (+ i 1)))
                                     headers)
                              headers))))))))))

(define (write-response port code reason headers)
  (format port "HTTP/1.1 ~a ~a\r\n" code reason)
  (for-each (lambda (entry) (format port "~a: ~a\r\n" (car entry) (cdr entry))) headers)
  (display "Connection: close\r\n\r\n" port)
  (force-output port))

(define (parse-range text size)
  (and text
       (let* ((s (string-trim-both text))
              (dash (string-index s #\-)))
         (and (string-prefix? "bytes=" s) dash
              (let ((a (string->number (substring s 6 dash)))
                    (bstr (substring s (+ dash 1))))
                (and a (>= a 0) (< a size)
                     (let ((b (if (string-null? bstr) (- size 1)
                                  (string->number bstr))))
                       (and b (>= b a) (cons a (min b (- size 1)))))))))))

(define (should-fail? range)
  (and fail-offset range (= (car range) fail-offset)
       (with-mutex lock
         (if (> failures-left 0)
             (begin (set! failures-left (- failures-left 1)) #t)
             #f))))

(define (record-request path range)
  (when log-file
    (with-mutex lock
      (call-with-output-file log-file
        (lambda (out)
          (format out "~a ~a~%" path (if range
                                        (format #f "bytes=~a-~a" (car range) (cdr range))
                                        "-")))
        #:exists 'append))))

(define (send-file port file start length head? fail?)
  (unless head?
    (let ((in (open-file file "rb")))
      (seek in start SEEK_SET)
      (let loop ((remaining length) (sent 0))
        (if (or (<= remaining 0)
                (and fail? (>= sent (* 1024 1024))))
            (begin
              (close-port in)
              (when (and fail? (>= sent (* 1024 1024)))
                (close-port port)))
            (let* ((data (get-bytevector-n in (min remaining 65536)))
                   (count (bytevector-length data)))
              (put-bytevector port data)
              (force-output port)
              (when (and rate (> rate 0))
                (let ((delay (inexact->exact
                              (round (* 1000000 (/ count rate))))))
                  (usleep delay)))
              (loop (- remaining count) (+ sent count))))))))

(define (serve-request! io request)
  (let* ((method (car request))
         (uri (cadr request))
         (headers (caddr request))
         (clean (car (string-split (string-drop uri 1) #\?)))
         (file (string-append root "/" clean))
         (known (hash-ref etags clean #f)))
    (cond
     ((or (not known) (not (string-prefix? root file)))
      (write-response io 404 "Not Found" '(("Content-Length" . "0"))))
     ((not (member method '("GET" "HEAD")))
      (write-response io 405 "Method Not Allowed" '(("Content-Length" . "0"))))
     (else
      (let* ((size (stat:size (stat file)))
             (raw-range (header-value headers "Range"))
             (range (parse-range raw-range size)))
        (record-request clean range)
        (if (and raw-range (not range))
            (write-response io 416 "Range Not Satisfiable"
              `(("Content-Range" . ,(format #f "bytes */~a" size))
                ("Content-Length" . "0") ("Accept-Ranges" . "bytes")))
            (let* ((start (if range (car range) 0))
                   (end (if range (cdr range) (- size 1)))
                   (length (if range (+ 1 (- end start)) size))
                   (fail? (should-fail? range)))
              (write-response io (if range 206 200)
                (if range "Partial Content" "OK")
                (append
                 `(("Accept-Ranges" . "bytes")
                   ("ETag" . ,(string-append "\"" known "\""))
                   ("Content-Length" . ,(number->string length)))
                 (if range
                     `(("Content-Range" . ,(format #f "bytes ~a-~a/~a" start end size)))
                     '())))
              (send-file io file start length (string=? method "HEAD") fail?))))))))

(define (serve-one sock)
  (dynamic-wind
    (lambda () #t)
    (lambda ()
      (catch #t
        (lambda ()
          (let ((request (parse-request sock)))
            (when request (serve-request! sock request))))
        (lambda (key . args)
          (format (current-error-port) "range-server: ~s ~s~%" key args))))
    (lambda () (catch #t (lambda () (close-port sock)) (lambda _ #t)))))

(scan-files root)
(let ((server (socket AF_INET SOCK_STREAM 0)))
  (setsockopt server SOL_SOCKET SO_REUSEADDR 1)
  (bind server (make-socket-address AF_INET INADDR_LOOPBACK port-number))
  (listen server 64)
  (format #t "Range server listening on 127.0.0.1:~a serving ~a~%" port-number root)
  (force-output)
  (let loop ()
    (let ((client (accept server)))
      (call-with-new-thread (lambda () (serve-one (car client))))
      (loop))))
