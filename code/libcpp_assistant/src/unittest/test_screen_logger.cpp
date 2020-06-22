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

#include "private/debug.h"
#include "base/debug.h"
#include "screen_logger.h"

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
    std::string string_value = "This is a string, not a char*.\n";

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

TEST(screen_logger, AllInOne)
{
    const char *TEST_STR = "屏幕日志记录器测试\n";
    const int TEST_STR_LEN = strlen(TEST_STR);
    const calib::enum_log_level LOG_LEVELS[] = {
        calib::LOG_LEVEL_DEBUG,
        calib::LOG_LEVEL_INFO,
        calib::LOG_LEVEL_WARNING,
        calib::LOG_LEVEL_ERROR,
        calib::LOG_LEVEL_CRITICAL
    };
    const int LEVEL_COUNT = sizeof(LOG_LEVELS) / sizeof(calib::enum_log_level);
    calib::screen_logger logger;
    const char *INITIAL_LOG_NAME = logger.log_name();
    const char *INITIAL_LOG_DIR = logger.log_directory();
    bool debug_macro_is_defined = calib::__debug_macro_is_defined();
    const int NULL_PARAM_RETCODE_DUE_TO_OPTIMIZATION = -1;

    printf("debug_macro_is_defined: %d\n", debug_macro_is_defined);
    printf("Initial open status: %d\n", logger.is_open());
    printf("Initial log level: %d\n", logger.log_level());
    printf("Initial log name: %s\n", logger.log_name());
    printf("Initial log directory: %s\n", logger.log_directory());

    /*
     * Tests Operations right after ScreenLogger instantiation.
     */
    ASSERT_TRUE(logger.is_open());
    ASSERT_EQ(calib::LOG_LEVEL_ALL, logger.log_level());
    printf(">>>> logger.Output(NO_LOG_PREFIX, level[%d ~ %d]):\n", 0 , LEVEL_COUNT - 1);
    for (int i = 0; i < LEVEL_COUNT; ++i)
    {
        ASSERT_EQ(debug_macro_is_defined ? CA_RET(NULL_PARAM) : NULL_PARAM_RETCODE_DUE_TO_OPTIMIZATION,
            logger.output(calib::NO_LOG_PREFIX, LOG_LEVELS[i], nullptr));
        ASSERT_GE(logger.output(calib::NO_LOG_PREFIX, LOG_LEVELS[i], "[Level: %d]: %s", LOG_LEVELS[i], TEST_STR), TEST_STR_LEN);
    }

    /*
     * It's okay to open the logger multiple times.
     */
    ASSERT_EQ(CA_RET_OK, logger.open());
    ASSERT_EQ(CA_RET_OK, logger.open());
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
        ASSERT_EQ(CA_RET(FILE_OR_STREAM_NOT_OPEN),
            logger.output(calib::NO_LOG_PREFIX, LOG_LEVELS[i], "[Level: %d]: %s", LOG_LEVELS[i], TEST_STR));
    }

    /*
     * Tests log name and directory.
     * They should keep unchanged no matter how they are set
     * because they are meaningless to ScreenLogger.
     */

    ASSERT_EQ(CA_RET_OK, logger.set_log_name(nullptr));
    ASSERT_STREQ(INITIAL_LOG_NAME, logger.log_name());
    ASSERT_EQ(CA_RET_OK, logger.set_log_name("whatever"));
    ASSERT_STREQ(INITIAL_LOG_NAME, logger.log_name());

    ASSERT_EQ(CA_RET_OK, logger.set_log_directory(nullptr));
    ASSERT_STREQ(INITIAL_LOG_DIR, logger.log_directory());
    ASSERT_EQ(CA_RET_OK, logger.set_log_directory("whatever"));
    ASSERT_STREQ(INITIAL_LOG_DIR, logger.log_directory());

    /*
     * Re-opens the logger for afterward testings.
     */
    ASSERT_EQ(CA_RET_OK, logger.open());
    ASSERT_TRUE(logger.is_open());

    /*
     * Tests log levels.
     */
    printf(">>>> logger.Output(NO_LOG_PREFIX), logger.d(), logger.i(), logger.w(), logger.e(), logger.f():\n");
    for (int i = 0; i < LEVEL_COUNT; ++i)
    {
        logger.set_log_level(LOG_LEVELS[i]);
        ASSERT_EQ(LOG_LEVELS[i], logger.log_level());

        calib::debug::redirect_debug_output((0 == i % 2) ? nullptr : stdout);

        printf("After log level being set to %d and redirect_debug_output(%s),"
            " contents that can be output:\n", LOG_LEVELS[i], (0 == i % 2) ? "nullptr" : "stdout");
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

        /*
         * TODO: compilation error
         */
        /*struct tmp_struct
        {
            calib::Logger::FormattedOutput print_func;
            calib::enum_log_level log_level;
        } tested_tmp_items[] = {
            { &(logger.d), calib::LOG_LEVEL_DEBUG },
            { logger.i, calib::LOG_LEVEL_INFO },
            { logger.w, calib::LOG_LEVEL_WARNING },
            { logger.e, calib::LOG_LEVEL_ERROR },
            { logger.f, calib::LOG_LEVEL_FATAL }
        };

        for (unsigned int j = 0; j < sizeof(tested_tmp_items) / sizeof(struct tmp_struct); ++j)
        {
            struct tmp_struct &item = tested_tmp_items[j];

            if (logger.log_level() <= item.log_level)
            {
                ASSERT_EQ(debug_macro_is_defined ? CA_RET(NULL_PARAM) : NULL_PARAM_RETCODE_DUE_TO_OPTIMIZATION, item.print_func(nullptr));
                ASSERT_GE(item.print_func("%s", TEST_STR), TEST_STR_LEN);
            }
            else
            {
                ASSERT_EQ(debug_macro_is_defined ? CA_RET(NULL_PARAM) : 0, item.print_func(nullptr));
                ASSERT_EQ(0, item.print_func("%s", TEST_STR));
            }
        }*/

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

    /*
     * Long time test.
     */
#if 0
    const int INTERVAL_SECS = 30;
    const int TOTAL_ROUNDS = (24 * 60 * 60) / INTERVAL_SECS;

    logger.set_log_level(calib::LOG_LEVEL_INFO);
    printf("Running long time test ...\n");
    for (int i = 0; i < TOTAL_ROUNDS; ++i)
    {
        ASSERT_GE(logger.i("[%d]\t%s", i, TEST_STR), TEST_STR_LEN);
        sleep(INTERVAL_SECS);
    }
#endif
}

