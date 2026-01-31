# curl Command-Line Usage Reference (HTTP/HTTPS)

A concise reference for implementing a curl-compatible command-line tool in C++, focused on HTTP and HTTPS protocols.

## URL Handling

### Supported Schemes
HTTP, HTTPS

### URL Format
- Scheme separator: `://` (accept 1-3 slashes)
- Without scheme: default to HTTP
- `--proto-default` changes default protocol
- User/password: `scheme://user:password@host/` (URL-encode special chars: `:` = `%3a`, `@` = `%40`)
- Port: `host:port` (0-65535)
- Fragment (`#anchor`): stripped, not sent over wire
- Path `/./` and `/../` normalized by default; `--path-as-is` preserves them

### URL Globbing
- Numeric ranges: `[1-100]`, `[001-100]` (zero-padded), `[0-100:2]` (step)
- Alphabetic ranges: `[a-z]`
- Lists: `{one,two,three}`
- Combinations: `{Ben,Alice}-{100x100,1000x1000}.jpg`
- Output variables: `#1`, `#2` for each glob in `-o`
- `-g` / `--globoff` disables globbing

### Query Strings
`--url-query` adds to query with automatic `&` separators:
- `content`: URL-encode content
- `=content`: URL-encode content (no leading `=`)
- `name=content`: URL-encode value only
- `@filename`: URL-encode file contents
- `name@filename`: name=urlencoded-file-content
- `+content`: add verbatim (no encoding)

---

## HTTP Methods & Data

### Method Selection
| Option | Method |
|--------|--------|
| (default) | GET |
| `-d`, `--data` | POST |
| `-F`, `--form` | POST (multipart) |
| `-I`, `--head` | HEAD |
| `-T`, `--upload-file` | PUT |
| `-X`, `--request METHOD` | Override method string |

### POST Data (`-d` / `--data`)
- `-d "data"` or `-d @filename`
- Multiple `-d` concatenated with `&`
- `--data-binary @file`: preserve CR/LF
- `--data-raw "@string"`: no file loading, send `@` literally
- `--data-urlencode`: URL-encode the data
  - `content`, `=content`, `name=content`, `@file`, `name@file`
- Default Content-Type: `application/x-www-form-urlencoded`
- `-G` / `--get`: convert POST to GET with query string

### JSON (`--json`)
Shortcut combining:
- `--data [arg]`
- `--header "Content-Type: application/json"`
- `--header "Accept: application/json"`

### Multipart Form (`-F` / `--form`)
- `-F "name=value"`: text field
- `-F "name=@filename"`: file upload
- `-F "name=<filename"`: file contents as value
- `-F "name=@file;type=mime"`: specify MIME type
- `-F "name=@file;filename=newname"`: override filename
- Content-Type: `multipart/form-data; boundary=...`

### PUT (`-T` / `--upload-file`)
- `-T localfile URL`: upload file
- `-T -`: read from stdin

---

## HTTP Headers & Options

### Custom Headers (`-H` / `--header`)
- `-H "Header: value"`: add/replace header
- `-H "Header:"`: remove header
- `-H "Header;"`: add header with empty value

### Common Headers
| Option | Header |
|--------|--------|
| `-A`, `--user-agent` | User-Agent (default: `curl/VERSION`) |
| `-e`, `--referer` | Referer |
| `-H "Host: X"` | Host |

### Response Headers
- `-i`, `--include`: output headers with body
- `-D`, `--dump-header FILE`: save headers to file

### HTTP Versions
| Option | Version |
|--------|---------|
| `--http0.9` | Accept HTTP/0.9 responses |
| `--http1.0` | HTTP/1.0 |
| `--http1.1` | HTTP/1.1 |
| `--http2` | HTTP/2 (with upgrade) |
| `--http2-prior-knowledge` | HTTP/2 directly |
| `--http3` | HTTP/3 (QUIC) |
| `--http3-only` | HTTP/3 only, no fallback |

---

## Output Control

### Output Destination
| Option | Behavior |
|--------|----------|
| (default) | stdout |
| `-o FILE` | write to FILE |
| `-O` | use remote filename |
| `--remote-name-all` | `-O` for all URLs |
| `-J`, `--remote-header-name` | use Content-Disposition filename |
| `--no-clobber` | don't overwrite existing files |
| `--remove-on-error` | delete partial downloads on error |

### Write-Out (`-w` / `--write-out`)
Format: `-w "format string"` or `-w @file`

Key variables:
- `%{http_code}`, `%{response_code}`: HTTP status
- `%{content_type}`: Content-Type header
- `%{size_download}`, `%{size_upload}`: bytes transferred
- `%{speed_download}`, `%{speed_upload}`: bytes/second
- `%{time_total}`, `%{time_connect}`, `%{time_starttransfer}`
- `%{url_effective}`: final URL after redirects
- `%{filename_effective}`: saved filename
- `%{num_redirects}`: redirect count
- `%{exitcode}`, `%{errormsg}`: error info
- `%{json}`: all variables as JSON
- `%header{name}`: specific response header
- `%{onerror}`: only output on error
- `%{stderr}`, `%{stdout}`: redirect output
- `%output{file}`: write to file

