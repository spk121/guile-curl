;; -*- Mode: scheme; -*-

;;; Copyright (C) 2011, 2013, 2014, 2019 Michael L. Gran

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

(define-module (curl)
  #:use-module (srfi srfi-1)
  #:use-module (ice-9 optargs)
  #:use-module (rnrs bytevectors)
  #:export (
            curl-easy-handle?
            curl-easy-init
            curl-easy-setopt
            curl-easy-perform
            curl-easy-cleanup
            curl-easy-reset
            curl-error-string
            curl-error-code
            %curl-dump-handle

            bv->url-encoded-bv

            CURLPROTO_ALL
            CURLPROTO_DICT
            CURLPROTO_FILE
            CURLPROTO_FTP
            CURLPROTO_FTPS
            CURLPROTO_GOPHER
            CURLPROTO_HTTP
            CURLPROTO_HTTPS
            CURLPROTO_IMAP
            CURLPROTO_IMAPS
            CURLPROTO_LDAP
            CURLPROTO_LDAPS
            CURLPROTO_POP3
            CURLPROTO_POP3S
            CURLPROTO_RTMP
            CURLPROTO_RTMPE
            CURLPROTO_RTMPS
            CURLPROTO_RTMPT
            CURLPROTO_RTMPTE
            CURLPROTO_RTMPTS
            CURLPROTO_RTSP
            CURLPROTO_SCP
            CURLPROTO_SFTP
            CURLPROTO_SMB
            CURLPROTO_SMBS
            CURLPROTO_SMTP
            CURLPROTO_SMTPS
            CURLPROTO_TELNET
            CURLPROTO_TFTP

            CURLPROXY_HTTP
            CURLPROXY_HTTPS
            CURLPROXY_HTTP_1_0
            CURLPROXY_SOCKS4
            CURLPROXY_SOCKS4A
            CURLPROXY_SOCKS5
            CURLPROXY_SOCKS5_HOSTNAME

            CURL_NETRC_OPTIONAL
            CURL_NETRC_IGNORED
            CURL_NETRC_REQUIRED

            CURLAUTH_BASIC
            CURLAUTH_DIGEST
            CURLAUTH_DIGEST_IE
            CURLAUTH_BEARER
            CURLAUTH_NEGOTIATE
            CURLAUTH_NTLM
            CURLAUTH_NTLM_WB
            CURLAUTH_ANY
            CURLAUTH_ANYSAFE
            CURLAUTH_ONLY

            CURL_REDIR_POST_301
            CURL_REDIR_POST_302
            CURL_REDIR_POST_ALL

            CURL_HTTP_VERSION_NONE
            CURL_HTTP_VERSION_1_0
            CURL_HTTP_VERSION_1_1
            CURL_HTTP_VERSION_2_0
            CURL_HTTP_VERSION_2TLS
            CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE

            CURLFTPAUTH_DEFAULT
            CURLFTPAUTH_SSL
            CURLFTPAUTH_TLS

            CURLFTPSSL_CCC_NONE
            CURLFTPSSL_CCC_PASSIVE
            CURLFTPSSL_CCC_ACTIVE

            CURLFTPMETHOD_MULTICWD
            CURLFTPMETHOD_NOCWD
            CURLFTPMETHOD_SINGLECWD

            CURL_RTSPREQ_OPTIONS
            CURL_RTSPREQ_DESCRIBE
            CURL_RTSPREQ_ANNOUNCE
            CURL_RTSPREQ_SETUP
            CURL_RTSPREQ_PLAY
            CURL_RTSPREQ_PAUSE
            CURL_RTSPREQ_TEARDOWN
            CURL_RTSPREQ_GET_PARAMETER
            CURL_RTSPREQ_SET_PARAMETER
            CURL_RTSPREQ_RECORD
            CURL_RTSPREQ_RECEIVE

            CURL_IPRESOLVE_WHATEVER
            CURL_IPRESOLVE_V4
            CURL_IPRESOLVE_V6

            CURLUSESSL_NONE
            CURLUSESSL_TRY
            CURLUSESSL_CONTROL
            CURLUSESSL_ALL

            CURL_SSLVERSION_DEFAULT
            CURL_SSLVERSION_TLSv1
            CURL_SSLVERSION_TLSv1_0
            CURL_SSLVERSION_TLSv1_1
            CURL_SSLVERSION_TLSv1_2
            CURL_SSLVERSION_TLSv1_3
            CURL_SSLVERSION_MAX_DEFAULT
            CURL_SSLVERSION_MAX_TLSv1_0
            CURL_SSLVERSION_MAX_TLSv1_1
            CURL_SSLVERSION_MAX_TLSv1_2
            CURL_SSLVERSION_MAX_TLSv1_3
            CURL_SSLVERSION_SSLv2
            CURL_SSLVERSION_SSLv3

            CURLGSSAPI_DELEGATION_FLAG
            CURLGSSAPI_DELEGATION_POLICY_FLAG

            CURLSSH_AUTH_PUBLICKEY
            CURLSSH_AUTH_PASSWORD
            CURLSSH_AUTH_HOST
            CURLSSH_AUTH_KEYBOARD
            CURLSSH_AUTH_ANY

            CURLE_OK
            CURLE_UNSUPPORTED_PROTOCOL
            CURLE_FAILED_INIT
            CURLE_URL_MALFORMAT
            CURLE_NOT_BUILT_IN
            CURLE_COULDNT_RESOLVE_PROXY
            CURLE_COULDNT_RESOLVE_HOST
            CURLE_COULDNT_CONNECT
            CURLE_FTP_WEIRD_SERVER_REPLY
            CURLE_REMOTE_ACCESS_DENIED
            CURLE_FTP_ACCEPT_FAILED
            CURLE_FTP_WEIRD_PASS_REPLY
            CURLE_FTP_ACCEPT_TIMEOUT
            CURLE_FTP_WEIRD_PASV_REPLY
            CURLE_FTP_WEIRD_227_FORMAT
            CURLE_FTP_CANT_GET_HOST
            CURLE_HTTP2
            CURLE_FTP_COULDNT_SET_TYPE
            CURLE_PARTIAL_FILE
            CURLE_FTP_COULDNT_RETR_FILE
            CURLE_FTP_QUOTE_ERROR
            CURLE_HTTP_RETURNED_ERROR
            CURLE_WRITE_ERROR
            CURLE_UPLOAD_FAILED
            CURLE_READ_ERROR
            CURLE_OUT_OF_MEMORY
            CURLE_OPERATION_TIMEDOUT
            CURLE_FTP_PORT_FAILED
            CURLE_FTP_COULDNT_USE_REST
            CURLE_RANGE_ERROR
            CURLE_HTTP_POST_ERROR
            CURLE_SSL_CONNECT_ERROR
            CURLE_BAD_DOWNLOAD_RESUME
            CURLE_FILE_COULDNT_READ_FILE
            CURLE_LDAP_CANNOT_BIND
            CURLE_LDAP_SEARCH_FAILED
            CURLE_FUNCTION_NOT_FOUND
            CURLE_ABORTED_BY_CALLBACK
            CURLE_BAD_FUNCTION_ARGUMENT
            CURLE_INTERFACE_FAILED
            CURLE_TOO_MANY_REDIRECTS
            CURLE_UNKNOWN_OPTION
            CURLE_TELNET_OPTION_SYNTAX
            CURLE_GOT_NOTHING
            CURLE_SSL_ENGINE_NOTFOUND
            CURLE_SSL_ENGINE_SETFAILED
            CURLE_SEND_ERROR
            CURLE_RECV_ERROR
            CURLE_SSL_CERTPROBLEM
            CURLE_SSL_CIPHER
            CURLE_PEER_FAILED_VERIFICATION
            CURLE_BAD_CONTENT_ENCODING
            CURLE_LDAP_INVALID_URL
            CURLE_FILESIZE_EXCEEDED
            CURLE_USE_SSL_FAILED
            CURLE_SEND_FAIL_REWIND
            CURLE_SSL_ENGINE_INITFAILED
            CURLE_LOGIN_DENIED
            CURLE_TFTP_NOTFOUND
            CURLE_TFTP_PERM
            CURLE_REMOTE_DISK_FULL
            CURLE_TFTP_ILLEGAL
            CURLE_TFTP_UNKNOWNID
            CURLE_REMOTE_FILE_EXISTS
            CURLE_TFTP_NOSUCHUSER
            CURLE_CONV_FAILED
            CURLE_CONV_REQD
            CURLE_SSL_CACERT_BADFILE
            CURLE_REMOTE_FILE_NOT_FOUND
            CURLE_SSH
            CURLE_SSL_SHUTDOWN_FAILED
            CURLE_AGAIN
            CURLE_SSL_CRL_BADFILE
            CURLE_SSL_ISSUER_ERROR
            CURLE_FTP_PRET_FAILED
            CURLE_RTSP_CSEQ_ERROR
            CURLE_FTP_BAD_FILE_LIST
            CURLE_CHUNK_FAILED
            CURLE_NO_CONNECTION_AVAILABLE
            CURLE_SSL_PINNEDPUBKEYNOTMATCH
            CURLE_SSL_INVALIDCERTSTATUS
            CURLE_HTTP2_STREAM
            CURLE_RECURSIVE_API_CALL
            
            CURLE_OK
            CURLE_UNSUPPORTED_PROTOCOL
            CURLE_FAILED_INIT
            CURLE_URL_MALFORMAT
            CURLE_URL_MALFORMAT_USER
            CURLE_COULDNT_RESOLVE_PROXY
            CURLE_COULDNT_RESOLVE_HOST
            CURLE_COULDNT_CONNECT
            CURLE_FTP_WEIRD_SERVER_REPLY
            CURLE_FTP_ACCESS_DENIED
            CURLE_FTP_USER_PASSWORD_INCORRECT
            CURLE_FTP_WEIRD_PASS_REPLY
            CURLE_FTP_WEIRD_USER_REPLY
            CURLE_FTP_WEIRD_PASV_REPLY
            CURLE_FTP_WEIRD_227_FORMAT
            CURLE_FTP_CANT_GET_HOST
            CURLE_FTP_CANT_RECONNECT
            CURLE_FTP_COULDNT_SET_BINARY
            CURLE_PARTIAL_FILE
            CURLE_FTP_COULDNT_RETR_FILE
            CURLE_FTP_WRITE_ERROR
            CURLE_FTP_QUOTE_ERROR
            CURLE_HTTP_RETURNED_ERROR
            CURLE_WRITE_ERROR
            CURLE_MALFORMAT_USER
            CURLE_UPLOAD_FAILED
            CURLE_READ_ERROR
            CURLE_OUT_OF_MEMORY
            CURLE_OPERATION_TIMEOUTED
            CURLE_FTP_COULDNT_SET_ASCII
            CURLE_FTP_PORT_FAILED
            CURLE_FTP_COULDNT_USE_REST
            CURLE_FTP_COULDNT_GET_SIZE
            CURLE_HTTP_RANGE_ERROR
            CURLE_HTTP_POST_ERROR
            CURLE_SSL_CONNECT_ERROR
            CURLE_BAD_DOWNLOAD_RESUME
            CURLE_FILE_COULDNT_READ_FILE
            CURLE_LDAP_CANNOT_BIND
            CURLE_LDAP_SEARCH_FAILED
            CURLE_LIBRARY_NOT_FOUND
            CURLE_FUNCTION_NOT_FOUND
            CURLE_ABORTED_BY_CALLBACK
            CURLE_BAD_FUNCTION_ARGUMENT
            CURLE_BAD_CALLING_ORDER
            CURLE_INTERFACE_FAILED
            CURLE_BAD_PASSWORD_ENTERED
            CURLE_TOO_MANY_REDIRECTS
            CURLE_UNKNOWN_TELNET_OPTION
            CURLE_TELNET_OPTION_SYNTAX
            CURLE_OBSOLETE
            CURLE_SSL_PEER_CERTIFICATE
            CURLE_GOT_NOTHING
            CURLE_SSL_ENGINE_NOTFOUND
            CURLE_SSL_ENGINE_SETFAILED
            CURLE_SEND_ERROR
            CURLE_RECV_ERROR
            CURLE_SHARE_IN_USE
            CURLE_SSL_CERTPROBLEM
            CURLE_SSL_CIPHER
            CURLE_SSL_CACERT
            CURLE_BAD_CONTENT_ENCODING
            CURLE_LDAP_INVALID_URL
            CURLE_FILESIZE_EXCEEDED
            CURLE_FTP_SSL_FAILED
            CURLE_SEND_FAIL_REWIND
            CURLE_SSL_ENGINE_INITFAILED
            CURLE_LOGIN_DENIED
            CURLE_TFTP_NOTFOUND
            CURLE_TFTP_PERM
            CURLE_TFTP_DISKFULL
            CURLE_TFTP_ILLEGAL
            CURLE_TFTP_UNKNOWNID
            CURLE_TFTP_EXISTS
            CURLE_TFTP_NOSUCHUSER
            CURLE_CONV_FAILED
            CURLE_CONV_REQD
            CURLE_SSL_CACERT_BADFILE
            CURLE_REMOTE_FILE_NOT_FOUND
            CURLE_SSH
            CURLE_SSL_SHUTDOWN_FAILED
            ))

