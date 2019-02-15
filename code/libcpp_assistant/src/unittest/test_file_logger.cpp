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

#include "common_headers.h"

#include "private/debug.h"
#include "base/debug.h"
#include "file_logger.h"

static void stream_logging_test(calib::logger &logger, calib::enum_log_level log_level)
{
    const char *prefix = "stream logging test with log level ";
    float float_value = 1.23;
    double double_value = 1.23456;
    long double long_double_value = 1.23456789;
    char char_value = 1;
    unsigned char unsigned_char_value = 2;
    short int short_int_value = 3;
    unsigned short int unsigned_short_int_value = 4;
    int int_value = 5;
    unsigned int unsigned_int_value = 6;
    long int long_int_value = 7;
    unsigned long int unsigned_long_int_value = 8;
    long long int long_long_int_value = 9;
    unsigned long long int unsigned_long_long_int_value = 10;
    std::string string_value = "This is a string, not a char*.";

    logger.get_stream(log_level) << prefix << log_level << ": bool: " << true;
    logger.get_stream(log_level) << prefix << log_level << ": float_value: " << float_value;
    logger.get_stream(log_level) << prefix << log_level << ": double_value: " << double_value;
    logger.get_stream(log_level) << prefix << log_level << ": long_double_value: " << long_double_value;
    logger.get_stream(log_level) << prefix << log_level << ": char_value: " << char_value;
    logger.get_stream(log_level) << prefix << log_level << ": unsigned_char_value: " << unsigned_char_value;
    logger.get_stream(log_level) << prefix << log_level << ": short_int_value: " << short_int_value;
    logger.get_stream(log_level) << prefix << log_level << ": unsigned_short_int_value: " << unsigned_short_int_value;
    logger.get_stream(log_level) << prefix << log_level << ": int_value: " << int_value;
    logger.get_stream(log_level) << prefix << log_level << ": unsigned_int_value: " << unsigned_int_value;
    logger.get_stream(log_level) << prefix << log_level << ": long_int_value: " << long_int_value;
    logger.get_stream(log_level) << prefix << log_level << ": unsigned_long_int_value: " << unsigned_long_int_value;
    logger.get_stream(log_level) << prefix << log_level << ": long_long_int_value: " << long_long_int_value;
    logger.get_stream(log_level) << prefix << log_level << ": unsigned_long_long_int_value: " << unsigned_long_long_int_value;
    logger.get_stream(log_level) << prefix << log_level << ": string_value: " << string_value;
}

