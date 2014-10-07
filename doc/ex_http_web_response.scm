;; This code requires Guile-2.0+
(use-modules (curl)
             (web response)
             (ice-9 pretty-print)
             (rnrs bytevectors))

(setlocale LC_ALL "")

;; Create a Curl handle
(let ((handle (curl-easy-init)))

  ;; Set the URL from which to get the data
  (curl-easy-setopt handle 'url "https://twitter.com/")

  ;; Request that the HTTP headers be included in the response
  (curl-easy-setopt handle 'header #t)

  ;; Get the result as a Latin-1 string
  (let* ((response-string (curl-easy-perform handle))

         ;; Create a string port from the response
         (response-port (open-input-string response-string))

         ;; Have the (web response) module to parse the response
         (response (read-response response-port))

         ;; Have the (web response) module extract the body from the
         ;; response
         (response-body (read-response-body response)))

    ;; Display the unpacked HTTP headers
    (display "Response header ----------")
    (newline)
    (write response)
    (newline)
    (newline)

    ;; Display information about the HTTP body
    (format #t "The body contains ~a bytes~%"
            (bytevector-length response-body))
    (format #t "The body is of type ~A~%"
            (car (response-content-type response)))
    (format #t "The encoding of the body is ~A~%"
            (assoc-ref (response-content-type response) 'charset))))
