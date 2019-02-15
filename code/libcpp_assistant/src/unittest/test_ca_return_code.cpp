/*
 * Copyright (c) 2017-2019, Wen Xiongchang <udc577 at 126 dot com>
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

#include <errno.h>
#include <string.h>

#include <fstream>

#include "common_headers.h"

#include "private/debug.h"
#include "base/ca_return_code.h"
#include "private/ca_return_code.h"

#define RET_SRC_FILE                "../native/base/ca_return_code.cpp"
#define RET_DESC_FILE               "/tmp/retcode_desc.txt"

TEST(ca_return_code, ReturnNumberDefinitions)
{
    ASSERT_EQ(CA_RET_OK, CA_RET(OK));
    ASSERT_EQ(CA_RET_OK, 0);

    ASSERT_EQ(CA_RET_GENERAL_FAILURE, CA_RET(GENERAL_FAILURE));
    ASSERT_EQ(CA_RET_OPERATION_NOT_PERMITTED, CA_RET(OPERATION_NOT_PERMITTED));
    ASSERT_EQ(CA_RET_GENERAL_FAILURE, CA_RET_OPERATION_NOT_PERMITTED);
    ASSERT_TRUE(CA_RET_GENERAL_FAILURE < 0);

    ASSERT_EQ(abs(calib::SYS_RET_CODE_BEGIN), abs(CA_RET_GENERAL_FAILURE) + 1);

    ASSERT_TRUE(calib::USER_RET_CODE_BEGIN < calib::SYS_RET_CODE_BEGIN);
    ASSERT_EQ(calib::USER_RET_CODE_BEGIN, CA_RET(NULL_PARAM));

    ASSERT_TRUE(calib::USER_RET_CODE_COUNT > 0);
    ASSERT_EQ(calib::USER_RET_CODE_COUNT, abs(calib::USER_RET_CODE_END - calib::USER_RET_CODE_BEGIN));

    ASSERT_EQ(abs(calib::USER_RET_CODE_END), abs(CA_RET(TARGET_NOT_READY)) + 1); // TODO: this will change everytime a new code is added.
}

static void test_invalid_conditions(void)
{
    char msg[512] = {0};
    const int MIN_BUF_LEN = MIN_RET_BUF_LEN();
    const int EXPECTED_FAILURE_CODE = -1;
    const int TESTED_RETCODE = 0;

    /*
     * validates @msg_capacity
     */
    for (int i = -1; i < MIN_BUF_LEN; ++i)
    {
        ASSERT_EQ(EXPECTED_FAILURE_CODE, calib::parse_return_code(TESTED_RETCODE, i, msg));
    }
}

static int generate_retcode_description_file(void)
{
    char cmd[1024] = {0};

    snprintf(cmd, sizeof(cmd), "grep \"S_DESCRIPTIONS\\[USER_RET_CODE_COUNT\\]\" " RET_SRC_FILE " -A %d"
        " | grep \\\" | awk -F \\\" '{ print $2 }' > " RET_DESC_FILE, calib::USER_RET_CODE_COUNT);

    //printf("generating %s: %s\n", RET_DESC_FILE, cmd);
    system(cmd);

    if (access(RET_DESC_FILE, R_OK) < 0)
    {
        fprintf(stderr, "failed to generate description file[%s]\n", RET_DESC_FILE);
        return -1;
    }

    return 0;
}

static void delete_retcode_description_file(void)
{
    //system("rm -f "RET_DESC_FILE);
    fprintf(stderr, "Please delete " RET_DESC_FILE " manually if needed.\n");
}

static int get_file_line_number(void)
{
    FILE *fp = nullptr;
    char output[32] = {0};
    const char *CMD = "wc -l " RET_DESC_FILE " | awk '{ print $1 }'";

    if (nullptr == (fp = popen(CMD, "r")))
    {
        fprintf(stderr, "popen(%s) failed: %s\n", CMD, strerror(errno));
        return -1;
    }

    while (nullptr != fgets(output, sizeof(output), fp))
    {
        printf("%s\n", output);
    }

    if (-1 == pclose(fp))
    {
        fprintf(stderr, "pclose() failed: %s\n", strerror(errno));
        return -1;
    }

    int lines = atoi(output);

    //printf(RET_DESC_FILE": %d lines\n", lines);

    return lines;
}

