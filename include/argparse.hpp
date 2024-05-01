
#ifndef __ARGPARSE_CPP_H__
#define __ARGPARSE_CPP_H__

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

template <typename T>
struct is_reference_wrapper : std::false_type {};

template <typename T>
struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type {};

template <typename T>
using is_reference_wrapper_t = typename is_reference_wrapper<T>::type;

template <typename T>
inline constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

template <typename T, typename = void>
struct is_flag_bindable_value : std::false_type {};

template <typename T>
struct is_flag_bindable_value<
    T,
    std::enable_if_t<std::is_same_v<bool, T> || std::is_same_v<int, T>>>
    : std::true_type {};

template <typename T>
using is_flag_bindable_value_t = typename is_flag_bindable_value<T>::type;
template <typename T>
inline constexpr bool is_flag_bindable_value_v =
    is_flag_bindable_value<T>::value;

template <typename T, typename = void>
struct is_option_bindable_value : std::false_type {};

template <typename T>
struct is_option_bindable_value<
    T,
    std::enable_if_t<std::is_same_v<bool, T> || std::is_same_v<int, T> ||
                     std::is_same_v<double, T> ||
                     std::is_same_v<std::string, T> ||
                     std::is_same_v<std::vector<std::string>, T> ||
                     std::is_same_v<std::map<std::string, std::string>, T>>>
    : std::true_type {};

template <typename T>
using is_option_bindable_value_t = typename is_option_bindable_value<T>::type;
template <typename T>
inline constexpr bool is_option_bindable_value_v =
    is_option_bindable_value<T>::value;

template <typename T, typename = void>
struct is_bindable_value : std::false_type {};

template <typename T>
struct is_bindable_value<
    T,
    std::enable_if_t<std::is_same_v<bool, T> || std::is_same_v<int, T> ||
                     std::is_same_v<double, T> ||
                     std::is_same_v<std::string, T> ||
                     std::is_same_v<std::vector<std::string>, T> ||
                     std::is_same_v<std::map<std::string, std::string>, T>>>
    : std::true_type {};

template <typename T>
using is_bindable_value_t = typename is_bindable_value<T>::type;
template <typename T>
inline constexpr bool is_bindable_value_v = is_bindable_value<T>::value;

template <typename T, typename = void>
struct is_position_bindable_value : std::false_type {};

template <typename T>
struct is_position_bindable_value<
    T,
    std::enable_if_t<std::is_same_v<std::string, T> ||
                     std::is_same_v<std::vector<std::string>, T> ||
                     std::is_same_v<std::map<std::string, std::string>, T>>>
    : std::true_type {};

template <typename T>
using is_position_bindable_value_t =
    typename is_position_bindable_value<T>::type;
template <typename T>
inline constexpr bool is_position_bindable_value_v =
    is_position_bindable_value<T>::value;

// element_size_is_2
template <typename T>
struct element_size_is_2 : public std::false_type {};

template <typename T, typename U>
struct element_size_is_2<std::pair<T, U>> : public std::true_type {};

template <typename T, typename U>
struct element_size_is_2<std::tuple<T, U>> : public std::true_type {};

template <typename T>
struct element_size_is_2<std::array<T, 2>> : public std::true_type {};

template <typename T>
inline constexpr bool element_size_is_2_v = element_size_is_2<T>::value;

template <typename T>
inline constexpr bool is_bindable_noncontainer_value_v =
    std::is_same_v<bool, T> || std::is_same_v<int, T> ||
    std::is_same_v<double, T> || std::is_same_v<std::string, T> ||
    element_size_is_2_v<T>;

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
    std::enable_if_t<is_bindable_noncontainer_value_v<T>>> : std::true_type {};

template <typename T, typename U>
struct is_option_bindable_container<
    std::map<T, U>,
    std::enable_if_t<is_bindable_noncontainer_value_v<T> &&
                     is_bindable_noncontainer_value_v<U>>> : std::true_type {};

template <typename T>
constexpr bool is_option_bindable_container_v =
    is_option_bindable_container<T>::value;

