/* Constants for guile-curl

   Copyright (c) 2013, 2019, 2021 Michael L. Gran

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <config.h>
#include <libguile.h>
#include <curl/curl.h>
#include "const.h"

#define I(x) SCM cl_ ## x
#define __I(x) SCM clb_ ## x
#define D(x) cl_ ## x = scm_permanent_object (scm_c_define (#x, scm_from_int ((int) x)))
#define __D(x) clb_ ## x = scm_permanent_object (scm_c_define (#x, scm_from_int ((int) x)))
#define ND(x) cl_ ## x = scm_permanent_object (scm_c_define (#x, SCM_BOOL_F))
#define __ND(x) clb_ ## x = scm_permanent_object (scm_c_define (#x, SCM_BOOL_F))

/* from curl_easy_setopt */

I(CURLOPT_VERBOSE);
I(CURLOPT_HEADER);
// CURLOPT_NOPROGRESS is deprecated
I(CURLOPT_NOSIGNAL);
I(CURLOPT_WILDCARDMATCH);
I(CURLOPT_READDATA);

// NETWORK OPTIONS
I(CURLOPT_URL);
I(CURLOPT_PATH_AS_IS);
I(CURLOPT_PROTOCOLS);
__I(CURLPROTO_ALL);
__I(CURLPROTO_DICT);
__I(CURLPROTO_FILE);
__I(CURLPROTO_FTP);
__I(CURLPROTO_FTPS);
__I(CURLPROTO_GOPHER);
__I(CURLPROTO_HTTP);
__I(CURLPROTO_HTTPS);
__I(CURLPROTO_IMAP);
__I(CURLPROTO_IMAPS);
__I(CURLPROTO_LDAP);
__I(CURLPROTO_LDAPS);
__I(CURLPROTO_POP3);
__I(CURLPROTO_POP3S);
__I(CURLPROTO_RTMP);
__I(CURLPROTO_RTMPE);
__I(CURLPROTO_RTMPS);
__I(CURLPROTO_RTMPT);
__I(CURLPROTO_RTMPTE);
__I(CURLPROTO_RTMPTS);
__I(CURLPROTO_RTSP);
__I(CURLPROTO_SCP);
__I(CURLPROTO_SFTP);
__I(CURLPROTO_SMB);
__I(CURLPROTO_SMBS);
__I(CURLPROTO_SMTP);
__I(CURLPROTO_SMTPS);
__I(CURLPROTO_TELNET);
__I(CURLPROTO_TFTP);
I(CURLOPT_REDIR_PROTOCOLS);
I(CURLOPT_DEFAULT_PROTOCOL);
I(CURLOPT_PROXY);
I(CURLOPT_PRE_PROXY);
I(CURLOPT_PROXYPORT);
I(CURLOPT_PROXYTYPE);
__I(CURLPROXY_HTTP);
__I(CURLPROXY_HTTPS);
__I(CURLPROXY_HTTP_1_0);
__I(CURLPROXY_SOCKS4);
__I(CURLPROXY_SOCKS4A);
__I(CURLPROXY_SOCKS5);
__I(CURLPROXY_SOCKS5_HOSTNAME);
I(CURLOPT_NOPROXY);
I(CURLOPT_HTTPPROXYTUNNEL);
I(CURLOPT_CONNECT_TO);
I(CURLOPT_SOCKS5_AUTH);
I(CURLOPT_SOCKS5_GSSAPI_SERVICE);
I(CURLOPT_SOCKS5_GSSAPI_NEC);
I(CURLOPT_PROXY_SERVICE_NAME);
I(CURLOPT_HAPROXYPROTOCOL);
I(CURLOPT_SERVICE_NAME);
I(CURLOPT_INTERFACE);
I(CURLOPT_LOCALPORT);
I(CURLOPT_LOCALPORTRANGE);
I(CURLOPT_DNS_CACHE_TIMEOUT);
// CURLOPT_DNS_USE_GLOBAL_CACHE is deprecated
I(CURLOPT_DOH_URL);
I(CURLOPT_BUFFERSIZE);
I(CURLOPT_PORT);
I(CURLOPT_TCP_FASTOPEN);
I(CURLOPT_TCP_NODELAY);
I(CURLOPT_ADDRESS_SCOPE);
I(CURLOPT_TCP_KEEPALIVE);
I(CURLOPT_TCP_KEEPIDLE);
I(CURLOPT_TCP_KEEPINTVL);
I(CURLOPT_UNIX_SOCKET_PATH);
I(CURLOPT_ABSTRACT_UNIX_SOCKET);

// NAMES AND PASSWORDS OPTIONS
I(CURLOPT_NETRC);
__I(CURL_NETRC_OPTIONAL);
__I(CURL_NETRC_IGNORED);
__I(CURL_NETRC_REQUIRED);
I(CURLOPT_NETRC_FILE);
I(CURLOPT_USERPWD);
I(CURLOPT_PROXYUSERPWD);
I(CURLOPT_USERNAME);
I(CURLOPT_PASSWORD);
I(CURLOPT_LOGIN_OPTIONS);
I(CURLOPT_PROXYUSERNAME);
I(CURLOPT_PROXYPASSWORD);
I(CURLOPT_HTTPAUTH);
__I(CURLAUTH_BASIC);
__I(CURLAUTH_DIGEST);
__I(CURLAUTH_DIGEST_IE);
__I(CURLAUTH_BEARER);
__I(CURLAUTH_NEGOTIATE);
__I(CURLAUTH_NTLM);
__I(CURLAUTH_NTLM_WB);
__I(CURLAUTH_ANY);
__I(CURLAUTH_ANYSAFE);
__I(CURLAUTH_ONLY);
I(CURLOPT_TLSAUTH_USERNAME);
I(CURLOPT_PROXY_TLSAUTH_USERNAME);
I(CURLOPT_TLSAUTH_PASSWORD);
I(CURLOPT_PROXY_TLSAUTH_PASSWORD);
I(CURLOPT_TLSAUTH_TYPE);
I(CURLOPT_PROXY_TLSAUTH_TYPE);
I(CURLOPT_PROXYAUTH);
I(CURLOPT_SASL_IR);
I(CURLOPT_XOAUTH2_BEARER);
I(CURLOPT_DISALLOW_USERNAME_IN_URL);

