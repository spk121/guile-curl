;;; Copyright (C) 2026 Michael L. Gran

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
             (curl))

(test-begin "sendrecv")

(define (simple-handler request request-body)
  (values (build-response #:code 200)
          (if (eq? (request-method request) 'GET)
              "Hello, Guile!"
              "Unexpected method")))

(define (start-web-server port)
  (begin-thread
   (run-server simple-handler 'http `(#:port ,port))))

(define (send-all* handle request send-fn error-code-fn sleep-fn)
  (let loop ((offset 0)
             (again-count 0))
    (if (>= offset (string-length request))
        #t
        (let* ((chunk (substring request offset (string-length request)))
               (n (send-fn handle chunk)))
          (cond
           ((number? n)
            (loop (+ offset n) 0))
           ((= (error-code-fn) CURLE_AGAIN)
            (if (> again-count 200)
                #f
                (begin
                  (sleep-fn 10000)
                  (loop offset (+ again-count 1)))))
           (else
            #f))))))

(define (receive-all* handle receive-fn error-code-fn sleep-fn)
  (let loop ((acc "")
             (again-count 0))
    (let ((chunk (receive-fn handle 4096)))
      (cond
       ((bytevector? chunk)
        (if (= (bytevector-length chunk) 0)
            acc
            (loop (string-append acc (utf8->string chunk)) 0)))
       ((= (error-code-fn) CURLE_AGAIN)
        (if (> again-count 200)
            acc
            (begin
              (sleep-fn 10000)
              (loop acc (+ again-count 1)))))
       (else
        #f)))))

(define (send-all handle request)
  (send-all* handle request curl-easy-send curl-error-code usleep))

(define (receive-all handle)
  (receive-all* handle curl-easy-receive curl-error-code usleep))

(define (run-again-retry-tests)
  (test-group "CURLE_AGAIN retry logic"
    (let ((send-calls 0)
          (last-error CURLE_OK))
      (define (fake-send _handle _chunk)
        (set! send-calls (+ send-calls 1))
        (cond
         ((= send-calls 1)
          (set! last-error CURLE_AGAIN)
          #f)
         (else
          (set! last-error CURLE_OK)
          64)))
      (define (fake-error) last-error)
      (define (fake-sleep _us) #t)

      (test-assert "send retries after CURLE_AGAIN"
        (send-all* #f (make-string 64 #\x) fake-send fake-error fake-sleep))
      (test-equal "send called twice"
        2
        send-calls))

    (let ((recv-calls 0)
          (last-error CURLE_OK))
      (define (fake-recv _handle _max-bytes)
        (set! recv-calls (+ recv-calls 1))
        (cond
         ((= recv-calls 1)
          (set! last-error CURLE_AGAIN)
          #f)
         ((= recv-calls 2)
          (set! last-error CURLE_OK)
          (string->utf8 "hello"))
         (else
          (set! last-error CURLE_OK)
          #vu8())))
      (define (fake-error) last-error)
      (define (fake-sleep _us) #t)

      (test-equal "receive retries after CURLE_AGAIN"
        "hello"
        (receive-all* #f fake-recv fake-error fake-sleep))
      (test-equal "receive called three times"
        3
        recv-calls))))

(define (test-sendrecv)
  (let* ((port-str (or (getenv "GUILE_CURL_TEST_PORT") "38089"))
         (port (string->number port-str))
         (url (format #f "http://localhost:~a/" port))
         (host-header (format #f "Host: localhost:~a" port))
         (request (string-append "GET / HTTP/1.1\r\n"
                                 host-header "\r\n"
                                 "Connection: close\r\n\r\n"))
         (handle (curl-easy-init)))
    (start-web-server port)
    (usleep 1000000)

    (test-group "connect-only send/receive"
      (curl-easy-setopt handle 'url url)
      (curl-easy-setopt handle 'connect-only #t)
      (test-assert "connect-only perform succeeds"
        (not (eq? #f (curl-easy-perform handle #f #f))))
      (test-assert "curl-easy-send sends request"
        (send-all handle request))
      (let ((response (receive-all handle)))
        (test-assert "curl-easy-receive returns response"
          (string? response))
        (test-assert "response contains status line"
          (string-contains response "HTTP/1.1 200"))
        (test-assert "response contains body"
          (string-contains response "Hello, Guile!"))))

    (curl-easy-cleanup handle)))

(test-sendrecv)
(run-again-retry-tests)

(test-end "sendrecv")
