
#ifndef __ARGPARSE_CPP_H__
#define __ARGPARSE_CPP_H__

//  TODO:
//  1. support POSIXLY_CORRECT
//  2. c++17
//  3. argparse.cpp is interface library
//  4. accept long options recognize unambiguous abbreviations of those options.
//  5. environment bind
//  6. Contents that qualify option values, such as ranges, lists, etc.
//  7. help info & usage
//  8. pargram name

#include <assert.h>
#include <algorithm>
#include <charconv>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace argparse {
namespace {

// is_structurally_bindable_2
template <typename T>
struct is_structurally_bindable_2 : public std::false_type {};

template <typename T, typename U>
struct is_structurally_bindable_2<std::pair<T, U>> : public std::true_type {};

template <typename T, typename U>
struct is_structurally_bindable_2<std::tuple<T, U>> : public std::true_type {};

template <typename T>
struct is_structurally_bindable_2<std::array<T, 2>> : public std::true_type {};

template <typename T>
auto constexpr static is_structurally_bindable_2_v =
    is_structurally_bindable_2<T>::value;

template <typename T>
auto constexpr static is_bind_value_base_type_v =
    std::is_same_v<bool, T> || std::is_same_v<int, T> ||
    std::is_same_v<double, T> || std::is_same_v<std::string, T> ||
    is_structurally_bindable_2_v<T>;

template <typename T>
auto constexpr static is_flag_bind_value_type_v =
    std::is_same_v<int, T> || std::is_same_v<bool, T>;

template <typename T>
auto constexpr static is_option_bind_value_type_v =
    is_bind_value_base_type_v<T> ||
    std::is_same_v<std::vector<std::string>, T> ||
    std::is_same_v<std::map<std::string, std::string>, T>;

template <typename T>
auto constexpr static is_position_bind_value_type_v =
    std::is_same_v<std::string, T> ||
    std::is_same_v<std::vector<std::string>, T> ||
    std::is_same_v<std::map<std::string, std::string>, T>;

// overloaded
template <typename... T>
struct overloaded : public T... {
  using T::operator()...;
};

template <typename... T>
overloaded(T...) -> overloaded<T...>;

// is_option_bind_container
template <typename T, typename = void>
struct is_option_bind_container : std::false_type {};

template <typename T>
struct is_option_bind_container<std::vector<T>,
                                std::enable_if_t<is_bind_value_base_type_v<T>>>
    : std::true_type {};

template <typename T, typename U>
struct is_option_bind_container<std::map<T, U>,
                                std::enable_if_t<is_bind_value_base_type_v<T> &&
                                                 is_bind_value_base_type_v<U>>>
    : std::true_type {};

template <typename T>
constexpr bool is_option_bind_container_v = is_option_bind_container<T>::value;

//**********************************
class StringUtil {
 public:
  inline static bool startswith(std::string const& str,
                                std::string const& prefix) {
    if (str.length() < prefix.length()) {
      return false;
    }
    auto it = str.begin();
    auto pit = prefix.begin();
    for (; pit != prefix.end(); it++, pit++) {
      if (*it != *pit) {
        return false;
      }
    }
    return true;
  }
  inline static bool endswith(std::string const& str,
                              std::string const& suffix) {
    if (str.length() < suffix.length()) {
      return false;
    }
    auto it = str.rbegin();
    auto sit = suffix.rbegin();
    for (; sit != suffix.rend(); it++, sit++) {
      if (*it != *sit) {
        return false;
      }
    }
    return true;
  }
  inline static std::vector<std::string> split(std::string const& s,
                                               char delimiter,
                                               int number = 0) {
    std::vector<std::string> result;
    std::stringstream ss{s};
    std::string token;
    int splitCount = 0;
    while ((number <= 0 || splitCount++ < number) &&
           getline(ss, token, delimiter)) {
      result.push_back(token);
      if (number > 0 && splitCount == number) {
        getline(ss, token);
        result.push_back(token);
      }
    }
    return result;
  }

