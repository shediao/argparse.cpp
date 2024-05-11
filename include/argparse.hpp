#ifndef ARGPARSE_CPP_H_
#define ARGPARSE_CPP_H_

//  TODO:
//     1. support POSIXLY_CORRECT
// [√] 2. c++17
// [√] 3. argparse.cpp is interface library
//     4. accept long options recognize unambiguous abbreviations of those
//     options.
//     5. environment bind
//     6. Contents that qualify option values, such as ranges, lists, etc.
// [√] 7. help info & usage
// [√] 8. pargram name
// [√] 9. alias
//     10. setopt(short, long) 支持getopt方式
//     11. all api test
//     12. top100 linux command test
//     13. subcommand support
// [√] 14. throw execption
//     15. CHECK & DCHECK
//     16. format help info

#include <algorithm>
#include <cassert>
#include <cctype>
#include <charconv>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace argparse {

inline void UNREACHABLE() {
  assert(false);
}
class bad_value_access : public std::exception {
 public:
  explicit bad_value_access(std::string msg) : msg(std::move(msg)) {}
  const char* what() const noexcept override { return msg.c_str(); }

 private:
  std::string msg;
};

class invalid_argument : public std::invalid_argument {
 public:
  explicit invalid_argument(const char* msg) : std::invalid_argument(msg) {}
  explicit invalid_argument(std::string const& msg)
      : std::invalid_argument(msg) {}
};

class flag_not_found : public std::exception {
 public:
  const char* what() const noexcept override {
    return "Flag not found Exception";
  }
};
class option_not_found : public std::exception {
 public:
  const char* what() const noexcept override {
    return "Option not found Exception";
  }
};
class positional_not_found : public std::exception {
 public:
  const char* what() const noexcept override {
    return "Positional not found Exception";
  }
};

template <typename T>
struct is_reference_wrapper : std::false_type {};

template <typename T>
struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type {};

template <typename T>
inline constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

template <typename T>
struct is_vector : std::false_type {};
template <typename T>
struct is_vector<std::vector<T>> : std::true_type {};
template <typename T>
inline constexpr bool is_vector_v = is_vector<T>::value;

template <typename T>
struct is_pair : std::false_type {};
template <typename T, typename U>
struct is_pair<std::pair<T, U>> : std::true_type {};
template <typename T>
inline constexpr bool is_pair_v = is_pair<T>::value;

template <typename T>
struct is_map : std::false_type {};
template <typename T, typename U>
struct is_map<std::map<T, U>> : std::true_type {};
template <typename T>
inline constexpr bool is_map_v = is_map<T>::value;

template <typename T>
struct is_need_split : std::false_type {};
template <typename T, typename U>
struct is_need_split<std::pair<T, U>> : std::true_type {};
template <typename T, typename U>
struct is_need_split<std::vector<std::pair<T, U>>> : std::true_type {};
template <typename T, typename U>
struct is_need_split<std::map<T, U>> : std::true_type {};
template <typename T>
struct is_need_split<std::vector<std::vector<T>>> : std::true_type {};
template <typename T>
inline constexpr bool is_need_split_v = is_need_split<T>::value;

template <typename T>
struct default_delimiter : std::integral_constant<char, '='> {};
template <typename T>
struct default_delimiter<std::vector<std::vector<T>>>
    : std::integral_constant<char, ','> {};
template <typename T>
inline constexpr char default_delimiter_v = default_delimiter<T>::value;

template <typename T, typename = void>
struct is_transformable_type : std::false_type {};

template <typename T>
struct is_transformable_type<
    T,
    std::enable_if_t<std::is_same_v<bool, T> || std::is_same_v<int, T> ||
                     std::is_same_v<double, T> ||
                     std::is_same_v<std::string, T>>> : std::true_type {};

template <typename T, typename U>
struct is_transformable_type<
    std::pair<T, U>,
    std::enable_if_t<(
        std::is_same_v<bool, T> || std::is_same_v<int, T> ||
        std::is_same_v<double, T> ||
        std::is_same_v<std::string, T>)&&(std::is_same_v<bool, U> ||
                                          std::is_same_v<int, U> ||
                                          std::is_same_v<double, U> ||
                                          std::is_same_v<std::string, U>)>>
    : std::true_type {};

template <typename T>
inline constexpr bool is_transformable_type_v = is_transformable_type<T>::value;

template <typename T, typename = void>
struct is_flag_bindable_value : std::false_type {};

template <typename T>
struct is_flag_bindable_value<
    T,
    std::enable_if_t<std::is_same_v<bool, T> || std::is_same_v<int, T>>>
    : std::true_type {};

template <typename T>
inline constexpr bool is_flag_bindable_value_v =
    is_flag_bindable_value<T>::value;

template <typename T, typename = void>
struct is_option_bindable_value : std::false_type {};

template <typename T>
struct is_option_bindable_value<T, std::enable_if_t<is_transformable_type_v<T>>>
    : std::true_type {};

template <typename T>
struct is_option_bindable_value<std::vector<T>,
                                std::enable_if_t<is_transformable_type_v<T>>>
    : std::true_type {};

template <typename T, typename U>
struct is_option_bindable_value<
    std::map<T, U>,
    std::enable_if_t<is_transformable_type_v<T> && is_transformable_type_v<U>>>
    : std::true_type {};

template <typename T>
inline constexpr bool is_option_bindable_value_v =
    is_option_bindable_value<T>::value;

template <typename T, typename = void>
struct is_position_bindable_value : std::false_type {};

template <typename T>
struct is_position_bindable_value<T,
                                  std::enable_if_t<is_transformable_type_v<T>>>
    : std::true_type {};

template <typename T>
struct is_position_bindable_value<std::vector<T>,
                                  std::enable_if_t<is_transformable_type_v<T>>>
    : std::true_type {};