(load-extension "libguile-curl" "cl_init")

(define (c-bool x)
  (if x
      1
      0))

(define (num->ascii-hex-num x)
  "Given a number between 0 and 15, it returns
the ASCII character code of the hex digit 0123456789ABCDEF"
  (cond
   ((and (>= x 0) (<= x 9))
    (+ x 48))               ; 47 == #\0
   ((and (>= x 10) (<= x 15))
    (+ (- x 10) 65))            ; 65 is #\A
   (else
    (error "out of range"))))

(define (u8->encoded-u8-triplet x)
  "Converts the u8 integer X into a list of three integers
that are the ASCII character codes of PERCENT_SIGN +
HEXADECIMAL + HEXADECIMAL"
  (list 37              ; PERCENT SIGN
        (num->ascii-hex-num (quotient x 16))
        (num->ascii-hex-num (remainder x 16))))

(define (bv->url-encoded-bv bv)
  "Creates a bytevector that contains a URL-encoded representation
of the original bytevector"
  (u8-list->bytevector
   (fold
    (lambda (x prev)
      (cond
       ((or (= x 33)            ; #\!
            (= x 36)            ; #\$
            (and (>= x 39) (<= x 57))   ; #\` to #\9
            (and (>= x 65) (<= x 90))   ; #\A to #\Z
            (= x 95)            ; #\_
            (and (>= x 97) (<= x 122))) ; #\a to #\z
        (append prev (list x)))
       (else
        (append prev (u8->encoded-u8-triplet x)))))
    '()
    (bytevector->u8-list bv))))

