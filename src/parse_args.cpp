//
// Copyright (c) 2025 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/burl
//

#include <boost/burl/parse_args.hpp>

#include <cstring>
#include <string_view>

namespace boost {
namespace burl {

namespace {

// Error category for argument parsing errors
class args_error_category : public std::error_category
{
public:
    char const*
    name() const noexcept override
    {
        return "boost.burl.args";
    }

    std::string
    message(int ev) const override
    {
        switch(ev)
        {
        case 1: return "unknown option";
        case 2: return "missing option value";
        case 3: return "invalid option value";
        default: return "unknown error";
        }
    }
};

std::error_category const&
args_category() noexcept
{
    static args_error_category const cat{};
    return cat;
}

std::error_code
make_args_error(int ev)
{
    return std::error_code(ev, args_category());
}

parse_result
make_error(std::string msg)
{
    parse_result result;
    result.ec = make_args_error(1);
    result.error_message = std::move(msg);
    return result;
}

parse_result
make_missing_value_error(std::string_view opt)
{
    parse_result result;
    result.ec = make_args_error(2);
    result.error_message = "option requires a value: ";
    result.error_message += opt;
    return result;
}

// Get next argument value for options that require one
// Returns nullptr if no value available
char const*
get_next_value(int& i, int argc, char const* const* argv)
{
    if(i + 1 < argc)
    {
        ++i;
        return argv[i];
    }
    return nullptr;
}

// Split --name=value into (name, value)
// Returns (name, nullptr) if no = present
std::pair<std::string_view, char const*>
split_long_option(std::string_view arg)
{
    // arg starts with "--", skip it
    arg = arg.substr(2);
    auto pos = arg.find('=');
    if(pos == std::string_view::npos)
        return {arg, nullptr};
    return {arg.substr(0, pos), arg.data() + pos + 1};
}

// Handle a long option (--name or --name=value)
// Returns false if option is unknown
bool
handle_long_option(
    std::string_view name,
    char const* value,
    burl_args& args,
    int& i,
    int argc,
    char const* const* argv,
    parse_result& result)
{
    // Options that don't take values
    if(name == "verbose")
    {
        args.verbose = true;
        return true;
    }
    if(name == "silent")
    {
        args.silent = true;
        return true;
    }
    if(name == "show-error")
    {
        args.show_error = true;
        return true;
    }
    if(name == "location")
    {
        args.follow_redirects = true;
        return true;
    }
    if(name == "insecure")
    {
        args.insecure = true;
        return true;
    }
    if(name == "include")
    {
        args.include_headers = true;
        return true;
    }
    if(name == "head")
    {
        args.head_only = true;
        return true;
    }
    if(name == "remote-name")
    {
        args.remote_name = true;
        return true;
    }
    if(name == "compressed")
    {
        args.compressed = true;
        return true;
    }
    if(name == "help")
    {
        args.help = true;
        return true;
    }
    if(name == "version")
    {
        args.version = true;
        return true;
    }

    // Auth type options (no value)
    if(name == "basic")
    {
        args.auth = auth_type::basic;
        return true;
    }
    if(name == "digest")
    {
        args.auth = auth_type::digest;
        return true;
    }
    if(name == "ntlm")
    {
        args.auth = auth_type::ntlm;
        return true;
    }
    if(name == "negotiate")
    {
        args.auth = auth_type::negotiate;
        return true;
    }
    if(name == "anyauth")
    {
        args.auth = auth_type::any;
        return true;
    }

    // Options that require values
    // If value wasn't provided via =, get next arg
    auto require_value = [&]() -> char const*
    {
        if(value)
            return value;
        return get_next_value(i, argc, argv);
    };

    if(name == "request")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--request");
            return false;
        }
        args.method = v;
        return true;
    }
    if(name == "data")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--data");
            return false;
        }
        args.data.push_back(v);
        return true;
    }
    if(name == "data-binary")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--data-binary");
            return false;
        }
        args.data_binary.push_back(v);
        return true;
    }
    if(name == "data-raw")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--data-raw");
            return false;
        }
        args.data_raw.push_back(v);
        return true;
    }
    if(name == "data-urlencode")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--data-urlencode");
            return false;
        }
        args.data_urlencode.push_back(v);
        return true;
    }
    if(name == "form")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--form");
            return false;
        }
        args.forms.push_back(v);
        return true;
    }
    if(name == "json")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--json");
            return false;
        }
        args.json = v;
        return true;
    }
    if(name == "upload-file")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--upload-file");
            return false;
        }
        args.upload_file = v;
        return true;
    }
    if(name == "header")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--header");
            return false;
        }
        args.headers.push_back(v);
        return true;
    }
    if(name == "user-agent")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--user-agent");
            return false;
        }
        args.user_agent = v;
        return true;
    }
    if(name == "referer")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--referer");
            return false;
        }
        args.referer = v;
        return true;
    }
    if(name == "output")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--output");
            return false;
        }
        args.output = v;
        return true;
    }
    if(name == "dump-header")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--dump-header");
            return false;
        }
        args.dump_header = v;
        return true;
    }
    if(name == "write-out")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--write-out");
            return false;
        }
        args.write_out = v;
        return true;
    }
    if(name == "user")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--user");
            return false;
        }
        args.user = v;
        return true;
    }
    if(name == "cookie")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--cookie");
            return false;
        }
        args.cookie = v;
        return true;
    }
    if(name == "cookie-jar")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--cookie-jar");
            return false;
        }
        args.cookie_jar = v;
        return true;
    }
    if(name == "cacert")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--cacert");
            return false;
        }
        args.cacert = v;
        return true;
    }
    if(name == "cert")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--cert");
            return false;
        }
        args.cert = v;
        return true;
    }
    if(name == "key")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--key");
            return false;
        }
        args.key = v;
        return true;
    }
    if(name == "proxy")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--proxy");
            return false;
        }
        args.proxy = v;
        return true;
    }
    if(name == "max-redirs")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--max-redirs");
            return false;
        }
        args.max_redirs = std::atoi(v);
        return true;
    }
    if(name == "max-time")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--max-time");
            return false;
        }
        args.max_time = std::atof(v);
        return true;
    }
    if(name == "connect-timeout")
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("--connect-timeout");
            return false;
        }
        args.connect_timeout = std::atof(v);
        return true;
    }

    // Unknown option
    result = make_error("unknown option: --" + std::string(name));
    return false;
}

