//
// Copyright (c) 2025 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/burl
//
// This file demonstrates all API features of boost::burl.
// Each function corresponds to an elegant usage from the
// reference Python requests-style API in reference/req.cpp.
//

#include <boost/burl/session.hpp>
#include <boost/capy/ex/run_async.hpp>
#include <boost/corosio/tls/context.hpp>

#include <iostream>
#include <thread>
#include <vector>

namespace burl = boost::burl;
namespace capy = boost::capy;
namespace http = boost::http;
namespace urls = boost::urls;
namespace json = boost::json;
namespace corosio = boost::corosio;

//==============================================================
// Example 1: Simple GET request
// 
// Reference: requests::get("https://api.github.com/users/octocat")
//==============================================================

capy::io_task<> example_simple_get(burl::session& s)
{
    auto [ec, r] = co_await s.get("https://api.github.com/users/octocat");
    
    if (ec.failed()) {
        std::cerr << "Error: " << ec.message() << "\n";
        co_return ec;
    }
    
    if (r.ok()) {
        std::cout << "Status: " << r.status_int() << " " << r.reason() << "\n";
        std::cout << "Body: " << r.text() << "\n";
    }
    
    co_return {};
}

//==============================================================
// Example 2: GET with query parameters
//
// Reference: requests::get(url, {params: {{"q", "..."}, {"sort", "..."}}})
//==============================================================

capy::io_task<> example_get_with_params(burl::session& s)
{
    // Build URL with query parameters using Boost.URL
    urls::url url("https://api.github.com/search/repos");
    url.params().append({"q", "requests+cpp"});
    url.params().append({"sort", "stars"});
    
    auto [ec, r] = co_await s.get(url);
    
    if (ec.failed())
        co_return ec;
    
    r.raise_for_status();
    std::cout << "Search results: " << r.text() << "\n";
    
    co_return {};
}

//==============================================================
// Example 3: POST with JSON body
//
// Reference: requests::post(url, {json: R"({...})"})
//==============================================================

capy::io_task<> example_post_json(burl::session& s)
{
    burl::request_options opts;
    opts.json = R"({"name": "new-repo", "private": false})";
    
    auto [ec, r] = co_await s.post("https://api.github.com/user/repos", opts);
    
    if (ec.failed())
        co_return ec;
    
    std::cout << "POST response: " << r.status_int() << "\n";
    std::cout << r.text() << "\n";
    
    co_return {};
}

//==============================================================
// Example 4: POST with form data
//
// Reference: requests::post(url, {data: "key=value&..."})
//==============================================================

capy::io_task<> example_post_form(burl::session& s)
{
    burl::request_options opts;
    opts.data = "username=admin&password=secret";
    
    // Content-Type is auto-set to application/x-www-form-urlencoded
    auto [ec, r] = co_await s.post("https://httpbin.org/post", opts);
    
    if (ec.failed())
        co_return ec;
    
    std::cout << "Form POST response: " << r.text() << "\n";
    
    co_return {};
}

//==============================================================
// Example 5: Request with custom headers
//
// Reference: requests::get(url, {headers: {...}})
//==============================================================

capy::io_task<> example_custom_headers(burl::session& s)
{
    burl::request_options opts;
    opts.headers = http::fields{};
    opts.headers->set(http::field::authorization, "Bearer token123");
    opts.headers->set(http::field::accept, "application/vnd.github.v3+json");
    opts.headers->set("X-Custom-Header", "custom-value");
    
    auto [ec, r] = co_await s.get("https://api.github.com/user", opts);
    
    if (ec.failed())
        co_return ec;
    
    std::cout << "Response with custom headers: " << r.status_int() << "\n";
    
    co_return {};
}

//==============================================================
// Example 6: Request with timeout
//
// Reference: requests::get(url, {timeout: timeout::from_pair(3.05, 27.0)})
//==============================================================

capy::io_task<> example_with_timeout(burl::session& s)
{
    burl::request_options opts;
    opts.timeout = std::chrono::milliseconds{5000};  // 5 second timeout
    
    auto [ec, r] = co_await s.get("https://httpbin.org/delay/2", opts);
    
    if (ec.failed()) {
        if (ec == burl::make_error_code(burl::error::timeout))
            std::cout << "Request timed out!\n";
        else
            std::cout << "Error: " << ec.message() << "\n";
        co_return ec;
    }
    
    std::cout << "Completed in " << r.elapsed.count() << "ms\n";
    
    co_return {};
}