(define (curl-easy-init)
  "Returns a curl handle that you must use as input to other functions.
This handle represents one connection to a server."
  (%curl-easy-init))

(define (curl-easy-handle? handle)
  "Returns #t if HANDLE is a CURL handle such as would be created by
`curl-easy-init'"
  (%curl-easy-handle? handle))

(define (curl-easy-cleanup handle)
  "This forcibly closes all connections that this handle has used
and has possibly kept open until now.  Any use of the handle
after it has been closed is invalid.

Calling this procedure is optional: handles will automatically
be freed when no longer in use."
  (%curl-easy-cleanup handle))

(define (curl-easy-reset handle)
  "Re-initializes all options previously set on this CURL handle
to its default values.

It does not change live connections, the Session ID cache, the DNS
cache, the cookies or the shared."
  (%curl-easy-reset handle))

(define symbol-options
  `(
    ;; BEHAVIOR OPTIONS
    (verbose ,CURLOPT_VERBOSE boolean)
    (header ,CURLOPT_HEADER boolean)
    ;; CURLOPT_NOPROGRESS is deprecated
    (nosignal ,CURLOPT_NOSIGNAL boolean)
    (wildcardmatch ,CURLOPT_WILDCARDMATCH boolean)

    ;; CALLBACK OPTIONS
    (readdata ,CURLOPT_READDATA input-port)

    ;; NETWORK OPTIONS
    (url ,CURLOPT_URL string)
    (path-as-is ,CURLOPT_PATH_AS_IS boolean)
    (protocols ,CURLOPT_PROTOCOLS integer)
    (redir-protocols ,CURLOPT_REDIR_PROTOCOLS integer)
    (default-protocol ,CURLOPT_DEFAULT_PROTOCOL string)
    (proxy ,CURLOPT_PROXY string)
    (pre-proxy ,CURLOPT_PRE_PROXY string)
    (proxyport ,CURLOPT_PROXYPORT integer)
    (proxytype ,CURLOPT_PROXYTYPE integer)
    (noproxy ,CURLOPT_NOPROXY string)
    (httpproxytunnel ,CURLOPT_HTTPPROXYTUNNEL boolean)
    (connect-to ,CURLOPT_CONNECT_TO slist)
    (socks5-auth, CURLOPT_SOCKS5_AUTH integer)
    (socks5-gssapi-service ,CURLOPT_SOCKS5_GSSAPI_SERVICE string)
    (socks5-gssapi-nec ,CURLOPT_SOCKS5_GSSAPI_NEC boolean)
    (proxy-service-name ,CURLOPT_PROXY_SERVICE_NAME string)
    (haproxyprotocol ,CURLOPT_HAPROXYPROTOCOL integer)
    (service-name ,CURLOPT_SERVICE_NAME string)
    (interface ,CURLOPT_INTERFACE string)
    (localport ,CURLOPT_LOCALPORT integer)
    (localportrange ,CURLOPT_LOCALPORTRANGE integer)
    (dns-cache-timeout ,CURLOPT_DNS_CACHE_TIMEOUT integer)
    ;; dns-use-global-cache is deprecated
    ;; (doh-url ,CURLOPT_DOH_URL string)
    (buffersize ,CURLOPT_BUFFERSIZE integer)
    (port ,CURLOPT_PORT integer)
    (tcp-fastopen ,CURLOPT_TCP_FASTOPEN boolean)
    (tcp-nodelay ,CURLOPT_TCP_NODELAY boolean)
    (address-scope ,CURLOPT_ADDRESS_SCOPE integer)
    (tcp-keepalive ,CURLOPT_TCP_KEEPALIVE boolean)
    (tcp-keepidle ,CURLOPT_TCP_KEEPIDLE integer)
    (tcp-keepintvl ,CURLOPT_TCP_KEEPINTVL integer)
    (unix-socket-path ,CURLOPT_UNIX_SOCKET_PATH string)
    (abstract-unix-socket ,CURLOPT_ABSTRACT_UNIX_SOCKET string)

    ;; NAMES AND PASSWORD OPTIONS
    (netrc ,CURLOPT_NETRC integer)
    (netrc-file ,CURLOPT_NETRC_FILE string)
    (userpwd ,CURLOPT_USERPWD string)
    (proxyuserpwd ,CURLOPT_PROXYUSERPWD string)
    (username ,CURLOPT_USERNAME string)
    (password ,CURLOPT_PASSWORD string)
    (login-options ,CURLOPT_LOGIN_OPTIONS string)
    (proxyusername ,CURLOPT_PROXYUSERNAME string)
    (proxypassword ,CURLOPT_PROXYPASSWORD string)
    (httpauth ,CURLOPT_HTTPAUTH integer)
    (tlsauth-type ,CURLOPT_TLSAUTH_TYPE string)
    (tlsauth-username ,CURLOPT_TLSAUTH_USERNAME string)
    (proxy-tlsauth-username ,CURLOPT_PROXY_TLSAUTH_USERNAME string)
    (tlsauth-password ,CURLOPT_TLSAUTH_PASSWORD string)
    (proxy-tlsauth-password ,CURLOPT_PROXY_TLSAUTH_PASSWORD string)
    (tlsauth-type ,CURLOPT_TLSAUTH_TYPE string)
    (proxy-tlsauth-type ,CURLOPT_PROXY_TLSAUTH_TYPE string)
    (proxyauth ,CURLOPT_PROXYAUTH integer)
    (sasl-ir ,CURLOPT_SASL_IR boolean)
    (xoauth2-bearer ,CURLOPT_XOAUTH2_BEARER string)
    (disallow-username-in-url ,CURLOPT_DISALLOW_USERNAME_IN_URL boolean)

    ;; HTTP OPTIONS
    (autoreferer ,CURLOPT_AUTOREFERER boolean)
    (accept-encoding ,CURLOPT_ACCEPT_ENCODING string)
    (transfer-encoding ,CURLOPT_TRANSFER_ENCODING string)
    (followlocation ,CURLOPT_FOLLOWLOCATION boolean)
    (unrestricted-auth ,CURLOPT_UNRESTRICTED_AUTH boolean)
    (maxredirs ,CURLOPT_MAXREDIRS integer)
    (postredir ,CURLOPT_POSTREDIR integer)
    (put ,CURLOPT_PUT boolean)
    (post ,CURLOPT_POST boolean)
    (postfields ,CURLOPT_POSTFIELDS bytevector)
    (postfieldsize ,CURLOPT_POSTFIELDSIZE integer)
    (postfieldsize-large ,CURLOPT_POSTFIELDSIZE_LARGE biginteger)
    (copypostfields ,CURLOPT_COPYPOSTFIELDS bytevector)
    (httppost ,CURLOPT_HTTPPOST httppost)
    (referer ,CURLOPT_REFERER string)
    (useragent ,CURLOPT_USERAGENT string)
    (httpheader ,CURLOPT_HTTPHEADER slist)
    (headeropt ,CURLOPT_HEADEROPT integer)
    (http200aliases ,CURLOPT_HTTP200ALIASES slist)
    (cookie ,CURLOPT_COOKIE string)
    (cookiefile ,CURLOPT_COOKIEFILE string)
    (cookiejar ,CURLOPT_COOKIEJAR string)
    (cookiesession ,CURLOPT_COOKIESESSION boolean)
    (cookielist ,CURLOPT_COOKIELIST string)
    (httpget ,CURLOPT_HTTPGET boolean)
    (request-target ,CURLOPT_REQUEST_TARGET string)
    (http-version ,CURLOPT_HTTP_VERSION integer)
    ;; (http09-allowed ,CURLOPT_HTTP09_ALLOWED boolean)
    (ignore-content-length ,CURLOPT_IGNORE_CONTENT_LENGTH boolean)
    (http-content-decoding ,CURLOPT_HTTP_CONTENT_DECODING boolean)
    (http-transfer-decoding ,CURLOPT_HTTP_TRANSFER_DECODING boolean)
    (expect-100-timeout-ms ,CURLOPT_EXPECT_100_TIMEOUT_MS integer)
    (pipewait ,CURLOPT_PIPEWAIT boolean)
    (stream-wait ,CURLOPT_STREAM_WEIGHT integer)
    
    ;; SMTP OPTIONS
    (mail-from ,CURLOPT_MAIL_FROM string)
    (mail-rcpt ,CURLOPT_MAIL_RCPT slist)
    (mail-auth ,CURLOPT_MAIL_AUTH string)

    ;; TFTP OPTIONS
    (tftp-blksize ,CURLOPT_TFTP_BLKSIZE integer)
    (tftp-no-options ,CURLOPT_TFTP_NO_OPTIONS boolean)

    ;; FTP OPTIONS
    (ftpport ,CURLOPT_FTPPORT string)
    (quote ,CURLOPT_QUOTE slist)
    (postquote ,CURLOPT_POSTQUOTE slist)
    (prequote ,CURLOPT_PREQUOTE slist)
    ;; (dirlistonly ,CURLOPT_DIRLISTONLY boolean)
    (append ,CURLOPT_APPEND boolean)
    (ftp-use-eprt ,CURLOPT_FTP_USE_EPRT boolean)
    (ftp-use-epsv ,CURLOPT_FTP_USE_EPSV boolean)
    (ftp-use-pret ,CURLOPT_FTP_USE_PRET boolean)
    (ftp-create-missing-dirs ,CURLOPT_FTP_CREATE_MISSING_DIRS boolean)
    (ftp-response-timeout ,CURLOPT_FTP_RESPONSE_TIMEOUT integer)
    (ftp-alternative-to-user ,CURLOPT_FTP_ALTERNATIVE_TO_USER string)
    (ftp-skip-pasv-ip ,CURLOPT_FTP_SKIP_PASV_IP boolean)
    (ftpsslauth ,CURLOPT_FTPSSLAUTH integer)
    (ftp-ssl-ccc ,CURLOPT_FTP_SSL_CCC integer)
    (ftp-account ,CURLOPT_FTP_ACCOUNT string)
    (ftp-filemethod ,CURLOPT_FTP_FILEMETHOD integer)

    ;; RTSP OPTIONS
    (rtsp-request ,CURLOPT_RTSP_REQUEST integer)
    (rtsp-session-id ,CURLOPT_RTSP_SESSION_ID string)
    (rtsp-stream-uri ,CURLOPT_RTSP_STREAM_URI string)
    (rtsp-transport ,CURLOPT_RTSP_TRANSPORT string)
    ;; rtsp-header is unimplmented
    (rtsp-client-cseq ,CURLOPT_RTSP_CLIENT_CSEQ integer)
    ;; rtsp-server-cseq is unimplemented

    ;; PROTOCOL OPTIONS
    (transfertext ,CURLOPT_TRANSFERTEXT boolean)
    (proxy-transfer-mode ,CURLOPT_PROXY_TRANSFER_MODE boolean)
    (crlf ,CURLOPT_CRLF boolean)
    (range ,CURLOPT_RANGE string)
    (resume-from ,CURLOPT_RESUME_FROM integer)
    (resume-from-large ,CURLOPT_RESUME_FROM_LARGE biginteger)
    (customrequest ,CURLOPT_CUSTOMREQUEST string)
    (filetime ,CURLOPT_FILETIME boolean)
    (dirlistonly ,CURLOPT_DIRLISTONLY boolean)
    (nobody ,CURLOPT_NOBODY boolean)
    (infilesize ,CURLOPT_INFILESIZE integer)
    (infilesize-large ,CURLOPT_INFILESIZE_LARGE biginteger)
    (upload ,CURLOPT_UPLOAD boolean)
    ;; (upload-buffersize ,CURLOPT_UPLOAD_BUFFERSIZE integer)
    (maxfilesize ,CURLOPT_MAXFILESIZE integer)
    (maxfilesize-large ,CURLOPT_MAXFILESIZE_LARGE biginteger)
    (timecondition ,CURLOPT_TIMECONDITION integer)
    (timevalue ,CURLOPT_TIMEVALUE integer)
    (timevalue-large ,CURLOPT_TIMEVALUE_LARGE biginteger)

    ;; CONNECTION OPTIONS
    (timeout ,CURLOPT_TIMEOUT integer)
    (timeout-ms ,CURLOPT_TIMEOUT_MS integer)
    (low-speed-limit ,CURLOPT_LOW_SPEED_LIMIT integer)
    (low-speed-time ,CURLOPT_LOW_SPEED_TIME integer)
    (max-send-speed-large ,CURLOPT_MAX_SEND_SPEED_LARGE biginteger)
    (max-recv-speed-large ,CURLOPT_MAX_RECV_SPEED_LARGE biginteger)
    (maxconnects ,CURLOPT_MAXCONNECTS integer)
    ;; closepolicy is obsolete
    (fresh-connect ,CURLOPT_FRESH_CONNECT boolean)
    (forbid-reuse ,CURLOPT_FORBID_REUSE boolean)
    (connecttimeout ,CURLOPT_CONNECTTIMEOUT integer)
    (connecttimeout-ms ,CURLOPT_CONNECTTIMEOUT_MS integer)
    (ipresolve ,CURLOPT_IPRESOLVE integer)
    (connect-only ,CURLOPT_CONNECT_ONLY boolean)
    (use-ssl ,CURLOPT_USE_SSL integer)
    (resolve ,CURLOPT_RESOLVE slist)
    (dns-interface ,CURLOPT_DNS_INTERFACE string)
    (dns-local-ip4 ,CURLOPT_DNS_LOCAL_IP4 string)
    (dns-local-ip6 ,CURLOPT_DNS_LOCAL_IP6 string)
    (dns-shuffle-addresses ,CURLOPT_DNS_SHUFFLE_ADDRESSES boolean)
    (dns-servers ,CURLOPT_DNS_SERVERS string)
    (accepttimeout-ms ,CURLOPT_ACCEPTTIMEOUT_MS integer)
    (happy-eyeballs-timeout-ms ,CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS integer)
    ;;(upkeep-interval-ms ,CURLOPT_UPKEEP_INTERVAL_MS integer)

    ;; SSL and SECURITY OPTIONS
    (sslcert ,CURLOPT_SSLCERT string)
    (proxy-sslcert ,CURLOPT_PROXY_SSLCERT string)
    (sslcerttype ,CURLOPT_SSLCERTTYPE string)
    (proxy-sslcerttype ,CURLOPT_PROXY_SSLCERTTYPE string)
    (sslkey ,CURLOPT_SSLKEY string)
    (proxy-sslkey ,CURLOPT_PROXY_SSLKEY string)
    (sslkeytype ,CURLOPT_SSLKEYTYPE string)
    (proxy-sslkeytype ,CURLOPT_PROXY_SSLKEYTYPE string)
    (keypasswd ,CURLOPT_KEYPASSWD string)
    (proxy-keypasswd ,CURLOPT_PROXY_KEYPASSWD string)
    (ssl-enable-alpn ,CURLOPT_SSL_ENABLE_ALPN boolean)
    (ssl-enable-npn ,CURLOPT_SSL_ENABLE_NPN boolean)
    (sslengine ,CURLOPT_SSLENGINE string)
    (sslengine-default ,CURLOPT_SSLENGINE_DEFAULT boolean)
    (ssl-falsestart ,CURLOPT_SSL_FALSESTART boolean)
    (sslversion ,CURLOPT_SSLVERSION integer)
    (proxy-sslversion ,CURLOPT_PROXY_SSLVERSION integer)
    (ssl-verifyhost ,CURLOPT_SSL_VERIFYHOST boolean)
    (proxy-ssl-verifyhost ,CURLOPT_PROXY_SSL_VERIFYHOST boolean)
    (ssl-verifypeer ,CURLOPT_SSL_VERIFYPEER boolean)
    (proxy-ssl-verifypeer ,CURLOPT_PROXY_SSL_VERIFYPEER boolean)
    (cainfo ,CURLOPT_CAINFO string)
    (proxy-cainfo ,CURLOPT_PROXY_CAINFO string)
    (issuercert ,CURLOPT_ISSUERCERT string)
    (capath ,CURLOPT_CAPATH string)
    (proxy-capath ,CURLOPT_PROXY_CAPATH string)
    (crlfile ,CURLOPT_CRLFILE string)
    (ssl-verifyhost ,CURLOPT_SSL_VERIFYHOST integer)
    (certinfo ,CURLOPT_CERTINFO boolean)
    (random-file ,CURLOPT_RANDOM_FILE string)
    (egdsocket ,CURLOPT_EGDSOCKET string)
    (ssl-cipher-list ,CURLOPT_SSL_CIPHER_LIST string)
    (ssl-sessionid-cache ,CURLOPT_SSL_SESSIONID_CACHE boolean)
    (krblevel ,CURLOPT_KRBLEVEL string)
    (gssapi-delegation ,CURLOPT_GSSAPI_DELEGATION integer)
    (ssh-auth-types ,CURLOPT_SSH_AUTH_TYPES integer)
    (ssh-host-public-key-md5 ,CURLOPT_SSH_HOST_PUBLIC_KEY_MD5 string)
    (ssh-public-keyfile ,CURLOPT_SSH_PUBLIC_KEYFILE string)
    (ssh-private-keyfile ,CURLOPT_SSH_PRIVATE_KEYFILE string)
    (ssh-known-hosts ,CURLOPT_SSH_KNOWNHOSTS string)
    ;; ssh-keyfunction is unimplemented because it is complicated
    ;; ssh-keydata
    ;; private
    ;; share
    (new-file-perms ,CURLOPT_NEW_FILE_PERMS integer)
    (new-directory-perms ,CURLOPT_NEW_DIRECTORY_PERMS integer)
    (telnetoptions ,CURLOPT_TELNETOPTIONS slist)
    ))