// HTTP OPTIONS
I(CURLOPT_AUTOREFERER);
I(CURLOPT_ACCEPT_ENCODING);
I(CURLOPT_TRANSFER_ENCODING);
I(CURLOPT_FOLLOWLOCATION);
I(CURLOPT_UNRESTRICTED_AUTH);
I(CURLOPT_MAXREDIRS);
I(CURLOPT_POSTREDIR);
__I(CURL_REDIR_POST_301);
__I(CURL_REDIR_POST_302);
__I(CURL_REDIR_POST_303);
__I(CURL_REDIR_POST_ALL);
I(CURLOPT_PUT);
I(CURLOPT_POST);
I(CURLOPT_POSTFIELDS);
I(CURLOPT_POSTFIELDSIZE);
I(CURLOPT_POSTFIELDSIZE_LARGE);
I(CURLOPT_COPYPOSTFIELDS);
I(CURLOPT_HTTPPOST);
I(CURLOPT_REFERER);
I(CURLOPT_USERAGENT);
I(CURLOPT_HTTPHEADER);
I(CURLOPT_HEADEROPT);
I(CURLOPT_PROXYHEADER);
I(CURLOPT_HTTP200ALIASES);
I(CURLOPT_COOKIE);
I(CURLOPT_COOKIEFILE);
I(CURLOPT_COOKIEJAR);
I(CURLOPT_COOKIESESSION);
I(CURLOPT_COOKIELIST);
I(CURLOPT_HTTPGET);
I(CURLOPT_REQUEST_TARGET);
I(CURLOPT_HTTP_VERSION);
__I(CURL_HTTP_VERSION_NONE);
__I(CURL_HTTP_VERSION_1_0);
__I(CURL_HTTP_VERSION_1_1);
__I(CURL_HTTP_VERSION_2_0);
__I(CURL_HTTP_VERSION_2TLS);
__I(CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE);
I(CURLOPT_HTTP09_ALLOWED);
I(CURLOPT_IGNORE_CONTENT_LENGTH);
I(CURLOPT_HTTP_CONTENT_DECODING);
I(CURLOPT_HTTP_TRANSFER_DECODING);
I(CURLOPT_EXPECT_100_TIMEOUT_MS);
I(CURLOPT_PIPEWAIT);
I(CURLOPT_STREAM_WEIGHT);

// SMTP OPTIONS
I(CURLOPT_MAIL_FROM);
I(CURLOPT_MAIL_RCPT);
I(CURLOPT_MAIL_AUTH);

// TFTP OPTIONS
I(CURLOPT_TFTP_BLKSIZE);
I(CURLOPT_TFTP_NO_OPTIONS);

// FTP OPTIONS
I(CURLOPT_FTPPORT);
I(CURLOPT_QUOTE);
I(CURLOPT_POSTQUOTE);
I(CURLOPT_PREQUOTE);
I(CURLOPT_APPEND);
I(CURLOPT_FTP_USE_EPRT);
I(CURLOPT_FTP_USE_EPSV);
I(CURLOPT_FTP_USE_PRET);
I(CURLOPT_FTP_CREATE_MISSING_DIRS);
I(CURLOPT_FTP_RESPONSE_TIMEOUT);
I(CURLOPT_FTP_ALTERNATIVE_TO_USER);
I(CURLOPT_FTP_SKIP_PASV_IP);
I(CURLOPT_FTPSSLAUTH);
__I(CURLFTPAUTH_DEFAULT);
__I(CURLFTPAUTH_SSL);
__I(CURLFTPAUTH_TLS);
I(CURLOPT_FTP_SSL_CCC);
__I(CURLFTPSSL_CCC_NONE);
__I(CURLFTPSSL_CCC_PASSIVE);
__I(CURLFTPSSL_CCC_ACTIVE);
I(CURLOPT_FTP_ACCOUNT);
I(CURLOPT_FTP_FILEMETHOD);
__I(CURLFTPMETHOD_MULTICWD);
__I(CURLFTPMETHOD_NOCWD);
__I(CURLFTPMETHOD_SINGLECWD);

// RSTP OPTIONS
I(CURLOPT_RTSP_REQUEST);
__I(CURL_RTSPREQ_OPTIONS);
__I(CURL_RTSPREQ_DESCRIBE);
__I(CURL_RTSPREQ_ANNOUNCE);
__I(CURL_RTSPREQ_SETUP);
__I(CURL_RTSPREQ_PLAY);
__I(CURL_RTSPREQ_PAUSE);
__I(CURL_RTSPREQ_TEARDOWN);
__I(CURL_RTSPREQ_GET_PARAMETER);
__I(CURL_RTSPREQ_SET_PARAMETER);
__I(CURL_RTSPREQ_RECORD);
__I(CURL_RTSPREQ_RECEIVE);
I(CURLOPT_RTSP_SESSION_ID);
I(CURLOPT_RTSP_STREAM_URI);
I(CURLOPT_RTSP_TRANSPORT);
// CURLOPT_RTSP_HEADER is unimplemented
I(CURLOPT_RTSP_CLIENT_CSEQ);
// CURLOPT_RTSP_SERVER_CSEQ is unimplemented