//==============================================================
// Example 7: Request with Basic auth
//
// Reference: requests::get(url, {auth: make_shared<http_basic_auth>(...)})
//==============================================================

capy::io_task<> example_basic_auth(burl::session& s)
{
    burl::request_options opts;
    opts.auth = std::make_shared<burl::http_basic_auth>("user", "passwd");
    
    auto [ec, r] = co_await s.get(
        "https://httpbin.org/basic-auth/user/passwd", opts);
    
    if (ec.failed())
        co_return ec;
    
    std::cout << "Basic auth result: " << r.status_int() << "\n";
    
    co_return {};
}

//==============================================================
// Example 8: Session with persistent cookies
//
// Reference: Session maintains cookies automatically across requests
//==============================================================

capy::io_task<> example_session_cookies(burl::session& s)
{
    // First request sets a cookie via Set-Cookie header
    auto [ec1, r1] = co_await s.get(
        "https://httpbin.org/cookies/set/sessionid/abc123");
    
    if (ec1.failed())
        co_return ec1;
    
    // Check cookies in the session's jar
    std::cout << "Cookies in jar: " << s.cookies().size() << "\n";
    for (auto const& c : s.cookies()) {
        std::cout << "  " << c.name << " = " << c.value << "\n";
    }
    
    // Next request automatically sends cookies
    auto [ec2, r2] = co_await s.get("https://httpbin.org/cookies");
    
    if (ec2.failed())
        co_return ec2;
    
    std::cout << "Cookies echoed back: " << r2.text() << "\n";
    
    co_return {};
}

//==============================================================
// Example 9: Session with default headers
//
// Reference: session.headers().set(...) for all requests
//==============================================================

capy::io_task<> example_session_defaults(burl::session& s)
{
    // Set headers that apply to all requests from this session
    s.headers().set(http::field::authorization, "Bearer mytoken");
    s.headers().set("X-Api-Version", "2.0");
    
    // All requests include these headers automatically
    auto [ec1, r1] = co_await s.get("https://api.example.com/resource1");
    auto [ec2, r2] = co_await s.get("https://api.example.com/resource2");
    auto [ec3, r3] = co_await s.post("https://api.example.com/resource3");
    
    (void)r1; (void)r2; (void)r3;
    
    s.close();
    
    co_return {};
}

//==============================================================
// Example 10: Handle redirects
//
// Reference: requests::get(url, {allow_redirects: true})
//==============================================================

capy::io_task<> example_redirects(burl::session& s)
{
    // Default: follows redirects automatically
    auto [ec, r] = co_await s.get("https://httpbin.org/redirect/3");
    
    if (ec.failed())
        co_return ec;
    
    std::cout << "Final URL: " << r.url.buffer() << "\n";
    std::cout << "Redirects followed: " << r.history.size() << "\n";
    
    // Check redirect history
    for (auto const& h : r.history) {
        std::cout << "  -> " << h.status_int() << " " << h.url.buffer() << "\n";
    }
    
    co_return {};
}

//==============================================================
// Example 11: Disable redirects
//
// Reference: requests::get(url, {allow_redirects: false})
//==============================================================

capy::io_task<> example_no_redirects(burl::session& s)
{
    burl::request_options opts;
    opts.allow_redirects = false;
    
    auto [ec, r] = co_await s.get("https://httpbin.org/redirect/1", opts);
    
    if (ec.failed())
        co_return ec;
    
    // Should get 302 instead of following redirect
    std::cout << "Status: " << r.status_int() << "\n";
    if (r.is_redirect()) {
        std::cout << "Would redirect to: " 
                  << r.message.at(http::field::location) << "\n";
    }
    
    co_return {};
}

//==============================================================
// Example 12: Disable SSL verification (not recommended for production)
//
// Reference: requests::get(url, {verify: false})
//==============================================================

capy::io_task<> example_ssl_verify(burl::session& s)
{
    burl::request_options opts;
    opts.verify = false;  // Disable verification
    
    auto [ec, r] = co_await s.get("https://self-signed.example.com", opts);
    
    if (ec.failed())
        co_return ec;
    
    std::cout << "Response from self-signed: " << r.status_int() << "\n";
    
    co_return {};
}

//==============================================================
// Example 13: TLS configuration with custom CA bundle
//
// Reference: requests::get(url, {verify: "/path/to/ca-bundle.crt"})
//==============================================================