TEST(file_logger, AllInOne)
{
    const char *TEST_STR = "文件日志记录器测试\n";
    const int TEST_STR_LEN = strlen(TEST_STR);
    const calib::enum_log_level LOG_LEVELS[] = {
        calib::LOG_LEVEL_DEBUG,
        calib::LOG_LEVEL_INFO,
        calib::LOG_LEVEL_WARNING,
        calib::LOG_LEVEL_ERROR,
        calib::LOG_LEVEL_CRITICAL
    };
    const int LEVEL_COUNT = sizeof(LOG_LEVELS) / sizeof(calib::enum_log_level);
    calib::file_logger logger;
    const char *INITIAL_LOG_NAME = logger.log_name();
    const char *INITIAL_LOG_DIR = logger.log_directory();
    int ret = -1;
    bool debug_macro_is_defined = calib::__debug_macro_is_defined();
    const int NULL_PARAM_RETCODE_DUE_TO_OPTIMIZATION = -1;

    printf("debug_macro_is_defined: %d\n", debug_macro_is_defined);
    printf("Initial open status: %d\n", logger.is_open());
    printf("Initial log level: %d\n", logger.log_level());
    printf("Initial log name: %s\n", logger.log_name());
    printf("Initial log directory: %s\n", logger.log_directory());

    calib::debug::redirect_debug_output(nullptr);

    /*
     * Tests Operations right after FileLogger instantiation.
     */
    ASSERT_FALSE(logger.is_open());
    ASSERT_EQ(calib::LOG_LEVEL_ALL, logger.log_level());
    ASSERT_STREQ(nullptr, logger.log_name());
    ASSERT_STREQ(nullptr, logger.log_directory());
    for (int i = 0; i < LEVEL_COUNT; ++i)
    {
        ASSERT_EQ(debug_macro_is_defined ? CA_RET(NULL_PARAM) : CA_RET(FILE_OR_STREAM_NOT_OPEN),
            logger.output(calib::NO_LOG_PREFIX, LOG_LEVELS[i], nullptr));
        ASSERT_EQ(CA_RET(FILE_OR_STREAM_NOT_OPEN),
            logger.output(calib::NO_LOG_PREFIX, LOG_LEVELS[i], "[Level: %d]: %s", LOG_LEVELS[i], TEST_STR));
    }

    //logger.Prepare(nullptr);

    /*
     * Tests Open() before setting name and directory.
     */
    ASSERT_EQ(CA_RET(TARGET_NOT_READY), logger.open());
    ASSERT_FALSE(logger.is_open());

    /*
     * Tests log name.
     */

    const char *OLD_LOG_NAME = "old_file";
    const int OLD_LOG_NAME_LEN = strlen(OLD_LOG_NAME);
    const char *NEW_LOG_NAME = "new_file";
    const int NEW_LOG_NAME_LEN = strlen(NEW_LOG_NAME);

    if (debug_macro_is_defined)
        ASSERT_EQ(CA_RET(NULL_PARAM), logger.set_log_name(nullptr));
    else
        ASSERT_DEATH(logger.set_log_name(nullptr), "");
    ASSERT_STREQ(INITIAL_LOG_NAME, logger.log_name());

    ASSERT_EQ(CA_RET_OK, logger.set_log_name(OLD_LOG_NAME)); // 1st success.
    ASSERT_TRUE(0 == strncmp(OLD_LOG_NAME, logger.log_name(), OLD_LOG_NAME_LEN));

    if (debug_macro_is_defined)
        ASSERT_EQ(CA_RET(NULL_PARAM), logger.set_log_name(nullptr));
    else
        ASSERT_DEATH(logger.set_log_name(nullptr), "");
    // Failure would not affect the previous value.
    ASSERT_TRUE(0 == strncmp(OLD_LOG_NAME, logger.log_name(), OLD_LOG_NAME_LEN));

    ASSERT_EQ(CA_RET_OK, logger.set_log_name(NEW_LOG_NAME)); // 2nd.a success.
    printf("The 1st time to set new name: %s\n", logger.log_name());
    ASSERT_TRUE(0 == strncmp(NEW_LOG_NAME, logger.log_name(), NEW_LOG_NAME_LEN));
    ASSERT_EQ(CA_RET_OK, logger.set_log_name(NEW_LOG_NAME)); // 2nd.b success.
    printf("The 2nd time to set new name: %s\n", logger.log_name());
    ASSERT_TRUE(0 == strncmp(NEW_LOG_NAME, logger.log_name(), NEW_LOG_NAME_LEN));

    ASSERT_EQ(CA_RET_OK, logger.set_log_name(OLD_LOG_NAME)); // Recovers the old name.
    ASSERT_TRUE(0 == strncmp(OLD_LOG_NAME, logger.log_name(), OLD_LOG_NAME_LEN));

    // TODO: long path testing ...

    ASSERT_EQ(CA_RET(TARGET_NOT_READY), logger.open()); // It still fails because log directory has not been set.
    ASSERT_FALSE(logger.is_open());

    /*
     * Tests log directory.
     */

    const char *ONE_LEVEL_NONEXISTENT_DIR = "./ca_test_nonexistent";
    const char *CMD_DEL_ONE_LEVEL_DIR = "rm -rf ./ca_test_nonexistent";
    const char *TWO_LEVEL_NONEXISTENT_DIR = "./another/ca_test_nonexistent";
    const char *CMD_DEL_TWO_LEVEL_DIR = "rm -rf ./another/ca_test_nonexistent";

    printf("Deleting %s and %s if they exist ...\n", ONE_LEVEL_NONEXISTENT_DIR, TWO_LEVEL_NONEXISTENT_DIR);
    system(CMD_DEL_ONE_LEVEL_DIR);
    system(CMD_DEL_TWO_LEVEL_DIR);

    if (debug_macro_is_defined)
        ASSERT_EQ(CA_RET(NULL_PARAM), logger.set_log_directory(nullptr));
    else
        ASSERT_DEATH(logger.set_log_directory(nullptr), "");
    ASSERT_STREQ(INITIAL_LOG_DIR, logger.log_directory());

    ASSERT_EQ(CA_RET_OK, logger.set_log_directory(ONE_LEVEL_NONEXISTENT_DIR)); // 1st success, will create one if it does not exist.
    ASSERT_STREQ(ONE_LEVEL_NONEXISTENT_DIR, logger.log_directory());

    ASSERT_EQ(CA_RET_OK, logger.set_log_directory(ONE_LEVEL_NONEXISTENT_DIR)); // 2nd success, it already exists.
    ASSERT_STREQ(ONE_LEVEL_NONEXISTENT_DIR, logger.log_directory());

    ret = logger.set_log_directory(TWO_LEVEL_NONEXISTENT_DIR); // Will fail.
    printf("Return code of set_log_directory(%s): %d\n", TWO_LEVEL_NONEXISTENT_DIR, ret);
    ASSERT_TRUE(CA_RET_OK != ret);
    ASSERT_STREQ(ONE_LEVEL_NONEXISTENT_DIR, logger.log_directory());

    // TODO: long path testing ...

    ASSERT_EQ(CA_RET_OK, logger.open()); // It works this time.
    ASSERT_TRUE(logger.is_open());
    ASSERT_EQ(CA_RET_OK, logger.open()); // It's okay to open the logger multiple times.
    ASSERT_TRUE(logger.is_open());

    /*
     * It's not allowed to set log name and directory when the logger is open.
     */

    ASSERT_EQ(CA_RET(DEVICE_BUSY), logger.set_log_name(NEW_LOG_NAME));
    ASSERT_TRUE(0 == strncmp(OLD_LOG_NAME, logger.log_name(), OLD_LOG_NAME_LEN));

    ASSERT_EQ(CA_RET(DEVICE_BUSY), logger.set_log_directory(TWO_LEVEL_NONEXISTENT_DIR));
    ASSERT_STREQ(ONE_LEVEL_NONEXISTENT_DIR, logger.log_directory());

    ASSERT_TRUE(logger.is_open());

    /*
     * It's okay to close the logger multiple times as well,
     * and we can not call Output() in this case.
     */
    ASSERT_EQ(CA_RET_OK, logger.close());
    ASSERT_EQ(CA_RET_OK, logger.close());
    ASSERT_FALSE(logger.is_open());
    for (int i = 0; i < LEVEL_COUNT; ++i)
    {
        ASSERT_EQ(debug_macro_is_defined ? CA_RET(NULL_PARAM) : CA_RET(FILE_OR_STREAM_NOT_OPEN),
            logger.output(calib::NO_LOG_PREFIX, LOG_LEVELS[i], nullptr));
        ASSERT_EQ(CA_RET(FILE_OR_STREAM_NOT_OPEN), logger.output(calib::NO_LOG_PREFIX, LOG_LEVELS[i], "[Level: %d]: %s", LOG_LEVELS[i], TEST_STR));
    }

    /*
     * Re-opens the logger for afterward testings.
     */
    ASSERT_EQ(CA_RET_OK, logger.open());
    ASSERT_TRUE(logger.is_open());

    calib::debug::redirect_debug_output(stdout);

    /*
     * Tests log levels.
     */
    for (int i = 0; i < LEVEL_COUNT; ++i)
    {
        logger.set_log_level(LOG_LEVELS[i]);
        ASSERT_EQ(LOG_LEVELS[i], logger.log_level());

        calib::debug::redirect_debug_output((0 == i % 2) ? nullptr : stdout);

        for (int j = 0; j < LEVEL_COUNT; ++j)
        {
            calib::enum_log_level level = LOG_LEVELS[j];
            bool is_enough_level = (level >= logger.log_level());
            const int EXPECTED_RET = ((!is_enough_level) ? (debug_macro_is_defined ? CA_RET(NULL_PARAM) : 0)
                : (debug_macro_is_defined ? CA_RET(NULL_PARAM) : NULL_PARAM_RETCODE_DUE_TO_OPTIMIZATION));

            ASSERT_EQ(EXPECTED_RET, logger.output(calib::NO_LOG_PREFIX, level, nullptr));

            if (is_enough_level)
                ASSERT_GE(logger.output(calib::NO_LOG_PREFIX, level, "[Level: %d]: %s", level, TEST_STR), TEST_STR_LEN);
            else
                ASSERT_EQ(0, logger.output(calib::NO_LOG_PREFIX, level, "[Level: %d]: %s", level, TEST_STR));
        }

        if (logger.log_level() <= calib::LOG_LEVEL_DEBUG)
        {
            ASSERT_EQ(debug_macro_is_defined ? CA_RET(NULL_PARAM) : NULL_PARAM_RETCODE_DUE_TO_OPTIMIZATION, logger.d(nullptr));
            ASSERT_GE(logger.d("%s", TEST_STR), TEST_STR_LEN);
        }
        else
        {
            ASSERT_EQ(debug_macro_is_defined ? CA_RET(NULL_PARAM) : 0, logger.d(nullptr));
            ASSERT_EQ(0, logger.d("%s", TEST_STR));
        }
        stream_logging_test(logger, calib::LOG_LEVEL_DEBUG);

        if (logger.log_level() <= calib::LOG_LEVEL_INFO)
        {
            ASSERT_EQ(debug_macro_is_defined ? CA_RET(NULL_PARAM) : NULL_PARAM_RETCODE_DUE_TO_OPTIMIZATION, logger.i(nullptr));
            ASSERT_GE(logger.i("%s", TEST_STR), TEST_STR_LEN);
        }
        else
        {
            ASSERT_EQ(debug_macro_is_defined ? CA_RET(NULL_PARAM) : 0, logger.i(nullptr));
            ASSERT_EQ(0, logger.i("%s", TEST_STR));
        }
        stream_logging_test(logger, calib::LOG_LEVEL_INFO);

        if (logger.log_level() <= calib::LOG_LEVEL_WARNING)
        {
            ASSERT_EQ(debug_macro_is_defined ? CA_RET(NULL_PARAM) : NULL_PARAM_RETCODE_DUE_TO_OPTIMIZATION, logger.w(nullptr));
            ASSERT_GE(logger.w("%s", TEST_STR), TEST_STR_LEN);
        }
        else
        {
            ASSERT_EQ(debug_macro_is_defined ? CA_RET(NULL_PARAM) : 0, logger.w(nullptr));
            ASSERT_EQ(0, logger.w("%s", TEST_STR));
        }
        stream_logging_test(logger, calib::LOG_LEVEL_WARNING);

        if (logger.log_level() <= calib::LOG_LEVEL_ERROR)
        {
            ASSERT_EQ(debug_macro_is_defined ? CA_RET(NULL_PARAM) : NULL_PARAM_RETCODE_DUE_TO_OPTIMIZATION, logger.e(nullptr));
            ASSERT_GE(logger.e("%s", TEST_STR), TEST_STR_LEN);
        }
        else
        {
            ASSERT_EQ(debug_macro_is_defined ? CA_RET(NULL_PARAM) : 0, logger.e(nullptr));
            ASSERT_EQ(0, logger.e("%s", TEST_STR));
        }
        stream_logging_test(logger, calib::LOG_LEVEL_ERROR);

        if (logger.log_level() <= calib::LOG_LEVEL_CRITICAL)
        {
            ASSERT_EQ(debug_macro_is_defined ? CA_RET(NULL_PARAM) : NULL_PARAM_RETCODE_DUE_TO_OPTIMIZATION, logger.c(nullptr));
            ASSERT_GE(logger.c("%s", TEST_STR), TEST_STR_LEN);
        }
        else
        {
            ASSERT_EQ(debug_macro_is_defined ? CA_RET(NULL_PARAM) : 0, logger.c(nullptr));
            ASSERT_EQ(0, logger.c("%s", TEST_STR));
        }
        stream_logging_test(logger, calib::LOG_LEVEL_CRITICAL);
    }

    ASSERT_EQ(CA_RET_OK, logger.close());
    ASSERT_FALSE(logger.is_open());

    ASSERT_EQ(CA_RET_OK, logger.set_log_line_limit(calib::LOG_LINE_LIMIT_MIN));

    ASSERT_EQ(CA_RET_OK, logger.open());
    ASSERT_TRUE(logger.is_open());

    /*
     * Long time test.
     */

    const int INTERVAL_SECS = 60 * 60;
    const int TOTAL_ROUNDS = (24 * 60 * 60) / INTERVAL_SECS;
    char chk_cmd[4096] = {0};

    logger.set_log_level(calib::LOG_LEVEL_DEBUG);
    printf("Running long time test ...\n");
    for (int i = 0; i < TOTAL_ROUNDS; ++i)
    {
        // TODO: tag of wired problem
        // redirect_debug_output(nullptr) -> debug_is_enabled(): false
        //     -> Output(calib::HAS_LOG_PREFIX): no line number at every line header.
        // redirect_debug_output(stdout) -> debug_is_enabled(): true
        //     -> Output(calib::HAS_LOG_PREFIX): has a line number at every line header.
        // Executing redirect_debug_output() here would not get the expected results above.
        calib::debug::redirect_debug_output((0 == i % 2) ? nullptr : stdout);

        ASSERT_EQ((0 == i % 2) ? nullptr : stdout, calib::debug::get_debug_output_holder());
        ASSERT_EQ((0 == i % 2) ? false : true, calib::debug_is_enabled());
        logger.i("i = %d, debug_is_enabled(): %d\n", i, calib::debug_is_enabled());

        memset(chk_cmd, 0, sizeof(chk_cmd));
        snprintf(chk_cmd, sizeof(chk_cmd), "wc -l %s/%s*", logger.log_directory(), logger.log_name());
        printf("Round[%d]: Before writing:\n", i);
        system(chk_cmd);

        for (int j = 0; j < logger.log_line_limit() - 1; ++j)
        {
            // TODO: tag of wired problem
            // Executing redirect_debug_output() here can get the expected results above. Why here?!
            calib::debug::redirect_debug_output((0 == i % 2) ? nullptr : stdout);
            //usleep(1); // does not make things work as redirect_debug_output() does.

            ASSERT_EQ((0 == i % 2) ? false : true, calib::debug_is_enabled());

            calib::enum_log_level level = LOG_LEVELS[j % calib::LOG_LEVEL_COUNT];
            int actual_output_len = logger.output(calib::HAS_LOG_PREFIX, level, "[%02d:%d]\t%s", i, j + 1, TEST_STR);

            if (level >= logger.log_level())
                ASSERT_GE(actual_output_len, TEST_STR_LEN);
            else
                ASSERT_EQ(actual_output_len, 0);
        }

        logger.flush();
        memset(chk_cmd, 0, sizeof(chk_cmd));
        // Do this again to fetch the newest name!
        snprintf(chk_cmd, sizeof(chk_cmd), "wc -l %s/%s*", logger.log_directory(), logger.log_name());
        printf("Round[%d]: After writing:\n", i);
        system(chk_cmd);

        if (i < 5) // Some pre-tests before long time tests.
            sleep(5);
#if 0 // enable it if you want to do a long time test.
        else

            sleep(INTERVAL_SECS);
#endif
    }

    ASSERT_EQ(CA_RET_OK, logger.close());
    ASSERT_FALSE(logger.is_open());
}

