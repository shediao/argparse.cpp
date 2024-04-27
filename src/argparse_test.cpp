#include "argparse.h"
#include <gtest/gtest.h>

TEST(ArgParser, parse0){
    argparse::ArgParser parser1;
    std::vector<const char*> cmd{"test0"};
    auto [code, msg] = parser1.parse(cmd.size(), cmd.data());
    EXPECT_EQ(code, 0);
    EXPECT_TRUE(msg.empty());

}
TEST(ArgParser, parse0_1){
    argparse::ArgParser parser1;
    std::vector<const char*> cmd{"test0", "-f"};
    auto [code, msg] = parser1.parse(cmd.size(), cmd.data());
    EXPECT_EQ(code, 1);
    EXPECT_EQ("invalid option -- -f", msg);

}
TEST(ArgParser, parse0_2){
    argparse::ArgParser parser1;
    std::vector<const char*> cmd{"test0", "--"};
    auto [code, msg] = parser1.parse(cmd.size(), cmd.data());
    EXPECT_EQ(code, 0);
    EXPECT_TRUE(msg.empty());

}

TEST(ArgParser, parser0_3) {
    bool flag_a{false};
    bool flag_b{false};
    bool flag_c{false};
    bool flag_d{false};
    bool flag_h{false};
    bool flag_1{false};
    bool flag_2{false};
    bool flag_3{false};
    int flag_v{false};
    std::map<std::string, std::string> option_e;
    std::vector<std::string> option_f;
    std::string option_o;

    argparse::ArgParser parser;
    parser.add_flag("-a",flag_a);
    parser.add_flag("-b", flag_b);
    parser.add_flag("-c", flag_c);
    parser.add_flag("-d", flag_d);
    parser.add_flag("-h,--help", flag_h);
    parser.add_flag("-1", flag_1);
    parser.add_flag("-2", flag_2);
    parser.add_flag("-3", flag_3);
    parser.add_flag("-v,--verbose", flag_v);
    parser.add_option("-e,--env", option_e);
    parser.add_option("-f,--file", option_f);
    parser.add_option("-o,--output", option_o);

    std::vector<const char*> cmd2{ "test1", "-1", "-b", "-2", "-3", "-o", "8", "-f", "myfile", "--file", "/path/to/myfile", "-e", "xxx=yyy", "-e", "e1=v1", "-e", "e2=v2", "-vvvv"};
    auto [code, error_msg] = parser.parse(cmd2.size(), cmd2.data());
    ASSERT_EQ(code, 0);
    EXPECT_EQ(flag_a, false);
    EXPECT_EQ(flag_b, true);
    EXPECT_EQ(flag_c, false);
    EXPECT_EQ(flag_d, false);
    EXPECT_EQ(flag_h,  false);
    EXPECT_EQ(flag_1, true);
    EXPECT_EQ(flag_2, true);
    EXPECT_EQ(flag_3, true);
    EXPECT_EQ(flag_v, 4);

    EXPECT_EQ(3, option_e.size());
    EXPECT_TRUE(option_e.find("e1") != option_e.end());
    EXPECT_TRUE(option_e.find("e2") != option_e.end());
    EXPECT_TRUE(option_e.find("xxx") != option_e.end());
    EXPECT_EQ(option_e.at("e1"), "v1");
    EXPECT_EQ(option_e.at("e2"), "v2");
    EXPECT_EQ(option_e.at("xxx"), "yyy");

    EXPECT_EQ("8", option_o);


    EXPECT_EQ(2, option_f.size());
    EXPECT_EQ("myfile", option_f[0]);
    EXPECT_EQ("/path/to/myfile", option_f[1]);

}

TEST(ArgParser, negate){
    argparse::ArgParser parser;
    bool is_release{true};
    parser.add_flag("-r,--release,!-d,!--debug", is_release);
    std::vector<const char*> cmd{"test0", "--debug", "--release", "-d"};
    auto [code, msg] = parser.parse(cmd.size(), cmd.data());
    EXPECT_EQ(code, 0) << msg;

    EXPECT_FALSE(is_release);
}