  inline static std::pair<std::string, std::string> split2(std::string const& s,
                                                           char delimiter) {
    auto it = find(s.begin(), s.end(), delimiter);
    return {std::string(s.begin(), it),
            std::string(it == s.end() ? s.end() : it, s.end())};
  }

  inline static std::string trim(std::string const& str) {
    std::string ret{str};
    ret.erase(ret.begin(), find_if(begin(ret), end(ret), [](unsigned char c) {
                return !std::isspace(c);
              }));
    ret.erase(find_if(rbegin(ret), rend(ret),
                      [](unsigned char c) { return !std::isspace(c); })
                  .base(),
              ret.end());
    return ret;
  }
  inline static bool is_short_opt(std::string const& opt) {
    return opt.size() >= 2 && opt[0] == '-' && opt[1] != '-';
  }
  inline static bool is_long_opt(std::string const& opt) {
    return opt.size() >= 3 && opt[0] == '-' && opt[1] == '-' && opt[2] != '-';
  }
  inline static bool is_dash_dash(std::string const& opt) {
    return opt == "--";
  }
  inline static bool is_position_arg(std::string const& str) {
    return !is_short_opt(str) && !is_long_opt(str) && !is_dash_dash(str);
  }
};

// bool
template <typename T, std::enable_if_t<std::is_same_v<bool, T>, int> = 0>
std::tuple<int, std::string, T> transform_value(std::string const& from) {
  std::string lower_from;
  std::transform(from.begin(), from.end(), std::back_inserter(lower_from),
                 [](unsigned char c) { return std::tolower(c); });
  if (lower_from == "true") {
    return {0, "", true};
  } else if (lower_from == "false") {
    return {0, "", false};
  } else {
    std::stringstream ss;
    ss << "'" << from << "' must be one of 'true' or 'false'";
    return {1, ss.str(), false};
  }
}

// int
template <typename T,
          std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<bool, T>,
                           int> = 0>
std::tuple<int, std::string, T> transform_value(std::string const& from) {
  T to{};
  auto [ptr, ec] = std::from_chars(from.data(), from.data() + from.size(), to);
  if (ec == std::errc::invalid_argument) {
    std::stringstream ss;
    ss << "'" << from << "' not invalid number";
    return {1, ss.str(), to};
  } else if (ec != std::errc{}) {
    std::stringstream ss;
    ss << "'" << from << "'  can't convert to a number";
    return {1, ss.str(), to};
  }
  return {0, "", to};
}

// double
template <typename T, std::enable_if_t<std::is_same_v<double, T>, int> = 0>
std::tuple<int, std::string, T> transform_value(std::string const& from) {
  std::size_t pos{};
  T to{std::stod(from, &pos)};
  if (pos == std::string::npos) {
    return {0, "", to};
  }
  {
    std::stringstream ss;
    ss << "'" << from << "' not invalid double value";
    return {1, ss.str(), to};
  }
}

// std::string
template <typename T, std::enable_if_t<std::is_same_v<std::string, T>, int> = 0>
std::tuple<int, std::string, T> transform_value(std::string const& from) {
  return {0, "", from};
}

// std::pair<K, V>, std::array<T, 2>, std::tuple<K,V>
template <typename T,
          std::enable_if_t<is_structurally_bindable_2_v<T>, int> = 0>
std::tuple<int, std::string, T> transform_value(std::string const& from) {
  auto it = from.find('=');
  if (it != std::string::npos) {
    auto key_str = from.substr(0, it);
    auto value_str = from.substr(it + 1);
    auto [err_k, msg_k, val_k] = transform_value<
        typename std::decay<decltype(std::get<0>(std::declval<T&>()))>::type>(
        key_str);
    auto [err_v, msg_v, val_v] = transform_value<
        typename std::decay<decltype(std::get<1>(std::declval<T&>()))>::type>(
        value_str);
    return {0, "", {val_k, val_v}};
  } else {
    std::stringstream ss;
    ss << "'" << from << "' is not a '=' separated string";
    return {1, ss.str(), {}};
  }
}

// container
template <typename T,
          std::enable_if_t<is_option_bind_container_v<T>, bool> = true>
std::pair<int, std::string> insert_or_replace_value(
    T& bind_value,
    std::string const& option_val) {
  std::insert_iterator<T> it(bind_value, bind_value.end());  // insert to end
  auto [ec, error_msg, result] =
      transform_value<typename T::value_type>(option_val);
  if (ec == 0) {
    *it = std::move(result);
  }
  return {ec, error_msg};
}

// non-container
template <typename T,
          std::enable_if_t<!is_option_bind_container_v<T>, bool> = false>
std::pair<int, std::string> insert_or_replace_value(
    T& bind_value,
    std::string const& option_val) {
  auto [ec, error_msg, result] = transform_value<T>(option_val);
  if (ec == 0) {
    bind_value = std::move(result);
  }
  return {ec, error_msg};
}
}  // namespace

