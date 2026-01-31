//
// Copyright (c) 2025 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/burl
//

#ifndef BOOST_BURL_PARSE_ARGS_HPP
#define BOOST_BURL_PARSE_ARGS_HPP

#include <boost/burl/fwd.hpp>

#include <optional>
#include <string>
#include <system_error>
#include <vector>

namespace boost {
namespace burl {

//----------------------------------------------------------

/** Authentication type for HTTP requests.
*/
enum class auth_type
{
    basic,
    digest,
    ntlm,
    negotiate,
    any
};

//----------------------------------------------------------

/** Parsed command-line arguments for burl.

    This structure holds all parsed command-line options
    in a format compatible with curl's option semantics.
*/
struct burl_args
{
    /// URLs (positional arguments)
    std::vector<std::string> urls;

    /// HTTP method (-X, --request)
    std::string method;

    //------------------------------------------------------
    // Data options
    //------------------------------------------------------

    /// Request body data (-d, --data)
    std::vector<std::string> data;

    /// Binary data (--data-binary)
    std::vector<std::string> data_binary;

    /// Raw data without @file processing (--data-raw)
    std::vector<std::string> data_raw;

    /// URL-encoded data (--data-urlencode)
    std::vector<std::string> data_urlencode;

    /// Multipart form data (-F, --form)
    std::vector<std::string> forms;

    /// JSON body data (--json)
    std::optional<std::string> json;

    /// Upload file (-T, --upload-file)
    std::optional<std::string> upload_file;

    //------------------------------------------------------
    // Header options
    //------------------------------------------------------

    /// Custom headers (-H, --header)
    std::vector<std::string> headers;

    /// User-Agent header (-A, --user-agent)
    std::optional<std::string> user_agent;

    /// Referer header (-e, --referer)
    std::optional<std::string> referer;

    //------------------------------------------------------
    // Output options
    //------------------------------------------------------

    /// Output file (-o, --output)
    std::optional<std::string> output;

    /// Use remote filename for output (-O, --remote-name)
    bool remote_name = false;

    /// Include response headers in output (-i, --include)
    bool include_headers = false;

    /// Fetch headers only (-I, --head)
    bool head_only = false;

    /// Dump headers to file (-D, --dump-header)
    std::optional<std::string> dump_header;

    /// Write output format (-w, --write-out)
    std::optional<std::string> write_out;

    //------------------------------------------------------
    // Authentication options
    //------------------------------------------------------

    /// Username:password (-u, --user)
    std::optional<std::string> user;

    /// Authentication type (--basic, --digest, etc.)
    auth_type auth = auth_type::basic;

    //------------------------------------------------------
    // Cookie options
    //------------------------------------------------------

    /// Cookie data or file (-b, --cookie)
    std::optional<std::string> cookie;

    /// Cookie jar file (-c, --cookie-jar)
    std::optional<std::string> cookie_jar;

    //------------------------------------------------------
    // TLS options
    //------------------------------------------------------

    /// Skip TLS verification (-k, --insecure)
    bool insecure = false;

    /// CA certificate file (--cacert)
    std::optional<std::string> cacert;

    /// Client certificate file (--cert)
    std::optional<std::string> cert;

    /// Client key file (--key)
    std::optional<std::string> key;

    //------------------------------------------------------
    // Proxy options
    //------------------------------------------------------

    /// Proxy URL (-x, --proxy)
    std::optional<std::string> proxy;

    //------------------------------------------------------
    // Behavior options
    //------------------------------------------------------

    /// Follow redirects (-L, --location)
    bool follow_redirects = false;

    /// Maximum redirects (--max-redirs)
    int max_redirs = 50;

    /// Maximum time in seconds (-m, --max-time)
    std::optional<double> max_time;

    /// Connection timeout (--connect-timeout)
    std::optional<double> connect_timeout;

    //------------------------------------------------------
    // Verbosity options
    //------------------------------------------------------

    /// Verbose output (-v, --verbose)
    bool verbose = false;

    /// Silent mode (-s, --silent)
    bool silent = false;

    /// Show errors in silent mode (-S, --show-error)
    bool show_error = false;

    //------------------------------------------------------
    // Misc options
    //------------------------------------------------------

    /// Compressed response (--compressed)
    bool compressed = false;

    /// Show help (--help, -h)
    bool help = false;

    /// Show version (--version, -V)
    bool version = false;
};

//----------------------------------------------------------

/** Result of parsing command-line arguments.
*/
struct parse_result
{
    /// Error code (success if parsing succeeded)
    std::error_code ec;

    /// Error message (empty if parsing succeeded)
    std::string error_message;

    /// Parsed arguments
    burl_args args;
};

//----------------------------------------------------------

/** Parse command-line arguments into burl_args.

    Parses command-line arguments using curl-compatible
    option syntax, including short options (-v), long
    options (--verbose), combined short options (-sS),
    and positional URL arguments.

    @param argc Argument count from main()
    @param argv Argument vector from main()

    @return Parse result containing error status and parsed args
*/
BOOST_BURL_DECL
parse_result
parse_args(int argc, char const* const* argv);

} // namespace burl
} // namespace boost

#endif