// Returns true if short option takes a value
bool
short_option_takes_value(char c)
{
    switch(c)
    {
    case 'd': // --data
    case 'F': // --form
    case 'H': // --header
    case 'A': // --user-agent
    case 'e': // --referer
    case 'o': // --output
    case 'D': // --dump-header
    case 'w': // --write-out
    case 'u': // --user
    case 'b': // --cookie
    case 'c': // --cookie-jar
    case 'x': // --proxy
    case 'X': // --request
    case 'm': // --max-time
    case 'T': // --upload-file
        return true;
    default:
        return false;
    }
}

// Handle a single short option
// Returns false on error
bool
handle_short_option(
    char c,
    char const* attached_value,
    burl_args& args,
    int& i,
    int argc,
    char const* const* argv,
    parse_result& result)
{
    // Options without values
    switch(c)
    {
    case 'v':
        args.verbose = true;
        return true;
    case 's':
        args.silent = true;
        return true;
    case 'S':
        args.show_error = true;
        return true;
    case 'L':
        args.follow_redirects = true;
        return true;
    case 'k':
        args.insecure = true;
        return true;
    case 'i':
        args.include_headers = true;
        return true;
    case 'I':
        args.head_only = true;
        return true;
    case 'O':
        args.remote_name = true;
        return true;
    case 'h':
        args.help = true;
        return true;
    case 'V':
        args.version = true;
        return true;
    }

    // Options with values
    auto require_value = [&]() -> char const*
    {
        if(attached_value && *attached_value)
            return attached_value;
        return get_next_value(i, argc, argv);
    };

    switch(c)
    {
    case 'X':
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("-X");
            return false;
        }
        args.method = v;
        return true;
    }
    case 'd':
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("-d");
            return false;
        }
        args.data.push_back(v);
        return true;
    }
    case 'F':
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("-F");
            return false;
        }
        args.forms.push_back(v);
        return true;
    }
    case 'H':
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("-H");
            return false;
        }
        args.headers.push_back(v);
        return true;
    }
    case 'A':
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("-A");
            return false;
        }
        args.user_agent = v;
        return true;
    }
    case 'e':
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("-e");
            return false;
        }
        args.referer = v;
        return true;
    }
    case 'o':
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("-o");
            return false;
        }
        args.output = v;
        return true;
    }
    case 'D':
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("-D");
            return false;
        }
        args.dump_header = v;
        return true;
    }
    case 'w':
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("-w");
            return false;
        }
        args.write_out = v;
        return true;
    }
    case 'u':
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("-u");
            return false;
        }
        args.user = v;
        return true;
    }
    case 'b':
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("-b");
            return false;
        }
        args.cookie = v;
        return true;
    }
    case 'c':
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("-c");
            return false;
        }
        args.cookie_jar = v;
        return true;
    }
    case 'x':
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("-x");
            return false;
        }
        args.proxy = v;
        return true;
    }
    case 'm':
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("-m");
            return false;
        }
        args.max_time = std::atof(v);
        return true;
    }
    case 'T':
    {
        auto v = require_value();
        if(!v)
        {
            result = make_missing_value_error("-T");
            return false;
        }
        args.upload_file = v;
        return true;
    }
    }

    // Unknown option
    result = make_error(std::string("unknown option: -") + c);
    return false;
}

// Handle short options string (may be combined like -sS)
bool
handle_short_options(
    std::string_view opts,
    burl_args& args,
    int& i,
    int argc,
    char const* const* argv,
    parse_result& result)
{
    for(std::size_t j = 0; j < opts.size(); ++j)
    {
        char c = opts[j];
        char const* attached = nullptr;

        // If this option takes a value and there are more chars,
        // the remaining chars are the value
        if(short_option_takes_value(c) && j + 1 < opts.size())
        {
            attached = opts.data() + j + 1;
            // Process this option with attached value, then done
            return handle_short_option(
                c, attached, args, i, argc, argv, result);
        }

        if(!handle_short_option(
            c, nullptr, args, i, argc, argv, result))
            return false;
    }
    return true;
}

} // namespace

BOOST_BURL_DECL
parse_result
parse_args(int argc, char const* const* argv)
{
    parse_result result;
    auto& args = result.args;

    for(int i = 1; i < argc; ++i)
    {
        std::string_view arg = argv[i];

        if(arg == "--")
        {
            // Everything after -- is a URL
            for(++i; i < argc; ++i)
                args.urls.push_back(argv[i]);
            break;
        }

        if(arg.size() > 2 && arg[0] == '-' && arg[1] == '-')
        {
            // Long option
            auto [name, value] = split_long_option(arg);
            if(!handle_long_option(
                name, value, args, i, argc, argv, result))
                return result;
        }
        else if(arg.size() > 1 && arg[0] == '-')
        {
            // Short option(s)
            if(!handle_short_options(
                arg.substr(1), args, i, argc, argv, result))
                return result;
        }
        else
        {
            // Positional argument (URL)
            args.urls.push_back(std::string(arg));
        }
    }

    return result;
}

} // namespace burl
} // namespace boost