class ArgParser;
class Flag {
  friend class ArgParser;

 public:
  template <typename T,
            typename = std::enable_if_t<is_flag_bind_value_type_v<T>>>
  Flag(std::string const& flag_desc, T& bind) : value(std::ref(bind)) {
    Flag_init(flag_desc);
  }
  template <typename T,
            typename = std::enable_if_t<is_flag_bind_value_type_v<T>>>
  Flag(std::string const& flag_desc) : value(T{}) {
    Flag_init(flag_desc);
  }

  template <typename T,
            typename = std::enable_if_t<is_flag_bind_value_type_v<T>>>
  T const& as() {
    if (std::holds_alternative<std::reference_wrapper<T>>(value)) {
      return std::get<std::reference_wrapper<T>>(value).get();
    }
    return std::get<T>(value);
  }

  Flag& help(std::string const& help) {
    help_msg = help;
    return *this;
  }

 private:
  bool is_my_flag(char flag) {
    return find(begin(short_names), end(short_names), flag) !=
               end(short_names) ||
           find(begin(negate_short_names), end(negate_short_names), flag) !=
               end(negate_short_names);
  }
  bool is_my_flag(std::string const& flag) {
    return find(begin(long_names), end(long_names), flag) != end(long_names) ||
           find(begin(negate_long_names), end(negate_long_names), flag) !=
               end(negate_long_names);
  }

  void hit(char flag) {
    count++;
    std::visit(
        overloaded{
            [flag, this](bool& x) {
              if (end(short_names) !=
                  find(begin(short_names), end(short_names), flag)) {
                x = true;
              }
              if (end(negate_short_names) != find(begin(negate_short_names),
                                                  end(negate_short_names),
                                                  flag)) {
                x = false;
              }
            },
            [flag,
             this]<typename T,
                   typename = std::enable_if_t<is_flag_bind_value_type_v<T>>>(
                T& x) {
              if (end(short_names) !=
                  find(begin(short_names), end(short_names), flag)) {
                x += 1;
              }
              if (end(negate_short_names) != find(begin(negate_short_names),
                                                  end(negate_short_names),
                                                  flag)) {
                x -= 1;
              }
            },
            [flag, this](std::reference_wrapper<bool>& x) {
              if (end(short_names) !=
                  find(begin(short_names), end(short_names), flag)) {
                x.get() = true;
              }
              if (end(negate_short_names) != find(begin(negate_short_names),
                                                  end(negate_short_names),
                                                  flag)) {
                x.get() = false;
              }
            },
            [flag,
             this]<typename T,
                   typename = std::enable_if_t<is_flag_bind_value_type_v<T>>>(
                std::reference_wrapper<T>& x) {
              if (end(short_names) !=
                  find(begin(short_names), end(short_names), flag)) {
                x.get() += 1;
              }
              if (end(negate_short_names) != find(begin(negate_short_names),
                                                  end(negate_short_names),
                                                  flag)) {
                x.get() -= 1;
              }
            }},
        value);
  }
  void hit(std::string const& flag) {
    count++;
    std::visit(
        overloaded{
            [&flag, this](bool& x) {
              if (end(long_names) !=
                  find(begin(long_names), end(long_names), flag)) {
                x = true;
              }
              if (end(negate_long_names) != find(begin(negate_long_names),
                                                 end(negate_long_names),
                                                 flag)) {
                x = false;
              }
            },
            [&flag,
             this]<typename T,
                   typename = std::enable_if_t<is_flag_bind_value_type_v<T>>>(
                T& x) {
              if (end(long_names) !=
                  find(begin(long_names), end(long_names), flag)) {
                x += 1;
              }
              if (end(negate_long_names) != find(begin(negate_long_names),
                                                 end(negate_long_names),
                                                 flag)) {
                x -= 1;
              }
            },
            [&flag, this](std::reference_wrapper<bool>& x) {
              if (end(long_names) !=
                  find(begin(long_names), end(long_names), flag)) {
                x.get() = true;
              }
              if (end(negate_long_names) != find(begin(negate_long_names),
                                                 end(negate_long_names),
                                                 flag)) {
                x.get() = false;
              }
            },
            [&flag,
             this]<typename T,
                   typename = std::enable_if_t<is_flag_bind_value_type_v<T>>>(
                std::reference_wrapper<T>& x) {
              if (end(long_names) !=
                  find(begin(long_names), end(long_names), flag)) {
                x.get() += 1;
              }
              if (end(negate_long_names) != find(begin(negate_long_names),
                                                 end(negate_long_names),
                                                 flag)) {
                x.get() -= 1;
              }
            }},
        value);
  }