template <typename T>
inline constexpr bool is_position_bindable_value_v =
    is_position_bindable_value<T>::value;

template <typename T>
inline constexpr bool is_bindable_value_v =
    is_flag_bindable_value_v<T> || is_option_bindable_value_v<T> ||
    is_position_bindable_value_v<T>;

// overloaded
template <typename... T>
struct overloaded : public T... {
  using T::operator()...;
};

template <typename... T>
overloaded(T...) -> overloaded<T...>;

// is_option_bindable_container
template <typename T, typename = void>
struct is_option_bindable_container : std::false_type {};

template <typename T>
struct is_option_bindable_container<
    std::vector<T>,
    std::enable_if_t<is_transformable_type_v<T>>> : std::true_type {};

template <typename T, typename U>
struct is_option_bindable_container<
    std::map<T, U>,
    std::enable_if_t<is_transformable_type_v<T>>> : std::true_type {};

template <typename T>
constexpr bool is_option_bindable_container_v =
    is_option_bindable_container<T>::value;

template <typename T1, typename T2>
struct merge_variant;
template <typename T1, typename... T2>
struct merge_variant<std::variant<T1>, std::variant<T2...>> {
  using type = std::variant<T1, T2...>;
};
template <typename T0, typename... T1, typename... T2>
struct merge_variant<std::variant<T0, T1...>, std::variant<T2...>> {
  using type = typename merge_variant<std::variant<T1...>,
                                      std::variant<T0, T2...>>::type;
};
template <typename... T>
struct make_variant_type {
  // TODO(shediao):
  using type1 =
      typename merge_variant<std::variant<T...>,
                             std::variant<std::reference_wrapper<T>...>>::type;
  using type2 = typename merge_variant<
      std::variant<std::vector<T>...>,
      std::variant<std::reference_wrapper<std::vector<T>>...>>::type;
  using type3 = typename merge_variant<
      std::variant<std::pair<std::string, T>...>,
      std::variant<std::reference_wrapper<std::pair<std::string, T>>...>>::type;
  using type4 = typename merge_variant<
      std::variant<std::vector<std::pair<std::string, T>>...>,
      std::variant<std::reference_wrapper<
          std::vector<std::pair<std::string, T>>>...>>::type;
  using type5 = typename merge_variant<
      std::variant<std::map<std::string, T>...>,
      std::variant<std::reference_wrapper<std::map<std::string, T>>...>>::type;

  using type = typename merge_variant<
      typename merge_variant<
          typename merge_variant<typename merge_variant<type1, type2>::type,
                                 type3>::type,
          type4>::type,
      type5>::type;
};

template <typename T>
struct bindable_type_info;

template <>
struct bindable_type_info<bool> {
  inline static std::string name() { return "bool"; }
};
template <>
struct bindable_type_info<int> {
  inline static std::string name() { return "int"; }
};
template <>
struct bindable_type_info<double> {
  inline static std::string name() { return "double"; }
};
template <>
struct bindable_type_info<std::string> {
  inline static std::string name() { return "std::string"; }
};

template <typename T, typename U>
struct bindable_type_info<std::pair<T, U>> {
  inline static std::string name() {
    return std::string("std::pair<") + bindable_type_info<T>::name() + "," +
           bindable_type_info<U>::name() + ">";
  }
};
template <typename T, typename U>
struct bindable_type_info<std::map<T, U>> {
  inline static std::string name() {
    return std::string("std::map<") + bindable_type_info<T>::name() + "," +
           bindable_type_info<U>::name() + ">";
  }
};

template <typename T>
struct bindable_type_info<std::vector<T>> {
  inline static std::string name() {
    return std::string("std::vector<") + bindable_type_info<T>::name() + ">";
  }
};

//**********************************
namespace StringUtil {
inline bool startswith(std::string const& str, std::string const& prefix) {
  if (str.length() < prefix.length()) {
    return false;
  }
  auto it = str.begin();
  auto pit = prefix.begin();
  while (pit != prefix.end()) {
    if (*it != *pit) {
      return false;
    }
    it++;
    pit++;
  }
  return true;
}
inline std::vector<std::string> split(std::string const& s,
                                      char delimiter,
                                      int number = 0) {
  std::vector<std::string> result;
  std::istringstream ss{s};
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

inline bool is_short_opt(std::string const& opt) {
  return opt.size() >= 2 && opt[0] == '-' && opt[1] != '-';
}
inline bool is_long_opt(std::string const& opt) {
  return opt.size() >= 3 && opt[0] == '-' && opt[1] == '-' && opt[2] != '-';
}
inline bool is_dash_dash(std::string const& opt) {
  return opt == "--";
}
inline bool is_position_arg(std::string const& str) {
  return !is_short_opt(str) && !is_long_opt(str) && !is_dash_dash(str);
}
}  // namespace StringUtil

// bool
template <typename T, std::enable_if_t<std::is_same_v<bool, T>, bool> = true>
void transform_value(std::string const& from, T& to) {
  std::string lower_from;
  std::transform(from.begin(), from.end(), std::back_inserter(lower_from),
                 [](unsigned char c) { return std::tolower(c); });
  if (lower_from == "true") {
    to = true;
  } else if (lower_from == "false") {
    to = false;
  } else {
    throw bad_value_access(std::string("err: ") + "'" + from + "'" + " => " +
                           bindable_type_info<T>::name());
  }
}

template <typename T,
          std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<bool, T>,
                           bool> = true>
void transform_value(std::string const& from, T& to) {
  auto [ptr, ec] = std::from_chars(from.data(), from.data() + from.size(), to);
  if (ec == std::errc::invalid_argument || ec != std::errc{}) {
    throw bad_value_access(std::string("err: ") + "'" + from + "'" + " => " +
                           bindable_type_info<T>::name());
  }
}

template <typename T, std::enable_if_t<std::is_same_v<double, T>, bool> = true>
void transform_value(std::string const& from, T& to) {
  try {
    to = std::stod(from, nullptr);
  } catch (std::invalid_argument const& e) {
    throw bad_value_access(std::string(e.what()) + ": '" + from + "'" + " => " +
                           bindable_type_info<T>::name());
  }
}

// std::string
template <typename T,
          std::enable_if_t<std::is_same_v<std::string, T>, bool> = true>
void transform_value(std::string const& from, T& to) {
  to = from;
}

template <typename T, std::enable_if_t<is_pair_v<T>, bool> = true>
void transform_value(std::string const& from, T& to, char delimiter = '=') {
  auto index = from.find(delimiter);
  if (index != std::string::npos) {
    transform_value(from.substr(0, index), to.first);
    transform_value(from.substr(index + 1), to.second);
  }
}

// container
template <typename T,
          std::enable_if_t<is_option_bindable_container_v<T>, bool> = true>
void insert_or_replace_value(T& bind_value,
                             std::string const& option_val,
                             char delimiter = '=') {
  if (option_val.empty()) {
    return;
  }
  std::insert_iterator<T> it(bind_value, bind_value.end());  // insert to end
  if constexpr (is_pair_v<typename T::value_type>) {
    using writable_pair = typename std::pair<
        typename std::remove_const_t<typename T::value_type::first_type>,
        typename T::value_type::second_type>;
    writable_pair result;
    transform_value<writable_pair>(option_val, result, delimiter);
    *it = std::move(result);
  } else {
    typename T::value_type result;
    transform_value<typename T::value_type>(option_val, result);
    *it = std::move(result);
  }
}

// non-container
template <typename T,
          std::enable_if_t<!is_option_bindable_container_v<T>, bool> = false>
void insert_or_replace_value(T& bind_value,
                             std::string const& option_val,
                             char delimiter = '=') {
  if (option_val.empty()) {
    return;
  }
  T result;
  if constexpr (is_pair_v<T>) {
    transform_value<T>(option_val, result, delimiter);
  } else {
    transform_value<T>(option_val, result);
  }
  bind_value = std::move(result);
}

class OptBase {
  friend class ArgParser;

