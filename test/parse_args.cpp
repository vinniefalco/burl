//
// Copyright (c) 2025 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/burl
//

#include <boost/burl/parse_args.hpp>

#include <cassert>
#include <cstring>
#include <vector>

namespace boost {
namespace burl {

namespace {

// Helper to create argc/argv from string list
class args_builder
{
    std::vector<std::string> strings_;
    std::vector<char const*> argv_;

public:
    args_builder(std::initializer_list<char const*> args)
    {
        strings_.reserve(args.size());
        argv_.reserve(args.size() + 1);
        for(auto s : args)
        {
            strings_.push_back(s);
            argv_.push_back(strings_.back().c_str());
        }
        argv_.push_back(nullptr);
    }

    int argc() const noexcept
    {
        return static_cast<int>(strings_.size());
    }

    char const* const* argv() const noexcept
    {
        return argv_.data();
    }
};

//----------------------------------------------------------
// URL parsing tests
//----------------------------------------------------------

void test_single_url()
{
    args_builder args{"burl", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.urls.size() == 1);
    assert(result.args.urls[0] == "https://example.com");
}

void test_multiple_urls()
{
    args_builder args{"burl", "https://a.com", "https://b.com", "https://c.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.urls.size() == 3);
    assert(result.args.urls[0] == "https://a.com");
    assert(result.args.urls[1] == "https://b.com");
    assert(result.args.urls[2] == "https://c.com");
}

void test_urls_after_double_dash()
{
    args_builder args{"burl", "--", "-not-an-option", "--also-not"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.urls.size() == 2);
    assert(result.args.urls[0] == "-not-an-option");
    assert(result.args.urls[1] == "--also-not");
}

//----------------------------------------------------------
// Short option tests
//----------------------------------------------------------

void test_short_verbose()
{
    args_builder args{"burl", "-v", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.verbose);
}

void test_short_silent()
{
    args_builder args{"burl", "-s", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.silent);
}

void test_short_combined()
{
    args_builder args{"burl", "-sS", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.silent);
    assert(result.args.show_error);
}

void test_short_combined_vsL()
{
    args_builder args{"burl", "-vsL", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.verbose);
    assert(result.args.silent);
    assert(result.args.follow_redirects);
}

void test_short_location()
{
    args_builder args{"burl", "-L", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.follow_redirects);
}

void test_short_insecure()
{
    args_builder args{"burl", "-k", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.insecure);
}

void test_short_include()
{
    args_builder args{"burl", "-i", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.include_headers);
}

void test_short_head()
{
    args_builder args{"burl", "-I", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.head_only);
}

//----------------------------------------------------------
// Short options with values
//----------------------------------------------------------

void test_short_data_separate()
{
    args_builder args{"burl", "-d", "key=value", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.data.size() == 1);
    assert(result.args.data[0] == "key=value");
}

void test_short_data_attached()
{
    args_builder args{"burl", "-dkey=value", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.data.size() == 1);
    assert(result.args.data[0] == "key=value");
}

void test_short_data_multiple()
{
    args_builder args{"burl", "-d", "a=1", "-d", "b=2", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.data.size() == 2);
    assert(result.args.data[0] == "a=1");
    assert(result.args.data[1] == "b=2");
}

void test_short_header()
{
    args_builder args{"burl", "-H", "Content-Type: application/json", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.headers.size() == 1);
    assert(result.args.headers[0] == "Content-Type: application/json");
}

void test_short_header_multiple()
{
    args_builder args{"burl", "-H", "Accept: */*", "-H", "X-Custom: foo", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.headers.size() == 2);
    assert(result.args.headers[0] == "Accept: */*");
    assert(result.args.headers[1] == "X-Custom: foo");
}

void test_short_output()
{
    args_builder args{"burl", "-o", "output.txt", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.output.has_value());
    assert(result.args.output.value() == "output.txt");
}

void test_short_output_attached()
{
    args_builder args{"burl", "-ooutput.txt", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.output.has_value());
    assert(result.args.output.value() == "output.txt");
}

void test_short_user()
{
    args_builder args{"burl", "-u", "admin:secret", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.user.has_value());
    assert(result.args.user.value() == "admin:secret");
}

void test_short_method()
{
    args_builder args{"burl", "-X", "POST", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.method == "POST");
}

void test_short_user_agent()
{
    args_builder args{"burl", "-A", "MyAgent/1.0", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.user_agent.has_value());
    assert(result.args.user_agent.value() == "MyAgent/1.0");
}

void test_short_referer()
{
    args_builder args{"burl", "-e", "https://google.com", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.referer.has_value());
    assert(result.args.referer.value() == "https://google.com");
}

void test_short_cookie()
{
    args_builder args{"burl", "-b", "session=abc123", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.cookie.has_value());
    assert(result.args.cookie.value() == "session=abc123");
}

void test_short_cookie_jar()
{
    args_builder args{"burl", "-c", "cookies.txt", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.cookie_jar.has_value());
    assert(result.args.cookie_jar.value() == "cookies.txt");
}

//----------------------------------------------------------
// Long option tests
//----------------------------------------------------------

void test_long_verbose()
{
    args_builder args{"burl", "--verbose", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.verbose);
}

void test_long_silent()
{
    args_builder args{"burl", "--silent", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.silent);
}

void test_long_location()
{
    args_builder args{"burl", "--location", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.follow_redirects);
}

void test_long_insecure()
{
    args_builder args{"burl", "--insecure", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.insecure);
}

void test_long_include()
{
    args_builder args{"burl", "--include", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.include_headers);
}

void test_long_head()
{
    args_builder args{"burl", "--head", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.head_only);
}

void test_long_compressed()
{
    args_builder args{"burl", "--compressed", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.compressed);
}

//----------------------------------------------------------
// Long options with values
//----------------------------------------------------------

void test_long_data_separate()
{
    args_builder args{"burl", "--data", "key=value", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.data.size() == 1);
    assert(result.args.data[0] == "key=value");
}

void test_long_data_equals()
{
    args_builder args{"burl", "--data=key=value", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.data.size() == 1);
    assert(result.args.data[0] == "key=value");
}

void test_long_header()
{
    args_builder args{"burl", "--header", "X-Custom: value", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.headers.size() == 1);
    assert(result.args.headers[0] == "X-Custom: value");
}

void test_long_header_equals()
{
    args_builder args{"burl", "--header=X-Custom: value", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.headers.size() == 1);
    assert(result.args.headers[0] == "X-Custom: value");
}

void test_long_output()
{
    args_builder args{"burl", "--output", "file.txt", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.output.value() == "file.txt");
}

void test_long_output_equals()
{
    args_builder args{"burl", "--output=file.txt", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.output.value() == "file.txt");
}

void test_long_request()
{
    args_builder args{"burl", "--request", "DELETE", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.method == "DELETE");
}

void test_long_user()
{
    args_builder args{"burl", "--user", "name:pass", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.user.value() == "name:pass");
}

void test_long_user_agent()
{
    args_builder args{"burl", "--user-agent", "Bot/2.0", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.user_agent.value() == "Bot/2.0");
}

void test_long_referer()
{
    args_builder args{"burl", "--referer", "https://ref.com", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.referer.value() == "https://ref.com");
}

void test_long_cookie()
{
    args_builder args{"burl", "--cookie", "name=val", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.cookie.value() == "name=val");
}

void test_long_cookie_jar()
{
    args_builder args{"burl", "--cookie-jar", "jar.txt", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.cookie_jar.value() == "jar.txt");
}

void test_long_max_time()
{
    args_builder args{"burl", "--max-time", "30.5", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.max_time.has_value());
    assert(result.args.max_time.value() == 30.5);
}

void test_long_connect_timeout()
{
    args_builder args{"burl", "--connect-timeout", "10", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.connect_timeout.has_value());
    assert(result.args.connect_timeout.value() == 10.0);
}

void test_long_max_redirs()
{
    args_builder args{"burl", "--max-redirs", "5", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.max_redirs == 5);
}

//----------------------------------------------------------
// Auth type tests
//----------------------------------------------------------

void test_auth_basic()
{
    args_builder args{"burl", "--basic", "-u", "user:pass", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.auth == auth_type::basic);
}

void test_auth_digest()
{
    args_builder args{"burl", "--digest", "-u", "user:pass", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.auth == auth_type::digest);
}

void test_auth_ntlm()
{
    args_builder args{"burl", "--ntlm", "-u", "user:pass", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.auth == auth_type::ntlm);
}

void test_auth_negotiate()
{
    args_builder args{"burl", "--negotiate", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.auth == auth_type::negotiate);
}

void test_auth_any()
{
    args_builder args{"burl", "--anyauth", "-u", "user:pass", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.auth == auth_type::any);
}

//----------------------------------------------------------
// TLS options tests
//----------------------------------------------------------

void test_cacert()
{
    args_builder args{"burl", "--cacert", "/path/to/ca.crt", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.cacert.value() == "/path/to/ca.crt");
}

void test_cert()
{
    args_builder args{"burl", "--cert", "/path/to/client.crt", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.cert.value() == "/path/to/client.crt");
}

void test_key()
{
    args_builder args{"burl", "--key", "/path/to/client.key", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.key.value() == "/path/to/client.key");
}

//----------------------------------------------------------
// Proxy tests
//----------------------------------------------------------

void test_short_proxy()
{
    args_builder args{"burl", "-x", "http://proxy:8080", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.proxy.value() == "http://proxy:8080");
}

void test_long_proxy()
{
    args_builder args{"burl", "--proxy", "socks5://localhost:1080", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.proxy.value() == "socks5://localhost:1080");
}

//----------------------------------------------------------
// Data variant tests
//----------------------------------------------------------

void test_data_binary()
{
    args_builder args{"burl", "--data-binary", "@file.bin", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.data_binary.size() == 1);
    assert(result.args.data_binary[0] == "@file.bin");
}

void test_data_raw()
{
    args_builder args{"burl", "--data-raw", "@literally", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.data_raw.size() == 1);
    assert(result.args.data_raw[0] == "@literally");
}

void test_data_urlencode()
{
    args_builder args{"burl", "--data-urlencode", "msg=hello world", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.data_urlencode.size() == 1);
    assert(result.args.data_urlencode[0] == "msg=hello world");
}

void test_form()
{
    args_builder args{"burl", "-F", "file=@upload.txt", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.forms.size() == 1);
    assert(result.args.forms[0] == "file=@upload.txt");
}

void test_json()
{
    args_builder args{"burl", "--json", R"({"key":"value"})", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.json.has_value());
    assert(result.args.json.value() == R"({"key":"value"})");
}

void test_upload_file()
{
    args_builder args{"burl", "-T", "file.txt", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.upload_file.value() == "file.txt");
}

//----------------------------------------------------------
// Error tests
//----------------------------------------------------------

void test_unknown_short_option()
{
    args_builder args{"burl", "-Z", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(result.ec.failed());
    assert(result.error_message.find("-Z") != std::string::npos);
}

void test_unknown_long_option()
{
    args_builder args{"burl", "--unknown-option", "https://example.com"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(result.ec.failed());
    assert(result.error_message.find("unknown-option") != std::string::npos);
}

void test_missing_value_short()
{
    args_builder args{"burl", "-d"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(result.ec.failed());
    assert(result.error_message.find("-d") != std::string::npos);
}

void test_missing_value_long()
{
    args_builder args{"burl", "--data"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(result.ec.failed());
    assert(result.error_message.find("--data") != std::string::npos);
}

//----------------------------------------------------------
// Complex combination tests
//----------------------------------------------------------

void test_typical_get()
{
    args_builder args{"burl", "-sL", "-H", "Accept: application/json",
        "-o", "out.json", "https://api.example.com/data"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.silent);
    assert(result.args.follow_redirects);
    assert(result.args.headers.size() == 1);
    assert(result.args.output.value() == "out.json");
    assert(result.args.urls.size() == 1);
}

void test_typical_post()
{
    args_builder args{"burl", "-X", "POST",
        "-H", "Content-Type: application/json",
        "-d", R"({"name":"test"})",
        "-u", "admin:secret",
        "https://api.example.com/create"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.method == "POST");
    assert(result.args.headers.size() == 1);
    assert(result.args.data.size() == 1);
    assert(result.args.user.value() == "admin:secret");
}

void test_help_flag()
{
    args_builder args{"burl", "--help"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.help);
}

void test_version_flag()
{
    args_builder args{"burl", "--version"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.version);
}

void test_no_arguments()
{
    args_builder args{"burl"};
    auto result = parse_args(args.argc(), args.argv());
    
    assert(!result.ec.failed());
    assert(result.args.urls.empty());
}

} // namespace

} // namespace burl
} // namespace boost

int main()
{
    using namespace boost::burl;

    // URL tests
    test_single_url();
    test_multiple_urls();
    test_urls_after_double_dash();

    // Short option tests (no value)
    test_short_verbose();
    test_short_silent();
    test_short_combined();
    test_short_combined_vsL();
    test_short_location();
    test_short_insecure();
    test_short_include();
    test_short_head();

    // Short option tests (with value)
    test_short_data_separate();
    test_short_data_attached();
    test_short_data_multiple();
    test_short_header();
    test_short_header_multiple();
    test_short_output();
    test_short_output_attached();
    test_short_user();
    test_short_method();
    test_short_user_agent();
    test_short_referer();
    test_short_cookie();
    test_short_cookie_jar();

    // Long option tests (no value)
    test_long_verbose();
    test_long_silent();
    test_long_location();
    test_long_insecure();
    test_long_include();
    test_long_head();
    test_long_compressed();

    // Long option tests (with value)
    test_long_data_separate();
    test_long_data_equals();
    test_long_header();
    test_long_header_equals();
    test_long_output();
    test_long_output_equals();
    test_long_request();
    test_long_user();
    test_long_user_agent();
    test_long_referer();
    test_long_cookie();
    test_long_cookie_jar();
    test_long_max_time();
    test_long_connect_timeout();
    test_long_max_redirs();

    // Auth type tests
    test_auth_basic();
    test_auth_digest();
    test_auth_ntlm();
    test_auth_negotiate();
    test_auth_any();

    // TLS tests
    test_cacert();
    test_cert();
    test_key();

    // Proxy tests
    test_short_proxy();
    test_long_proxy();

    // Data variant tests
    test_data_binary();
    test_data_raw();
    test_data_urlencode();
    test_form();
    test_json();
    test_upload_file();

    // Error tests
    test_unknown_short_option();
    test_unknown_long_option();
    test_missing_value_short();
    test_missing_value_long();

    // Complex tests
    test_typical_get();
    test_typical_post();
    test_help_flag();
    test_version_flag();
    test_no_arguments();

    return 0;
}
