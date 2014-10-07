;; This code requires Guile-2.0+
(use-modules (curl))

(setlocale LC_ALL "")
(define handle (curl-easy-init))
(curl-easy-setopt handle 'url "http://www.gnu.org")
(display (utf8->string (curl-easy-perform handle #t)))