 protected:
  enum class Type { FLAG, ALIAS_FLAG, OPTION, POSITIONAL };

 public:
  virtual ~OptBase() = default;
  OptBase::Type virtual type() const = 0;
  bool is_flag() const {
    return Type::FLAG == this->type() || is_alias_flag();
  };
  bool is_alias_flag() const { return Type::ALIAS_FLAG == this->type(); };
  bool is_option() const { return Type::OPTION == this->type(); };
  bool is_positional() const { return Type::POSITIONAL == this->type(); };

  template <typename T, typename = std::enable_if_t<is_bindable_value_v<T>>>
  T const& as() const {
    try {
      if (std::holds_alternative<std::reference_wrapper<T>>(value)) {
        return std::get<std::reference_wrapper<T>>(value).get();
      }
      return std::get<T>(value);
    } catch (std::bad_variant_access const& e) {
      throw bad_value_access(std::string(e.what()) +
                             " as: " + value_type_name() + " => " +
                             bindable_type_info<T>::name());
    }
  }

  template <typename T, typename = std::enable_if_t<is_bindable_value_v<T>>>
  OptBase& bind(T& bind_val) {
    if (is_flag()) {
      if constexpr (is_flag_bindable_value_v<T>) {
        value = std::ref(bind_val);
      } else {
        throw bad_value_access(std::string("flag can't bind the type: ") +
                               bindable_type_info<T>::name());
      }
    } else if (is_option()) {
      if constexpr (is_option_bindable_value_v<T>) {
        value = std::ref(bind_val);
      } else {
        throw bad_value_access(std::string("option can't bind the type: ") +
                               bindable_type_info<T>::name());
      }
    } else if (is_positional()) {
      if constexpr (is_position_bindable_value_v<T>) {
        value = std::ref(bind_val);
      } else {
        throw bad_value_access(std::string("positional can't bind the type: ") +
                               bindable_type_info<T>::name());
      }
    }
    return *this;
  }

  OptBase& help(std::string const& help) {
    help_msg = help;
    return *this;
  }
  OptBase& value_help(std::string const& helper) {
    this->value_placeholder = helper;
    return *this;
  }

  // for const char*
  OptBase& set_default(std::string const& def_val) {
    return set_default<std::string>(def_val);
  }
  template <typename T, typename = std::enable_if_t<is_bindable_value_v<T>>>
  OptBase& set_default(T const& def_val) {
    current_is_default_value = true;
    return set_init_value(def_val);
  }

  // template <typename T>
  // OptBase& check(std::function<bool(T const&)> f) {
  //   auto check_result =
  //       std::visit(overloaded{[](auto const& /*v*/) -> bool {
  //                               throw bad_value_access("check failed");
  //                             },
  //                             [&f](T const& v) -> bool {
  //                               if (!f(v)) {
  //                                 throw bad_value_access("check failed");
  //                               }
  //                               return true;
  //                             }},
  //                  value);
  //   if (!check_result) {
  //     throw bad_value_access("check failed");
  //   }
  //   return *this;
  // }

  [[nodiscard]] int count() const { return hit_count; }
  OptBase(OptBase const&) = delete;
  OptBase(OptBase&&) = delete;
  OptBase& operator=(OptBase const&) = delete;
  OptBase& operator=(OptBase&&) = delete;

 protected:
  using value_type = make_variant_type<bool, int, double, std::string>::type;
  template <typename T>
  struct identity {
    using type = T;
  };

  template <typename T, typename = std::enable_if_t<is_bindable_value_v<T>>>
  OptBase(identity<T> /*unused*/, T& bind) : value(std::ref(bind)) {}

  template <typename T, typename = std::enable_if_t<is_bindable_value_v<T>>>
  explicit OptBase(identity<T> /*unused*/) : value(T{}) {}