  std::string usage() {
    std::stringstream ss;
    auto short_it = begin(short_names);
    if (short_it != end(short_names)) {
      ss << "  -" << *short_it;
      short_it++;
    }
    for (; short_it != end(short_names); ++short_it) {
      ss << ", -" << *short_it;
    }

    auto long_it = begin(long_names);
    if (long_it != end(long_names)) {
      if (short_names.empty()) {
        ss << "  --" << *long_it;
      } else {
        ss << ", --" << *long_it;
      }
      long_it++;
    }

    for (; long_it != end(long_names); long_it++) {
      ss << ", --" << *long_it;
    }
    ss << "\n";

    if (!help_msg.empty()) {
      ss << "         " << help_msg << "\n";
    }

    return ss.str();
  }

 private:
  void Flag_init(std::string const& flag_desc) {
    auto all = StringUtil::split(flag_desc, ',');
    assert(!all.empty());
    for (auto const& f : all) {
      assert(f.size() >= 2);
      if (f[0] == '!') {
        assert(f[1] == '-');
        if (f[1] == '-' && f[2] != '-') {
          negate_short_names.push_back(f[2]);
        } else if (f[1] == '-' && f[2] == '-' && f[3] != '-') {
          negate_long_names.push_back(f.substr(3));
        }
      } else {
        assert(f[0] == '-');
        if (f[0] == '-' && f[1] != '-') {
          short_names.push_back(f[1]);
        } else if (f[0] == '-' && f[1] == '-' && f[2] != '-') {
          long_names.push_back(f.substr(2));
        }
      }
    }
  }

 private:
  std::vector<char> short_names{};
  std::vector<std::string> long_names{};
  std::vector<char> negate_short_names{};
  std::vector<std::string> negate_long_names{};
  std::string help_msg{};
  int count{0};
  std::variant<std::reference_wrapper<bool>,
               std::reference_wrapper<int>,
               bool,
               int>
      value;
};

class Option {
  friend class ArgParser;

 public:
  template <typename T,
            typename = std::enable_if_t<is_option_bind_value_type_v<T>>>
  Option(std::string const& option_desc, T& bind) : value(std::ref(bind)) {
    Option_init(option_desc);
  }
  template <typename T,
            typename = std::enable_if_t<is_option_bind_value_type_v<T>>>
  Option(std::string const& option_desc) : value(T{}) {
    Option_init(option_desc);
  }

  template <typename T>
  T const& as() {
    if (std::holds_alternative<std::reference_wrapper<T>>(value)) {
      return std::get<std::reference_wrapper<T>>(value).get();
    }
    return std::get<T>(value);
  }

  Option& help(std::string const& help) {
    help_msg = help;
    return *this;
  }

