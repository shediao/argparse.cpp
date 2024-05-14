#include "type_info.hpp"

#include <gtest/gtest.h>
#include <tuple>
#include <type_traits>
#include <variant>
#include "argparse.hpp"

using namespace argparse;

TEST(TypeInfo, ConcatVariant) {
  static_assert(std::is_same_v<concat_variant_t<std::variant<bool, int>>,
                               std::variant<bool, int>>);
  static_assert(
      std::is_same_v<
          concat_variant_t<std::variant<bool, int>, std::variant<long, double>>,
          std::variant<bool, int, long, double>>);

  static_assert(
      std::is_same_v<
          concat_variant_t<std::variant<bool, int>, std::variant<long, double>,
                           std::variant<char, std::string>>,
          std::variant<bool, int, long, double, char, std::string>>);
}

TEST(TypeInfo, make_pairs_variant) {
  using namespace std;
  static_assert(is_same_v<make_pairs_variant_t<int, bool>,
                          variant<pair<int, int>, pair<int, bool>,
                                  pair<bool, int>, pair<bool, bool>>>);

  static_assert(
      is_same_v<make_pairs_variant_t<int, bool, long>,
                variant<pair<int, int>, pair<int, bool>, pair<int, long>,
                        pair<bool, int>, pair<long, int>, pair<bool, bool>,
                        pair<bool, long>, pair<long, bool>, pair<long, long>>>);
}

TEST(TypeInfo, make_map_variant) {
  using namespace std;
  static_assert(is_same_v<make_map_variant_t<int, bool>,
                          variant<map<int, int>, map<int, bool>, map<bool, int>,
                                  map<bool, bool>>>);

  static_assert(
      is_same_v<make_map_variant_t<int, bool, long>,
                variant<map<int, int>, map<int, bool>, map<int, long>,
                        map<bool, int>, map<long, int>, map<bool, bool>,
                        map<bool, long>, map<long, bool>, map<long, long>>>);
}

TEST(TypeInfo, make_vector_variant) {
  using namespace std;
  static_assert(
      is_same_v<make_vector_variant_t<std::variant<int, long, double>>,
                std::variant<std::vector<int>, vector<long>, vector<double>>>);
}

TEST(TypeInfo, make_variant) {
  using namespace std;
  static_assert(
      std::variant_size_v<make_variant_t<bool, int, double, string>> == 56);
  static_assert(
      is_same_v<
          make_variant_t<bool, int, double>,
          std::variant<bool, int, double, pair<bool, bool>, pair<bool, int>,
                       pair<bool, double>, pair<int, bool>, pair<double, bool>,
                       pair<int, int>, pair<int, double>, pair<double, int>,
                       pair<double, double>, map<bool, bool>, map<bool, int>,
                       map<bool, double>, map<int, bool>, map<double, bool>,
                       map<int, int>, map<int, double>, map<double, int>,
                       map<double, double>, vector<bool>, vector<int>,
                       vector<double>, vector<pair<bool, bool>>,
                       vector<pair<bool, int>>, vector<pair<bool, double>>,
                       vector<pair<int, bool>>, vector<pair<double, bool>>,
                       vector<pair<int, int>>, vector<pair<int, double>>,
                       vector<pair<double, int>>, vector<pair<double, double>>>

          >);
}