  virtual void hit(char short_name) = 0;
  virtual void hit(std::string const& long_name) = 0;
  virtual void hit(char short_name, std::string const& val) = 0;
  virtual void hit(std::string const& long_name, std::string const& val) = 0;
  [[nodiscard]] virtual std::string usage() const = 0;
  [[nodiscard]] virtual std::string short_usage() const = 0;

  [[nodiscard]] virtual bool contains(std::string const& name) const {
    return end(flag_and_option_names) !=
           find(begin(flag_and_option_names), end(flag_and_option_names), name);
  }
  void add_option_name(std::string opt_name) {
    flag_and_option_names.push_back(std::move(opt_name));
  }
  std::vector<std::string> const& option_names() const {
    return flag_and_option_names;
  }

  template <typename T, typename = std::enable_if_t<is_bindable_value_v<T>>>
  OptBase& set_init_value(T const& init_val) {
    std::visit(
        [this, &init_val](auto& val) {
          using type = std::remove_reference_t<decltype(val)>;
          if constexpr (is_reference_wrapper_v<type>) {
            if constexpr (std::is_same_v<typename type::type, T>) {
              val.get() = init_val;
            } else {
              throw bad_value_access(std::string("err: ") + value_type_name() +
                                     " <= " + bindable_type_info<T>::name());
            }
          } else {
            if constexpr (std::is_same_v<type, T>) {
              val = init_val;
            } else {
              throw bad_value_access(std::string("err: ") + value_type_name() +
                                     " <= " + bindable_type_info<T>::name());
            }
          }
        },
        value);
    return *this;
  }

  std::string value_type_name() const {
    return std::visit(
        overloaded{[](auto& v) {
          using type =
              std::remove_const_t<std::remove_reference_t<decltype(v)>>;
          if constexpr (is_reference_wrapper_v<type>) {
            return bindable_type_info<typename type::type>::name();
          } else {
            return bindable_type_info<type>::name();
          }
        }},
        value);
  }

  std::string const& get_value_help() const { return value_placeholder; }
  std::string const& get_help() const { return help_msg; }
  void increment_count() { hit_count++; }
  std::vector<std::string> flag_and_option_names;
  std::string help_msg;
  std::string value_placeholder;
  value_type value;
  bool current_is_default_value{false};
  int hit_count{0};
};
class ArgParser;
class Flag : public OptBase {
  friend class ArgParser;

 public:
  OptBase::Type type() const override { return OptBase::Type::FLAG; };

 protected:
  template <typename T,
            typename = std::enable_if_t<is_flag_bindable_value_v<T>>>
  std::unique_ptr<Flag> static make_flag(std::string const& flag_desc,
                                         T& bind) {
    return std::unique_ptr<Flag>(new Flag(flag_desc, bind));
  }

  template <typename T,
            typename = std::enable_if_t<is_flag_bindable_value_v<T>>>
  std::unique_ptr<Flag> static make_flag(std::string const& flag_desc) {
    return std::unique_ptr<Flag>(new Flag(flag_desc, OptBase::identity<T>{}));
  }

  template <typename T,
            typename = std::enable_if_t<is_flag_bindable_value_v<T>>>
  Flag(std::string const& flag_desc, T& bind)
      : OptBase(typename OptBase::identity<T>{}, bind) {
    Flag_init(flag_desc);
  }
  template <typename T,
            typename = std::enable_if_t<is_flag_bindable_value_v<T>>>
  Flag(std::string const& flag_desc, OptBase::identity<T> /*unused*/)
      : OptBase(typename OptBase::identity<T>{}) {
    Flag_init(flag_desc);
  }
  [[nodiscard]] bool negate_contains(std::string const& flag) const {
    return find(begin(negate_flag_names), end(negate_flag_names), flag) !=
           end(negate_flag_names);
  }

  [[nodiscard]] bool contains(std::string const& flag) const override {
    return OptBase::contains(flag) || negate_contains(flag);
  }

  void hit(char const flag) override { hit(std::string(1, flag)); }
  void hit(std::string const& flag) override { hit_impl(flag); }
  void hit(char const flag, std::string const& val) override {
    hit(std::string(1, flag), val);
  }
  void hit(std::string const& /*flag*/, std::string const& /*val*/) override {
    UNREACHABLE();
  }

  [[nodiscard]] std::string usage() const override {
    std::ostringstream ss;
    if (auto it = cbegin(option_names()); it != cend(option_names())) {
      ss << "  ";
      ss << (it->length() == 1 ? "-" : "--") << *it;
      while (++it != end(option_names())) {
        ss << ", " << (it->length() == 1 ? "-" : "--") << *it;
      }
    }
    if (!get_help().empty()) {
      ss << "\n";
      ss << "            " << get_help();
    }
    return ss.str();
  }
  [[nodiscard]] std::string short_usage() const override {
    std::ostringstream ss;
    if (auto it = cbegin(option_names()); it != cend(option_names())) {
      ss << "[";
      ss << (it->length() == 1 ? "-" : "--") << *it;
      while (++it != end(option_names())) {
        ss << "|" << (it->length() == 1 ? "-" : "--") << *it;
      }
      ss << "]";
    }
    return ss.str();
  }