void example_custom_ca()
{
    corosio::io_context ioc;
    corosio::tls::context tls_ctx;
    
    // Configure TLS with custom CA file
    tls_ctx.load_verify_file("/path/to/custom/ca-bundle.crt");
    tls_ctx.set_verify_mode(corosio::tls::verify_mode::peer);
    
    burl::session s(ioc, tls_ctx);
    
    // Requests will verify against custom CA
    // ...
}

//==============================================================
// Example 14: Client certificate authentication
//
// Reference: requests::get(url, {cert: {...}})
//==============================================================

void example_client_cert()
{
    corosio::io_context ioc;
    corosio::tls::context tls_ctx;
    
    // Configure client certificate
    tls_ctx.use_certificate_file(
        "/path/to/client.crt", corosio::tls::file_format::pem);
    tls_ctx.use_private_key_file(
        "/path/to/client.key", corosio::tls::file_format::pem);
    tls_ctx.set_default_verify_paths();
    
    burl::session s(ioc, tls_ctx);
    
    // Requests will present client certificate
    // ...
}

//==============================================================
// Example 15: Error handling with raise_for_status()
//
// Reference: try { r.raise_for_status(); } catch (http_error& e) {...}
//==============================================================

capy::io_task<> example_error_handling(burl::session& s)
{
    auto [ec, r] = co_await s.get("https://httpbin.org/status/404");
    
    if (ec.failed()) {
        // Network-level errors
        std::cout << "Network error: " << ec.message() << "\n";
        co_return ec;
    }
    
    try {
        r.raise_for_status();
        std::cout << "Request succeeded\n";
    }
    catch (burl::http_error const& e) {
        // HTTP 4xx or 5xx error
        std::cout << "HTTP error: " << e.what() << "\n";
        std::cout << "Status: " << e.status_code() << "\n";
        std::cout << "URL: " << e.url() << "\n";
    }
    
    co_return {};
}

//==============================================================
// Example 16: Streaming response (large files)
//
// Reference: requests::get(url, {stream: true}); r.iter_content()
//==============================================================

capy::io_task<> example_streaming(burl::session& s)
{
    // Get streaming response for large files
    auto [ec, r] = co_await s.get_streamed(
        "https://httpbin.org/bytes/10000");
    
    if (ec.failed())
        co_return ec;
    
    std::cout << "Status: " << r.status_int() << "\n";
    
    // Read body incrementally without buffering entire response
    std::size_t total = 0;
    capy::const_buffer arr[16];
    
    while (true) {
        auto [err, count] = co_await r.body.pull(arr, 16);
        
        if (err.failed()) {
            std::cerr << "Read error: " << err.message() << "\n";
            break;
        }
        
        if (count == 0)
            break;  // End of body
        
        // Calculate bytes in this batch
        std::size_t batch = 0;
        for (std::size_t i = 0; i < count; ++i)
            batch += arr[i].size();
        
        total += batch;
        r.body.consume(batch);
    }
    
    std::cout << "Downloaded " << total << " bytes\n";
    
    co_return {};
}

//==============================================================
// Example 17: JSON response parsing
//
// Reference: auto data = r.json();
//==============================================================

capy::io_task<> example_json_response(burl::session& s)
{
    // Request with JSON parsing - body is json::value
    auto [ec, r] = co_await s.get(
        "https://api.github.com/users/octocat",
        burl::as_json);
    
    if (ec.failed())
        co_return ec;
    
    if (r.ok()) {
        // r.body is json::value - access fields directly
        std::cout << "Login: " << r.body.at("login").as_string() << "\n";
        std::cout << "ID: " << r.body.at("id").as_int64() << "\n";
    }
    
    co_return {};
}

//==============================================================
// Example 18: Custom type deserialization
//
// Reference: Deserialize JSON response into C++ struct
//==============================================================

struct GitHubUser
{
    std::string login;
    int id;
    std::string avatar_url;
    std::string type;
};

// Requires BOOST_DESCRIBE_STRUCT for reflection
// BOOST_DESCRIBE_STRUCT(GitHubUser, (), (login, id, avatar_url, type))

capy::io_task<> example_custom_type(burl::session& s)
{
    // Deserialize response directly into custom type
    auto [ec, r] = co_await s.get(
        "https://api.github.com/users/octocat",
        burl::as_type<GitHubUser>);
    
    if (ec.failed())
        co_return ec;
    
    if (r.ok()) {
        // r.body is GitHubUser
        std::cout << "User: " << r.body.login << "\n";
        std::cout << "ID: " << r.body.id << "\n";
    }
    
    co_return {};
}

