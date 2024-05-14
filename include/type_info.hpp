#ifndef _ARGPARSE_TYPE_INFO_
#define _ARGPARSE_TYPE_INFO_
#include <map>
#include <variant>
#include <vector>
namespace argparse {

template <typename... T>
struct concat_variant;

template <typename... T1>
struct concat_variant<std::variant<T1...>> {
  using type = std::variant<T1...>;
};
template <typename... T1, typename... T2>
struct concat_variant<std::variant<T1...>, std::variant<T2...>> {
  using type = std::variant<T1..., T2...>;
};

template <typename... T1, typename... T2>
struct concat_variant<std::variant<T1...>, T2...> {
  using type =
      typename concat_variant<std::variant<T1...>,
                              typename concat_variant<T2...>::type>::type;
};

template <typename... T>
using concat_variant_t = typename concat_variant<T...>::type;

template <typename... T>
struct make_pairs_variant;
template <typename T>
struct make_pairs_variant<T> {
  using type = std::variant<std::pair<T, T>>;
};
template <typename T, typename... R>
struct make_pairs_variant<T, R...> {
  using type = concat_variant_t<
      std::variant<std::pair<T, T>, std::pair<T, R>..., std::pair<R, T>...>,
      typename make_pairs_variant<R...>::type>;
};
template <typename... T>
using make_pairs_variant_t = typename make_pairs_variant<T...>::type;

template <typename... T>
struct make_map_variant;
template <typename T>
struct make_map_variant<T> {
  using type = std::variant<std::map<T, T>>;
};
template <typename T, typename... R>
struct make_map_variant<T, R...> {
  using type = concat_variant_t<
      std::variant<std::map<T, T>, std::map<T, R>..., std::map<R, T>...>,
      typename make_map_variant<R...>::type>;
};
template <typename... T>
using make_map_variant_t = typename make_map_variant<T...>::type;

template <typename T>
struct make_vector_variant;
template <typename... T>
struct make_vector_variant<std::variant<T...>> {
  using type = std::variant<std::vector<T>...>;
};
template <typename T>
using make_vector_variant_t = typename make_vector_variant<T>::type;

template <typename... T>
struct make_variant {
  using single_variant = std::variant<T...>;
  using pairs_varint = make_pairs_variant_t<T...>;
  using map_variant = make_map_variant_t<T...>;
  using vector_variant =
      make_vector_variant_t<concat_variant_t<single_variant, pairs_varint>>;

  using type = concat_variant_t<single_variant,
                                pairs_varint,
                                map_variant,
                                vector_variant>;
};

template <typename... T>
using make_variant_t = typename make_variant<T...>::type;

}  // namespace argparse
#endif  // _ARGPARSE_TYPE_INFO_