 private:
  void Option_init(std::string const& option_desc) {
    auto all = StringUtil::split(option_desc, ',');
    assert(!all.empty());
    for (auto const& f : all) {
      assert(f.size() >= 2);
      if (f[0] == '-' && f[1] != '-') {
        short_names.push_back(f[1]);
      } else if (f[0] == '-' && f[1] == '-' && f[2] != '-') {
        long_names.push_back(f.substr(2));
      }
    }
  }
  bool is_my_option(char option) {
    return find(begin(short_names), end(short_names), option) !=
           end(short_names);
  }
  bool is_my_option(std::string const& option) {
    return find(begin(long_names), end(long_names), option) != end(long_names);
  }

  std::pair<int, std::string> hit(char short_opt, std::string const& opt_val) {
    count++;
    return std::visit(
        overloaded{
            [&opt_val]<typename T,
                       typename =
                           std::enable_if_t<is_option_bind_value_type_v<T>>>(
                T& x) -> std::pair<int, std::string> {
              return insert_or_replace_value(x, opt_val);
            },
            [&opt_val]<typename T>(
                std::reference_wrapper<T>& x) -> std::pair<int, std::string> {
              return insert_or_replace_value(x.get(), opt_val);
            }},
        value);
  }
  std::pair<int, std::string> hit(std::string const& long_opt,
                                  std::string const& opt_val) {
    count++;
    return std::visit(
        overloaded{
            [&opt_val]<typename T,
                       typename =
                           std::enable_if_t<is_option_bind_value_type_v<T>>>(
                T& x) -> std::pair<int, std::string> {
              return insert_or_replace_value(x, opt_val);
            },
            [&opt_val]<typename T>(
                std::reference_wrapper<T>& x) -> std::pair<int, std::string> {
              return insert_or_replace_value(x.get(), opt_val);
            }},
        value);
  }

  std::string usage() {
    std::stringstream ss;
    auto short_it = begin(short_names);
    if (short_it != end(short_names)) {
      ss << "  -" << *short_it;
      short_it++;
    }
    for (; short_it != end(short_names); ++short_it) {
      ss << ", -" << *short_it;
    }

    auto long_it = begin(long_names);
    if (long_it != end(long_names)) {
      if (short_names.empty()) {
        ss << "  --" << *long_it;
      } else {
        ss << ", --" << *long_it;
      }
      long_it++;
    }

    for (; long_it != end(long_names); long_it++) {
      ss << ", --" << *long_it;
    }
    if (!long_names.empty()) {
      if (val_help_msg.empty()) {
        ss << "=<TEXT>";
      } else {
        ss << "=<" << val_help_msg << ">";
      }
    }
    ss << "\n";

    if (!help_msg.empty()) {
      ss << "         " << help_msg << "\n";
    }

    return ss.str();
  }

 private:
  std::vector<char> short_names;
  std::vector<std::string> long_names;
  std::string help_msg;
  std::string val_help_msg;
  int count;
  std::variant<std::reference_wrapper<std::string>,
               std::reference_wrapper<std::vector<std::string>>,
               std::reference_wrapper<std::map<std::string, std::string>>,
               std::string,
               std::vector<std::string>,
               std::map<std::string, std::string>>
      value;
};

class ArgParser {
  class FlagNotFoundException : public std::exception {
    public:
      const char * what () {
          return "Flag not found Exception";
      }
  };
  class OptionNotFoundException : public std::exception {
    public:
      const char * what () {
          return "Option not found Exception";
      }
  };

 public:
  ArgParser() {}
  explicit ArgParser(std::string const& description)
      : description(description) {}

  ArgParser& set_program_name(std::string const& program_name) {
    this->program_name = program_name;
    return *this;
  }

  template <typename T,
            typename = std::enable_if_t<is_flag_bind_value_type_v<T>>>
  Flag& add_flag(std::string const& flag_desc, T& bind) {
    auto x = std::make_unique<Flag>(flag_desc, bind);
    auto p = x.get();
    assert(end(p->short_names) ==
           find_if(begin(p->short_names), end(p->short_names),
                   [this](char f) { return get_flag(f).has_value(); }));
    assert(end(p->long_names) == find_if(begin(p->long_names),
                                         end(p->long_names),
                                         [this](std::string const& f) {
                                           return get_flag(f).has_value();
                                         }));
    flags.push_back(std::move(x));
    return *p;
  }