---

## Redirects

### Following Redirects
- `-L`, `--location`: follow redirects
- `--max-redirs N`: limit (default: 50)
- `--location-trusted`: send auth to redirected hosts

### Redirect Method Handling
- 301/302/303: converts POST to GET (browser behavior)
- 307/308: preserves original method
- `--post301`, `--post302`, `--post303`: force POST on redirect

---

## Authentication

### Basic Auth
- `-u user:password` or `--user user:password`
- In URL: `http://user:password@host/`
- `-u user:` prompts for password

### Auth Methods
| Option | Method |
|--------|--------|
| `--basic` | Basic (default) |
| `--digest` | Digest |
| `--ntlm` | NTLM |
| `--negotiate` | Negotiate/SPNEGO |
| `--anyauth` | auto-select safest |

### Proxy Auth
- `-U user:password` or `--proxy-user`
- `--proxy-basic`, `--proxy-digest`, `--proxy-ntlm`, `--proxy-negotiate`, `--proxy-anyauth`

### .netrc
- `-n`, `--netrc`: use ~/.netrc
- `--netrc-file FILE`: specify file
- `--netrc-optional`: use if exists

Format:
```
machine example.com login user password secret
default login anonymous password user@domain
```

---

## Cookies

### Cookie Engine
- `-b ""`: enable empty cookie jar
- `-b FILE` / `--cookie FILE`: read cookies from file
- `-c FILE` / `--cookie-jar FILE`: write cookies to file
- `-b "name=value"`: send specific cookie
- `-j`, `--junk-session-cookies`: discard session cookies

### Cookie File Format (Netscape)
Tab-separated: `domain`, `subdomains`, `path`, `secure`, `expiry`, `name`, `value`

---

## TLS/SSL

### TLS Options
| Option | Purpose |
|--------|---------|
| `-k`, `--insecure` | skip certificate verification |
| `--cacert FILE` | CA bundle file |
| `--capath DIR` | CA directory |
| `--ca-native` | use system CA store |
| `--cert FILE` | client certificate |
| `--cert-type TYPE` | PEM, DER, ENG |
| `--key FILE` | private key |
| `--key-type TYPE` | PEM, DER, ENG |
| `--pass PHRASE` | key passphrase |

### TLS Versions
| Option | Minimum Version |
|--------|-----------------|
| `--sslv2`, `--sslv3` | Legacy (avoid) |
| `--tlsv1` | TLS 1.0+ |
| `--tlsv1.0` | TLS 1.0+ |
| `--tlsv1.1` | TLS 1.1+ |
| `--tlsv1.2` | TLS 1.2+ |
| `--tlsv1.3` | TLS 1.3+ |

### Other TLS
- `--ciphers LIST`: specify ciphers
- `--pinnedpubkey HASH`: pin public key
- `--cert-status`: OCSP stapling

---

## Proxy

### Proxy Types
- `-x`, `--proxy HOST:PORT`: HTTP proxy (default port 1080)
- `--proxy http://...`: explicit HTTP
- `--proxy https://...`: HTTPS proxy
- `--proxy socks4://...`: SOCKS4
- `--proxy socks4a://...`: SOCKS4a
- `--proxy socks5://...`: SOCKS5
- `--proxy socks5h://...`: SOCKS5 with remote DNS
- `--socks4`, `--socks4a`, `--socks5`, `--socks5-hostname`: explicit options

### Proxy Options
- `-p`, `--proxytunnel`: tunnel through proxy (CONNECT)
- `--proxy1.0`: use HTTP/1.0 for CONNECT
- `--proxy-http2`: use HTTP/2 with HTTPS proxy
- `--noproxy LIST`: bypass proxy for hosts

### Environment Variables
- `http_proxy`, `https_proxy`, `all_proxy`
- `NO_PROXY`: comma-separated bypass list
- Note: `http_proxy` lowercase only (CGI security)

---

## Connection Control

### DNS & Resolve
- `--resolve host:port:address`: custom DNS entry
- `--connect-to host:port:newhost:newport`: redirect connection
- `--dns-servers LIST`: custom DNS servers (c-ares only)
- `--dns-interface IF`: DNS interface

### Timeouts
| Option | Purpose |
|--------|---------|
| `-m`, `--max-time SECS` | total transfer timeout |
| `--connect-timeout SECS` | connection timeout |
| `--speed-time SECS` | low speed time |
| `--speed-limit BYTES` | low speed threshold |
| `--expect100-timeout SECS` | Expect: 100-continue wait |

### Connection Options
- `--interface IF`: bind to interface/IP
- `--local-port RANGE`: local port range
- `--keepalive-time SECS`: keepalive interval
- `--no-keepalive`: disable TCP keepalive