//==============================================================
// Example 19: All HTTP methods
//
// Reference: get, post, put, patch, delete_, head, options
//==============================================================

capy::io_task<> example_all_methods(burl::session& s)
{
    urls::url_view url("https://httpbin.org/anything");
    
    auto [ec1, r1] = co_await s.get(url);
    auto [ec2, r2] = co_await s.post(url);
    auto [ec3, r3] = co_await s.put(url);
    auto [ec4, r4] = co_await s.patch(url);
    auto [ec5, r5] = co_await s.delete_(url);
    auto [ec6, r6] = co_await s.head(url);
    auto [ec7, r7] = co_await s.options(url);
    
    // Generic request method with any HTTP method
    auto [ec8, r8] = co_await s.request(http::method::get, url);
    
    (void)r1; (void)r2; (void)r3; (void)r4;
    (void)r5; (void)r6; (void)r7; (void)r8;
    
    std::cout << "All HTTP methods executed\n";
    
    co_return {};
}

//==============================================================
// Example 20: Session-level authentication
//
// Reference: session.set_auth(make_shared<http_basic_auth>(...))
//==============================================================

capy::io_task<> example_session_auth(burl::session& s)
{
    // Set authentication on the session - applies to all requests
    s.set_auth(std::make_shared<burl::http_basic_auth>("user", "pass"));
    
    auto [ec1, r1] = co_await s.get("https://api.example.com/endpoint1");
    auto [ec2, r2] = co_await s.get("https://api.example.com/endpoint2");
    
    (void)r1; (void)r2;
    
    co_return {};
}

//==============================================================
// Example 21: Bearer token authentication
//
// Reference: auth: make_shared<http_bearer_auth>("token")
//==============================================================

capy::io_task<> example_bearer_auth(burl::session& s)
{
    s.set_auth(std::make_shared<burl::http_bearer_auth>("my-api-token"));
    
    auto [ec, r] = co_await s.get("https://httpbin.org/bearer");
    
    if (ec.failed())
        co_return ec;
    
    std::cout << "Bearer auth result: " << r.status_int() << "\n";
    
    co_return {};
}

//==============================================================
// Example 22: Per-request authentication override
//
// Reference: Override session auth for specific request
//==============================================================

capy::io_task<> example_per_request_auth(burl::session& s)
{
    // Session has one auth...
    s.set_auth(std::make_shared<burl::http_bearer_auth>("default-token"));
    
    // But this request uses different auth
    burl::request_options opts;
    opts.auth = std::make_shared<burl::http_basic_auth>("user", "pass");
    
    auto [ec, r] = co_await s.get(
        "https://httpbin.org/basic-auth/user/pass", opts);
    
    if (ec.failed())
        co_return ec;
    
    std::cout << "Per-request auth result: " << r.status_int() << "\n";
    
    co_return {};
}

//==============================================================
// Example 23: Access response headers
//
// Reference: r.headers() - case insensitive header access
//==============================================================

capy::io_task<> example_response_headers(burl::session& s)
{
    auto [ec, r] = co_await s.get("https://httpbin.org/headers");
    
    if (ec.failed())
        co_return ec;
    
    // Access headers directly via http::response
    if (r.message.exists(http::field::content_type)) {
        std::cout << "Content-Type: " 
                  << r.message.at(http::field::content_type) << "\n";
    }
    
    // Iterate all headers
    std::cout << "All headers:\n";
    for (auto const& h : r.message) {
        std::cout << "  " << h.name << ": " << h.value << "\n";
    }
    
    co_return {};
}

//==============================================================
// Example 24: Access URL components
//
// Reference: r.url contains the final URL after redirects
//==============================================================

capy::io_task<> example_url_components(burl::session& s)
{
    auto [ec, r] = co_await s.get("https://httpbin.org/get?foo=bar&baz=123");
    
    if (ec.failed())
        co_return ec;
    
    // Access URL components via urls::url
    std::cout << "Final URL: " << r.url.buffer() << "\n";
    std::cout << "Scheme: " << r.url.scheme() << "\n";
    std::cout << "Host: " << r.url.host() << "\n";
    std::cout << "Path: " << r.url.path() << "\n";
    
    // Access query parameters
    std::cout << "Query params:\n";
    for (auto param : r.url.params()) {
        std::cout << "  " << param.key << " = " << param.value << "\n";
    }
    
    co_return {};
}