  template <typename T,
            typename = std::enable_if_t<is_option_bind_value_type_v<T>>>
  Option& add_option(std::string const& option_desc, T& bind) {
    auto x = std::make_unique<Option>(option_desc, bind);
    auto p = x.get();
    assert(end(p->short_names) ==
           find_if(begin(p->short_names), end(p->short_names),
                   [this](char f) { return get_flag(f).has_value(); }));
    assert(end(p->long_names) == find_if(begin(p->long_names),
                                         end(p->long_names),
                                         [this](std::string const& f) {
                                           return get_flag(f).has_value();
                                         }));
    options.push_back(std::move(x));
    return *p;
  }

  template <typename T,
            typename = std::enable_if_t<is_position_bind_value_type_v<T>>>
  void add_position_arg(T& bind) {
    position_args.emplace_back(std::ref(bind));
  }
  template <typename T,
            typename = std::enable_if_t<is_position_bind_value_type_v<T>>>
  void add_position_arg() {
    position_args.emplace_back(T{});
  }

  std::string usage() {
    std::stringstream ss;
    ss << (program_name.empty() ? "?" : program_name) << " [OPTION]... ";
    if (not position_args.empty()) {
      ss << " [--] [args....]";
    }
    ss << "\n\n";
    ss << description << "\n\n";
    for (auto it = begin(flags); it != end(flags); ++it) {
      ss << (*it)->usage() << "\n";
    }
    for (auto it = begin(options); it != end(options); ++it) {
      ss << (*it)->usage() << "\n";
    }

    return ss.str();
  }

  std::pair<int /*exit code*/, std::string /*error message*/> parse(
      int argc,
      const char* argv[]);

  template <typename T,
            typename = std::enable_if_t<std::is_same_v<T, char> ||
                                        std::is_same_v<T, std::string>>>
  Flag& flag(T const& flag) {
    auto it = find_if(begin(flags), end(flags),
                      [flag](auto const& f) { return f->is_my_flag(flag); });
    if (it != end(flags)) {
      return *((*it).get());
    }
    throw FlagNotFoundException{};
  }

  template <typename T,
            typename = std::enable_if_t<std::is_same_v<T, char> ||
                                        std::is_same_v<T, std::string>>>
  Option& option(T const& opt) {
    auto it = find_if(begin(options), end(options),
                      [opt](auto const& f) { return f->is_my_option(opt); });
    if (end(options) != it) {
      return *((*it).get());
    }
    throw OptionNotFoundException{};
  }

 private:
  template <typename T,
            typename = std::enable_if_t<std::is_same_v<T, char> ||
                                        std::is_same_v<T, std::string>>>
  std::optional<Flag*> get_flag(T const& flag) {
    auto it = find_if(begin(flags), end(flags),
                      [flag](auto const& f) { return f->is_my_flag(flag); });
    if (it != end(flags)) {
      return (*it).get();
    }
    return {};
  }
  template <typename T,
            typename = std::enable_if_t<std::is_same_v<T, char> ||
                                        std::is_same_v<T, std::string>>>
  std::optional<Option*> get_option(T const& opt) {
    auto it = find_if(begin(options), end(options),
                      [opt](auto const& f) { return f->is_my_option(opt); });
    if (end(options) != it) {
      return (*it).get();
    }
    return {};
  }

  std::string description{};
  std::string program_name{};

  std::vector<std::unique_ptr<Flag>> flags{};
  std::vector<std::unique_ptr<Option>> options{};

  std::vector<
      std::variant<std::reference_wrapper<std::string>,
                   std::reference_wrapper<std::vector<std::string>>,
                   std::reference_wrapper<std::map<std::string, std::string>>,
                   std::string,
                   std::vector<std::string>,
                   std::map<std::string, std::string>>>
      position_args;
};

}  // namespace argparse

#endif  // __ARGPARSE_CPP_H__
