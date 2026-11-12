(use-modules (curl) (rnrs bytevectors) (rnrs io ports)
             (ice-9 ports) (srfi srfi-64))

(test-begin "file descriptor transfer")

(define source-path (format #f "/tmp/guile-curl-fd-source-~a" (getpid)))
(define target-path (format #f "/tmp/guile-curl-fd-target-~a" (getpid)))
(define source-bytes (make-bytevector 1048576))
(do ((i 0 (+ i 1))) ((= i (bytevector-length source-bytes)))
  (bytevector-u8-set! source-bytes i (modulo (* i 37) 256)))
(call-with-output-file source-path
  (lambda (port) (put-bytevector port source-bytes)))
(call-with-output-file target-path (lambda (port) (put-bytevector port source-bytes)))

(define handle (curl-easy-init))
(curl-easy-setopt handle 'url (string-append "file://" source-path))
(call-with-output-file target-path
  (lambda (port)
    (let ((count (curl-easy-perform-to-fd handle (port->fdes port) 0)))
      (test-equal "positioned transfer byte count" (bytevector-length source-bytes)
        count)
      (test-equal "progress pair after transfer"
        (cons (bytevector-length source-bytes) (bytevector-length source-bytes))
        (curl-easy-progress handle)))))
(test-assert "positioned transfer content"
  (call-with-input-file target-path
    (lambda (port) (bytevector=? source-bytes (get-bytevector-all port)))))
(curl-easy-cleanup handle)
(delete-file source-path)
(delete-file target-path)
(test-end "file descriptor transfer")
