/*
 * Copyright (c) 2017-2020, Wen Xiongchang <udc577 at 126 dot com>
 * All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must
 * not claim that you wrote the original software. If you use this
 * software in a product, an acknowledgment in the product documentation
 * would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 */

// NOTE: The original author also uses (short/code) names listed below,
//       for convenience or for a certain purpose, at different places:
//       wenxiongchang, wxc, Damon Wen, udc577

#include "common_headers.h"

#include "ca_string.h"

TEST(ca_string, split)
{
#define PRINT_STR_FRAGMENTS(str_holder)         \
        for (size_t i = 0; i < str_holder.size(); ++i) \
        {\
            printf(#str_holder"[%lu]: %s\n", i, str_holder[i].c_str()); \
        }

    const char *DELIM_SLASH = "/";
    const char *DELIM_COMMA = ",";
    std::vector<std::string> result;

    ASSERT_EQ(CA_RET(OK), calib::str::split(DELIM_SLASH, strlen(DELIM_SLASH), DELIM_SLASH, result));
    ASSERT_EQ(0, result.size());
    ASSERT_EQ(CA_RET(OK), calib::str::split("/foo", strlen("/foo"), DELIM_SLASH, result));
    ASSERT_EQ(1, result.size());
    ASSERT_EQ(CA_RET(OK), calib::str::split("foo/", strlen("foo/"), DELIM_SLASH, result));
    ASSERT_EQ(1, result.size());

    /*
     * Tests a short string.
     */

    const char *SHORT_STR = "/usr/local/include/cpp_assistant/";
    const int SHORT_STR_LEN = strlen(SHORT_STR);

#if 0 // TODO: will crash if using gcc 7.3
    ASSERT_EQ(CA_RET(INVALID_PARAM_VALUE), calib::str::split(nullptr, 0, nullptr, result));
    ASSERT_EQ(0, result.size());
    ASSERT_EQ(CA_RET(INVALID_PARAM_VALUE), calib::str::split(SHORT_STR, -SHORT_STR_LEN, nullptr, result));
    ASSERT_EQ(0, result.size());
    ASSERT_EQ(CA_RET(INVALID_PARAM_VALUE), calib::str::split(SHORT_STR, 0, nullptr, result));
    ASSERT_EQ(0, result.size());
    ASSERT_EQ(CA_RET(INVALID_PARAM_VALUE), calib::str::split(SHORT_STR, SHORT_STR_LEN, nullptr, result));
    ASSERT_EQ(0, result.size());
#endif
    ASSERT_EQ(CA_RET(OK), calib::str::split(SHORT_STR, SHORT_STR_LEN, DELIM_SLASH, result));
    ASSERT_EQ(4, result.size());
    PRINT_STR_FRAGMENTS(result);
    ASSERT_EQ(CA_RET(OK), calib::str::split(SHORT_STR, SHORT_STR_LEN, DELIM_COMMA, result));
    ASSERT_EQ(1, result.size());
    PRINT_STR_FRAGMENTS(result);

    /*
     * Tests a long string.
     */

    const int LONG_STR_CAPACITY = CA_MAX_LEN_IN_STACK + 32;
    char *long_str = (char*)calloc(LONG_STR_CAPACITY, sizeof(char));
    int long_str_len = 0;

    ASSERT_TRUE(nullptr != long_str);

    const char *NODE_1 = "哈哈";
    const char *NODE_2 = "呵呵";
    const int NODE_LEN = strlen(NODE_1);
    const int NODE_COUNT = CA_MAX_LEN_IN_STACK / (NODE_LEN + strlen(DELIM_SLASH)) + 1;

    for (int i = 0; i < NODE_COUNT; ++i)
    {
        strcat(long_str, DELIM_SLASH);
        strcat(long_str, (0 == i % 2) ? NODE_1 : NODE_2);
    }
    long_str_len = strlen(long_str);
    ASSERT_GE(long_str_len, CA_MAX_LEN_IN_STACK);

    ASSERT_EQ(CA_RET(OK), calib::str::split(long_str, long_str_len, DELIM_SLASH, result));
    ASSERT_EQ(NODE_COUNT, result.size());
    for (int i = 0; i < NODE_COUNT; ++i)
    {
        if (0 == i % 2)
            ASSERT_STREQ(NODE_1, result[i].c_str());
        else
            ASSERT_STREQ(NODE_2, result[i].c_str());
    }
    PRINT_STR_FRAGMENTS(result);
    ASSERT_EQ(CA_RET(OK), calib::str::split(long_str, long_str_len, DELIM_COMMA, result));
    ASSERT_EQ(1, result.size());
    ASSERT_GE(result[0].length(), CA_MAX_LEN_IN_STACK);
    PRINT_STR_FRAGMENTS(result);

    free(long_str);

    printf("NOTE: Re-run this test with valgrind to check if there are any memory leakages.\n");
}

TEST(ca_string, get_directory)
{
    const char *path = "path";
    int path_len = strlen(path);
    char result[4] = {0};

#if 0 // TODO: will crash if using gcc 7.3
    ASSERT_EQ(CA_RET(INVALID_PARAM_VALUE), calib::str::get_directory(nullptr, path_len, result));
    ASSERT_EQ(CA_RET(INVALID_PARAM_VALUE), calib::str::get_directory(path, 0, result));
    ASSERT_EQ(CA_RET(INVALID_PARAM_VALUE), calib::str::get_directory(path, path_len, nullptr));
    ASSERT_EQ(CA_RET(INVALID_PARAM_VALUE), calib::str::get_directory(nullptr, 0, nullptr));
#endif
    ASSERT_EQ(strlen("."), calib::str::get_directory(path, path_len, result));
    ASSERT_STREQ(result, ".");

    ASSERT_STREQ(calib::str::get_directory(".", strlen(".")).c_str(), ".");
    ASSERT_STREQ(calib::str::get_directory("./", strlen("./")).c_str(), ".");
    ASSERT_STREQ(calib::str::get_directory("./a", strlen("./a")).c_str(), ".");
    ASSERT_STREQ(calib::str::get_directory("./a/", strlen("./a/")).c_str(), "./a");
    ASSERT_STREQ(calib::str::get_directory("..", strlen("..")).c_str(), "..");
    ASSERT_STREQ(calib::str::get_directory("../", strlen("../")).c_str(), "..");
    ASSERT_STREQ(calib::str::get_directory("../b/a", strlen("../b/a")).c_str(), "../b");
    ASSERT_STREQ(calib::str::get_directory("../b/a/", strlen("../b/a/")).c_str(), "../b/a");
    ASSERT_STREQ(calib::str::get_directory("/", strlen("/")).c_str(), "/");
    ASSERT_STREQ(calib::str::get_directory("/usr/local/lib/", strlen("/usr/local/lib/")).c_str(), "/usr/local/lib");
    ASSERT_STREQ(calib::str::get_directory("/usr/local/lib/liba.so", strlen("/usr/local/lib/liba.so")).c_str(), "/usr/local/lib");
}

TEST(ca_string, split_dir_and_basename)
{
    std::string path;
    std::pair<std::string, std::string> result;

    result = calib::str::split_dir_and_basename(path);
    ASSERT_STREQ(result.first.c_str(), "");
    ASSERT_STREQ(result.second.c_str(), "");

    path = "/";
    result = calib::str::split_dir_and_basename(path);
    ASSERT_STREQ(result.first.c_str(), "/");
    ASSERT_STREQ(result.second.c_str(), "");

    path = ".";
    result = calib::str::split_dir_and_basename(path);
    ASSERT_STREQ(result.first.c_str(), ".");
    ASSERT_STREQ(result.second.c_str(), "");

    path = "./";
    result = calib::str::split_dir_and_basename(path);
    ASSERT_STREQ(result.first.c_str(), ".");
    ASSERT_STREQ(result.second.c_str(), "");

    path = "..";
    result = calib::str::split_dir_and_basename(path);
    ASSERT_STREQ(result.first.c_str(), "..");
    ASSERT_STREQ(result.second.c_str(), "");

    path = "../";
    result = calib::str::split_dir_and_basename(path);
    ASSERT_STREQ(result.first.c_str(), "..");
    ASSERT_STREQ(result.second.c_str(), "");

    path = ".xx";
    result = calib::str::split_dir_and_basename(path);
    ASSERT_STREQ(result.first.c_str(), ".");
    ASSERT_STREQ(result.second.c_str(), ".xx");

    path = "xx.";
    result = calib::str::split_dir_and_basename(path, '/', ".");
    ASSERT_STREQ(result.first.c_str(), ".");
    ASSERT_STREQ(result.second.c_str(), "xx");

    path = "/home/xx.txt";
    result = calib::str::split_dir_and_basename(path);
    ASSERT_STREQ(result.first.c_str(), "/home");
    ASSERT_STREQ(result.second.c_str(), "xx.txt");
    result = calib::str::split_dir_and_basename(path, '/', ".txt");
    ASSERT_STREQ(result.first.c_str(), "/home");
    ASSERT_STREQ(result.second.c_str(), "xx");
    result = calib::str::split_dir_and_basename(path, '/', ".*");
    ASSERT_STREQ(result.first.c_str(), "/home");
    ASSERT_STREQ(result.second.c_str(), "xx");
    result = calib::str::split_dir_and_basename(path, '/', ".TXT");
    ASSERT_STREQ(result.first.c_str(), "/home");
    ASSERT_STREQ(result.second.c_str(), "xx.txt");
    result = calib::str::split_dir_and_basename(path, '/', ".TXT", false);
    ASSERT_STREQ(result.first.c_str(), "/home");
    ASSERT_STREQ(result.second.c_str(), "xx");
}

TEST(ca_string, absolute_path_apis)
{
    const std::string &cmd_program_path = get_commandline_arguments()[0];
    const std::string &fixed_absolute_path = calib::str::get_absolute_path(cmd_program_path.c_str());
    const std::string &self_absolute_path = calib::str::get_self_absolute_path();

    printf("Program path fetched from command line: %s\n", cmd_program_path.c_str());
    printf("Path after being fixed by calib::str::get_absolute_path(): %s\n", fixed_absolute_path.c_str());
    printf("Program path fetched using calib::str::get_self_absolute_path() directly: %s\n", self_absolute_path.c_str());

    ASSERT_STREQ(fixed_absolute_path.c_str(), self_absolute_path.c_str());
    //ASSERT_STREQ(calib::str::get_absolute_path(nullptr).c_str(), "");  // TODO: will crash if using gcc 7.3
}