 private:
  void Flag_init(std::string const& flag_desc) {
    // TODO(shediao): trim left and right blank
    // flag is ,
    auto all = StringUtil::split(flag_desc, ',');
    if (all.empty()) {
      throw std::logic_error("flag name is empty");
    }
    for (auto f : all) {
      bool is_negate{false};
      if (f.empty()) {
        throw std::logic_error("flag item name is empty");
      }
      f = std::string(
          find_if_not(begin(f), end(f), [](char c) { return std::isspace(c); }),
          find_if_not(rbegin(f), rend(f), [](char c) {
            return std::isspace(c);
          }).base());

      if (f[0] == '!') {
        is_negate = true;
        f = f.substr(1);
      }

      if (StringUtil::startswith(f, "--")) {
        f = f.substr(2);
      } else if (StringUtil::startswith(f, "-")) {
        f = f.substr(1);
      }

      if (f.empty()) {
        throw std::logic_error("flag item name is empty");
      }
      if (f[0] == '-') {
        throw std::logic_error("flag item name is startswith '-'");
      }
      if (is_negate) {
        negate_flag_names.push_back(f);
      } else {
        add_option_name(std::move(f));
      }
    }
  }
  template <typename ShortOrLong>
  void hit_impl(ShortOrLong const& flag) {
    increment_count();
    std::visit(
        overloaded{
            [this, &flag](bool& val) {
              if (negate_contains(flag)) {
                val = false;
              } else if (OptBase::contains(flag)) {
                val = true;
              } else {
                UNREACHABLE();
              }
            },
            [this, &flag](std::reference_wrapper<bool>& val) {
              if (negate_contains(flag)) {
                val.get() = false;
              } else if (OptBase::contains(flag)) {
                val.get() = true;
              } else {
                UNREACHABLE();
              }
            },
            [this, &flag](auto& val) {
              using type = std::remove_reference_t<decltype(val)>;
              if constexpr (is_reference_wrapper_v<type>) {
                if constexpr (is_flag_bindable_value_v<typename type::type>) {
                  if (current_is_default_value) {
                    val.get() = 0;
                  }
                  if (negate_contains(flag)) {
                    val.get() -= 1;
                  } else if (OptBase::contains(flag)) {
                    val.get() += 1;
                  } else {
                    UNREACHABLE();
                  }
                } else {
                  UNREACHABLE();
                }
              } else {
                if constexpr (is_flag_bindable_value_v<type>) {
                  if (current_is_default_value) {
                    val = 0;
                  }
                  if (negate_contains(flag)) {
                    val -= 1;
                  } else if (OptBase::contains(flag)) {
                    val += 1;
                  } else {
                    UNREACHABLE();
                  }
                } else {
                  UNREACHABLE();
                }
              }
            }},
        value);
    current_is_default_value = false;
  }

  std::vector<std::string> negate_flag_names{};
};

class AliasFlag : public Flag {
  friend class ArgParser;

 public:
  OptBase::Type type() const override { return OptBase::Type::ALIAS_FLAG; };

 protected:
  std::unique_ptr<AliasFlag> static make_flag(
      std::string const& flag_desc,
      std::pair<std::string, std::string> option) {
    return std::unique_ptr<AliasFlag>(
        new AliasFlag(flag_desc, std::move(option)));
  }
  explicit AliasFlag(std::string const& flag_desc,
                     std::pair<std::string, std::string> option)
      : Flag(flag_desc, OptBase::identity<bool>{}),
        option_name(std::move(std::move(option).first)),
        option_value(std::move(std::move(option).second)) {}
  void hit(char const flag) override { Flag::hit(flag); }
  void hit(std::string const& flag) override { Flag::hit(flag); }

 private:
  std::string option_name;
  std::string option_value;
};

class Option : public OptBase {
  friend class ArgParser;

 public:
  OptBase::Type type() const override { return OptBase::Type::OPTION; };

 protected:
  template <typename T,
            typename = std::enable_if_t<is_option_bindable_value_v<T>>>
  std::unique_ptr<Option> static make_option(std::string const& option_desc,
                                             T& bind) {
    return std::unique_ptr<Option>(new Option(option_desc, bind));
  }

  template <typename T,
            typename = std::enable_if_t<is_option_bindable_value_v<T>>>
  std::unique_ptr<Option> static make_option(std::string const& option_desc) {
    return std::unique_ptr<Option>(
        new Option(option_desc, OptBase::identity<T>{}));
  }

  template <typename T,
            typename = std::enable_if_t<is_option_bindable_value_v<T>>>
  Option(std::string const& option_desc, T& bind)
      : OptBase(typename OptBase::identity<T>{}, bind) {
    Option_init(option_desc);
  }
  template <typename T,
            typename = std::enable_if_t<is_option_bindable_value_v<T>>>
  Option(std::string const& option_desc, OptBase::identity<T> /*unused*/)
      : OptBase(typename OptBase::identity<T>{}) {
    Option_init(option_desc);
  }
  [[nodiscard]] std::string usage() const override {
    std::ostringstream ss;
    auto it = begin(option_names());
    if (it != end(option_names())) {
      ss << "  ";
      ss << (it->length() == 1 ? "-" : "--") << *it;
      while (++it != end(option_names())) {
        ss << "," << (it->length() == 1 ? "-" : "--") << *it;
      }
      if (get_value_help().empty()) {
        ss << "=<TEXT>";
      } else {
        ss << "=<" << get_value_help() << ">";
      }
    }

    if (!get_help().empty()) {
      ss << "\n";
      ss << "            " << get_help();
    }
    return ss.str();
  }
  [[nodiscard]] std::string short_usage() const override {
    std::ostringstream ss;
    if (auto it = cbegin(option_names()); it != cend(option_names())) {
      ss << "[";
      ss << (it->length() == 1 ? "-" : "--") << *it;
      while (++it != end(option_names())) {
        ss << "|" << (it->length() == 1 ? "-" : "--") << *it;
      }
      ss << " ";
      if (get_value_help().empty()) {
        ss << "<TEXT>";
      } else {
        ss << "<" << get_value_help() << ">";
      }
      ss << "]";
    }
    return ss.str();
  }

  void hit(std::string const& /*long_name*/, std::string const& val) override {
    return hit_impl(val);
  }
  void hit(char /*short_name*/, std::string const& val) override {
    return hit_impl(val);
  }
  void hit(std::string const& /*flag*/) override { UNREACHABLE(); }
  void hit(char const /*flag*/) override { UNREACHABLE(); }

