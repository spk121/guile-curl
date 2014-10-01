(use-modules (curl)
             (rnrs bytevectors)
             (web http)
             (web request)
             (ice-9 rdelim))
(define handle (curl-easy-init))
(curl-easy-setopt handle 'url "http://www.w3.org")
(let* ((header-and-body (curl-easy-perform handle #f #t))
       (header-string (car header-and-body))
       (body-string (cadr header-and-body))

       ;; Convert the Header string into a port
       (header-port (open-input-string header-string)))

  (display "The HTTP Header-------")
  (newline)
  (display header-string)
  (display "----------------------")
  (newline)

  ;; Read off the HTTP/1.1 line
  (read-line header-port)

  ;; Parse the HTTP Headers
  (let* ((header-list (read-headers header-port))
         (content-type (assoc-ref header-list 'content-type))
         (charset (assoc-ref content-type 'charset)))
    (display "Encoding as described in the HTTP Header: ")
    (display charset)
    (newline)))