---

## Transfer Control

### Rate Limiting
- `--limit-rate RATE`: max transfer speed (supports K, M, G suffixes)
- `--rate N/UNIT`: request rate (e.g., `2/s`, `10/m`, `100/h`)

### Ranges & Resume
- `-r`, `--range RANGE`: byte range (e.g., `0-499`, `500-`, `-500`)
- `-C`, `--continue-at OFFSET`: resume from offset
- `-C -`: auto-detect resume offset

### Retries
- `--retry N`: retry count
- `--retry-delay SECS`: fixed retry delay
- `--retry-max-time SECS`: max retry duration
- `--retry-connrefused`: retry on connection refused
- `--retry-all-errors`: retry on any error

### Parallel Transfers
- `-Z`, `--parallel`: parallel mode
- `--parallel-max N`: max parallel (default: 50)
- `--parallel-immediate`: prefer new connections

---

## Compression

- `--compressed`: request & decompress gzip/brotli/zstd
- `--tr-encoding`: Transfer-Encoding compression

---

## Verbose & Debug

### Verbosity
| Option | Output |
|--------|--------|
| `-v`, `--verbose` | detailed output |
| `-s`, `--silent` | suppress progress/errors |
| `-S`, `--show-error` | show errors with `-s` |
| `-q` | disable .curlrc |

### Tracing
- `--trace FILE`: full trace with hex
- `--trace-ascii FILE`: trace without hex
- `--trace-time`: add timestamps
- `--trace-ids`: add connection/transfer IDs
- `--trace-config AREAS`: trace specific areas (tls, http/2, http/3, all)

---

## Progress Meter

- Default: shows progress to terminal
- `-#`, `--progress-bar`: simple progress bar
- `-s`: disable progress
- Parallel mode uses different format

Fields: `% Total`, `% Received`, `% Xferd`, `Dload Speed`, `Upload Speed`, `Time Total`, `Time Current`, `Time Left`, `Current Speed`

---

## File Size Limits

- `--max-filesize BYTES`: abort if file exceeds size

---

## Conditional Requests

### Time-Based
- `-z "DATE"`: If-Modified-Since
- `-z "-DATE"`: If-Unmodified-Since
- `-z FILE`: use file's timestamp
- `--remote-time`: set local file time to remote

### ETag-Based
- `--etag-save FILE`: save ETag
- `--etag-compare FILE`: use saved ETag

---

## HSTS & Alt-Svc

- `--hsts FILE`: HSTS cache
- `--alt-svc FILE`: Alt-Svc cache

---

## Config Files

### Default Locations
1. `$CURL_HOME/.curlrc`
2. `$XDG_CONFIG_HOME/.curlrc`
3. `$HOME/.curlrc`
4. Windows: `%USERPROFILE%\.curlrc`, `%APPDATA%\.curlrc`

### Config Syntax
```
# comment
url = https://example.com
user-agent = "MyApp/1.0"
location
header = "X-Custom: value"
```

### Variables
- `--variable name=value` or `--variable name@file`
- `--variable %ENVVAR`: import environment
- `--expand-*` options: use `{{name}}` expansion
- Functions: `{{name:trim}}`, `{{name:json}}`, `{{name:url}}`, `{{name:b64}}`

---

## Exit Codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Unsupported protocol |
| 2 | Initialization failed |
| 3 | Malformed URL |
| 5 | Could not resolve proxy |
| 6 | Could not resolve host |
| 7 | Failed to connect |
| 22 | HTTP error (with `--fail`) |
| 23 | Write error |
| 26 | Read error |
| 28 | Timeout |
| 35 | TLS handshake failed |
| 47 | Too many redirects |
| 51 | SSL certificate verification failed |
| 52 | Empty reply from server |
| 56 | Network receive error |
| 60 | CA cert problem |
| 67 | Login denied |

Use `--fail-early` to exit on first URL failure.

---

## Option Separators

- `--next` / `-:`: separate options per URL
- Options before `--next` apply to preceding URLs

Example:
```bash
curl -L http://a.com --next -d "data" http://b.com
```

---

## Quick Reference: Most Common Options

```
curl [options] URL

Output:
  -o FILE           Write to file
  -O                Use remote filename
  -i                Include headers in output
  -s                Silent mode
  -v                Verbose

Data:
  -d DATA           POST data
  -F name=value     Multipart form
  -T FILE           Upload file (PUT)
  -X METHOD         Custom method

Headers:
  -H "Header: val"  Custom header
  -A "agent"        User-Agent
  -e URL            Referer
  -b "cookies"      Send cookies
  -c FILE           Save cookies

Auth:
  -u user:pass      Basic auth
  -k                Insecure (skip TLS verify)

Behavior:
  -L                Follow redirects
  -m SECS           Max time
  --connect-timeout Connection timeout
  -x proxy:port     Use proxy
  -w FORMAT         Write-out format

Info:
  -V, --version     Show version
  -h, --help        Show help
```