 private:
  void Option_init(std::string const& option_desc) {
    auto all = StringUtil::split(option_desc, ',');
    if (all.empty()) {
      throw std::logic_error("option name is empty");
    }
    for (auto f : all) {
      f = std::string(
          find_if_not(begin(f), end(f), [](char c) { return std::isspace(c); }),
          find_if_not(rbegin(f), rend(f), [](char c) {
            return std::isspace(c);
          }).base());
      if (StringUtil::startswith(f, "--")) {
        f = f.substr(2);
      } else if (StringUtil::startswith(f, "-")) {
        f = f.substr(1);
      }
      if (f.empty()) {
        throw std::logic_error("option item name is empty");
      }
      add_option_name(std::move(f));
    }
  }

  void hit_impl(std::string const& opt_val) {
    increment_count();
    std::visit(
        [&opt_val, this](auto& x) {
          using T = std::remove_reference_t<decltype(x)>;
          if constexpr (is_reference_wrapper_v<T>) {
            if constexpr (is_option_bindable_container_v<typename T::type>) {
              if (current_is_default_value) {
                x.get().clear();
              }
            }
            insert_or_replace_value(x.get(), opt_val, delimiter);
          } else {
            if constexpr (is_option_bindable_container_v<T>) {
              if (current_is_default_value) {
                x.clear();
              }
            }
            insert_or_replace_value(x, opt_val, delimiter);
          }
        },
        value);
    current_is_default_value = false;
  }

  char delimiter{'\0'};
};

class Positional : public OptBase {
  friend class ArgParser;

 public:
  OptBase::Type type() const override { return OptBase::Type::POSITIONAL; };

 protected:
  template <typename T>
  static std::unique_ptr<Positional> make_positional(std::string const& name,
                                                     T& bind) {
    return std::unique_ptr<Positional>(new Positional(name, bind));
  }
  template <typename T>
  static std::unique_ptr<Positional> make_positional(std::string const& name) {
    return std::unique_ptr<Positional>(
        new Positional(name, OptBase::identity<T>{}));
  }

  void hit(char /*short_name*/) override { UNREACHABLE(); };
  void hit(std::string const& /*long_name*/) override { UNREACHABLE(); };
  void hit(char /*short_name*/, std::string const& val) override {
    UNREACHABLE();
  };
  void hit(std::string const& long_name, std::string const& val) override {
    std::visit(
        overloaded{[&val, this](auto& v) {
          using type = std::remove_reference_t<decltype(v)>;
          if constexpr (is_reference_wrapper_v<type>) {
            if constexpr (is_option_bindable_container_v<typename type::type>) {
              if (current_is_default_value) {
                v.get().clear();
              }
            }
            insert_or_replace_value(v.get(), val, delimiter);
            if constexpr (!is_option_bindable_container_v<
                              typename type::type>) {
              can_set_value = false;
            }
          } else {
            if constexpr (is_option_bindable_container_v<type>) {
              if (current_is_default_value) {
                v.clear();
              }
            }
            insert_or_replace_value(v, val, delimiter);
            if constexpr (!is_option_bindable_container_v<type>) {
              can_set_value = false;
            }
          }
        }},
        value);
    current_is_default_value = false;
  };

  [[nodiscard]] std::string usage() const override {
    std::ostringstream ss;
    if (!option_names().empty()) {
      ss << "  ";
      ss << (get_value_help().empty() ? option_names()[0] : get_value_help());
      if (!get_help().empty()) {
        ss << "\n";
        ss << "            " << get_help();
      }
    }
    return ss.str();
  };
  [[nodiscard]] std::string short_usage() const override {
    std::ostringstream ss;
    auto is_multi_positional = std::visit(
        [](auto& v) {
          using type =
              std::remove_const_t<std::remove_reference_t<decltype(v)>>;
          if constexpr (is_reference_wrapper_v<type>) {
            return (is_vector_v<typename type::type> ||
                    is_map_v<typename type::type>);
          } else {
            return (is_vector_v<type> || is_map_v<type>);
          }
        },
        value);
    if (!option_names().empty()) {
      if (is_multi_positional) {
        if (get_value_help().empty()) {
          ss << option_names()[0] << "...";
        } else {
          ss << get_value_help() << "...";
        }
      } else {
        if (get_value_help().empty()) {
          ss << option_names()[0];
        } else {
          ss << get_value_help();
        }
      }
    }
    return ss.str();
  }

  template <typename T>
  Positional(std::string const& name, T& bind)
      : OptBase(OptBase::identity<T>{}, bind) {
    add_option_name(name);
  }

  template <typename T>
  Positional(std::string const& name, OptBase::identity<T>)
      : OptBase(OptBase::identity<T>{}) {
    add_option_name(name);
  }
  bool can_set_value{true};
  char delimiter{'\0'};
};

class ArgParser {
 public:
  ArgParser() = default;
  explicit ArgParser(std::string desc) : description(std::move(desc)) {}

  ArgParser& set_program_name(std::string const& programName) {
    this->program_name = programName;
    return *this;
  }

  template <typename T,
            typename = std::enable_if_t<is_flag_bindable_value_v<T>>>
  Flag& add_flag(std::string const& flag_desc, T& bind) {
    auto x = Flag::make_flag(flag_desc, bind);
    auto p = x.get();
    // TODO(shediao): check flag already exists
    all_options.push_back(std::move(x));
    return *p;
  }

  template <typename T = bool,
            typename = std::enable_if_t<is_flag_bindable_value_v<T>>>
  Flag& add_flag(std::string const& flag_desc) {
    auto x = Flag::make_flag<T>(flag_desc);
    auto p = x.get();
    // TODO(shediao): check flag already exists
    all_options.push_back(std::move(x));
    return *p;
  }
  AliasFlag& add_alias_flag(
      std::string const& flag_desc,
      std::pair<std::string, std::string> option_key_value) {
    auto x = AliasFlag::make_flag(flag_desc, std::move(option_key_value));
    auto* p = x.get();
    // TODO(shediao): check flag already exists
    all_options.push_back(std::move(x));
    return *p;
  }

