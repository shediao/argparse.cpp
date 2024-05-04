
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
//     9. alias
//     10. setopt(short, long) 支持getopt方式
//     11. all api test
//     12. top100 linux command test
//     13. subcommand support
//     14. throw execption

#include <algorithm>
#include <cassert>
#include <charconv>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace argparse {

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
  explicit invalid_argument(std::string const& msg) : std::invalid_argument(msg) {}
};

namespace {

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
    std::enable_if_t<
        (std::is_same_v<bool, T> || std::is_same_v<int, T> ||
         std::is_same_v<double, T> || std::is_same_v<std::string, T>) &&
        (std::is_same_v<bool, U> || std::is_same_v<int, U> ||
         std::is_same_v<double, U> || std::is_same_v<std::string, U>)>>
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

//**********************************
namespace StringUtil {
inline bool startswith(std::string const& str, std::string const& prefix) {
  if (str.length() < prefix.length()) {
    return false;
  }
  auto it = str.begin();
  auto pit = prefix.begin();
  while(pit != prefix.end()) {
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
    std::stringstream ss;
    ss << "'" << from << "' is neither 'true' nor 'false'";
    throw bad_value_access(ss.str());
  }
}

template <typename T,
          std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<bool, T>,
                           bool> = true>
void transform_value(std::string const& from, T& to) {
  auto [ptr, ec] = std::from_chars(from.data(), from.data() + from.size(), to);
  if (ec == std::errc::invalid_argument) {
    std::stringstream ss;
    ss << "'" << from << "' not invalid number";
    throw bad_value_access(ss.str());
  }
  if (ec != std::errc{}) {
    std::stringstream ss;
    ss << "'" << from << "'  can't convert to a number";
    throw bad_value_access(ss.str());
  }
}

template <typename T, std::enable_if_t<std::is_same_v<double, T>, bool> = true>
void transform_value(std::string const& from, T& to) {
  try {
    to = std::stod(from, nullptr);
  } catch (std::invalid_argument const& e) {
    throw invalid_argument(e.what());
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
}  // namespace

class Base {
  friend class ArgParser;

 public:
  enum class Type { FLAG, ALIAS_FLAG, OPTION, POSITIONAL };
  virtual ~Base() = default;
  Base::Type virtual type() const = 0;
  bool is_flag() const { return Type::FLAG == this->type() || is_alias_flag(); };
  bool is_alias_flag() const { return Type::ALIAS_FLAG == this->type(); };
  bool is_option() const { return Type::OPTION == this->type(); };
  bool is_positional() const { return Type::POSITIONAL == this->type(); };

  template <typename T, typename = std::enable_if_t<is_bindable_value_v<T>>>
  T const& as() const {
    if (std::holds_alternative<std::reference_wrapper<T>>(value)) {
      return std::get<std::reference_wrapper<T>>(value).get();
    }
    return std::get<T>(value);
  }

  template <typename T, typename = std::enable_if_t<is_bindable_value_v<T>>>
  Base& bind(T& bind_val) {
    if (is_flag()) {
      if constexpr (is_flag_bindable_value_v<T>) {
        value = std::ref(bind_val);
      } else {
        throw std::bad_variant_access();
      }
    } else if (is_option()) {
      if constexpr (is_option_bindable_value_v<T>) {
        value = std::ref(bind_val);
      } else {
        throw std::bad_variant_access();
      }
      assert(is_option_bindable_value_v<T>);
    } else if (is_positional()) {
      if constexpr (is_position_bindable_value_v<T>) {
        value = std::ref(bind_val);
      } else {
        throw std::bad_variant_access();
      }
    }
    return *this;
  }

  Base& help(std::string const& help) {
    help_msg = help;
    return *this;
  }
  Base& value_help(std::string const& helper) {
    this->value_placeholder = helper;
    return *this;
  }

  // for const char*
  Base& set_default(std::string const& def_val) {
    return set_default<std::string>(def_val);
  }
  template <typename T, typename = std::enable_if_t<is_bindable_value_v<T>>>
  Base& set_default(T const& def_val) {
    std::visit(
        [&def_val](auto& val) {
          using type = std::remove_reference_t<decltype(val)>;
          if constexpr (is_reference_wrapper_v<type>) {
            if constexpr (std::is_assignable_v<decltype(val.get()),
                                               decltype(def_val)>) {
              val.get() = def_val;
            } else {
              throw std::bad_variant_access();
            }
          } else {
            if constexpr (std::is_assignable_v<decltype(val),
                                               decltype(def_val)>) {
              val = def_val;
            } else {
              throw std::bad_variant_access();
            }
          }
        },
        value);
    return *this;
  }

  [[nodiscard]] int count() const { return hit_count; }
  template <typename T>
  struct identity {
    using type = T;
  };

  Base(Base const&) = delete;
  Base(Base&&) = delete;
  Base& operator=(Base const&) = delete;
  Base& operator=(Base &&) = delete;

  using value_type = make_variant_type<bool, int, double, std::string>::type;
 protected:
  template <typename T, typename = std::enable_if_t<is_bindable_value_v<T>>>
  Base(identity<T> /*unused*/, T& bind) : value(std::ref(bind)) {}

  template <typename T, typename = std::enable_if_t<is_bindable_value_v<T>>>
  explicit Base(identity<T> /*unused*/) : value(T{}) {}

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
  std::vector<std::string> flag_and_option_names;
  std::string help_msg;
  std::string value_placeholder;
  value_type value;
  int hit_count{0};
};
class ArgParser;
class Flag : public Base {
  friend class ArgParser;
 public:
  Base::Type type() const override { return Base::Type::FLAG; };

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
  Flag(std::string const& flag_desc, Base::identity<T> /*unused*/)
      : Base(typename Base::identity<T>{}) {
    Flag_init(flag_desc);
  }
  [[nodiscard]] bool negate_contains(std::string const& flag) const {
    return find(begin(negate_flag_names), end(negate_flag_names), flag) !=
           end(negate_flag_names);
  }

  [[nodiscard]] bool contains(std::string const& flag) const override {
    return Base::contains(flag) || negate_contains(flag);
  }

  void hit(char const flag) override { hit(std::string(1, flag)); }
  void hit(std::string const& flag) override { hit_impl(flag); }
  void hit(char const flag, std::string const& val) override {
    hit(std::string(1, flag), val);
  }
  void hit(std::string const&  /*flag*/, std::string const&  /*val*/) override {
    assert(false);
  }

  [[nodiscard]] std::string usage() const override {
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
  [[nodiscard]] std::string short_usage() const override {
    return "";
  }

 private:
  void Flag_init(std::string const& flag_desc) {
    // TODO(shediao): 
    // flag is ,
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
    std::visit(overloaded{
        [this, &flag](bool& val){
          if (negate_contains(flag)) {
            val = false;
          } else if (Base::contains(flag)) {
            val = true;
          } else {
            // throw execption
          }
        },
        [this, &flag](std::reference_wrapper<bool>& val){
          if (negate_contains(flag)) {
            val.get() = false;
          } else if (Base::contains(flag)) {
            val.get() = true;
          } else {
            // throw execption
          }
        },
        [this, &flag](auto& val){
          using type = std::remove_reference_t<decltype(val)>;
          if constexpr (is_reference_wrapper_v<type>) {
            if constexpr (is_flag_bindable_value_v<typename type::type>) {
              if (negate_contains(flag)) {
                val.get() -= 1;
              } else if (Base::contains(flag)) {
                val.get() += 1;
              } else {
                assert(false);
                //TODO(shediao): throw execption
              }
            } else {
              assert(false);
              //TODO(shediao): throw execption
            }
          } else {
            if constexpr (is_flag_bindable_value_v<type>) {
              if (negate_contains(flag)) {
                val -= 1;
              } else if (Base::contains(flag)) {
                val += 1;
              } else {
                assert(false);
                //TODO(shediao): throw execption
              }
            } else {
              assert(false);
              //TODO(shediao): throw execption
            }
          }
        }
        }, value);
  }

  std::vector<std::string> negate_flag_names{};
};

class AliasFlag : public Flag {
  friend class ArgParser;

 public:
  Base::Type type() const override { return Base::Type::ALIAS_FLAG; };

 protected:
  std::unique_ptr<AliasFlag> static make_flag(std::string const& flag_desc,
                                              std::string const& option_name,
                                              std::string const& option_value) {
    return std::unique_ptr<AliasFlag>(
        new AliasFlag(flag_desc, option_name, option_value));
  }
  explicit AliasFlag(std::string const& flag_desc,
                     std::string const& option_name,
                     std::string const& option_value)
      : Flag(flag_desc, Base::identity<bool>{}),
        option_name(option_name),
        option_value(option_value) {}
  void hit(char const flag) override { Flag::hit(flag); }
  void hit(std::string const& flag) override { Flag::hit(flag); }

 private:
  std::string option_name;
  std::string option_value;
};

class Option : public Base {
  friend class ArgParser;

 public:
  Base::Type type() const override { return Base::Type::OPTION; };

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
  [[nodiscard]] std::string usage() const override {
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
    if (value_placeholder.empty()) {
      ss << "=<TEXT>";
    } else {
      ss << "=<" << value_placeholder << ">";
    }

    ss << "\n";

    if (!help_msg.empty()) {
      ss << "         " << help_msg << "\n";
    }

    return ss.str();
  }
  [[nodiscard]] std::string short_usage() const override {
    return "";
  }

  void hit(std::string const&, std::string const& val) override {
    return hit_impl(val);
  }
  void hit(char, std::string const& val) override { return hit_impl(val); }
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

  void hit_impl(std::string const& opt_val) {
    hit_count++;
    std::visit(
        [&opt_val, this](auto& x) {
          using T = std::remove_reference_t<decltype(x)>;
          if constexpr (is_reference_wrapper_v<T>) {
            insert_or_replace_value(x.get(), opt_val, delimiter);
          } else {
            insert_or_replace_value(x, opt_val, delimiter);
          }
        },
        value);
  }

 private:
  char delimiter{'\0'};
};

class Positional : public Base {
  friend class ArgParser;

 public:
  Base::Type type() const override { return Base::Type::POSITIONAL; };

 protected:
  template <typename T>
  static std::unique_ptr<Positional> make_positional(std::string const& name,
                                                     T& bind) {
    return std::unique_ptr<Positional>(new Positional(name, bind));
  }
  template <typename T>
  static std::unique_ptr<Positional> make_positional(std::string const& name) {
    return std::unique_ptr<Positional>(
        new Positional(name, Base::identity<T>{}));
  }

  void hit(char short_name) override { assert(false); };
  void hit(std::string const& long_name) override { assert(false); };
  void hit(char short_name, std::string const& val) override { assert(false); };
  void hit(std::string const& long_name, std::string const& val) override {
    std::visit(overloaded{[&val, this](auto& v) {
                 using type = std::remove_reference_t<decltype(v)>;
                 if constexpr (is_reference_wrapper_v<type>) {
                   insert_or_replace_value(v.get(), val, delimiter);
                   if constexpr (!is_option_bindable_container_v<
                                     typename type::type>) {
                     can_set_value = false;
                   }
                 } else {
                   insert_or_replace_value(v, val, delimiter);
                   if constexpr (!is_option_bindable_container_v<type>) {
                     can_set_value = false;
                   }
                 }
               }},
               value);
  };

  [[nodiscard]] std::string usage() const override { return ""; };
  [[nodiscard]] std::string short_usage() const override {
    return "";
  }

 protected:
  template <typename T>
  Positional(std::string const& name, T& bind)
      : Base(Base::identity<T>{}, bind) {
    flag_and_option_names.push_back(name);
  }

  template <typename T>
  Positional(std::string const& name, Base::identity<T>)
      : Base(Base::identity<T>{}) {
    flag_and_option_names.push_back(name);
  }
  bool can_set_value{true};
  char delimiter{'\0'};
};

class ArgParser {
  class FlagNotFoundException : public std::exception {
   public:
    const char* what() const noexcept { return "Flag not found Exception"; }
  };
  class OptionNotFoundException : public std::exception {
   public:
    const char* what() const noexcept { return "Option not found Exception"; }
  };
  class PositionalFoundException : public std::exception {
   public:
    const char* what() const noexcept { return "Positional not found Exception"; }
  };

  struct PositionValueVisitor {
    PositionValueVisitor(std::string const& opt_val) : opt_val(opt_val) {}
    template <typename T,
              std::enable_if_t<is_option_bindable_value_v<T>, bool> = true>
    void operator()(T& x) {
      insert_or_replace_value(x, opt_val);
    }
    template <typename T,
              std::enable_if_t<is_option_bindable_value_v<T>, bool> = true>
    void operator()(std::reference_wrapper<T>& x) {
      insert_or_replace_value(x.get(), opt_val);
    }

    std::string const& opt_val;
  };

 public:
  ArgParser() = default;
  explicit ArgParser(std::string const& desc)
      : description(desc) {}

  ArgParser& set_program_name(std::string const& programName) {
    this->program_name = programName;
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

  template <typename T = bool,
            typename = std::enable_if_t<is_flag_bindable_value_v<T>>>
  Flag& add_flag(std::string const& flag_desc) {
    auto x = Flag::make_flag<T>(flag_desc);
    auto p = x.get();
    all_options.push_back(std::move(x));
    return *p;
  }
  AliasFlag& add_alias_flag(std::string const& flag_desc,
                            std::string const& option_name,
                            std::string const& option_value) {
    auto x = AliasFlag::make_flag(flag_desc, option_name, option_value);
    auto *p = x.get();
    all_options.push_back(std::move(x));
    return *p;
  }

  template <typename T,
            typename = std::enable_if_t<!is_need_split_v<T> &&
                                        is_option_bindable_value_v<T>>>
  Option& add_option(std::string const& option_desc, T& bind) {
    auto x = Option::make_option(option_desc, bind);
    auto p = x.get();
    all_options.push_back(std::move(x));
    return *p;
  }

  template <typename T = std::string,
            typename = std::enable_if_t<!is_need_split_v<T> &&
                                        is_option_bindable_value_v<T>>>
  Option& add_option(std::string const& option_desc) {
    auto x = Option::make_option<T>(option_desc);
    auto p = x.get();
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
    all_options.push_back(std::move(x));
    return *p;
  }
  template <typename T = std::vector<std::string>,
            typename = std::enable_if_t<!is_need_split_v<T> &&
                                        is_position_bindable_value_v<T>>>
  Positional& add_positional(std::string const& name) {
    auto x = Positional::make_positional<T>(name);
    auto p = x.get();
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
    std::stringstream ss;
    ss << "\n" << (program_name.empty() ? "?" : program_name) << " [OPTION]... ";
    if (!any_of(begin(all_options), end(all_options),
                [](auto& o) { return o->is_positional(); })) {
      ss << " [--] [args....]";
    }
    ss << "\n";
    ss << description << "\n\n";
    for (auto& all_option : all_options) {
      ss << all_option->usage() << "\n";
    }

    return ss.str();
  }

  void set_unknown_option_as_start_of_positionals() {
    unknown_option_as_start_of_positionals = true;
  }

  std::pair<int, std::string> parse(int argc, const char* argv[]) {
    std::vector<std::string> command_line_args{argv, argv + argc};

    if (command_line_args.empty()) {
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

    auto current_position_it =
        find_if(begin(all_options), end(all_options),
                [](auto& o) { return o->is_positional(); });

    while (current != command_line_args.end()) {
      // position args
      // 1. -
      // 2. ---
      // 3. ""
      // 4. xxxx

      auto next = std::next(current);

      auto const& curr_arg = *current;

      if (StringUtil::is_position_arg(curr_arg)) {
        while (!((Positional*)current_position_it->get())->can_set_value &&
               current_position_it != all_options.end()) {
          current_position_it =
              find_if(std::next(current_position_it), end(all_options),
                      [](auto& o) { return o->is_positional(); });
        }
        if (current_position_it == all_options.end()) {
          std::stringstream ss;
          ss << "invalid option -- -" << curr_arg;
          return {1, ss.str()};
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
                assert(false);
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
                std::stringstream ss;
                ss << "option requires an argument -- -" << *short_p;
                return {1, ss.str()};
              } else {
                if ((!next->empty() && (*next)[0] == '-')) {
                  std::stringstream ss;
                  ss << "option requires an argument -- -" << *short_p;
                  return {1, ss.str()};
                } else {
                  (*short_option)->hit(*short_p, *next);
                  short_p = curr_arg.end();
                  next = std::next(next);
                }
              }
            }
          } else {
            if (unknown_option_as_start_of_positionals) {
              break;
            } else {
              std::stringstream ss;
              ss << "invalid option -- -" << *short_p;
              return {1, ss.str()};
            }
          }
        }
        current = next;
      } else if (StringUtil::is_long_opt(curr_arg)) {
        // long
        if (auto i = curr_arg.find('=', 2); i != std::string::npos) {
          std::string option = curr_arg.substr(2, i - 2);
          if (auto long_opt = get_option(option); long_opt.has_value()) {
            (*long_opt)->hit(option, curr_arg.substr(i + 1));
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
              (*long_opt)->hit(option, *next);
              next = std::next(next);
            }
          } else {
            if (unknown_option_as_start_of_positionals) {
              break;
            } else {
              std::stringstream ss;
              ss << "invalid option -- --" << option;
              return {1, ss.str()};
            }
          }
        }
        current = next;
      } else {
        assert(false);
        // CHECK(false)
      }
    }

    for (; current != command_line_args.end(); current++) {
      while (!((Positional*)current_position_it->get())->can_set_value &&
             current_position_it != all_options.end()) {
        current_position_it =
            find_if(std::next(current_position_it), end(all_options),
                    [](auto& o) { return o->is_positional(); });
      }
      if (current_position_it == all_options.end()) {
        std::stringstream ss;
        ss << "invalid option -- -" << *current;
        return {1, ss.str()};
      }
      (*current_position_it)->hit("", *current);
    }
    return {0, ""};
  }

  std::optional<Base*> get(std::string const& f) {
    auto it = find_if(begin(all_options), end(all_options),
                      [&f](auto const& f1) { return f1->contains(f); });
    if (it != end(all_options)) {
      return (*it).get();
    }
    return std::nullopt;
  }

  Base& operator[](std::string const& f) {
    auto x = get(f);
    if (x.has_value()) {
      return *(x.value());
    }
    throw FlagNotFoundException{};
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

  std::vector<std::unique_ptr<Base>> all_options{};
};

}  // namespace argparse

#endif  // ARGPARSE_CPP_H_
