(use-modules (lib)
	     (curl))

(with-test-prefix
 "curl-easy-init"
 (pass-if "returns a handle"
	  (handle? (curl-easy-init))))

(define (check-setopt opt val)
  (false-if-exception
   (let ((h (curl-easy-init)))
     (curl-easy-setopt h opt val)
     #t)))

(define (true-if-exception x)
  (not (false-if-exception x)))

(with-test-prefix
 "curl-easy-setopt"
 (pass-if "'verbose #t" (check-setopt 'verbose #t))
 (pass-if "'header #t" (check-setopt 'header #t))
 (pass-if "'nosignal #t" (check-setopt 'nosignal #t))
 (pass-if "'wildcardmatch #t" (check-setopt 'wildcardmatch #t))

 ;; NETWORK OPTIONS
 (pass-if "'url \"http://localhost\"" (check-setopt 'url "http://localhost"))
 (pass-if "'protocols CURLPROTO_HTTP" (check-setopt 'protocols CURLPROTO_HTTP))
 (pass-if "'redir-protocols CURLPROTO_HTTP"
	  (check-setopt 'redir-protocols CURLPROTO_HTTP))
 (pass-if "'proxy \"http://localhost\""
	  (check-setopt 'proxy "http://localhost"))
 (pass-if "'proxyport 8080" (check-setopt 'proxyport 8080))
 (pass-if "'proxytype CURLPROXY_HTTP" (check-setopt 'proxytype CURLPROXY_HTTP))
 (pass-if "'noproxy \"locahost\"" (check-setopt 'noproxy "localhost"))
 (pass-if "'httpproxytunnel #t" (check-setopt 'httpproxytunnel #t))
 (pass-if "'socks5-gssapi-service \"rcmd/server-fqdn\""
	  (check-setopt 'socks5-gssapi-service "rcmd/server-fqdn"))
 (pass-if "'socks5-gssapi-nec #t" (check-setopt 'socks5-gssapi-nec #t))
 (pass-if "'interface \"localhost\"" (check-setopt 'interface "localhost"))
 (pass-if "'localport 8000" (check-setopt 'localport 8000))
 (pass-if "'localportrange 100" (check-setopt 'localportrange 100))
 (pass-if "'dns-cache-timeout 5" (check-setopt 'dns-cache-timeout 5))
 (pass-if "'buffersize 1024" (check-setopt 'buffersize 1024))
 (pass-if "'port 8080" (check-setopt 'port 8080))
 (pass-if "'tcp-nodelay #t" (check-setopt 'tcp-nodelay #t))
 (pass-if "'address-scope 0" (check-setopt 'address-scope 0))

 ;; AUTHENTICATION
 (pass-if "'netrc CURL_NETRC_OPTIONAL"
	  (check-setopt 'netrc CURL_NETRC_OPTIONAL))
 (pass-if "'netrc-file \"net.rc\"" (check-setopt 'netrc-file "net.rc"))
 (pass-if "'userpwd \"1234\"" (check-setopt 'userpwd "1234"))
 (pass-if "'proxyuserpwd \"1234\"" (check-setopt 'proxyuserpwd "1234"))
 (pass-if "'username \"root\"" (check-setopt 'username "root"))
 (pass-if "'password \"1234\"" (check-setopt 'password "1234"))
 (pass-if "'proxyusername \"root\"" (check-setopt 'proxyusername "root"))
 (pass-if "'proxypassword \"1234\"" (check-setopt 'proxypassword "1234"))
 (pass-if "'httpauth CURLAUTH_BASIC" (check-setopt 'httpauth CURLAUTH_BASIC))
 (pass-if "'proxyauth CURLAUTH_BASIC" (check-setopt 'proxyauth CURLAUTH_BASIC))

 ;; HTTP OPTIONS
 (pass-if "'autoreferer #t" (check-setopt 'autoreferer #t))
 (pass-if "'followlocation #t" (check-setopt 'followlocation #t))
 (pass-if "'unrestricted-auth #t" (check-setopt 'unrestricted-auth #t))
 (pass-if "'maxredirs 100" (check-setopt 'maxredirs 100))
 (pass-if "'postredir CURL_REDIR_POST_301"
	  (check-setopt 'postredir CURL_REDIR_POST_301))
 (pass-if "'put #t" (check-setopt 'put #t))
 (pass-if "'post #t" (check-setopt 'post #t))
 ;; RNRS_BYTEVECTORS_BEGIN
 (pass-if "'postfields #vu(0 1 2)" (check-setopt 'postfields #vu8(0 1 2)))
 ;; RNRS_BYTEVECTORS_END
 (pass-if "'postfields #\\nul #\\soh #\\stx" (check-setopt 'postfields (string #\nul #\soh #\stx)))
 (pass-if "'cookie \"a=1\"" (check-setopt 'cookie "a=1"))
 (pass-if "'cookiefile \"cookie.txt\"" (check-setopt 'cookiefile "cookie.txt"))
 (pass-if "'cookiejar \"cookie.txt\"" (check-setopt 'cookiejar "cookie.txt"))
 (pass-if "'cookiesession #t" (check-setopt 'cookiesession #t))
 (pass-if "'cookielist \"ALL\"" (check-setopt 'cookielist "ALL"))
 (pass-if "'httpget #t" (check-setopt 'httpget #t))
 (pass-if "'http-version CURL_HTTP_VERSION_1_1"
	  (check-setopt 'http-version CURL_HTTP_VERSION_1_1))
 (pass-if "'ignore-content-length #t" (check-setopt 'ignore-content-length #t))
 (pass-if "'http-content-decoding #t" (check-setopt 'http-content-decoding #t))
 (pass-if "'http-transfer-decoding #t" (check-setopt 'http-transfer-decoding #t)))