  template <typename T,
            typename = std::enable_if_t<!is_need_split_v<T> &&
                                        is_option_bindable_value_v<T>>>
  Option& add_option(std::string const& option_desc, T& bind) {
    auto x = Option::make_option(option_desc, bind);
    auto p = x.get();
    // TODO(shediao): check option already exists
    all_options.push_back(std::move(x));
    return *p;
  }

  template <typename T = std::string,
            typename = std::enable_if_t<!is_need_split_v<T> &&
                                        is_option_bindable_value_v<T>>>
  Option& add_option(std::string const& option_desc) {
    auto x = Option::make_option<T>(option_desc);
    auto p = x.get();
    // TODO(shediao): check option already exists
    all_options.push_back(std::move(x));
    return *p;
  }

  template <typename T,
            typename = std::enable_if_t<is_need_split_v<T> &&
                                        is_option_bindable_value_v<T>>>
  Option& add_option(std::string const& option_desc,
                     T& bind,
                     char delimiter = default_delimiter_v<T>) {
    auto x = Option::make_option(option_desc, bind);
    auto p = x.get();
    all_options.push_back(std::move(x));
    p->delimiter = delimiter;
    return *p;
  }

  template <typename T = std::string,
            typename = std::enable_if_t<is_need_split_v<T> &&
                                        is_option_bindable_value_v<T>>>
  Option& add_option(std::string const& option_desc,
                     char delimiter = default_delimiter_v<T>) {
    auto x = Option::make_option<T>(option_desc);
    auto p = x.get();
    // TODO(shediao): check option already exists
    all_options.push_back(std::move(x));
    p->delimiter = delimiter;
    return *p;
  }

  template <typename T,
            typename = std::enable_if_t<!is_need_split_v<T> &&
                                        is_position_bindable_value_v<T>>>
  Positional& add_positional(std::string const& name, T& bind) {
    auto x = Positional::make_positional(name, bind);
    auto p = x.get();
    // TODO(shediao): check positional already exists
    all_options.push_back(std::move(x));
    return *p;
  }
  template <typename T = std::vector<std::string>,
            typename = std::enable_if_t<!is_need_split_v<T> &&
                                        is_position_bindable_value_v<T>>>
  Positional& add_positional(std::string const& name) {
    auto x = Positional::make_positional<T>(name);
    auto p = x.get();
    // TODO(shediao): check positional already exists
    all_options.push_back(std::move(x));
    return *p;
  }

  template <typename T,
            typename = std::enable_if_t<is_need_split_v<T> &&
                                        is_position_bindable_value_v<T>>>
  Positional& add_positional(std::string const& name,
                             T& bind,
                             char delimiter = default_delimiter_v<T>) {
    auto x = Positional::make_positional(name, bind);
    auto p = x.get();
    all_options.push_back(std::move(x));
    p->delimiter = delimiter;
    return *p;
  }
  template <typename T = std::vector<std::string>,
            typename = std::enable_if_t<is_need_split_v<T> &&
                                        is_position_bindable_value_v<T>>>
  Positional& add_positional(std::string const& name,
                             char delimiter = default_delimiter_v<T>) {
    auto x = Positional::make_positional<T>(name);
    auto p = x.get();
    all_options.push_back(std::move(x));
    p->delimiter = delimiter;
    return *p;
  }

  std::string usage() {
    std::ostringstream ss;
    ss << "usage: " << (program_name.empty() ? "?" : program_name);
    for (auto const& opt : all_options) {
      if (!opt->is_positional()) {
        ss << " " << opt->short_usage();
      }
    }
    for (auto const& opt : all_options) {
      if (opt->is_positional()) {
        ss << " " << opt->short_usage();
      }
    }
    ss << "\n\n";
    if (!description.empty()) {
      ss << description << "\n\n";
    }

    if (any_of(begin(all_options), end(all_options),
               [](auto& opt) { return opt->is_positional(); })) {
      ss << "Positional arguments:\n";
      for (auto const& opt : all_options) {
        if (opt->is_positional()) {
          ss << opt->usage() << "\n\n";
        }
      }
    }

    if (any_of(begin(all_options), end(all_options),
               [](auto& opt) { return !opt->is_positional(); })) {
      ss << "Options:\n";
      for (auto const& opt : all_options) {
        if (!opt->is_positional()) {
          ss << opt->usage() << "\n\n";
        }
      }
    }

    return ss.str();
  }

  void set_unknown_option_as_start_of_positionals() {
    unknown_option_as_start_of_positionals = true;
  }

