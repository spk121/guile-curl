;; -*- Mode: scheme; -*-

;;; Copyright (C) 2025 Michael L. Gran

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

(use-modules (srfi srfi-64)
             (web server)
             (web response)
             (web request)
             (ice-9 threads)
             (curl))

(test-begin "getinfo")

;; Define a simple web server handler
(define (simple-handler request request-body)
  (values (build-response #:code 200)
          "Hello, Guile!"))

;; Start the web server in a separate thread
(define (start-web-server port)
  (begin-thread
   (run-server simple-handler 'http `(#:port ,port))))

(define (test-curl-getinfo-http)
  (let ((port 8080)
        (url "http://localhost:8080")
        (handle (curl-easy-init)))
    (start-web-server port)
    ;; Give the server a moment to start
    (usleep 100000)

    ;; Perform curl operation in a separate thread from the web server
    (let ((transfer-thread
           (begin-thread
            (curl-easy-setopt handle 'url url)
            (curl-easy-setopt handle 'verbose #t)
            (curl-easy-perform handle))))
      ;; Wait for transfer to complete
      (join-thread transfer-thread)

      ;; Get info from curl handle
      (test-group "curl-easy-getinfo"
        (test-equal "HTTP Version"
          (curl-easy-getinfo handle 'http-version)
          CURL_HTTP_VERSION_1_1)

        (test-equal "HTTP response code"
          200
          (curl-easy-getinfo handle 'response-code))

        (test-assert "Total time"
          (number? (curl-easy-getinfo handle 'total-time)))

        (test-assert "Content length"
          (equal?
           (curl-easy-getinfo handle 'content-length-download-t)
           (string-length "Hello, Guile!")))

        (test-assert "Active socket"
          (number?
           (curl-easy-getinfo handle 'activesocket)))
        )

      ;; Cleanup
      (curl-easy-cleanup handle))))

(test-curl-getinfo-http)
    
(test-end "getinfo")
