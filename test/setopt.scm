;; -*- Mode: scheme; -*-

;;; Copyright (C) 2013, 2014, 2021 Michael L. Gran

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

(use-modules (curl) (srfi srfi-64))

(false-if-exception (delete-file "cookie.txt"))

(test-begin "setopt")
(test-assert "curl-easy-init returns a handle"
  (curl-easy-handle? (curl-easy-init)))

(define (check-setopt opt val)
  (false-if-exception
   (let ((h (curl-easy-init)))
     (curl-easy-setopt h opt val)
     #t)))

(define (true-if-exception x)
  (not (false-if-exception x)))

(test-assert "'verbose #t" (check-setopt 'verbose #t))
(test-assert "'header #t" (check-setopt 'header #t))
(test-assert "'nosignal #t" (check-setopt 'nosignal #t))
(test-assert "'wildcardmatch #t" (check-setopt 'wildcardmatch #t))

;; NETWORK OPTIONS
(test-assert "'url \"http://localhost\"" (check-setopt 'url "http://localhost"))
(test-assert "'protocols CURLPROTO_HTTP" (check-setopt 'protocols CURLPROTO_HTTP))
(test-assert "'redir-protocols CURLPROTO_HTTP"
  (check-setopt 'redir-protocols CURLPROTO_HTTP))
(test-assert "'proxy \"http://localhost\""
  (check-setopt 'proxy "http://localhost"))
(test-assert "'proxyport 8080" (check-setopt 'proxyport 8080))
(test-assert "'proxytype CURLPROXY_HTTP" (check-setopt 'proxytype CURLPROXY_HTTP))
(test-assert "'noproxy \"locahost\"" (check-setopt 'noproxy "localhost"))
(test-assert "'httpproxytunnel #t" (check-setopt 'httpproxytunnel #t))
(test-assert "'socks5-gssapi-service \"rcmd/server-fqdn\""
  (check-setopt 'socks5-gssapi-service "rcmd/server-fqdn"))
(test-assert "'socks5-gssapi-nec #t" (check-setopt 'socks5-gssapi-nec #t))
(test-assert "'interface \"localhost\"" (check-setopt 'interface "localhost"))
(test-assert "'localport 8000" (check-setopt 'localport 8000))
(test-assert "'localportrange 100" (check-setopt 'localportrange 100))
(test-assert "'dns-cache-timeout 5" (check-setopt 'dns-cache-timeout 5))
(test-assert "'buffersize 1024" (check-setopt 'buffersize 1024))
(test-assert "'port 8080" (check-setopt 'port 8080))
(test-assert "'tcp-nodelay #t" (check-setopt 'tcp-nodelay #t))
(test-assert "'address-scope 0" (check-setopt 'address-scope 0))

;; AUTHENTICATION
(test-assert "'netrc CURL_NETRC_OPTIONAL"
  (check-setopt 'netrc CURL_NETRC_OPTIONAL))
(test-assert "'netrc-file \"net.rc\"" (check-setopt 'netrc-file "net.rc"))
(test-assert "'userpwd \"1234\"" (check-setopt 'userpwd "1234"))
(test-assert "'proxyuserpwd \"1234\"" (check-setopt 'proxyuserpwd "1234"))
(test-assert "'username \"root\"" (check-setopt 'username "root"))
(test-assert "'password \"1234\"" (check-setopt 'password "1234"))
(test-assert "'proxyusername \"root\"" (check-setopt 'proxyusername "root"))
(test-assert "'proxypassword \"1234\"" (check-setopt 'proxypassword "1234"))
(test-assert "'httpauth CURLAUTH_BASIC" (check-setopt 'httpauth CURLAUTH_BASIC))
(test-assert "'proxyauth CURLAUTH_BASIC" (check-setopt 'proxyauth CURLAUTH_BASIC))

;; HTTP OPTIONS
(test-assert "'autoreferer #t" (check-setopt 'autoreferer #t))
(test-assert "'followlocation #t" (check-setopt 'followlocation #t))
(test-assert "'unrestricted-auth #t" (check-setopt 'unrestricted-auth #t))
(test-assert "'maxredirs 100" (check-setopt 'maxredirs 100))
(test-assert "'postredir CURL_REDIR_POST_301"
  (check-setopt 'postredir CURL_REDIR_POST_301))
(test-assert "'put #t" (check-setopt 'put #t))
(test-assert "'post #t" (check-setopt 'post #t))
;; RNRS_BYTEVECTORS_BEGIN
(test-assert "'postfields #vu(0 1 2)" (check-setopt 'postfields #vu8(0 1 2)))
;; RNRS_BYTEVECTORS_END
(test-assert "'postfields #\\nul #\\soh #\\stx" (check-setopt 'postfields (string #\nul #\soh #\stx)))
(test-assert "'cookie \"a=1\"" (check-setopt 'cookie "a=1"))
(test-assert "'cookiefile \"cookie.txt\"" (check-setopt 'cookiefile "cookie.txt"))
(test-assert "'cookiejar \"cookie.txt\"" (check-setopt 'cookiejar "cookie.txt"))
(test-assert "'cookiesession #t" (check-setopt 'cookiesession #t))
(test-assert "'cookielist \"ALL\"" (check-setopt 'cookielist "ALL"))
(test-assert "'httpget #t" (check-setopt 'httpget #t))
(test-assert "'http-version CURL_HTTP_VERSION_1_1"
  (check-setopt 'http-version CURL_HTTP_VERSION_1_1))
(test-assert "'ignore-content-length #t" (check-setopt 'ignore-content-length #t))
(test-assert "'http-content-decoding #t" (check-setopt 'http-content-decoding #t))
(test-assert "'http-transfer-decoding #t" (check-setopt 'http-transfer-decoding #t))
(false-if-exception (delete-file "cookie.txt"))

(test-end "setopt")
