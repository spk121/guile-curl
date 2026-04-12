;;; Copyright (C) 2025 Michael L. Gran

;;; This program is free software: you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation, either version 3 of the License, or
;;; (at your option) any later version.

;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;;; GNU General Public License for more details.

;;; You should have received a copy of the GNU General Public License
;;; along with this program. If not, see http://www.gnu.org/licenses/.

(use-modules (srfi srfi-64)
             (web server)
             (web response)
             (web request)
             (ice-9 threads)
             (rnrs bytevectors)
             (rnrs io ports)
             (curl)
             )

(test-begin "mime-post")

(define (fails? thunk)
  (not (false-if-exception
        (begin
          (thunk)
          #t))))

(define (with-temp-file proc)
  (let* ((tmpdir (or (getenv "TMPDIR")
                     (getenv "TEMP")
                     "/tmp"))
         (name (string-append tmpdir "/mimepost-test.XXXXXX"))
         (port (mkstemp! name)))
    (let ((res (with-throw-handler
                #t
                (lambda ()
                  (proc name port))
                (lambda _
                  (delete-file name)))))
      (delete-file name)
      res)))

;; Define a simple web server handler that echoes the request body for POST
(define (simple-handler request request-body)
  (define (body->string body)
    (cond
     ((string? body)
      body)
     ((bytevector? body)
      (utf8->string body))
     ((input-port? body)
      (let ((bv (get-bytevector-all body)))
        (if (eof-object? bv)
            ""
            (utf8->string bv))))
     (else
      (format #f "~a" body))))
  (values (build-response #:code 200)
          (if (eq? (request-method request) 'POST)
              (body->string request-body)
              "Hello, Guile!")))

;; Start the web server in a separate thread
(define (start-web-server port)
  (begin-thread
   (run-server simple-handler 'http `(#:port ,port))))

(define (test-curl-mime-post)
  (let* ((port-str (or (getenv "GUILE_CURL_TEST_PORT") "38088"))
        (port (string->number port-str))
        (url (format #f "http://localhost:~a" port))
        (handle (curl-easy-init)))
    (start-web-server port)
    (usleep 1000000)

    ;; Ensure we are talking to the local test server, not an unrelated service.
    (test-group "server sanity"
                (curl-easy-setopt handle 'url url)
                (let ((response (curl-easy-perform handle #f #f)))
                  (test-assert "GET sanity response"
                    (string-contains response "Hello, Guile!"))))

    ;; Test cases
    (let* ((test-cases
            `((simple-text . ((mimedata . (((name . "numbers")
                                            (data . "12345"))))
                              (checks . (("name=\"numbers\"" #t)
                                         ("12345" #t)))))
              (port-data . ((mimedata . (((name . "portdata")
                                          (port 8 ,(open-input-string "portdata")))))
                            (checks . (("name=\"portdata\"" #t)
                                       ("portdata" #t)))))
              (multiple-parts . ((mimedata . (((name . "numbers")
                                               (data . "12345"))
                                              ((name . "letters")
                                               (data . "abcde"))))
                                 (checks . (("name=\"numbers\"" #t)
                                            ("12345" #t)
                                            ("name=\"letters\"" #t)
                                            ("abcde" #t)))))
              (with-type-and-filename . ((mimedata . (((name . "file")
                                                       (data . "content")
                                                       (type . "text/plain")
                                                       (filename . "test.txt"))))
                                         (checks . (("name=\"file\"" #t)
                                                    ("filename=\"test.txt\"" #t)
                                                    ("Content-Type: text/plain" #t)
                                                    ("content" #t))))))))

      (for-each (lambda (test-case)
                  (let* ((name (car test-case))
                         (props (cdr test-case))
                         (setup (assoc-ref props 'setup))
                          (mimedata (assoc-ref props 'mimedata))
                         (checks (assoc-ref props 'checks))
                         (cleanup (assoc-ref props 'cleanup))
                         (extra-val (if setup
                                        (setup)
                                        #f)))

                    (test-group (symbol->string name)
                                (curl-easy-setopt handle 'url url)
                                (curl-easy-setopt handle 'verbose #f)
                                (curl-easy-setopt handle 'mimepost mimedata)
                                (let ((response (curl-easy-perform handle #f #f)))
                                  (for-each (lambda (check)
                                              (test-assert (format #f "response contains ~s" (car check))
                                                (string-contains response (car check))))
                                            checks)
                                  (test-equal "HTTP response code"
                                    200
                                    (curl-easy-getinfo handle 'response-code)))

                                (if cleanup
                                    (cleanup extra-val)))))
                test-cases))

    (test-group "filedata"

                (with-temp-file
                 (lambda (name port)
                   (display "filecontent" port)
                   (force-output port)
                   (curl-easy-setopt handle 'url url)
                   (curl-easy-setopt handle 'verbose #f)
                   (curl-easy-setopt handle 'mimepost
                                     `(((name . "file")
                                        (filedata . ,name))))
                   (let* ((raw (curl-easy-perform handle #f #f))
                          (response (if (string? raw) raw (utf8->string raw))))

                     (test-assert "response contains name=\"file\""
                       (string-contains response "name=\"file\""))
                     (test-assert (format #f "response contains filename=~S" (basename name))
                       (string-contains response
                                        (format #f "filename=~S" (basename name))))
                     (test-assert "response contains filecontent"
                       (string-contains response "filecontent"))))))

    (test-group "mimepost validation"
                (curl-easy-setopt handle 'url url)
                (test-assert "reject non-list mimepost"
                  (fails? (lambda ()
                            (curl-easy-setopt handle 'mimepost 'not-a-list))))
                (test-assert "reject empty mimepost list"
                  (fails? (lambda ()
                            (curl-easy-setopt handle 'mimepost '()))))
                (test-assert "reject part without name"
                  (fails? (lambda ()
                            (curl-easy-setopt handle 'mimepost
                                              '(((data . "12345")))))))
                (test-assert "reject part without content"
                  (fails? (lambda ()
                            (curl-easy-setopt handle 'mimepost
                                              '(((name . "x")))))))
                (test-assert "reject unknown part key"
                  (fails? (lambda ()
                            (curl-easy-setopt handle 'mimepost
                                              '(((name . "x")
                                                 (data . "123")
                                                 (bogus . "y")))))))
                (test-assert "reject wrong data type"
                  (fails? (lambda ()
                            (curl-easy-setopt handle 'mimepost
                                              '(((name . "x")
                                                 (data . 123)))))))
                (test-assert "reject malformed port tuple arity"
                  (fails? (lambda ()
                            (curl-easy-setopt handle 'mimepost
                                              `(((name . "x")
                                                 (port ,(open-input-string "abc"))))))))
                (test-assert "reject negative port size"
                  (fails? (lambda ()
                            (curl-easy-setopt handle 'mimepost
                                              `(((name . "x")
                                                 (port -1 ,(open-input-string "abc"))))))))
                (test-assert "reject non-string headers"
                  (fails? (lambda ()
                            (curl-easy-setopt handle 'mimepost
                                              '(((name . "x")
                                                 (data . "123")
                                                 (headers . ("X-Test: yes" 42)))))))))

    ;; Common getinfo tests after one perform
    (test-group "curl-easy-getinfo"
                (test-assert "Total time"
                  (number? (curl-easy-getinfo handle 'total-time)))

                (test-assert "Content length"
                  (number? (curl-easy-getinfo handle 'content-length-download-t)))

                (test-assert "Active socket"
                  (number? (curl-easy-getinfo handle 'activesocket))))

    ;; Cleanup
    (curl-easy-cleanup handle)))

(test-curl-mime-post)

(test-end "mime-post")