  void parse(int argc, const char* const* argv) {
    add_help_flag_if_needed();
    std::vector<std::string> command_line_args{argv, argv + argc};

    if (command_line_args.empty()) {
      return;
    }

    auto current = command_line_args.begin();

    if (auto& first = *command_line_args.begin();
        !first.empty() && first[0] != '-') {
      if (program_name.empty()) {
        auto it = std::find_if(rbegin(first), rend(first),
                               [](char c) { return c == '/' || c == '\\'; });
        program_name = std::string(it.base(), first.end());
      }
      // 0 ==> skip program file path
      current = std::next(command_line_args.begin());
    }

    auto current_position_it =
        find_if(begin(all_options), end(all_options),
                [](auto& o) { return o->is_positional(); });

    while (current != command_line_args.end()) {
      auto next = std::next(current);

      auto const& curr_arg = *current;

      if (StringUtil::is_position_arg(curr_arg)) {
        while (current_position_it != all_options.end() &&
               !(dynamic_cast<Positional*>(current_position_it->get()))
                    ->can_set_value) {
          current_position_it =
              find_if(std::next(current_position_it), end(all_options),
                      [](auto& o) { return o->is_positional(); });
        }
        if (current_position_it == all_options.end()) {
          throw invalid_argument("unrecognized arguments: " + curr_arg);
        }
        (*current_position_it)->hit("", curr_arg);
        current = next;
      } else if (StringUtil::is_dash_dash(curr_arg)) {
        // --
        if (!unknown_option_as_start_of_positionals) {
          current = next;
        }
        break;
      } else if (StringUtil::is_short_opt(curr_arg)) {
        // short
        auto short_p = curr_arg.begin() + 1;
        while (short_p != curr_arg.end()) {
          if (auto short_flag = get_flag(*short_p); short_flag.has_value()) {
            (*short_flag)->hit(*short_p);
            if ((*short_flag)->is_alias_flag()) {
              auto* alis_flag = dynamic_cast<AliasFlag*>(*short_flag);
              if (auto option = get_option(alis_flag->option_name);
                  option.has_value()) {
                option.value()->hit(alis_flag->option_name,
                                    alis_flag->option_value);
              } else {
                UNREACHABLE();
              }
            }
            short_p++;
          } else if (auto short_option = get_option(*short_p);
                     short_option.has_value()) {
            if (short_p + 1 != curr_arg.end()) {
              (*short_option)
                  ->hit(*short_p, std::string(short_p + 1, curr_arg.end()));
              short_p = curr_arg.end();
            } else {
              if (next == command_line_args.end()) {
                throw invalid_argument("option requires an argument: -" +
                                       std::string(1, *short_p));
              }
              if ((!next->empty() && (*next)[0] == '-')) {
                throw invalid_argument("option requires an argument: -" +
                                       std::string(1, *short_p));
              }
              (*short_option)->hit(*short_p, *next);
              short_p = curr_arg.end();
              next = std::next(next);
            }
          } else {
            if (unknown_option_as_start_of_positionals) {
              break;
            }
            throw invalid_argument("invalid option: -" +
                                   std::string(1, *short_p));
          }
        }
        current = next;
      } else if (StringUtil::is_long_opt(curr_arg)) {
        // long
        if (auto i = curr_arg.find('=', 2); i != std::string::npos) {
          std::string const option = curr_arg.substr(2, i - 2);
          if (auto long_opt = get_option(option); long_opt.has_value()) {
            (*long_opt)->hit(option, curr_arg.substr(i + 1));
          } else {
            throw invalid_argument("invalid option: --" + option);
          }
        } else {
          std::string const option = curr_arg.substr(2);
          if (auto long_flag = get_flag(option); long_flag.has_value()) {
            (*long_flag)->hit(option);
          } else if (auto long_opt = get_option(option); long_opt.has_value()) {
            if (std::next(current) == command_line_args.end()) {
              throw invalid_argument("option requires an argument: --" +
                                     option);
            }
            if (!next->empty() && (*next)[0] == '-') {
              throw invalid_argument("option requires an argument: --" +
                                     option);
            }
            (*long_opt)->hit(option, *next);
            next = std::next(next);
          } else {
            if (unknown_option_as_start_of_positionals) {
              break;
            }
            throw invalid_argument("invalid option: --" + option);
          }
        }
        current = next;
      } else {
        UNREACHABLE();
      }
    }

    for (; current != command_line_args.end(); current++) {
      while (!(dynamic_cast<Positional*>(current_position_it->get()))
                  ->can_set_value &&
             current_position_it != all_options.end()) {
        current_position_it =
            find_if(std::next(current_position_it), end(all_options),
                    [](auto& o) { return o->is_positional(); });
      }
      if (current_position_it == all_options.end()) {
        throw invalid_argument("unrecognized arguments: " + *current);
      }
      (*current_position_it)->hit("", *current);
    }
  }

  std::optional<OptBase*> get(std::string const& f) {
    auto it = find_if(begin(all_options), end(all_options),
                      [&f](auto const& f1) { return f1->contains(f); });
    if (it != end(all_options)) {
      return (*it).get();
    }
    return std::nullopt;
  }

  OptBase& operator[](std::string const& f) {
    auto x = get(f);
    if (x.has_value()) {
      return *(x.value());
    }
    throw option_not_found{};
  }

 private:
  void add_help_flag_if_needed() {
    auto has_h_flag = get_flag("h").has_value();
    auto has_help_flag = get_flag("help").has_value();
    const char* msg = "display this help and exit";
    if (!has_h_flag && !has_help_flag) {
      add_flag("-h,--help").help(msg);
      return;
    }
    if (!has_h_flag) {
      add_flag("-h").help(msg);
      return;
    }
    if (!has_help_flag) {
      add_flag("--help").help(msg);
      return;
    }
  }
  std::optional<Flag*> get_flag(char const flag) {
    return get_flag(std::string(1, flag));
  }
  std::optional<Flag*> get_flag(std::string const& flag) {
    auto it = find_if(
        begin(all_options), end(all_options),
        [flag](auto const& f) { return f->is_flag() && f->contains(flag); });
    if (it != end(all_options)) {
      return dynamic_cast<Flag*>((*it).get());
    }
    return {};
  }
  std::optional<Option*> get_option(char const opt) {
    return get_option(std::string(1, opt));
  }
  std::optional<Option*> get_option(std::string const& opt) {
    auto it = find_if(
        begin(all_options), end(all_options),
        [opt](auto const& f) { return f->is_option() && f->contains(opt); });
    if (end(all_options) != it) {
      return dynamic_cast<Option*>((*it).get());
    }
    return {};
  }

  std::string description{};
  std::string program_name{};
  bool unknown_option_as_start_of_positionals{false};

  std::vector<std::unique_ptr<OptBase>> all_options{};
};

}  // namespace argparse

#endif  // ARGPARSE_CPP_H_