// PROTOCOL OPTIONS
I(CURLOPT_TRANSFERTEXT);
I(CURLOPT_PROXY_TRANSFER_MODE);
I(CURLOPT_CRLF);
I(CURLOPT_RANGE);
I(CURLOPT_RESUME_FROM);
I(CURLOPT_RESUME_FROM_LARGE);
I(CURLOPT_CUSTOMREQUEST);
I(CURLOPT_FILETIME);
I(CURLOPT_DIRLISTONLY);
I(CURLOPT_NOBODY);
I(CURLOPT_INFILESIZE);
I(CURLOPT_INFILESIZE_LARGE);
I(CURLOPT_UPLOAD);
I(CURLOPT_UPLOAD_BUFFERSIZE);
I(CURLOPT_MAXFILESIZE);
I(CURLOPT_MAXFILESIZE_LARGE);
I(CURLOPT_TIMECONDITION);
I(CURLOPT_TIMEVALUE);
I(CURLOPT_TIMEVALUE_LARGE);

// CONNECTION OPTIONS
I(CURLOPT_TIMEOUT);
I(CURLOPT_TIMEOUT_MS);
I(CURLOPT_LOW_SPEED_LIMIT);
I(CURLOPT_LOW_SPEED_TIME);
I(CURLOPT_MAX_SEND_SPEED_LARGE);
I(CURLOPT_MAX_RECV_SPEED_LARGE);
I(CURLOPT_MAXCONNECTS);
// CURLOPT_CLOSEPOLICY is obsolete
I(CURLOPT_FRESH_CONNECT);
I(CURLOPT_FORBID_REUSE);
I(CURLOPT_CONNECTTIMEOUT);
I(CURLOPT_CONNECTTIMEOUT_MS);
I(CURLOPT_IPRESOLVE);
__I(CURL_IPRESOLVE_WHATEVER);
__I(CURL_IPRESOLVE_V4);
__I(CURL_IPRESOLVE_V6);
I(CURLOPT_CONNECT_ONLY);
I(CURLOPT_USE_SSL);
__I(CURLUSESSL_NONE);
__I(CURLUSESSL_TRY);
__I(CURLUSESSL_CONTROL);
__I(CURLUSESSL_ALL);
I(CURLOPT_RESOLVE);
I(CURLOPT_DNS_INTERFACE);
I(CURLOPT_DNS_LOCAL_IP4);
I(CURLOPT_DNS_LOCAL_IP6);
I(CURLOPT_DNS_SERVERS);
I(CURLOPT_DNS_SHUFFLE_ADDRESSES);
I(CURLOPT_ACCEPTTIMEOUT_MS);
I(CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS);
I(CURLOPT_UPKEEP_INTERVAL_MS);

// SSL AND SECURITY OPTIONS
I(CURLOPT_SSLCERT);
I(CURLOPT_PROXY_SSLCERT);
I(CURLOPT_SSLCERTTYPE);
I(CURLOPT_PROXY_SSLCERTTYPE);
I(CURLOPT_SSLKEY);
I(CURLOPT_PROXY_SSLKEY);
I(CURLOPT_SSLKEYTYPE);
I(CURLOPT_PROXY_SSLKEYTYPE);
I(CURLOPT_KEYPASSWD);
I(CURLOPT_PROXY_KEYPASSWD);
I(CURLOPT_SSL_ENABLE_ALPN);
I(CURLOPT_SSL_ENABLE_NPN);
I(CURLOPT_SSLENGINE);
I(CURLOPT_SSLENGINE_DEFAULT);
I(CURLOPT_SSL_FALSESTART);
I(CURLOPT_SSLVERSION);
__I(CURL_SSLVERSION_DEFAULT);
__I(CURL_SSLVERSION_TLSv1);
__I(CURL_SSLVERSION_TLSv1_0);
__I(CURL_SSLVERSION_TLSv1_1);
__I(CURL_SSLVERSION_TLSv1_2);
__I(CURL_SSLVERSION_TLSv1_3);
__I(CURL_SSLVERSION_MAX_DEFAULT);
__I(CURL_SSLVERSION_MAX_TLSv1_0);
__I(CURL_SSLVERSION_MAX_TLSv1_1);
__I(CURL_SSLVERSION_MAX_TLSv1_2);
__I(CURL_SSLVERSION_MAX_TLSv1_3);
__I(CURL_SSLVERSION_SSLv2);
__I(CURL_SSLVERSION_SSLv3);
I(CURLOPT_PROXY_SSLVERSION);
I(CURLOPT_SSL_VERIFYHOST);
I(CURLOPT_PROXY_SSL_VERIFYHOST);
I(CURLOPT_SSL_VERIFYPEER);
I(CURLOPT_PROXY_SSL_VERIFYPEER);
I(CURLOPT_CAINFO);
I(CURLOPT_PROXY_CAINFO);
I(CURLOPT_ISSUERCERT);
I(CURLOPT_CAPATH);
I(CURLOPT_PROXY_CAPATH);
I(CURLOPT_CRLFILE);
I(CURLOPT_SSL_VERIFYHOST);
I(CURLOPT_CERTINFO);
I(CURLOPT_RANDOM_FILE);
I(CURLOPT_EGDSOCKET);
I(CURLOPT_SSL_CIPHER_LIST);
I(CURLOPT_SSL_SESSIONID_CACHE);
I(CURLOPT_KRBLEVEL);
I(CURLOPT_GSSAPI_DELEGATION);
__I(CURLGSSAPI_DELEGATION_FLAG);
__I(CURLGSSAPI_DELEGATION_POLICY_FLAG);
I(CURLOPT_SSH_AUTH_TYPES);
__I(CURLSSH_AUTH_PUBLICKEY);
__I(CURLSSH_AUTH_PASSWORD);
__I(CURLSSH_AUTH_HOST);
__I(CURLSSH_AUTH_KEYBOARD);
__I(CURLSSH_AUTH_ANY);
I(CURLOPT_SSH_HOST_PUBLIC_KEY_MD5);
I(CURLOPT_SSH_PUBLIC_KEYFILE);
I(CURLOPT_SSH_PRIVATE_KEYFILE);
I(CURLOPT_SSH_KNOWNHOSTS);
// CURLOPT_SSH_KEYFUNCTION is complicated
//__I(CURLKHSTAT_FINE_ADD_TO_FILE);
//__I(CURLKHSTAT_FINE);
//__I(CURLKHSTAT_REJECT);
//__I(CURLKHSTAT_DEFER);
//I(CURLOPT_SSH_KEYDATA);
//I(CURLOPT_PRIVATE);
//I(CURLOPT_SHARE);
I(CURLOPT_NEW_FILE_PERMS);
I(CURLOPT_NEW_DIRECTORY_PERMS);
I(CURLOPT_TELNETOPTIONS);