// TODO: how to test behaviors under multi-thread environment ?

static void test_message_displaying(void)
{
    char msg[512] = {0};
    const int EXPECTED_OK_CODE = 0;
    const int TESTED_POSITIVE_NUM = 1;

    /*
     * validates positive number(s)
     */
    ASSERT_EQ(EXPECTED_OK_CODE, calib::parse_return_code(TESTED_POSITIVE_NUM, sizeof(msg), msg));
    printf("%d:\t%s\n", TESTED_POSITIVE_NUM, msg);

    /*
     * validates special numbers
     */

    ASSERT_EQ(EXPECTED_OK_CODE, calib::parse_return_code(CA_RET_OK, sizeof(msg), msg));
    ASSERT_STREQ(SUCCESS_RET_STRING, msg);
    printf("%d:\t%s\n", CA_RET_OK, msg);

    ASSERT_EQ(EXPECTED_OK_CODE, calib::parse_return_code(CA_RET_GENERAL_FAILURE, sizeof(msg), msg));
    printf("%d:\t%s\n", CA_RET_GENERAL_FAILURE, msg);

    /*
     * validates system return codes
     */

    const int SYS_RET_BEGIN = calib::SYS_RET_CODE_BEGIN, SYS_RET_END = -150;

    for (int i = SYS_RET_BEGIN; i > SYS_RET_END; --i)
    {
        ASSERT_EQ(EXPECTED_OK_CODE, calib::parse_return_code(i, sizeof(msg), msg));
        ASSERT_TRUE('\0' != msg[0]);
        printf("%d:\t%s\n", i, msg);
    }

    /*
     * validates user return codes
     */

    ASSERT_EQ(0, generate_retcode_description_file());

    if (get_file_line_number() <= 0)
    {
        fprintf(stderr, "WARNING: generate_retcode_description_file() failed,"
            " check the codes and their meanings yourself.\n");
        for (int i = 1; i <= calib::USER_RET_CODE_COUNT; ++i)
        {
            int retcode = calib::USER_RET_CODE_BEGIN - i + 1;

            ASSERT_EQ(EXPECTED_OK_CODE, calib::parse_return_code(retcode, sizeof(msg), msg));
            printf("%d:\t%s\n", retcode, msg);
        }

        delete_retcode_description_file();

        return;
    }

    std::ifstream desc_file;
    std::string line_str;
    int line_num = 1;

    desc_file.open(RET_DESC_FILE, std::ios::in | std::ios::binary);
    ASSERT_TRUE(desc_file.good());

    while (std::getline(desc_file, line_str, '\n'))
    {
        int retcode = calib::USER_RET_CODE_BEGIN - line_num + 1;

        ASSERT_EQ(EXPECTED_OK_CODE, calib::parse_return_code(retcode, sizeof(msg), msg));
        ASSERT_STREQ(line_str.c_str(), msg);
        printf("%d:\t%s\n", retcode, msg);

        line_str.clear();

        ++line_num;
        if (line_num > calib::USER_RET_CODE_COUNT)
            break;
    }

    desc_file.close();

    ASSERT_EQ(line_num, calib::USER_RET_CODE_COUNT + 1);

    delete_retcode_description_file();
}

TEST(ca_return_code, parse_return_code)
{
    test_invalid_conditions();
    test_message_displaying();
}

TEST(ca_return_code_DeathTest, parse_return_code)
{
    char msg[512] = {0};
    const int TESTED_RETCODE = 0;

    if (calib::__debug_macro_is_defined())
        ASSERT_EQ(-1, calib::parse_return_code(TESTED_RETCODE, sizeof(msg), nullptr));
    else
        ASSERT_DEATH(calib::parse_return_code(TESTED_RETCODE, sizeof(msg), nullptr), "");

}

