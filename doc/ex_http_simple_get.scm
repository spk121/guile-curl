;; This code requires Guile-1.8 or greater
(use-modules (curl))

(define handle (curl-easy-init))
(curl-easy-setopt handle 'url "http://www.gnu.org")
(display (curl-easy-perform handle))
