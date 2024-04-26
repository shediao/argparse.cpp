#include "argparse.h"

#include <assert.h>
#include <iterator>
#include <format>

namespace argparse {
namespace {
    bool is_short_opt(std::string const& opt) {
        return opt.size() >= 2 && opt[0] == '-' && opt[1] != '-';
    }
    bool is_long_opt(std::string const& opt) {
        return opt.size() >= 3 && opt[0] == '-' && opt[1] == '-' && opt[2] != '-';
    }
    bool is_dash_dash(std::string const& opt) {
        return opt == "--";
    }
    bool is_position_arg(std::string const& str) {
        return !is_short_opt(str) && ! is_long_opt(str) && !is_dash_dash(str);
    }
}
std::pair<ArgParser::ParseErrorCode, std::string> ArgParser::parse(int argc, const char* argv[]) {

    using ArgParser::ParseErrorCode::PARSE_FAILURE;
    using ArgParser::ParseErrorCode::PARSE_SUCCESS;

    std::vector<std::string> command_line_args{argv, argv + argc};

    if (command_line_args.size() == 0) { return {ParseErrorCode::PARSE_SUCCESS, ""}; }

    auto current = command_line_args.begin();

    if (auto& first = *command_line_args.begin(); !first.empty() && first[0] != '-') {
        // 0 ==> skip program file path
        current = std::next(command_line_args.begin());
    }

    auto current_position_it = position_args.begin();

    int index = 0;
    while(current != command_line_args.end()) {
        // position args
        // 1. -
        // 2. ---
        // 3. ""
        // 4. xxxx

        auto next = std::next(current);

        auto const& curr_arg = *current;

        if (is_position_arg(curr_arg)) {
            if (current_position_it == position_args.end()) {
                return {PARSE_FAILURE, std::format("invalid option -- -{}", curr_arg)};
            }
            if ((*current_position_it).index() == 2) {
                auto const i = curr_arg.find('=');
                if (i == std::string::npos) {
                    current_position_it++;
                }
            }

            bool revisit = false;
            std::visit(overloaded{
                    [&current_position_it, &curr_arg](std::reference_wrapper<std::string> &x) -> std::pair<int, std::string> {
                        x.get() = curr_arg;
                        current_position_it++;
                        return {0, ""};
                    },
                    [&current_position_it, &curr_arg](std::reference_wrapper<std::vector<std::string>> &x) -> std::pair<int, std::string> {
                        x.get().push_back(curr_arg);
                        return {0, ""};
                    },
                    [&revisit, &current_position_it, &curr_arg](std::reference_wrapper<std::map<std::string, std::string>> &x) -> std::pair<int, std::string> {
                        auto const i = curr_arg.find('=');
                        if (i == std::string::npos) {
                            current_position_it++;
                            revisit = true;
                        } else {
                            x.get().emplace(curr_arg.substr(0, i), curr_arg.substr(i+1));
                        }
                        return {0, ""};
                    },
                },(*current_position_it));
            if (revisit) {
                continue;
            }
            current = next;
        } else if (is_dash_dash(curr_arg)) {
            // --
            current = next;
            break;
        } else if (is_short_opt(curr_arg)) {
            // short
            auto short_p = curr_arg.begin() + 1;
            while (short_p != curr_arg.end()) {
                if (auto short_flag = get_flag(*short_p); short_flag.has_value()) {
                    (*short_flag)->hit(*short_p);
                    short_p++;
                } else if (auto short_option = get_option(*short_p); short_option.has_value()) {
                    if (short_p + 1  != curr_arg.end()) {
                        (*short_option)->hit(*short_p, std::string(short_p+1, curr_arg.end()));
                        short_p = curr_arg.end();
                    } else {
                        if (next == command_line_args.end()) {
                            return {PARSE_FAILURE, std::format("option requires an argument -- -{}", *short_p)};
                        } else {
                            if ((!next->empty() && (*next)[0] == '-')) {
                                return {PARSE_FAILURE, std::format("option requires an argument -- -{}", *short_p)};
                            } else {
                                (*short_option)->hit(*short_p, *next);
                                short_p = curr_arg.end();
                                next = std::next(next);
                            }
                        }
                    }
                } else {
                    return {PARSE_FAILURE, std::format("invalid option -- -{}", *short_p)};
                }
            }
            current = next;
        } else if (is_long_opt(curr_arg)) {
            // long
            if (auto i = curr_arg.find('=', 2); i != std::string::npos) {
                std::string option = curr_arg.substr(2,i);
                if (auto long_opt = get_option(option); long_opt.has_value()) {
                    (*long_opt)->hit(option, curr_arg.substr(i+1));
                } else {
                    return {PARSE_FAILURE, std::format("invalid option -- --{}", option)};
                }
            } else {
                std::string option = curr_arg.substr(2,i);
                if (auto long_flag = get_flag(option); long_flag.has_value()) {
                    (*long_flag)->hit(option);
                } else if (auto long_opt = get_option(option); long_opt.has_value()) {
                    if (std::next(current) == command_line_args.end()) {
                        return {PARSE_FAILURE, std::format("option requires an argument -- --{}", option)};
                    } else if (!next->empty() && (*next)[0] == '-') {
                        return {PARSE_FAILURE, std::format("option requires an argument -- --{}", option)};
                    } else {
                        (*long_opt)->hit(option, *next);
                        next = std::next(next);
                    }
                } else {
                    return {PARSE_FAILURE, std::format("invalid option -- --{}", option)};
                }
            }
            current = next;
        } else {
            assert(false);
            // CHECK(false)
        }
    }

    for (;current != command_line_args.end(); current++) {
       auto [code, err_msg] = std::visit(overloaded{
               [&current]<typename T>(std::reference_wrapper<T> &x) -> std::pair<int, std::string> {
                   return insert_or_replace_value(x.get(), *current);
               },
           },(*current_position_it));
       if (code != 0) {
           return {PARSE_FAILURE, err_msg};
       }
    }
    return {PARSE_SUCCESS, ""};
}
}  // namespace

