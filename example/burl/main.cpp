//
// Copyright (c) 2025 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/burl
//

#include <boost/burl/parse_args.hpp>
#include <boost/burl/session.hpp>
#include <boost/burl/auth.hpp>

#include <boost/capy/ex/run_async.hpp>
#include <boost/corosio/io_context.hpp>
#include <boost/corosio/tls/context.hpp>
#include <boost/http/method.hpp>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

namespace burl = boost::burl;
namespace corosio = boost::corosio;
namespace capy = boost::capy;
namespace http = boost::http;

namespace {

constexpr char const* version_string = "burl 0.1.0";

constexpr char const* help_text = R"(Usage: burl [options...] <url>

Options:
  -d, --data <data>        Send data in POST request
  -H, --header <header>    Add custom header
  -o, --output <file>      Write output to file
  -v, --verbose            Verbose output
  -s, --silent             Silent mode
  -S, --show-error         Show errors in silent mode
  -L, --location           Follow redirects
  -u, --user <user:pass>   Server authentication
  -k, --insecure           Skip TLS verification
  -X, --request <method>   HTTP method to use
  -A, --user-agent <name>  User-Agent header
  -e, --referer <url>      Referer header
  -b, --cookie <data>      Cookie data
  -c, --cookie-jar <file>  Cookie jar file
  -i, --include            Include response headers
  -I, --head               Fetch headers only
  -m, --max-time <secs>    Maximum time for request
      --connect-timeout <secs>  Connection timeout
      --max-redirs <num>   Maximum redirects
      --compressed         Request compressed response
      --cacert <file>      CA certificate file
      --cert <file>        Client certificate
      --key <file>         Client key
  -x, --proxy <url>        Proxy URL
  -h, --help               Show this help
  -V, --version            Show version
)";

void
print_help()
{
    std::cout << help_text;
}

void
print_version()
{
    std::cout << version_string << '\n';
}

http::method
string_to_method(std::string const& s)
{
    if(s.empty() || s == "GET")
        return http::method::get;
    if(s == "POST")
        return http::method::post;
    if(s == "PUT")
        return http::method::put;
    if(s == "DELETE")
        return http::method::delete_;
    if(s == "PATCH")
        return http::method::patch;
    if(s == "HEAD")
        return http::method::head;
    if(s == "OPTIONS")
        return http::method::options;
    return http::method::get;
}

capy::io_task<int>
run_request(
    burl::session& sess,
    burl::burl_args const& args)
{
    if(args.urls.empty())
    {
        std::cerr << "burl: no URL specified\n";
        std::cerr << "Try 'burl --help' for more information.\n";
        co_return 1;
    }

    // Determine HTTP method
    auto method = string_to_method(args.method);
    if(args.head_only)
        method = http::method::head;

    // Build request options
    burl::request_options opts;

    // Add custom headers
    if(!args.headers.empty())
    {
        opts.headers = http::fields{};
        for(auto const& h : args.headers)
        {
            auto pos = h.find(':');
            if(pos != std::string::npos)
            {
                auto name = h.substr(0, pos);
                auto value = h.substr(pos + 1);
                // Trim leading whitespace from value
                while(!value.empty() && value[0] == ' ')
                    value = value.substr(1);
                opts.headers->set(name, value);
            }
        }
    }

    // Add data
    if(!args.data.empty())
    {
        std::string body;
        for(std::size_t i = 0; i < args.data.size(); ++i)
        {
            if(i > 0)
                body += '&';
            body += args.data[i];
        }
        opts.data = std::move(body);
        if(method == http::method::get)
            method = http::method::post;
    }

    // Add JSON body
    if(args.json.has_value())
    {
        opts.json = args.json.value();
        if(method == http::method::get)
            method = http::method::post;
    }

    // Set auth
    if(args.user.has_value())
    {
        auto const& u = args.user.value();
        auto pos = u.find(':');
        std::string username = (pos != std::string::npos)
            ? u.substr(0, pos) : u;
        std::string password = (pos != std::string::npos)
            ? u.substr(pos + 1) : "";
        opts.auth = std::make_shared<burl::http_basic_auth>(
            std::move(username), std::move(password));
    }

    // Set redirect handling
    if(args.follow_redirects)
        opts.max_redirects = args.max_redirs;
    else
        opts.max_redirects = 0;

    // Set TLS verification
    opts.verify = !args.insecure;

    // Set timeout
    if(args.max_time.has_value())
    {
        opts.timeout = std::chrono::milliseconds(
            static_cast<int>(args.max_time.value() * 1000));
    }

    // Process each URL
    for(auto const& url : args.urls)
    {
        auto [ec, resp] = co_await sess.request(method, url, opts);

        if(ec.failed())
        {
            if(!args.silent || args.show_error)
                std::cerr << "burl: " << ec.message() << '\n';
            co_return 1;
        }

        // Output
        std::ostream* out = &std::cout;
        std::ofstream file;
        if(args.output.has_value())
        {
            file.open(args.output.value());
            if(!file)
            {
                std::cerr << "burl: cannot open output file: "
                    << args.output.value() << '\n';
                co_return 1;
            }
            out = &file;
        }

        // Include headers if requested
        if(args.include_headers)
        {
            *out << "HTTP/" << resp.version_major << '.'
                 << resp.version_minor << ' '
                 << resp.status_code << ' '
                 << resp.reason << "\r\n";
            for(auto const& f : resp.headers)
                *out << f.name << ": " << f.value << "\r\n";
            *out << "\r\n";
        }

        // Output body (unless HEAD request)
        if(!args.head_only)
            *out << resp.body;
    }

    co_return 0;
}

} // namespace

int main(int argc, char** argv)
{
    // Parse command line
    auto result = burl::parse_args(argc, const_cast<char const* const*>(argv));
    if(result.ec.failed())
    {
        std::cerr << "burl: " << result.error_message << '\n';
        return 1;
    }

    auto const& args = result.args;

    // Handle --help
    if(args.help)
    {
        print_help();
        return 0;
    }

    // Handle --version
    if(args.version)
    {
        print_version();
        return 0;
    }

    // No URLs is an error (unless --help/--version)
    if(args.urls.empty())
    {
        std::cerr << "burl: no URL specified\n";
        std::cerr << "Try 'burl --help' for more information.\n";
        return 1;
    }

    // Create io_context and TLS context
    corosio::io_context ioc;
    corosio::tls::context tls_ctx;

    // Configure TLS
    tls_ctx.set_default_verify_paths();
    if(args.insecure)
        tls_ctx.set_verify_mode(corosio::tls::verify_mode::none);

    if(args.cacert.has_value())
        tls_ctx.load_verify_file(args.cacert.value());

    // Create session
    burl::session sess(ioc, tls_ctx);

    // Configure session from args
    if(args.user_agent.has_value())
        sess.headers().set(http::field::user_agent, args.user_agent.value());
    else
        sess.headers().set(http::field::user_agent, version_string);

    if(args.referer.has_value())
        sess.headers().set(http::field::referer, args.referer.value());

    if(args.follow_redirects)
        sess.set_max_redirects(args.max_redirs);
    else
        sess.set_max_redirects(0);

    // Run the request
    int exit_code = 0;
    capy::run_async(ioc.get_executor())(
        [&]() -> capy::io_task<>
        {
            exit_code = co_await run_request(sess, args);
            co_return {};
        }());

    ioc.run();

    return exit_code;
}
