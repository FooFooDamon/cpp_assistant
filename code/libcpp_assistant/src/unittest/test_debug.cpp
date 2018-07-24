/*
 * Copyright (c) 2018, Wen Xiongchang <udc577 at 126 dot com>
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

#include "singleton.h"
#include "screen_logger.h"

#define DEFINE_FOO_F(x)                 class Foo_F_##x {\
public: \
    void print(int &count_var) {\
        LOGF_C(x, "I'm LOGF_C(" #x "), number: %d\n", count_var++); \
    } \
}

DEFINE_FOO_F(D);
DEFINE_FOO_F(I);
DEFINE_FOO_F(W);
DEFINE_FOO_F(E);
DEFINE_FOO_F(C);

#define DEFINE_FOO_S(x)                 class Foo_S_##x {\
public: \
    void print(int &count_var) {\
        LOGS_C(x) << "I'm LOGS_C(" #x "), number: " << count_var++; \
    } \
}

DEFINE_FOO_S(D);
DEFINE_FOO_S(I);
DEFINE_FOO_S(W);
DEFINE_FOO_S(E);
DEFINE_FOO_S(C);

TEST(debug, AllInOne)
{
    int count = 1;

#define TEST_CA_LOG(x)                  \
    RLOGS(x) << "I'm RLOGS(" #x "), number: " << count++; \
    LOGS(x) << "I'm LOGS(" #x "), number: " << count++; \
    Foo_S_##x foo_s_##x; \
    foo_s_##x.print(count); \
    LOGS_NS(x, none::foo) << "I'm LOGS_NS(" #x "), number: " << count++; \
\
    fprintf(GET_LOG_HOLDER(), "\n"); \
    RLOGF(x, "I'm RLOGF(" #x "), number: %d\n", count++); \
    LOGF(x, "I'm LOGF(" #x "), number: %d\n", count++); \
    Foo_F_##x foo_f_##x; \
    foo_f_##x.print(count); \
    LOGF_NS(x, none::foo, "I'm LOGF_NS(" #x "), number: %d\n", count++);

    TEST_CA_LOG(D);
    TEST_CA_LOG(I);
    TEST_CA_LOG(W);
    TEST_CA_LOG(E);
    TEST_CA_LOG(C);
}