//**********************************
namespace StringUtil {
inline bool startswith(std::string const& str, std::string const& prefix) {
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
inline bool endswith(std::string const& str, std::string const& suffix) {
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
inline std::vector<std::string> split(std::string const& s,
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

inline std::pair<std::string, std::string> split2(std::string const& s,
                                                  char delimiter) {
  auto it = find(s.begin(), s.end(), delimiter);
  return {std::string(s.begin(), it),
          std::string(it == s.end() ? s.end() : it, s.end())};
}

inline std::string trim(std::string const& str) {
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
template <typename T, std::enable_if_t<element_size_is_2_v<T>, int> = 0>
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
          std::enable_if_t<is_option_bindable_container_v<T>, bool> = true>
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
          std::enable_if_t<!is_option_bindable_container_v<T>, bool> = false>
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

class Base {
  template <typename T>
  class DefaultValueVisitor {
   public:
    DefaultValueVisitor(T const& def) : default_value(def) {}

    template <typename U>
    void operator()(U& val) {
      if constexpr (is_reference_wrapper_v<U>) {
        if constexpr (std::is_assignable_v<decltype(val.get()),
                                           decltype(default_value)>) {
          val.get() = default_value;
        } else {
          assert(false);
        }
      } else {
        if constexpr (std::is_assignable_v<decltype(val),
                                           decltype(default_value)>) {
          val = default_value;
        } else {
          assert(false);
        }
      }
    }

   private:
    T const& default_value;
  };
  friend class ArgParser;

 public:
  virtual ~Base() = default;
  virtual bool is_flag() const = 0;
  virtual bool is_option() const = 0;

  template <typename T, typename = std::enable_if_t<is_bindable_value_v<T>>>
  T const& as() const {
    if (std::holds_alternative<std::reference_wrapper<T>>(value)) {
      return std::get<std::reference_wrapper<T>>(value).get();
    }
    return std::get<T>(value);
  }

  Base& help(std::string const& help) {
    help_msg = help;
    return *this;
  }

  Base& set_default(std::string const& def_val) {
    std::visit(DefaultValueVisitor(def_val), value);
    return *this;
  }
  template <typename T, typename = std::enable_if_t<is_bindable_value_v<T>>>
  Base& set_default(T const& def_val) {
    std::visit(DefaultValueVisitor(def_val), value);
    return *this;
  }

  int count() const { return hit_count; }

 protected:
  Base(Base const&) = delete;
  Base(Base&&) = delete;
  Base& operator=(Base const&) = delete;
  template <typename T>
  struct identity {
    using type = T;
  };
  template <typename T, typename = std::enable_if_t<is_bindable_value_v<T>>>
  Base(identity<T> t, T& bind) : value(std::ref(bind)) {}

  template <typename T, typename = std::enable_if_t<is_bindable_value_v<T>>>
  Base(identity<T> t) : value(T{}) {}

  virtual void hit(char short_name) = 0;
  virtual void hit(std::string const& long_name) = 0;
  virtual std::pair<int, std::string> hit(char short_name,
                                          std::string const& val) = 0;
  virtual std::pair<int, std::string> hit(std::string const& long_name,
                                          std::string const& val) = 0;
  virtual std::string usage() const = 0;

  virtual bool contains(std::string const& name) const {
    return end(flag_and_option_names) !=
           find(begin(flag_and_option_names), end(flag_and_option_names), name);
  }
  std::vector<std::string> flag_and_option_names;
  std::string help_msg;
  std::variant<std::reference_wrapper<bool>,
               std::reference_wrapper<int>,
               bool,
               int,
               std::reference_wrapper<std::string>,
               std::reference_wrapper<std::vector<std::string>>,
               std::reference_wrapper<std::map<std::string, std::string>>,
               std::string,
               std::vector<std::string>,
               std::map<std::string, std::string>>
      value;
  int hit_count{0};
};
class ArgParser;
class Flag : public Base {
  friend class ArgParser;

  template <typename ShortOrLong>
  struct ValueVisitor {
    ValueVisitor(Flag const& obj, ShortOrLong const& flag)
        : flagObj{obj}, flag{flag} {}
    void operator()(bool& x) {
      if (flagObj.negate_contains(flag)) {
        x = false;
      } else if (flagObj.Base::contains(flag)) {
        x = true;
      }
    }
    void operator()(std::reference_wrapper<bool>& x) {
      if (flagObj.negate_contains(flag)) {
        x.get() = false;
      } else if (flagObj.Base::contains(flag)) {
        x.get() = true;
      }
    }

    template <typename T,
              std::enable_if_t<is_flag_bindable_value_v<T>, bool> = true>
    void operator()(T& x) {
      if (flagObj.negate_contains(flag)) {
        x -= 1;
      } else if (flagObj.Base::contains(flag)) {
        x += 1;
      }
    }
    template <typename T,
              std::enable_if_t<is_flag_bindable_value_v<T>, bool> = true>
    void operator()(std::reference_wrapper<T>& x) {
      if (flagObj.negate_contains(flag)) {
        x -= 1;
      } else if (flagObj.Base::contains(flag)) {
        x += 1;
      }
    }

    template <typename T,
              std::enable_if_t<!is_flag_bindable_value_v<T> &&
                                   !is_reference_wrapper_v<T>,
                               bool> = true>
    void operator()(T& x) {
      assert(false);
    }
    template <typename T,
              std::enable_if_t<!is_flag_bindable_value_v<T> &&
                                   is_reference_wrapper_v<T>,
                               bool> = true>
    void operator()(T& x) {
      assert(false);
    }

    Flag const& flagObj;
    ShortOrLong const& flag;
  };

 public:
  bool is_flag() const override { return true; };
  bool is_option() const override { return false; };

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
    return std::unique_ptr<Flag>(new Flag(flag_desc, Base::identity<T>{}));
  }

  template <typename T,
            typename = std::enable_if_t<is_flag_bindable_value_v<T>>>
  Flag(std::string const& flag_desc, T& bind)
      : Base(typename Base::identity<T>{}, bind) {
    Flag_init(flag_desc);
  }
  template <typename T,
            typename = std::enable_if_t<is_flag_bindable_value_v<T>>>
  Flag(std::string const& flag_desc, Base::identity<T>)
      : Base(typename Base::identity<T>{}) {
    Flag_init(flag_desc);
  }
  bool negate_contains(std::string const& flag) const {
    return find(begin(negate_flag_names), end(negate_flag_names), flag) !=
           end(negate_flag_names);
  }

  bool contains(std::string const& flag) const override {
    return Base::contains(flag) || negate_contains(flag);
  }

  void hit(char const flag) override { hit(std::string(1, flag)); }
  void hit(std::string const& flag) override { hit_impl(flag); }
  std::pair<int, std::string> hit(char const flag,
                                  std::string const& val) override {
    return hit(std::string(1, flag), val);
  }
  std::pair<int, std::string> hit(std::string const& flag,
                                  std::string const& val) override {
    assert(false);
    return {1, "flag not hold a value"};
  }

  std::string usage() const override {
    std::stringstream ss;
    auto it = begin(flag_and_option_names);
    if (it != end(flag_and_option_names)) {
      if (it->length() == 1) {
        ss << "-" << *it;
      } else {
        ss << "--" << *it;
      }
      it++;
    }
    if (it != end(flag_and_option_names)) {
      if (it->length() == 1) {
        ss << ",-" << *it;
      } else {
        ss << ",--" << *it;
      }
      it++;
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
    for (auto f : all) {
      bool is_negate{false};
      assert(!f.empty());

      if (f[0] == '!') {
        is_negate = true;
        f = f.substr(1);
      }

      if (StringUtil::startswith(f, "--")) {
        f = f.substr(2);
      } else if (StringUtil::startswith(f, "-")) {
        f = f.substr(1);
      }

      assert(!f.empty());
      assert(f[0] != '-');
      if (is_negate) {
        negate_flag_names.push_back(f);
      } else {
        flag_and_option_names.push_back(f);
      }
    }
  }
  template <typename ShortOrLong>
  void hit_impl(ShortOrLong const& flag) {
    hit_count++;
    std::visit(ValueVisitor(*this, flag), value);
  }

 private:
  std::vector<std::string> negate_flag_names{};
};

class Option : public Base {
  friend class ArgParser;

  struct ValueVisitor {
    ValueVisitor(std::string const& opt_val) : opt_val(opt_val) {}
    template <typename T,
              std::enable_if_t<is_option_bindable_value_v<T>, bool> = true>
    std::pair<int, std::string> operator()(T& x) {
      return insert_or_replace_value(x, opt_val);
    }
    template <typename T,
              std::enable_if_t<is_option_bindable_value_v<T>, bool> = true>
    std::pair<int, std::string> operator()(std::reference_wrapper<T>& x) {
      return insert_or_replace_value(x.get(), opt_val);
    }

    std::string const& opt_val;
  };

 public:
  bool is_flag() const override { return false; };
  bool is_option() const override { return true; };

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
        new Option(option_desc, Base::identity<T>{}));
  }

  template <typename T,
            typename = std::enable_if_t<is_option_bindable_value_v<T>>>
  Option(std::string const& option_desc, T& bind)
      : Base(typename Base::identity<T>{}, bind) {
    Option_init(option_desc);
  }
  template <typename T,
            typename = std::enable_if_t<is_option_bindable_value_v<T>>>
  Option(std::string const& option_desc, Base::identity<T>)
      : Base(typename Base::identity<T>{}) {
    Option_init(option_desc);
  }
  std::string usage() const override {
    std::stringstream ss;
    auto it = begin(flag_and_option_names);
    if (it != end(flag_and_option_names)) {
      if (it->length() == 1) {
        ss << "-" << *it;
      } else {
        ss << "--" << *it;
      }
      it++;
    }

    if (it != end(flag_and_option_names)) {
      if (it->length() == 1) {
        ss << ",-" << *it;
      } else {
        ss << ",--" << *it;
      }
      it++;
    }
    if (val_help_msg.empty()) {
      ss << "=<TEXT>";
    } else {
      ss << "=<" << val_help_msg << ">";
    }

    ss << "\n";

    if (!help_msg.empty()) {
      ss << "         " << help_msg << "\n";
    }

    return ss.str();
  }

  std::pair<int, std::string> hit(std::string const&,
                                  std::string const& val) override {
    return hit_impl(val);
  }
  std::pair<int, std::string> hit(char, std::string const& val) override {
    return hit_impl(val);
  }
  void hit(std::string const& flag) override { assert(false); }
  void hit(char const flag) override { assert(false); }

 private:
  void Option_init(std::string const& option_desc) {
    auto all = StringUtil::split(option_desc, ',');
    assert(!all.empty());
    for (auto f : all) {
      if (StringUtil::startswith(f, "--")) {
        f = f.substr(2);
      } else if (StringUtil::startswith(f, "-")) {
        f = f.substr(1);
      }
      assert(!f.empty());
      flag_and_option_names.push_back(f);
    }
  }

  std::pair<int, std::string> hit_impl(std::string const& opt_val) {
    hit_count++;
    return std::visit(ValueVisitor(opt_val), value);
  }

 private:
  std::string val_help_msg;
};

class ArgParser {
  class FlagNotFoundException : public std::exception {
   public:
    const char* what() { return "Flag not found Exception"; }
  };
  class OptionNotFoundException : public std::exception {
   public:
    const char* what() { return "Option not found Exception"; }
  };

  struct PositionValueVisitor {
    PositionValueVisitor(std::string const& opt_val) : opt_val(opt_val) {}
    template <typename T,
              std::enable_if_t<is_option_bindable_value_v<T>, bool> = true>
    std::pair<int, std::string> operator()(T& x) {
      return insert_or_replace_value(x, opt_val);
    }
    template <typename T,
              std::enable_if_t<is_option_bindable_value_v<T>, bool> = true>
    std::pair<int, std::string> operator()(std::reference_wrapper<T>& x) {
      return insert_or_replace_value(x.get(), opt_val);
    }

    std::string const& opt_val;
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
            typename = std::enable_if_t<is_flag_bindable_value_v<T>>>
  Flag& add_flag(std::string const& flag_desc, T& bind) {
    auto x = Flag::make_flag(flag_desc, bind);
    auto p = x.get();
    all_options.push_back(std::move(x));
    return *p;
  }

  template <typename T,
            typename = std::enable_if_t<is_flag_bindable_value_v<T>>>
  Flag& add_flag(std::string const& flag_desc) {
    auto x = Flag::make_flag<T>(flag_desc);
    auto p = x.get();
    all_options.push_back(std::move(x));
    return *p;
  }

  template <typename T,
            typename = std::enable_if_t<is_option_bindable_value_v<T>>>
  Option& add_option(std::string const& option_desc, T& bind) {
    auto x = Option::make_option(option_desc, bind);
    auto p = x.get();
    all_options.push_back(std::move(x));
    return *p;
  }

  template <typename T,
            typename = std::enable_if_t<is_option_bindable_value_v<T>>>
  Option& add_option(std::string const& option_desc) {
    auto x = Option::make_option<T>(option_desc);
    auto p = x.get();
    all_options.push_back(std::move(x));
    return *p;
  }

  template <typename T,
            typename = std::enable_if_t<is_position_bindable_value_v<T>>>
  void add_position_arg(T& bind) {
    position_args.emplace_back(std::ref(bind));
  }
  template <typename T,
            typename = std::enable_if_t<is_position_bindable_value_v<T>>>
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
    for (auto it = begin(all_options); it != end(all_options); ++it) {
      ss << (*it)->usage() << "\n";
    }

    return ss.str();
  }

  std::pair<int, std::string> parse(int argc, const char* argv[]) {
    std::vector<std::string> command_line_args{argv, argv + argc};

    if (command_line_args.size() == 0) {
      return {0, ""};
    }

    auto current = command_line_args.begin();

    if (auto& first = *command_line_args.begin();
        !first.empty() && first[0] != '-') {
      if (program_name.empty()) {
        program_name = first;
      }
      // 0 ==> skip program file path
      current = std::next(command_line_args.begin());
    }

    auto current_position_it = position_args.begin();

    int index = 0;
    while (current != command_line_args.end()) {
      // position args
      // 1. -
      // 2. ---
      // 3. ""
      // 4. xxxx

      auto next = std::next(current);

      auto const& curr_arg = *current;

      if (StringUtil::is_position_arg(curr_arg)) {
        if (current_position_it == position_args.end()) {
          std::stringstream ss;
          ss << "invalid option -- -" << curr_arg;
          return {1, ss.str()};
        }
        if ((*current_position_it).index() == 2) {
          auto const i = curr_arg.find('=');
          if (i == std::string::npos) {
            current_position_it++;
          }
        }

        bool revisit = false;
        std::visit(
            overloaded{
                [&current_position_it,
                 &curr_arg](std::string& x) -> std::pair<int, std::string> {
                  x = curr_arg;
                  current_position_it++;
                  return {0, ""};
                },
                [&current_position_it, &curr_arg](std::vector<std::string>& x)
                    -> std::pair<int, std::string> {
                  x.push_back(curr_arg);
                  return {0, ""};
                },
                [&revisit, &current_position_it,
                 &curr_arg](std::map<std::string, std::string>& x)
                    -> std::pair<int, std::string> {
                  auto const i = curr_arg.find('=');
                  if (i == std::string::npos) {
                    current_position_it++;
                    revisit = true;
                  } else {
                    x.emplace(curr_arg.substr(0, i), curr_arg.substr(i + 1));
                  }
                  return {0, ""};
                },
                [&current_position_it,
                 &curr_arg](std::reference_wrapper<std::string>& x)
                    -> std::pair<int, std::string> {
                  x.get() = curr_arg;
                  current_position_it++;
                  return {0, ""};
                },
                [&current_position_it,
                 &curr_arg](std::reference_wrapper<std::vector<std::string>>& x)
                    -> std::pair<int, std::string> {
                  x.get().push_back(curr_arg);
                  return {0, ""};
                },
                [&revisit, &current_position_it, &curr_arg](
                    std::reference_wrapper<std::map<std::string, std::string>>&
                        x) -> std::pair<int, std::string> {
                  auto const i = curr_arg.find('=');
                  if (i == std::string::npos) {
                    current_position_it++;
                    revisit = true;
                  } else {
                    x.get().emplace(curr_arg.substr(0, i),
                                    curr_arg.substr(i + 1));
                  }
                  return {0, ""};
                },
            },
            (*current_position_it));
        if (revisit) {
          continue;
        }
        current = next;
      } else if (StringUtil::is_dash_dash(curr_arg)) {
        // --
        current = next;
        break;
      } else if (StringUtil::is_short_opt(curr_arg)) {
        // short
        auto short_p = curr_arg.begin() + 1;
        while (short_p != curr_arg.end()) {
          if (auto short_flag = get_flag(*short_p); short_flag.has_value()) {
            (*short_flag)->hit(*short_p);
            short_p++;
          } else if (auto short_option = get_option(*short_p);
                     short_option.has_value()) {
            if (short_p + 1 != curr_arg.end()) {
              if (auto const [ret, err] =
                      (*short_option)
                          ->hit(*short_p,
                                std::string(short_p + 1, curr_arg.end()));
                  ret != 0) {
                return {ret, err};
              }
              short_p = curr_arg.end();
            } else {
              if (next == command_line_args.end()) {
                std::stringstream ss;
                ss << "option requires an argument -- -" << *short_p;
                return {1, ss.str()};
              } else {
                if ((!next->empty() && (*next)[0] == '-')) {
                  std::stringstream ss;
                  ss << "option requires an argument -- -" << *short_p;
                  return {1, ss.str()};
                } else {
                  if (auto const [ret, err] =
                          (*short_option)->hit(*short_p, *next);
                      ret != 0) {
                    return {ret, err};
                  }
                  short_p = curr_arg.end();
                  next = std::next(next);
                }
              }
            }
          } else {
            std::stringstream ss;
            ss << "invalid option -- -" << *short_p;
            return {1, ss.str()};
          }
        }
        current = next;
      } else if (StringUtil::is_long_opt(curr_arg)) {
        // long
        if (auto i = curr_arg.find('=', 2); i != std::string::npos) {
          std::string option = curr_arg.substr(2, i - 2);
          if (auto long_opt = get_option(option); long_opt.has_value()) {
            if (auto const [ret, msg] =
                    (*long_opt)->hit(option, curr_arg.substr(i + 1));
                ret != 0) {
              return {ret, msg};
            }
          } else {
            std::stringstream ss;
            ss << "invalid option -- --" << option;
            return {1, ss.str()};
          }
        } else {
          std::string option = curr_arg.substr(2);
          if (auto long_flag = get_flag(option); long_flag.has_value()) {
            (*long_flag)->hit(option);
          } else if (auto long_opt = get_option(option); long_opt.has_value()) {
            if (std::next(current) == command_line_args.end()) {
              std::stringstream ss;
              ss << "option requires an argument -- --" << option;
              return {1, ss.str()};
            } else if (!next->empty() && (*next)[0] == '-') {
              std::stringstream ss;
              ss << "option requires an argument -- --" << option;
              return {1, ss.str()};
            } else {
              if (auto const [ret, err] = (*long_opt)->hit(option, *next);
                  ret != 0) {
                return {ret, err};
              }
              next = std::next(next);
            }
          } else {
            std::stringstream ss;
            ss << "invalid option -- --" << option;
            return {1, ss.str()};
          }
        }
        current = next;
      } else {
        assert(false);
        // CHECK(false)
      }
    }

    for (; current != command_line_args.end(); current++) {
      auto [code, err_msg] =
          std::visit(PositionValueVisitor(*current), (*current_position_it));
      if (code != 0) {
        return {1, err_msg};
      }
    }
    return {0, ""};
  }

  Base& operator[](std::string const& f) {
    auto it = find_if(begin(all_options), end(all_options),
                      [&f](auto const& f1) { return f1->contains(f); });
    if (it != end(all_options)) {
      return *((*it).get());
    }
    throw FlagNotFoundException{};
  }

  Flag& flag(std::string const& f) {
    auto it = find_if(
        begin(all_options), end(all_options),
        [&f](auto const& f1) { return f1->is_flag() && f1->contains(f); });
    if (it != end(all_options)) {
      return *static_cast<Flag*>((*it).get());
    }
    throw FlagNotFoundException{};
  }

  Option& option(std::string const& opt) {
    auto it = find_if(
        begin(all_options), end(all_options),
        [&opt](auto const& f) { return f->is_option() && f->contains(opt); });
    if (end(all_options) != it) {
      return *static_cast<Option*>((*it).get());
    }
    throw OptionNotFoundException{};
  }

 private:
  std::optional<Flag*> get_flag(char const flag) {
    return get_flag(std::string(1, flag));
  }
  std::optional<Flag*> get_flag(std::string const& flag) {
    auto it = find_if(
        begin(all_options), end(all_options),
        [flag](auto const& f) { return f->is_flag() && f->contains(flag); });
    if (it != end(all_options)) {
      return static_cast<Flag*>((*it).get());
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
      return static_cast<Option*>((*it).get());
    }
    return {};
  }

  std::string description{};
  std::string program_name{};

  std::vector<std::unique_ptr<Base>> all_options{};

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