(define (curl-easy-setopt handle option arg)
  "Apply the option and argument to a given handle.
Returns #t on success and #f on failure."

  (let ((value (assq-ref symbol-options option)))
    (if value
        (let ((option (car value))
              (type (cadr value)))
          (cond
           ((or (and (eq? type 'integer)
                     (integer? arg))
                (and (eq? type 'string)
                     (string? arg)))
            (%curl-easy-setopt handle option arg #f))
           ((and (eq? type 'biginteger)
                 (integer? arg))
            (%curl-easy-setopt handle option arg #t))
           ((eq? type 'boolean)
            (%curl-easy-setopt handle option (c-bool arg) #f))
           ((and (eq? type 'bytevector)
                 (or
                  (string? arg)
                  (bytevector? arg)
                  ))
            (%curl-easy-setopt handle option arg #f))
           ((and (eq? type 'slist)
                 (list? arg)
                 (every string? arg))
            (%curl-easy-setopt handle option arg #f))
           ((and (eq? type 'httppost)
                 (list? arg))
            (%curl-easy-setopt handle option arg #f))
           ((and (eq? type 'input-port)
                 (input-port? arg))
            (%curl-easy-setopt handle option arg #f))
           ((not (member type '(integer boolean string biginteger slist httppost bytevector)))
            (error (format #f "unimplemented type: ~a" type)))
           (else
            (error (format #f "wrong type arg: ~a" arg)))))
        ;; else
        (error (format #f "unknown option: ~a" option)))))

(define* (curl-easy-perform handle #:optional (bytevector? #f) (header? #f))
  "This function is called after the init and all the curl-easy-setopt
calls are made, and will perform the transfer as described in the
options.  It must be called with the same handle as input as the
curl_easy_init call returned."
  (%curl-easy-perform handle bytevector? header?))

(define (curl-error-string)
  "Returns information about the last error as a string."
  (%curl-error-string))

(define (curl-error-code)
  "Returns the libcurl error ID of the last error."
  (%curl-error-code))