/* ERROR CODES */

I(CURLE_OK);
I(CURLE_UNSUPPORTED_PROTOCOL);
I(CURLE_FAILED_INIT);
I(CURLE_URL_MALFORMAT);
I(CURLE_NOT_BUILT_IN);
I(CURLE_COULDNT_RESOLVE_PROXY);
I(CURLE_COULDNT_RESOLVE_HOST);
I(CURLE_COULDNT_CONNECT);
I(CURLE_FTP_WEIRD_SERVER_REPLY);
I(CURLE_REMOTE_ACCESS_DENIED);
I(CURLE_FTP_ACCEPT_FAILED);
I(CURLE_FTP_WEIRD_PASS_REPLY);
I(CURLE_FTP_ACCEPT_TIMEOUT);
I(CURLE_FTP_WEIRD_PASV_REPLY);
I(CURLE_FTP_WEIRD_227_FORMAT);
I(CURLE_FTP_CANT_GET_HOST);
I(CURLE_HTTP2);
I(CURLE_FTP_COULDNT_SET_TYPE);
I(CURLE_PARTIAL_FILE);
I(CURLE_FTP_COULDNT_RETR_FILE);
I(CURLE_FTP_QUOTE_ERROR);
I(CURLE_HTTP_RETURNED_ERROR);
I(CURLE_WRITE_ERROR);
I(CURLE_UPLOAD_FAILED);
I(CURLE_READ_ERROR);
I(CURLE_OUT_OF_MEMORY);
I(CURLE_OPERATION_TIMEDOUT);
I(CURLE_FTP_PORT_FAILED);
I(CURLE_FTP_COULDNT_USE_REST);
I(CURLE_RANGE_ERROR);
I(CURLE_HTTP_POST_ERROR);
I(CURLE_SSL_CONNECT_ERROR);
I(CURLE_BAD_DOWNLOAD_RESUME);
I(CURLE_FILE_COULDNT_READ_FILE);
I(CURLE_LDAP_CANNOT_BIND);
I(CURLE_LDAP_SEARCH_FAILED);
I(CURLE_FUNCTION_NOT_FOUND);
I(CURLE_ABORTED_BY_CALLBACK);
I(CURLE_BAD_FUNCTION_ARGUMENT);
I(CURLE_INTERFACE_FAILED);
I(CURLE_TOO_MANY_REDIRECTS);
I(CURLE_UNKNOWN_OPTION);
I(CURLE_TELNET_OPTION_SYNTAX);
I(CURLE_GOT_NOTHING);
I(CURLE_SSL_ENGINE_NOTFOUND);
I(CURLE_SSL_ENGINE_SETFAILED);
I(CURLE_SEND_ERROR);
I(CURLE_RECV_ERROR);
I(CURLE_SSL_CERTPROBLEM);
I(CURLE_SSL_CIPHER);
I(CURLE_PEER_FAILED_VERIFICATION);
//I(CURLE_SSL_CACERT);
I(CURLE_BAD_CONTENT_ENCODING);
I(CURLE_LDAP_INVALID_URL);
I(CURLE_FILESIZE_EXCEEDED);
I(CURLE_USE_SSL_FAILED);
I(CURLE_SEND_FAIL_REWIND);
I(CURLE_SSL_ENGINE_INITFAILED);
I(CURLE_LOGIN_DENIED);
I(CURLE_TFTP_NOTFOUND);
I(CURLE_TFTP_PERM);
I(CURLE_REMOTE_DISK_FULL);
I(CURLE_TFTP_ILLEGAL);
I(CURLE_TFTP_UNKNOWNID);
I(CURLE_REMOTE_FILE_EXISTS);
I(CURLE_TFTP_NOSUCHUSER);
I(CURLE_CONV_FAILED);
I(CURLE_CONV_REQD);
I(CURLE_SSL_CACERT_BADFILE);
I(CURLE_REMOTE_FILE_NOT_FOUND);
I(CURLE_SSH);
I(CURLE_SSL_SHUTDOWN_FAILED);
I(CURLE_AGAIN);
I(CURLE_SSL_CRL_BADFILE);
I(CURLE_SSL_ISSUER_ERROR);
I(CURLE_FTP_PRET_FAILED);
I(CURLE_RTSP_CSEQ_ERROR);
I(CURLE_FTP_BAD_FILE_LIST);
I(CURLE_CHUNK_FAILED);
I(CURLE_NO_CONNECTION_AVAILABLE);
I(CURLE_SSL_PINNEDPUBKEYNOTMATCH);
I(CURLE_SSL_INVALIDCERTSTATUS);
I(CURLE_HTTP2_STREAM);
I(CURLE_RECURSIVE_API_CALL);

