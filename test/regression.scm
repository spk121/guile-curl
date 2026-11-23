;; -*- Mode: scheme; -*-

;;; Copyright (C) 2026 Michael L. Gran

;;; This program is free software: you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation, either version 3 of the License, or
;;; (at your option) any later version.

;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.

;;; You should have received a copy of the GNU General Public License
;;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

;;; Regression tests for memory-management and callback bugs in the C
;;; code.  They talk to Guile's own HTTP server, run in a thread of this
;;; process on a free port on the loopback interface.

(use-modules (srfi srfi-64)
             (web server)
             (web request)
             (web response)
             (web uri)
             (ice-9 threads)
             (ice-9 weak-vector)
             (ice-9 binary-ports)
             (rnrs bytevectors)
             (rnrs io ports)
             (curl))

(test-begin "regression")

;;; The test server.  Every response body is the request method, a
;;; newline, then the request body, so tests can see exactly what curl
;;; sent.  Responses also carry an "X-Test: 1" header.  The path /big
;;; instead returns BIG-BODY, a 1 MB body containing every byte value.

(define big-body
  (let ((bv (make-bytevector (* 1024 1024))))
    (do ((i 0 (1+ i))) ((= i (bytevector-length bv)) bv)
      (bytevector-u8-set! bv i (modulo (* i 7) 256)))))

(define (echo-handler request body)
  (if (equal? (uri-path (request-uri request)) "/big")
      (values (build-response
               #:code 200
               #:headers '((content-type . (application/octet-stream))))
              big-body)
      (echo-reply request body)))

(define (echo-reply request body)
  (call-with-values open-bytevector-output-port
    (lambda (port get-bytes)
      (put-bytevector port
                      (string->utf8
                       (string-append
                        (symbol->string (request-method request)) "\n")))
      (when (bytevector? body)
        (put-bytevector port body))
      (values (build-response
               #:code 200
               #:headers '((content-type . (application/octet-stream))
                           (x-test . "1")))
              (get-bytes)))))