//==============================================================
// Example 25: Setting session timeout default
//
// Reference: session.set_timeout(...)
//==============================================================

void example_session_timeout()
{
    corosio::io_context ioc;
    corosio::tls::context tls_ctx;
    tls_ctx.set_default_verify_paths();
    
    burl::session s(ioc, tls_ctx);
    
    // Set default timeout for all requests
    s.set_timeout(std::chrono::milliseconds{10000});  // 10 seconds
    
    // All requests will use this timeout unless overridden
}

//==============================================================
// Example 26: Setting max redirects
//
// Reference: session.set_max_redirects(n)
//==============================================================

void example_max_redirects()
{
    corosio::io_context ioc;
    corosio::tls::context tls_ctx;
    tls_ctx.set_default_verify_paths();
    
    burl::session s(ioc, tls_ctx);
    
    // Limit redirects (default is typically 30)
    s.set_max_redirects(5);
    
    // Too many redirects will return error::too_many_redirects
}

//==============================================================
// Example 27: Multi-threaded usage
//
// Reference: Run io_context from multiple threads
//==============================================================

void example_multithreaded()
{
    corosio::io_context ioc;
    corosio::tls::context tls_ctx;
    tls_ctx.set_default_verify_paths();
    
    burl::session s(ioc, tls_ctx);
    
    // User runs io_context from multiple threads
    // Note: session itself is not thread-safe; use one session per thread
    // or synchronize access externally
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&] { ioc.run(); });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}

//==============================================================
// Example 28: Basic session usage pattern
//
// Reference: Complete setup and usage pattern
//==============================================================

void example_basic_session()
{
    corosio::io_context ioc;
    corosio::tls::context tls_ctx;
    
    // Configure TLS
    tls_ctx.set_default_verify_paths();
    
    burl::session s(ioc, tls_ctx);
    
    // Configure session defaults
    s.headers().set(http::field::user_agent, "MyApp/1.0");
    s.set_timeout(std::chrono::milliseconds{30000});
    
    // Launch work and run
    capy::run_async(ioc.get_executor())([&]() -> capy::io_task<> {
        auto [ec, r] = co_await s.get("https://example.com");
        if (ec.failed()) {
            std::cerr << "Error: " << ec.message() << "\n";
        } else {
            std::cout << "Status: " << r.status_int() << "\n";
        }
        co_return {};
    }());
    
    ioc.run();
}

//==============================================================
// Example 29: Response elapsed time
//
// Reference: r.elapsed() - time taken for request
//==============================================================

capy::io_task<> example_elapsed_time(burl::session& s)
{
    auto [ec, r] = co_await s.get("https://httpbin.org/get");
    
    if (ec.failed())
        co_return ec;
    
    std::cout << "Request took " << r.elapsed.count() << "ms\n";
    
    co_return {};
}

//==============================================================
// Example 30: Check response status
//
// Reference: r.ok(), r.status_int(), r.reason()
//==============================================================

capy::io_task<> example_status_check(burl::session& s)
{
    auto [ec, r] = co_await s.get("https://httpbin.org/status/201");
    
    if (ec.failed())
        co_return ec;
    
    // Various ways to check status
    if (r.ok()) {
        std::cout << "Success! ";
    }
    
    std::cout << "Status: " << r.status_int() 
              << " " << r.reason() << "\n";
    
    // Access enum form
    if (r.status() == http::status::created) {
        std::cout << "Resource created!\n";
    }
    
    co_return {};
}

//==============================================================
// Main - demonstrates session setup
//==============================================================

int main()
{
    std::cout << "Boost.Burl Usage Examples\n";
    std::cout << "=========================\n\n";
    
    // Basic setup pattern
    corosio::io_context ioc;
    corosio::tls::context tls_ctx;
    tls_ctx.set_default_verify_paths();
    
    burl::session s(ioc, tls_ctx);
    
    // Launch a task
    capy::run_async(ioc.get_executor())([&]() -> capy::io_task<> {
        // Run examples
        co_await example_simple_get(s);
        co_await example_status_check(s);
        
        std::cout << "\nAll examples completed.\n";
        co_return {};
    }());
    
    // Run the event loop
    ioc.run();
    
    // This file demonstrates API usage patterns.
    // The actual implementations are stubs that return not_implemented.
    
    std::cout << "\nAll examples compile successfully.\n";
    std::cout << "Implementation pending in Phase 2.\n";
    
    return 0;
}
