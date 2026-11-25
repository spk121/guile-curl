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
;;; instead returns BIG-BODY, a 1 MB body containing every byte value,
;;; and the path /missing returns a 404 error.

(define big-body
  (let ((bv (make-bytevector (* 1024 1024))))
    (do ((i 0 (1+ i))) ((= i (bytevector-length bv)) bv)
      (bytevector-u8-set! bv i (modulo (* i 7) 256)))))

(define (echo-handler request body)
  (let ((path (uri-path (request-uri request))))
    (cond
     ((equal? path "/big")
      (values (build-response
               #:code 200
               #:headers '((content-type . (application/octet-stream))))
              big-body))
     ((equal? path "/missing")
      (values (build-response #:code 404) "not found"))
     (else
      (echo-reply request body)))))

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

;;; Forms (CURLOPT_MIMEPOST).  These began as regression tests for the
;;; old curl_formadd code (a form set on the wrong object, freed wrongly,
;;; fields dropped or missing their filename) and now check the same
;;; behaviour of the curl_mime implementation.

(test-group "mimepost"
  (let ((h (make-handle)))
    (curl-easy-setopt h 'mimepost '(("field" "value")))
    (let ((reply (curl-easy-perform h)))
      (test-assert "request is a POST" (contains? reply "POST\n"))
      (test-assert "form field name is sent"
        (contains? reply "name=\"field\""))
      (test-assert "form field value is sent"
        (contains? reply "\r\n\r\nvalue\r\n")))))

;;; Setting a form again must free the old one, and the form's names and
;;; data must be copied, not borrowed.

(test-group "mimepost replaced and copied"
  (let ((h (make-handle)))
    (curl-easy-setopt h 'mimepost '(("old1" "a") ("old2" "b") ("old3" "c")))
    (curl-easy-setopt h 'mimepost
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

;;; A field with an empty value must be sent, not dropped, and a field
;;; that cannot be sent as given must raise an error.

(test-group "mimepost empty values and errors"
  (let ((h (make-handle)))
    (curl-easy-setopt h 'mimepost '(("empty" "") ("full" "x")))
    (test-assert "a field with an empty value is sent"
      (contains? (curl-easy-perform h)
                 "name=\"empty\"\r\n\r\n\r\n"))
    (test-equal "a field name containing NUL raises an error"
      'misc-error
      (catch #t
        (lambda ()
          (curl-easy-setopt h 'mimepost
                            (list (list (string #\a #\nul #\b) "x")))
          'no-error)
        (lambda (key . args) key)))))

;;; Every string in a field is checked, including the MIME type of a
;;; 4-element field.

(define (setopt-error-key h option value)
  (catch #t
    (lambda () (curl-easy-setopt h option value) 'no-error)
    (lambda (key . args) key)))

(test-group "mimepost validation"
  (let ((h (make-handle)))
    (test-equal "a MIME type containing NUL raises an error"
      'misc-error
      (setopt-error-key h 'mimepost
                        (list (list "f" "x" (string #\a #\nul) "f.txt"))))
    (test-equal "a filename containing NUL raises an error"
      'misc-error
      (setopt-error-key h 'mimepost
                        (list (list "f" "x" "text/plain" (string #\a #\nul)))))
    (test-assert "a non-string MIME type in a 4-element field is rejected"
      (not (eq? 'no-error
                (setopt-error-key h 'mimepost '(("f" "x" 42 "f.txt"))))))))

;;; 'httppost, removed in 1.0.0, is back in 1.0.1 as another name for
;;; 'mimepost, for programs written for earlier versions.  It must send
;;; exactly the same request.

(define (without-boundaries text)
  "Replace the random multipart boundary in TEXT, a reply from the echo
server, with a fixed string.  (Guile's regular expressions cannot be used,
since the text may contain NUL characters.)"
  (let* ((start (string-index text #\-))
         (boundary (substring text start (string-contains text "\r\n" start))))
    (let loop ((from 0) (parts '()))
      (let ((at (string-contains text boundary from)))
        (if at
            (loop (+ at (string-length boundary))
                  (cons* "--BOUNDARY" (substring text from at) parts))
            (string-concatenate-reverse (cons (substring text from) parts)))))))

(define (form-request option fields)
  "Send FIELDS with OPTION, and return what the server received, without
the boundaries."
  (let ((h (make-handle)))
    (curl-easy-setopt h option fields)
    (without-boundaries (curl-easy-perform h))))

(test-group "httppost"
  (let ((fields (list '("a" "1")
                      '("typed" "text" "text/plain")
                      '("empty" "")
                      (list "bin" (u8-list->bytevector '(0 1 255 0)))
                      '("upload" "data" "text/plain" "notes.txt"))))
    (test-assert "the request is a multipart POST"
      (string-prefix? "POST\n--BOUNDARY" (form-request 'httppost fields)))
    (test-equal "it sends exactly what mimepost sends"
      (form-request 'mimepost fields)
      (form-request 'httppost fields)))
  (let ((h (make-handle)))
    (curl-easy-setopt h 'httppost '(("a" "1")))
    (curl-easy-setopt h 'httppost '())
    (curl-easy-setopt h 'httpget #t)
    (test-equal "an empty form clears the form" "GET\n" (curl-easy-perform h)))
  (test-equal "invalid fields are rejected as for mimepost"
    (setopt-error-key (make-handle) 'mimepost '(("f" "x" 42 "f.txt")))
    (setopt-error-key (make-handle) 'httppost '(("f" "x" 42 "f.txt")))))

;;; The fourth element of a field is a filename, which sends the field as
;;; a file.

(test-group "mimepost filename"
  (let ((h (make-handle)))
    (curl-easy-setopt h 'mimepost
                      (list (list "up" (u8-list->bytevector '(1 0 2))
                                  "application/octet-stream" "data.bin")))
    ;; Collect, so that any buffer curl wrongly borrowed could be reused.
    (gc)
    (let ((reply (curl-easy-perform h)))
      (test-assert "the filename is sent"
        (contains? reply "name=\"up\"; filename=\"data.bin\""))
      (test-assert "the data is sent intact"
        (contains? reply (string-append "\r\n\r\n"
                                        (string (integer->char 1) #\nul
                                                (integer->char 2))
                                        "\r\n"))))))

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

;;; An empty form, '(), must not be taken for an empty list of strings.

(test-group "empty mimepost"
  (let ((h (make-handle)))
    (curl-easy-setopt h 'mimepost '(("field" "value")))
    (test-assert "an empty form is accepted"
      (catch #t
        (lambda () (curl-easy-setopt h 'mimepost '()) #t)
        (lambda args #f)))
    (curl-easy-setopt h 'httpget #t)
    (test-equal "and clears the form"
      "GET\n"
      (curl-easy-perform h))))

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

;;; A wrong argument for 'readdata reported "unimplemented type:
;;; input-port" because input-port was missing from the list of
;;; implemented types.

(test-group "readdata wrong type"
  (let ((h (make-handle)))
    (test-equal "a non-port argument is reported as a wrong type"
      "wrong type arg: not a port"
      (catch 'misc-error
        (lambda () (curl-easy-setopt h 'readdata "not a port") #f)
        (lambda (key subr fmt args . rest) (apply format #f fmt args))))))

;;; Options and getinfo values were guarded with #ifdef, but libcurl
;;; declares them as enum constants, not macros, so the guards were
;;; never true and these were always #f.  Options from libcurl 7.76.0 or
;;; earlier (the minimum supported) must always exist; newer getinfo
;;; values exist when libcurl is new enough.

(test-group "version-dependent constants"
  (test-assert "CURLOPT_DOH_URL (7.62.0) is defined"
    (integer? (@@ (curl) CURLOPT_DOH_URL)))
  (test-assert "CURLE_RECURSIVE_API_CALL (7.59.0) is defined"
    (integer? CURLE_RECURSIVE_API_CALL))
  (let ((h (make-handle)))
    (test-assert "'doh-url can be set"
      (catch #t
        (lambda () (curl-easy-setopt h 'doh-url "") #t)
        (lambda args #f)))
    (curl-easy-perform h)
    (test-assert "getinfo conn-id works when libcurl has it (8.2.0)"
      (or (not CURLINFO_CONN_ID)
          (integer? (curl-easy-getinfo h 'conn-id))))))

;;; curl-easy-perform-to-fd, curl-easy-progress and curl-easy-cancel!
;;; (from spk121/guile-curl#19 by Nala Ginrut).

(define (temp-file)
  "Return the name of a new, empty temporary file."
  (let* ((port (mkstemp! (string-copy "/tmp/guile-curl-test-XXXXXX")))
         (name (port-filename port)))
    (close-port port)
    name))

(define (file-bytes name)
  (call-with-input-file name get-bytevector-all #:binary #t))

(define (call-with-fd name mode proc)
  "Call PROC with a file descriptor for NAME opened with MODE."
  (let* ((port (open-file name mode))
         (result (proc (fileno port))))
    (close-port port)
    result))

(test-group "perform-to-fd"
  (let ((h (make-handle))
        (name (temp-file)))
    (curl-easy-setopt h 'url (string-append base-url "big"))
    (test-equal "returns the number of bytes written"
      (bytevector-length big-body)
      (call-with-fd name "wb" (lambda (fd) (curl-easy-perform-to-fd h fd))))
    (test-assert "writes the body to the descriptor"
      (bytevector=? big-body (file-bytes name)))
    (test-equal "progress reports the final counts"
      (cons (bytevector-length big-body) (bytevector-length big-body))
      (curl-easy-progress h))
    (delete-file name))

  (let ((h (make-handle))
        (name (temp-file)))
    (call-with-output-file name (lambda (port) (display "0123456789" port)))
    (test-equal "writes at an offset without truncating"
      "0123GET\n89"
      (begin
        (call-with-fd name "r+b" (lambda (fd) (curl-easy-perform-to-fd h fd 4)))
        (utf8->string (file-bytes name))))
    (delete-file name))

  (let ((h (make-handle))
        (name (temp-file)))
    (test-equal "a read-only descriptor fails" #f
      (call-with-fd name "rb" (lambda (fd) (curl-easy-perform-to-fd h fd))))
    (test-equal "with CURLE_WRITE_ERROR" CURLE_WRITE_ERROR (curl-error-code))
    (test-assert "and the system error in the message"
      (string-prefix? "write failed: " (curl-error-string)))
    (delete-file name))

  ;; The transfer runs outside Guile mode; reading an upload from a port
  ;; must re-enter it.
  (let ((h (make-handle))
        (name (temp-file)))
    (curl-easy-setopt h 'readdata (open-input-string "hello"))
    (curl-easy-setopt h 'upload #t)
    (curl-easy-setopt h 'infilesize 5)
    (curl-easy-setopt h 'httpheader '("Expect:"))
    (test-equal "uploads from a port"
      "PUT\nhello"
      (begin
        (call-with-fd name "wb" (lambda (fd) (curl-easy-perform-to-fd h fd)))
        (utf8->string (file-bytes name))))
    (delete-file name))

  (let ((h (make-handle))
        (name (temp-file)))
    (curl-easy-setopt h 'readdata
                      (make-custom-binary-input-port
                       "failing-port"
                       (lambda (bv start count) (error "read failed"))
                       #f #f #f))
    (curl-easy-setopt h 'upload #t)
    (curl-easy-setopt h 'infilesize 10)
    (curl-easy-setopt h 'httpheader '("Expect:"))
    (test-equal "an error reading the upload is raised"
      'misc-error
      (catch #t
        (lambda ()
          (call-with-fd name "wb" (lambda (fd) (curl-easy-perform-to-fd h fd)))
          'no-error)
        (lambda (key . args) key)))
    (curl-easy-reset h)
    (curl-easy-setopt h 'url base-url)
    (curl-easy-setopt h 'noproxy "*")
    (test-equal "and the handle can be used again" "GET\n" (curl-easy-perform h))
    (delete-file name)))

(test-group "cancel"
  (let ((h (make-handle))
        (name (temp-file)))
    (curl-easy-setopt h 'url (string-append base-url "big"))
    ;; Throttle the download so that it is still running when cancelled.
    (curl-easy-setopt h 'max-recv-speed-large 65536)
    (let ((worker
           (call-with-fd name "wb"
             (lambda (fd)
               (let ((thread (begin-thread
                              ;; The error code is per thread, so read it
                              ;; here, in the transferring thread.
                              (let ((result (curl-easy-perform-to-fd h fd)))
                                (cons result (curl-error-code))))))
                 ;; Wait until the transfer is under way.
                 (let wait ((tries 0))
                   (when (and (zero? (car (curl-easy-progress h)))
                              (< tries 200))
                     (usleep 50000)
                     (wait (1+ tries))))
                 (let ((start (get-internal-real-time)))
                   (curl-easy-cancel! h)
                   (let ((outcome (join-thread thread)))
                     (list outcome
                           (/ (- (get-internal-real-time) start)
                              internal-time-units-per-second)))))))))
      (let ((result (car (car worker)))
            (code (cdr (car worker)))
            (seconds (cadr worker)))
        (test-equal "a cancelled transfer returns #f" #f result)
        (test-equal "with CURLE_ABORTED_BY_CALLBACK"
          CURLE_ABORTED_BY_CALLBACK code)
        (test-assert "and stops promptly" (< seconds 3))))
    ;; The cancel request must not affect later transfers on the handle.
    (curl-easy-setopt h 'max-recv-speed-large 0)
    (test-assert "a later curl-easy-perform on the handle works"
      (let ((body (curl-easy-perform h #t)))
        (and body (bytevector=? body big-body))))
    (delete-file name)))

;;; Options, getinfo values and procedures added for libcurl 8.15
;;; (spk121/guile-curl#16).

(define (setopt-ok? h option value)
  (catch #t
    (lambda () (curl-easy-setopt h option value) #t)
    (lambda args #f)))

(test-group "new options"
  (let ((h (make-handle)))
    (test-assert "boolean: ssl-verifystatus" (setopt-ok? h 'ssl-verifystatus #f))
    (test-assert "integer: ssl-options"
      (setopt-ok? h 'ssl-options CURLSSLOPT_NO_REVOKE))
    (test-assert "string: pinnedpublickey" (setopt-ok? h 'pinnedpublickey ""))
    (test-assert "list of strings: proxyheader"
      (setopt-ok? h 'proxyheader '("X-Proxy: 1")))
    (test-assert "list of strings: rtspheader"
      (setopt-ok? h 'rtspheader '("X-Rtsp: 1")))
    (test-assert "blob: sslcert-blob, as a bytevector"
      (setopt-ok? h 'sslcert-blob (u8-list->bytevector '(1 2 3))))
    (test-assert "blob: sslkey-blob, as a string"
      (setopt-ok? h 'sslkey-blob "not a key"))
    (test-equal "a blob rejects other types"
      #f (setopt-ok? h 'sslcert-blob 42))
    ;; The options above do not affect a plain HTTP request.
    (curl-easy-setopt h 'sslcert-blob #vu8())
    (curl-easy-setopt h 'sslkey-blob #vu8())
    (test-equal "the handle still works" "GET\n" (curl-easy-perform h)))

  (let ((h (make-handle)))
    (curl-easy-setopt h 'url (string-append base-url "missing"))
    (test-assert "without failonerror a 404 is returned as a body"
      (string? (curl-easy-perform h)))
    (curl-easy-setopt h 'failonerror #t)
    (test-equal "with failonerror a 404 fails" #f (curl-easy-perform h))
    (test-equal "with CURLE_HTTP_RETURNED_ERROR"
      CURLE_HTTP_RETURNED_ERROR (curl-error-code)))

  (let ((h (make-handle)))
    (if (@@ (curl) CURLOPT_PROTOCOLS_STR)
        (begin
          (curl-easy-setopt h 'protocols-str "ftp")
          (test-equal "protocols-str restricts the protocols"
            #f (curl-easy-perform h)))
        (test-assert "protocols-str is reported as unsupported"
          (not (setopt-ok? h 'protocols-str "ftp")))))

  ;; tcp-keepcnt needs libcurl 8.9.0; check the error on older ones.
  (let ((h (make-handle)))
    (if (@@ (curl) CURLOPT_TCP_KEEPCNT)
        (test-assert "tcp-keepcnt can be set" (setopt-ok? h 'tcp-keepcnt 3))
        (test-equal "an option libcurl lacks raises a clear error"
          "option tcp-keepcnt is not supported by this version of libcurl"
          (catch #t
            (lambda () (curl-easy-setopt h 'tcp-keepcnt 3) #f)
            (lambda (key subr fmt args . rest)
              (apply format #f fmt args)))))))

(test-group "new getinfo values"
  (let ((h (make-handle)))
    (curl-easy-setopt h 'url (string-append base-url "big"))
    (curl-easy-perform h #t)
    (test-equal "content-type" "application/octet-stream"
      (curl-easy-getinfo h 'content-type))
    (test-equal "certinfo is empty without TLS" '()
      (curl-easy-getinfo h 'certinfo))))

(test-group "response headers"
  (let ((h (make-handle)))
    (curl-easy-perform h)
    ;; The header API needs libcurl 7.83.0; older ones raise an error.
    (if (catch 'misc-error
          (lambda () (curl-easy-header h "X-Test") #t)
          (lambda args #f))
        (begin
          (test-equal "curl-easy-header" "1" (curl-easy-header h "X-Test"))
          (test-equal "is case-insensitive" "1" (curl-easy-header h "x-test"))
          (test-equal "returns #f for a missing header" #f
            (curl-easy-header h "X-No-Such-Header"))
          (test-equal "returns #f for a missing occurrence" #f
            (curl-easy-header h "X-Test" 1))
          (test-equal "curl-easy-headers lists the headers" "1"
            (assoc-ref (map (lambda (p) (cons (string-downcase (car p)) (cdr p)))
                            (curl-easy-headers h))
                       "x-test")))
        (test-equal "raises an error on older libcurl" 'misc-error
          (catch #t (lambda () (curl-easy-header h "X-Test") #f)
            (lambda (key . args) key))))))

(define (receive-until h done? timeout-seconds)
  "Receive from H until DONE? accepts the text so far, the connection
closes, or TIMEOUT-SECONDS pass.  Returns the text received."
  (let ((deadline (+ (current-time) timeout-seconds)))
    (let loop ((text ""))
      (if (or (done? text) (> (current-time) deadline))
          text
          (let ((bv (curl-easy-receive h 4096)))
            (cond
             ((not bv)                  ; CURLE_AGAIN: nothing yet
              (usleep 10000)
              (loop text))
             ((zero? (bytevector-length bv)) text)   ; connection closed
             (else
              (loop (string-append text (utf8->string bv))))))))))

(test-group "send and receive"
  (let ((h (make-handle)))
    (curl-easy-setopt h 'connect-only #t)
    (curl-easy-perform h)
    (let ((request "GET / HTTP/1.0\r\nHost: 127.0.0.1\r\n\r\n"))
      (test-equal "curl-easy-send sends the request"
        (string-length request)
        (curl-easy-send h request))
      (let ((reply (receive-until h (lambda (text) (string-suffix? "GET\n" text))
                                  10)))
        (test-assert "curl-easy-receive gets the response"
          (and (string-prefix? "HTTP/1." reply)
               (string-suffix? "\r\n\r\nGET\n" reply))))))
  (let ((h (make-handle)))
    (test-equal "send without a connection fails" #f
      (curl-easy-send h #vu8(1 2 3)))))

(test-group "upkeep"
  (let ((h (make-handle)))
    (curl-easy-perform h)
    (test-equal "curl-easy-upkeep" #t (curl-easy-upkeep h))))

(test-end "regression")