(define (start-test-server)
  "Start the echo server on a free loopback port in a new thread.
Return the server's base URL."
  (let ((sock (socket PF_INET SOCK_STREAM 0)))
    (setsockopt sock SOL_SOCKET SO_REUSEADDR 1)
    (bind sock AF_INET INADDR_LOOPBACK 0)
    (let ((port (sockaddr:port (getsockname sock))))
      (begin-thread
       (run-server echo-handler 'http `(#:socket ,sock)))
      (format #f "http://127.0.0.1:~a/" port))))

(define base-url (start-test-server))

(define (make-handle)
  (let ((h (curl-easy-init)))
    (curl-easy-setopt h 'url base-url)
    ;; Never send these requests through a proxy from the environment.
    (curl-easy-setopt h 'noproxy "*")
    h))

(define (contains? str substr)
  (and (string? str) (string-contains str substr) #t))

;;; A request made with header? true left curl's HEADERDATA pointing at
;;; a dead stack frame, which the next request without headers used.
;;; Made from the same C stack depth, the second request would reuse the
;;; same frame and hide the bug, so it is made from a deeper C stack:
;;; each with-continuation-barrier re-enters the VM from C.

(define (call-with-deeper-c-stack depth thunk)
  (if (zero? depth)
      (thunk)
      (with-continuation-barrier
       (lambda () (call-with-deeper-c-stack (- depth 1) thunk)))))

(test-group "header callback is cleared between transfers"
  (let* ((h (make-handle))
         (first (curl-easy-perform h #f #t))
         (second (call-with-deeper-c-stack
                  20 (lambda () (curl-easy-perform h)))))
    (test-assert "headers are returned when requested"
      (and (list? first) (contains? (car first) "X-Test: 1")))
    (test-equal "body of the request with headers" "GET\n" (cadr first))
    (test-equal "next request without headers returns only the body"
      "GET\n" second)))

;;; CURLOPT_HTTPPOST was set on the wrong object, so the form was never
;;; sent and the heap was corrupted.

(test-group "httppost"
  (let ((h (make-handle)))
    (curl-easy-setopt h 'httppost '(("field" "value")))
    (let ((reply (curl-easy-perform h)))
      (test-assert "request is a POST" (contains? reply "POST\n"))
      (test-assert "form field name is sent"
        (contains? reply "name=\"field\""))
      (test-assert "form field value is sent"
        (contains? reply "\r\n\r\nvalue\r\n")))))

;;; Setting a form again freed the old one incorrectly; form names and
;;; contents must be copied by curl, not borrowed.

(test-group "httppost replaced and copied"
  (let ((h (make-handle)))
    (curl-easy-setopt h 'httppost '(("old1" "a") ("old2" "b") ("old3" "c")))
    (curl-easy-setopt h 'httppost
                      (list (list "bin" (u8-list->bytevector '(65 0 66 0 67)))
                            (list "typed" "text" "text/plain")))
    ;; Collect, so that any buffer curl wrongly borrowed could be reused.
    (gc)
    (let ((reply (curl-easy-perform h)))
      (test-assert "only the newest form is sent"
        (and (contains? reply "name=\"bin\"")
             (not (contains? reply "name=\"old1\""))))
      (test-assert "binary contents with NUL bytes are sent intact"
        (contains? reply (string #\A #\nul #\B #\nul #\C)))
      (test-assert "content type is sent"
        (contains? reply "Content-Type: text/plain")))))

;;; curl_formadd errors were ignored: an empty value became a NULL
;;; pointer, which curl rejected, and the field silently disappeared.

(test-group "httppost errors are not ignored"
  (let ((h (make-handle)))
    (curl-easy-setopt h 'httppost '(("empty" "") ("full" "x")))
    (test-assert "a field with an empty value is sent"
      (contains? (curl-easy-perform h)
                 "name=\"empty\"\r\n\r\n\r\n"))
    (test-equal "a field curl rejects raises an error"
      'misc-error
      (catch #t
        (lambda ()
          (curl-easy-setopt h 'httppost
                            (list (list (string #\a #\nul #\b) "x")))
          'no-error)
        (lambda (key . args) key)))))

;;; The CURLOPT_READDATA port was only referenced from curl, so the GC
;;; could collect it while curl still used it.

(define (set-upload-port! h weak data)
  (let ((port (open-input-string data)))
    (weak-vector-set! weak 0 port)
    (curl-easy-setopt h 'readdata port)))

(test-group "readdata port is kept alive"
  (let ((h (make-handle))
        (weak (make-weak-vector 1 #f))
        (data (make-string 500 #\x)))
    (set-upload-port! h weak data)
    (gc) (gc) (gc)
    (test-assert "port is not collected while the handle holds it"
      (weak-vector-ref weak 0))
    (curl-easy-setopt h 'upload #t)
    (curl-easy-setopt h 'infilesize (string-length data))
    (curl-easy-setopt h 'httpheader '("Expect:"))
    (test-equal "uploaded data comes from the port"
      (string-append "PUT\n" data)
      (curl-easy-perform h))))

;;; A Scheme error inside the read callback unwound through libcurl,
;;; leaving the handle stuck in the middle of a transfer.

(test-group "errors in callbacks"
  (let ((h (make-handle))
        (port (make-custom-binary-input-port
               "failing-port"
               (lambda (bv start count) (error "read failed"))
               #f #f #f)))
    (curl-easy-setopt h 'upload #t)
    (curl-easy-setopt h 'infilesize 10)
    (curl-easy-setopt h 'httpheader '("Expect:"))
    (curl-easy-setopt h 'readdata port)
    (test-equal "the read error is raised by curl-easy-perform"
      'misc-error
      (catch #t
        (lambda () (curl-easy-perform h) 'no-error)
        (lambda (key . args) key)))
    (curl-easy-reset h)
    (curl-easy-setopt h 'url base-url)
    (curl-easy-setopt h 'noproxy "*")
    (test-equal "the handle can be used again afterwards"
      "GET\n"
      (curl-easy-perform h))))

;;; Errors from the C code were raised with scm_error and a key of #f,
;;; which made `throw' itself fail with a wrong-type-arg error and lost
;;; the intended message.

(test-group "errors from C have a proper key and message"
  (let ((h (make-handle)))
    (test-equal "invalid postfields raises misc-error with its message"
      '(misc-error "CURLOPT_POSTFIELDS requires 8-bit string or bytevector data")
      (catch #t
        (lambda () (curl-easy-setopt h 'postfields "λ") 'no-error)
        (lambda (key subr message . rest) (list key message))))))

;;; Every curl_easy_setopt failure was reported as "bad handle", and
;;; curl-error-code was not set.

(test-group "curl-easy-setopt reports curl's error"
  (let* ((h (make-handle))
         (message (catch 'misc-error
                    (lambda () (curl-easy-setopt h 'maxredirs -5) #f)
                    (lambda (key subr fmt args . rest)
                      (apply format #f fmt args)))))
    (test-assert "an error is raised" (string? message))
    (test-assert "the message is not \"bad handle\""
      (not (equal? message "bad handle")))
    (test-equal "curl-error-code is set"
      CURLE_BAD_FUNCTION_ARGUMENT
      (curl-error-code))))

;;; CURLINFO_SPEED_UPLOAD_T was exported but missing from the C code,
;;; so asking for it raised an "unimplemented option type" error.

(test-group "getinfo speed-upload-t"
  (let ((h (make-handle)))
    (curl-easy-perform h)
    (test-assert "returns an integer"
      (integer? (curl-easy-getinfo h 'speed-upload-t)))))

;;; Only successful transfers were counted, and getinfo refuses to run
;;; before the first one, so nothing could be asked after a failure.

(define (closed-port-url)
  "Return a URL for a loopback port with nothing listening on it."
  (let ((sock (socket PF_INET SOCK_STREAM 0)))
    (bind sock AF_INET INADDR_LOOPBACK 0)
    (let ((port (sockaddr:port (getsockname sock))))
      (close-port sock)
      (format #f "http://127.0.0.1:~a/" port))))

(test-group "getinfo after a failed transfer"
  (let ((h (make-handle)))
    (curl-easy-setopt h 'url (closed-port-url))
    (test-equal "the transfer fails" #f (curl-easy-perform h))
    (test-assert "os-errno can be retrieved"
      (integer? (curl-easy-getinfo h 'os-errno)))))

;;; copypostfields with a bytevector fell through to "unimplemented
;;; option type", and strings were cut off at the first NUL.

(test-group "copypostfields"
  (let ((h (make-handle)))
    (curl-easy-setopt h 'copypostfields (u8-list->bytevector '(65 0 66)))
    (test-equal "a bytevector is sent intact"
      (string-append "POST\n" (string #\A #\nul #\B))
      (curl-easy-perform h))
    (curl-easy-setopt h 'copypostfields (string #\x #\nul #\y))
    (test-equal "a string with a NUL is sent intact"
      (string-append "POST\n" (string #\x #\nul #\y))
      (curl-easy-perform h))))

;;; An empty form, '(), was taken for an empty list of strings and
;;; rejected.

(test-group "empty httppost"
  (let ((h (make-handle)))
    (test-assert "an empty form is accepted"
      (catch #t
        (lambda () (curl-easy-setopt h 'httppost '()) #t)
        (lambda args #f)))))

;;; Received data was appended by copying the whole body again for each
;;; chunk.  Check that the buffered replacement returns large bodies
;;; intact, with one character per byte in string mode.

(test-group "large response bodies"
  (let ((h (make-handle)))
    (curl-easy-setopt h 'url (string-append base-url "big"))
    (test-assert "as a bytevector"
      (bytevector=? big-body (curl-easy-perform h #t)))
    (test-assert "as a string, one character per byte"
      (let ((str (curl-easy-perform h #f)))
        (and (= (string-length str) (bytevector-length big-body))
             (let loop ((i 0))
               (or (= i (string-length str))
                   (and (= (char->integer (string-ref str i))
                           (bytevector-u8-ref big-body i))
                        (loop (1+ i))))))))))

;;; The error code and error string were process-wide, so a failure in
;;; one thread overwrote the error another thread was about to read.

(test-group "error state is per thread"
  (let ((h (make-handle)))
    (curl-easy-setopt h 'url (closed-port-url))
    (curl-easy-perform h)
    (let ((main-code (curl-error-code))
          (main-string (curl-error-string)))
      (join-thread
       (begin-thread
        (let ((h2 (make-handle)))
          (curl-easy-setopt h2 'url "nosuchprotocol://example/")
          (curl-easy-perform h2))))
      (test-equal "the main thread's error code is CURLE_COULDNT_CONNECT"
        CURLE_COULDNT_CONNECT main-code)
      (test-equal "another thread's failure does not change it"
        main-code (curl-error-code))
      (test-equal "nor the error string"
        main-string (curl-error-string)))))

;;; 'connect-to was accepted by the Scheme layer but not handled in C,
;;; so it always raised an error.

(test-group "connect-to"
  (let* ((h (make-handle))
         (port (string->number
                (car (last-pair (string-split
                                 (string-trim-right base-url #\/) #\:))))))
    ;; Host names under .invalid never resolve, so the request can only
    ;; succeed if curl connects where 'connect-to tells it to.
    (curl-easy-setopt h 'url "http://guile-curl-test.invalid/")
    (curl-easy-setopt h 'connect-to
                      (list (format #f "guile-curl-test.invalid:80:127.0.0.1:~a"
                                    port)))
    (test-equal "the request reaches the host given by connect-to"
      "GET\n"
      (curl-easy-perform h))
    (test-assert "connect-to can be set again"
      (catch #t
        (lambda () (curl-easy-setopt h 'connect-to '()) #t)
        (lambda args #f)))))

(test-end "regression")
