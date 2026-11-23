;; Post a multipart/form-data form, as an HTML form with a file upload
;; would, using the 'mimepost option.  This code requires guile-curl
;; 1.0.0 or later.
;;
;; Each field is a list of a name, the data (a string or a bytevector),
;; and optionally a MIME type and a filename.  A field with a filename is
;; sent as a file upload; its data is the file's contents, so nothing is
;; read from disk.
;;
;; httpbin.org/post replies with a JSON description of the form it
;; received, which this example displays.

(use-modules (curl)
             (rnrs bytevectors))

(define handle (curl-easy-init))
(curl-easy-setopt handle 'url "https://httpbin.org/post")

(curl-easy-setopt handle 'mimepost
                  (list
                   ;; A plain text field.
                   '("title" "Meeting notes")
                   ;; A file upload with a MIME type and a filename.
                   '("upload" "First line\nSecond line\n"
                     "text/plain" "notes.txt")
                   ;; Binary data can be given as a bytevector.
                   (list "checksum" (u8-list->bytevector '(222 173 190 239))
                         "application/octet-stream")))

(let ((reply (curl-easy-perform handle)))
  (if reply
      (display reply)
      (format #t "Request failed: ~a~%" (curl-error-string))))

(curl-easy-cleanup handle)