void
cl_init_const ()
{

    D(CURLOPT_VERBOSE);
    D(CURLOPT_HEADER);
// CURLOPT_NOPROGRESS is deprecated
    D(CURLOPT_NOSIGNAL);
    D(CURLOPT_WILDCARDMATCH);
    D(CURLOPT_READDATA);

// NETWORK OPTIONS
    D(CURLOPT_URL);
    D(CURLOPT_PATH_AS_IS);
    D(CURLOPT_PROTOCOLS);
    __D(CURLPROTO_ALL);
    __D(CURLPROTO_DICT);
    __D(CURLPROTO_FILE);
    __D(CURLPROTO_FTP);
    __D(CURLPROTO_FTPS);
    __D(CURLPROTO_GOPHER);
    __D(CURLPROTO_HTTP);
    __D(CURLPROTO_HTTPS);
    __D(CURLPROTO_IMAP);
    __D(CURLPROTO_IMAPS);
    __D(CURLPROTO_LDAP);
    __D(CURLPROTO_LDAPS);
    __D(CURLPROTO_POP3);
    __D(CURLPROTO_POP3S);
    __D(CURLPROTO_RTMP);
    __D(CURLPROTO_RTMPE);
    __D(CURLPROTO_RTMPS);
    __D(CURLPROTO_RTMPT);
    __D(CURLPROTO_RTMPTE);
    __D(CURLPROTO_RTMPTS);
    __D(CURLPROTO_RTSP);
    __D(CURLPROTO_SCP);
    __D(CURLPROTO_SFTP);
    __D(CURLPROTO_SMB);
    __D(CURLPROTO_SMBS);
    __D(CURLPROTO_SMTP);
    __D(CURLPROTO_SMTPS);
    __D(CURLPROTO_TELNET);
    __D(CURLPROTO_TFTP);
    D(CURLOPT_REDIR_PROTOCOLS);
    D(CURLOPT_DEFAULT_PROTOCOL);
    D(CURLOPT_PROXY);
    D(CURLOPT_PRE_PROXY);
    D(CURLOPT_PROXYPORT);
    D(CURLOPT_PROXYTYPE);
    __D(CURLPROXY_HTTP);
    __D(CURLPROXY_HTTPS);
    __D(CURLPROXY_HTTP_1_0);
    __D(CURLPROXY_SOCKS4);
    __D(CURLPROXY_SOCKS4A);
    __D(CURLPROXY_SOCKS5);
    __D(CURLPROXY_SOCKS5_HOSTNAME);
    D(CURLOPT_NOPROXY);
    D(CURLOPT_HTTPPROXYTUNNEL);
    D(CURLOPT_CONNECT_TO);
    D(CURLOPT_SOCKS5_AUTH);
    D(CURLOPT_SOCKS5_GSSAPI_SERVICE);
    D(CURLOPT_SOCKS5_GSSAPI_NEC);
    D(CURLOPT_PROXY_SERVICE_NAME);
#ifdef CURLOPT_HAPROXYPROTOCOL
    D(CURLOPT_HAPROXYPROTOCOL);
#else
    ND(CURLOPT_HAPROXYPROTOCOL);
#endif
    D(CURLOPT_SERVICE_NAME);
    D(CURLOPT_INTERFACE);
    D(CURLOPT_LOCALPORT);
    D(CURLOPT_LOCALPORTRANGE);
    D(CURLOPT_DNS_CACHE_TIMEOUT);
// CURLOPT_DNS_USE_GLOBAL_CACHE is deprecated
#ifdef CURLOPT_DOH_URL
    D(CURLOPT_DOH_URL);
#else
    ND(CURLOPT_DOH_URL);
#endif
    D(CURLOPT_BUFFERSIZE);
    D(CURLOPT_PORT);
    D(CURLOPT_TCP_FASTOPEN);
    D(CURLOPT_TCP_NODELAY);
    D(CURLOPT_ADDRESS_SCOPE);
    D(CURLOPT_TCP_KEEPALIVE);
    D(CURLOPT_TCP_KEEPIDLE);
    D(CURLOPT_TCP_KEEPINTVL);
    D(CURLOPT_UNIX_SOCKET_PATH);
    D(CURLOPT_ABSTRACT_UNIX_SOCKET);

// NAMES AND PASSWORDS OPTIONS
    D(CURLOPT_NETRC);
    __D(CURL_NETRC_OPTIONAL);
    __D(CURL_NETRC_IGNORED);
    __D(CURL_NETRC_REQUIRED);
    D(CURLOPT_NETRC_FILE);
    D(CURLOPT_USERPWD);
    D(CURLOPT_PROXYUSERPWD);
    D(CURLOPT_USERNAME);
    D(CURLOPT_PASSWORD);
    D(CURLOPT_LOGIN_OPTIONS);
    D(CURLOPT_PROXYUSERNAME);
    D(CURLOPT_PROXYPASSWORD);
    D(CURLOPT_HTTPAUTH);
    __D(CURLAUTH_BASIC);
    __D(CURLAUTH_DIGEST);
    __D(CURLAUTH_DIGEST_IE);
#ifdef CURLAUTH_BEARER
    __D(CURLAUTH_BEARER);
#else
    __ND(CURLAUTH_BEARER);
#endif
    __D(CURLAUTH_NEGOTIATE);
    __D(CURLAUTH_NTLM);
    __D(CURLAUTH_NTLM_WB);
    __D(CURLAUTH_ANY);
    __D(CURLAUTH_ANYSAFE);
    __D(CURLAUTH_ONLY);
    D(CURLOPT_TLSAUTH_USERNAME);
    D(CURLOPT_PROXY_TLSAUTH_USERNAME);
    D(CURLOPT_TLSAUTH_PASSWORD);
    D(CURLOPT_PROXY_TLSAUTH_PASSWORD);
    D(CURLOPT_TLSAUTH_TYPE);
    D(CURLOPT_PROXY_TLSAUTH_TYPE);
    D(CURLOPT_PROXYAUTH);
    D(CURLOPT_SASL_IR);
    D(CURLOPT_XOAUTH2_BEARER);
#ifdef CURLOPT_DISALLOW_USERNAME_IN_URL
    D(CURLOPT_DISALLOW_USERNAME_IN_URL);
#else
    ND(CURLOPT_DISALLOW_USERNAME_IN_URL);
#endif

// HTTP OPTIONS
    D(CURLOPT_AUTOREFERER);
    D(CURLOPT_ACCEPT_ENCODING);
    D(CURLOPT_TRANSFER_ENCODING);
    D(CURLOPT_FOLLOWLOCATION);
    D(CURLOPT_UNRESTRICTED_AUTH);
    D(CURLOPT_MAXREDIRS);
    D(CURLOPT_POSTREDIR);
    __D(CURL_REDIR_POST_301);
    __D(CURL_REDIR_POST_302);
    __D(CURL_REDIR_POST_303);
    __D(CURL_REDIR_POST_ALL);
    D(CURLOPT_PUT);
    D(CURLOPT_POST);
    D(CURLOPT_POSTFIELDS);
    D(CURLOPT_POSTFIELDSIZE);
    D(CURLOPT_POSTFIELDSIZE_LARGE);
    D(CURLOPT_COPYPOSTFIELDS);
    D(CURLOPT_HTTPPOST);
    D(CURLOPT_REFERER);
    D(CURLOPT_USERAGENT);
    D(CURLOPT_HTTPHEADER);
    D(CURLOPT_HEADEROPT);
    D(CURLOPT_PROXYHEADER);
    D(CURLOPT_HTTP200ALIASES);
    D(CURLOPT_COOKIE);
    D(CURLOPT_COOKIEFILE);
    D(CURLOPT_COOKIEJAR);
    D(CURLOPT_COOKIESESSION);
    D(CURLOPT_COOKIELIST);
    D(CURLOPT_HTTPGET);
    D(CURLOPT_REQUEST_TARGET);
    D(CURLOPT_HTTP_VERSION);
    __D(CURL_HTTP_VERSION_NONE);
    __D(CURL_HTTP_VERSION_1_0);
    __D(CURL_HTTP_VERSION_1_1);
    __D(CURL_HTTP_VERSION_2_0);
    __D(CURL_HTTP_VERSION_2TLS);
    __D(CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE);
#ifdef CURLOPT_HTTP09_ALLOWED
    D(CURLOPT_HTTP09_ALLOWED);
#else
    ND(CURLOPT_HTTP09_ALLOWED);
#endif
    D(CURLOPT_IGNORE_CONTENT_LENGTH);
    D(CURLOPT_HTTP_CONTENT_DECODING);
    D(CURLOPT_HTTP_TRANSFER_DECODING);
    D(CURLOPT_EXPECT_100_TIMEOUT_MS);
    D(CURLOPT_PIPEWAIT);
    D(CURLOPT_STREAM_WEIGHT);

// SMTP OPTIONS
    D(CURLOPT_MAIL_FROM);
    D(CURLOPT_MAIL_RCPT);
    D(CURLOPT_MAIL_AUTH);

// TFTP OPTIONS
    D(CURLOPT_TFTP_BLKSIZE);
    D(CURLOPT_TFTP_NO_OPTIONS);

// FTP OPTIONS
    D(CURLOPT_FTPPORT);
    D(CURLOPT_QUOTE);
    D(CURLOPT_POSTQUOTE);
    D(CURLOPT_PREQUOTE);
    D(CURLOPT_APPEND);
    D(CURLOPT_FTP_USE_EPRT);
    D(CURLOPT_FTP_USE_EPSV);
    D(CURLOPT_FTP_USE_PRET);
    D(CURLOPT_FTP_CREATE_MISSING_DIRS);
    D(CURLOPT_FTP_RESPONSE_TIMEOUT);
    D(CURLOPT_FTP_ALTERNATIVE_TO_USER);
    D(CURLOPT_FTP_SKIP_PASV_IP);
    D(CURLOPT_FTPSSLAUTH);
    __D(CURLFTPAUTH_DEFAULT);
    __D(CURLFTPAUTH_SSL);
    __D(CURLFTPAUTH_TLS);
    D(CURLOPT_FTP_SSL_CCC);
    __D(CURLFTPSSL_CCC_NONE);
    __D(CURLFTPSSL_CCC_PASSIVE);
    __D(CURLFTPSSL_CCC_ACTIVE);
    D(CURLOPT_FTP_ACCOUNT);
    D(CURLOPT_FTP_FILEMETHOD);
    __D(CURLFTPMETHOD_MULTICWD);
    __D(CURLFTPMETHOD_NOCWD);
    __D(CURLFTPMETHOD_SINGLECWD);

// RSTP OPTIONS
    D(CURLOPT_RTSP_REQUEST);
    __D(CURL_RTSPREQ_OPTIONS);
    __D(CURL_RTSPREQ_DESCRIBE);
    __D(CURL_RTSPREQ_ANNOUNCE);
    __D(CURL_RTSPREQ_SETUP);
    __D(CURL_RTSPREQ_PLAY);
    __D(CURL_RTSPREQ_PAUSE);
    __D(CURL_RTSPREQ_TEARDOWN);
    __D(CURL_RTSPREQ_GET_PARAMETER);
    __D(CURL_RTSPREQ_SET_PARAMETER);
    __D(CURL_RTSPREQ_RECORD);
    __D(CURL_RTSPREQ_RECEIVE);
    D(CURLOPT_RTSP_SESSION_ID);
    D(CURLOPT_RTSP_STREAM_URI);
    D(CURLOPT_RTSP_TRANSPORT);
// CURLOPT_RTSP_HEADER is unimplemented
    D(CURLOPT_RTSP_CLIENT_CSEQ);
// CURLOPT_RTSP_SERVER_CSEQ is unimplemented

// PROTOCOL OPTIONS
    D(CURLOPT_TRANSFERTEXT);
    D(CURLOPT_PROXY_TRANSFER_MODE);
    D(CURLOPT_CRLF);
    D(CURLOPT_RANGE);
    D(CURLOPT_RESUME_FROM);
    D(CURLOPT_RESUME_FROM_LARGE);
    D(CURLOPT_CUSTOMREQUEST);
    D(CURLOPT_FILETIME);
    D(CURLOPT_DIRLISTONLY);
    D(CURLOPT_NOBODY);
    D(CURLOPT_INFILESIZE);
    D(CURLOPT_INFILESIZE_LARGE);
    D(CURLOPT_UPLOAD);
#ifdef CURLOPT_UPLOAD_BUFFERSIZE
    D(CURLOPT_UPLOAD_BUFFERSIZE);
#else
    ND(CURLOPT_UPLOAD_BUFFERSIZE);
#endif
    D(CURLOPT_MAXFILESIZE);
    D(CURLOPT_MAXFILESIZE_LARGE);
    D(CURLOPT_TIMECONDITION);
    D(CURLOPT_TIMEVALUE);
#ifdef CURLOPT_TIMEVALUE_LARGE
    D(CURLOPT_TIMEVALUE_LARGE);
#else
    ND(CURLOPT_TIMEVALUE_LARGE);
#endif

// CONNECTION OPTIONS
    D(CURLOPT_TIMEOUT);
    D(CURLOPT_TIMEOUT_MS);
    D(CURLOPT_LOW_SPEED_LIMIT);
    D(CURLOPT_LOW_SPEED_TIME);
    D(CURLOPT_MAX_SEND_SPEED_LARGE);
    D(CURLOPT_MAX_RECV_SPEED_LARGE);
    D(CURLOPT_MAXCONNECTS);
// CURLOPT_CLOSEPOLICY is obsolete
    D(CURLOPT_FRESH_CONNECT);
    D(CURLOPT_FORBID_REUSE);
    D(CURLOPT_CONNECTTIMEOUT);
    D(CURLOPT_CONNECTTIMEOUT_MS);
    D(CURLOPT_IPRESOLVE);
    __D(CURL_IPRESOLVE_WHATEVER);
    __D(CURL_IPRESOLVE_V4);
    __D(CURL_IPRESOLVE_V6);
    D(CURLOPT_CONNECT_ONLY);
    D(CURLOPT_USE_SSL);
    __D(CURLUSESSL_NONE);
    __D(CURLUSESSL_TRY);
    __D(CURLUSESSL_CONTROL);
    __D(CURLUSESSL_ALL);
    D(CURLOPT_RESOLVE);
    D(CURLOPT_DNS_INTERFACE);
    D(CURLOPT_DNS_LOCAL_IP4);
    D(CURLOPT_DNS_LOCAL_IP6);
    D(CURLOPT_DNS_SERVERS);
#ifdef CURLOPT_DNS_SHUFFLE_ADDRESSES
    D(CURLOPT_DNS_SHUFFLE_ADDRESSES);
#else
    ND(CURLOPT_DNS_SHUFFLE_ADDRESSES);
#endif
    D(CURLOPT_ACCEPTTIMEOUT_MS);
#ifdef CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS
    D(CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS);
#else
    ND(CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS);
#endif
#ifdef CURLOPT_UPKEEP_INTERVAL_MS
    D(CURLOPT_UPKEEP_INTERVAL_MS);
#else
    ND(CURLOPT_UPKEEP_INTERVAL_MS);
#endif

// SSL AND SECURITY OPTIONS
    D(CURLOPT_SSLCERT);
    D(CURLOPT_PROXY_SSLCERT);
    D(CURLOPT_SSLCERTTYPE);
    D(CURLOPT_PROXY_SSLCERTTYPE);
    D(CURLOPT_SSLKEY);
    D(CURLOPT_PROXY_SSLKEY);
    D(CURLOPT_SSLKEYTYPE);
    D(CURLOPT_PROXY_SSLKEYTYPE);
    D(CURLOPT_KEYPASSWD);
    D(CURLOPT_PROXY_KEYPASSWD);
    D(CURLOPT_SSL_ENABLE_ALPN);
    D(CURLOPT_SSL_ENABLE_NPN);
    D(CURLOPT_SSLENGINE);
    D(CURLOPT_SSLENGINE_DEFAULT);
    D(CURLOPT_SSL_FALSESTART);
    D(CURLOPT_SSLVERSION);
    __D(CURL_SSLVERSION_DEFAULT);
    __D(CURL_SSLVERSION_TLSv1);
    __D(CURL_SSLVERSION_TLSv1_0);
    __D(CURL_SSLVERSION_TLSv1_1);
    __D(CURL_SSLVERSION_TLSv1_2);
    __D(CURL_SSLVERSION_TLSv1_3);
    __D(CURL_SSLVERSION_MAX_DEFAULT);
    __D(CURL_SSLVERSION_MAX_TLSv1_0);
    __D(CURL_SSLVERSION_MAX_TLSv1_1);
    __D(CURL_SSLVERSION_MAX_TLSv1_2);
    __D(CURL_SSLVERSION_MAX_TLSv1_3);
    __D(CURL_SSLVERSION_SSLv2);
    __D(CURL_SSLVERSION_SSLv3);
    D(CURLOPT_PROXY_SSLVERSION);
    D(CURLOPT_SSL_VERIFYHOST);
    D(CURLOPT_PROXY_SSL_VERIFYHOST);
    D(CURLOPT_SSL_VERIFYPEER);
    D(CURLOPT_PROXY_SSL_VERIFYPEER);
    D(CURLOPT_CAINFO);
    D(CURLOPT_PROXY_CAINFO);
    D(CURLOPT_ISSUERCERT);
    D(CURLOPT_CAPATH);
    D(CURLOPT_PROXY_CAPATH);
    D(CURLOPT_CRLFILE);
    D(CURLOPT_SSL_VERIFYHOST);
    D(CURLOPT_CERTINFO);
    D(CURLOPT_RANDOM_FILE);
    D(CURLOPT_EGDSOCKET);
    D(CURLOPT_SSL_CIPHER_LIST);
    D(CURLOPT_SSL_SESSIONID_CACHE);
    D(CURLOPT_KRBLEVEL);
    D(CURLOPT_GSSAPI_DELEGATION);
    __D(CURLGSSAPI_DELEGATION_FLAG);
    __D(CURLGSSAPI_DELEGATION_POLICY_FLAG);
    D(CURLOPT_SSH_AUTH_TYPES);
    __D(CURLSSH_AUTH_PUBLICKEY);
    __D(CURLSSH_AUTH_PASSWORD);
    __D(CURLSSH_AUTH_HOST);
    __D(CURLSSH_AUTH_KEYBOARD);
    __D(CURLSSH_AUTH_ANY);
    D(CURLOPT_SSH_HOST_PUBLIC_KEY_MD5);
    D(CURLOPT_SSH_PUBLIC_KEYFILE);
    D(CURLOPT_SSH_PRIVATE_KEYFILE);
    D(CURLOPT_SSH_KNOWNHOSTS);
// CURLOPT_SSH_KEYFUNCTION is complicated
//__D(CURLKHSTAT_FINE_ADD_TO_FILE);
//__D(CURLKHSTAT_FINE);
//__D(CURLKHSTAT_REJECT);
//__D(CURLKHSTAT_DEFER);
//D(CURLOPT_SSH_KEYDATA);
//D(CURLOPT_PRIVATE);
//D(CURLOPT_SHARE);
    D(CURLOPT_NEW_FILE_PERMS);
    D(CURLOPT_NEW_DIRECTORY_PERMS);
    D(CURLOPT_TELNETOPTIONS);

/* ERROR CODES */

    D(CURLE_OK);
    D(CURLE_UNSUPPORTED_PROTOCOL);
    D(CURLE_FAILED_INIT);
    D(CURLE_URL_MALFORMAT);
    D(CURLE_NOT_BUILT_IN);
    D(CURLE_COULDNT_RESOLVE_PROXY);
    D(CURLE_COULDNT_RESOLVE_HOST);
    D(CURLE_COULDNT_CONNECT);
    D(CURLE_FTP_WEIRD_SERVER_REPLY);
    D(CURLE_REMOTE_ACCESS_DENIED);
    D(CURLE_FTP_ACCEPT_FAILED);
    D(CURLE_FTP_WEIRD_PASS_REPLY);
    D(CURLE_FTP_ACCEPT_TIMEOUT);
    D(CURLE_FTP_WEIRD_PASV_REPLY);
    D(CURLE_FTP_WEIRD_227_FORMAT);
    D(CURLE_FTP_CANT_GET_HOST);
    D(CURLE_HTTP2);
    D(CURLE_FTP_COULDNT_SET_TYPE);
    D(CURLE_PARTIAL_FILE);
    D(CURLE_FTP_COULDNT_RETR_FILE);
    D(CURLE_FTP_QUOTE_ERROR);
    D(CURLE_HTTP_RETURNED_ERROR);
    D(CURLE_WRITE_ERROR);
    D(CURLE_UPLOAD_FAILED);
    D(CURLE_READ_ERROR);
    D(CURLE_OUT_OF_MEMORY);
    D(CURLE_OPERATION_TIMEDOUT);
    D(CURLE_FTP_PORT_FAILED);
    D(CURLE_FTP_COULDNT_USE_REST);
    D(CURLE_RANGE_ERROR);
    D(CURLE_HTTP_POST_ERROR);
    D(CURLE_SSL_CONNECT_ERROR);
    D(CURLE_BAD_DOWNLOAD_RESUME);
    D(CURLE_FILE_COULDNT_READ_FILE);
    D(CURLE_LDAP_CANNOT_BIND);
    D(CURLE_LDAP_SEARCH_FAILED);
    D(CURLE_FUNCTION_NOT_FOUND);
    D(CURLE_ABORTED_BY_CALLBACK);
    D(CURLE_BAD_FUNCTION_ARGUMENT);
    D(CURLE_INTERFACE_FAILED);
    D(CURLE_TOO_MANY_REDIRECTS);
    D(CURLE_UNKNOWN_OPTION);
    D(CURLE_TELNET_OPTION_SYNTAX);
    D(CURLE_GOT_NOTHING);
    D(CURLE_SSL_ENGINE_NOTFOUND);
    D(CURLE_SSL_ENGINE_SETFAILED);
    D(CURLE_SEND_ERROR);
    D(CURLE_RECV_ERROR);
    D(CURLE_SSL_CERTPROBLEM);
    D(CURLE_SSL_CIPHER);
    D(CURLE_PEER_FAILED_VERIFICATION);
//D(CURLE_SSL_CACERT);
    D(CURLE_BAD_CONTENT_ENCODING);
    D(CURLE_LDAP_INVALID_URL);
    D(CURLE_FILESIZE_EXCEEDED);
    D(CURLE_USE_SSL_FAILED);
    D(CURLE_SEND_FAIL_REWIND);
    D(CURLE_SSL_ENGINE_INITFAILED);
    D(CURLE_LOGIN_DENIED);
    D(CURLE_TFTP_NOTFOUND);
    D(CURLE_TFTP_PERM);
    D(CURLE_REMOTE_DISK_FULL);
    D(CURLE_TFTP_ILLEGAL);
    D(CURLE_TFTP_UNKNOWNID);
    D(CURLE_REMOTE_FILE_EXISTS);
    D(CURLE_TFTP_NOSUCHUSER);
    D(CURLE_CONV_FAILED);
    D(CURLE_CONV_REQD);
    D(CURLE_SSL_CACERT_BADFILE);
    D(CURLE_REMOTE_FILE_NOT_FOUND);
    D(CURLE_SSH);
    D(CURLE_SSL_SHUTDOWN_FAILED);
    D(CURLE_AGAIN);
    D(CURLE_SSL_CRL_BADFILE);
    D(CURLE_SSL_ISSUER_ERROR);
    D(CURLE_FTP_PRET_FAILED);
    D(CURLE_RTSP_CSEQ_ERROR);
    D(CURLE_FTP_BAD_FILE_LIST);
    D(CURLE_CHUNK_FAILED);
    D(CURLE_NO_CONNECTION_AVAILABLE);
    D(CURLE_SSL_PINNEDPUBKEYNOTMATCH);
    D(CURLE_SSL_INVALIDCERTSTATUS);
    D(CURLE_HTTP2_STREAM);
#ifdef CURLE_RECURSIVE_API_CALL
    D(CURLE_RECURSIVE_API_CALL);
#else
    ND(CURLE_RECURSIVE_API_CALL);
#endif
}
