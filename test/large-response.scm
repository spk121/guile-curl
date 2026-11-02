;; Exercise bytevector and Latin-1 response accumulation with a large body.
(use-modules (curl) (rnrs bytevectors) (rnrs io ports)
             (ice-9 ports) (ice-9 popen) (srfi srfi-64))

(test-begin "large response")

(define size (* 64 1024 1024))
(define path (format #f "/tmp/guile-curl-large-~a" (getpid)))
(define source
  (let* ((pipe (open-pipe* OPEN_READ "head" "-c" (number->string size)
                           "/dev/urandom"))
         (bytes (get-bytevector-all pipe)))
    (close-pipe pipe)
    (call-with-output-file path
      (lambda (port) (put-bytevector port bytes)))
    bytes))
(define url (string-append "file://" path))
(define handle (curl-easy-init))
(curl-easy-setopt handle 'url url)
(define start (get-internal-real-time))
(define response (curl-easy-perform handle #t))
(define elapsed (/ (- (get-internal-real-time) start)
                   internal-time-units-per-second))
(test-equal "bytevector response length" size (bytevector-length response))
(test-assert "bytevector response content" (bytevector=? source response))
(test-assert "64 MiB bytevector transfer finishes under five seconds"
  (< elapsed 5))
(define string-response (curl-easy-perform handle))
(test-equal "string response length" size (string-length string-response))
(test-assert "string response content"
  (let loop ((i 0))
    (or (= i size)
        (and (= (char->integer (string-ref string-response i))
                (bytevector-u8-ref source i))
             (loop (+ i 1))))))
(curl-easy-cleanup handle)
(delete-file path)
(test-end "large response")
