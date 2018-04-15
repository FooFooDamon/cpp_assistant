/*
 * Copyright (c) 2017, Wen Xiongchang <udc577 at 126 dot com>
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

#define CA_USE_STL 1 // gtest uses STL, thus it's difficult to test the version without STL.
#include "floating_point.h"

typedef unsigned int InvalidType;

#if 0
#define PRINT_FLOATING_POINT_VALUE(val, n_decimal_places)   \
    if (typeid(float) == typeid(val)) printf(#val"(type:%s): %f", typeid(val).name(), static_cast<float>(val)); \
    else if (typeid(double) == typeid(val)) printf(#val"(type:%s): %lf", typeid(val).name(), static_cast<double>(val)); \
    else printf(#val"(type:%s): %Lf", typeid(val).name(), static_cast<long double>(val)); \
    std::cout << " with printf() or " << val << " with std::cout"<< std::endl
#else
#define PRINT_FLOATING_POINT_VALUE(val, n_decimal_places)   do{ \
    char fmt[64] = {0}; \
    if (typeid(float) == typeid(val)) snprintf(fmt, sizeof(fmt), "%%.%df", n_decimal_places); \
    else if (typeid(double) == typeid(val)) snprintf(fmt, sizeof(fmt), "%%.%dlf", n_decimal_places); \
    else snprintf(fmt, sizeof(fmt), "%%.%dLf", n_decimal_places); \
    printf(#val"(type:%s): ", typeid(val).name()); \
    printf(fmt, val); \
    std::cout << " with printf() or " << val << " with std::cout"<< std::endl; \
}while(0)
#endif

#define PRINT_DECIMAL_PLACE_COUNT(var)          printf(#var": %d\n", var)
#define PRINT_FLOATING_POINT_STRING(var)        printf(#var": %s\n", var)

static void test_limit_definitions(void)
{

#define PRINT_TYPE_SIZE(type)                   printf("sizeof("#type"): %lu\n", sizeof(type))
#define PRINT_LIMIT_VALUE(limit_definition)     printf(#limit_definition": %d\n", limit_definition)

    SET_G_RET_FAILURE();

    PRINT_TYPE_SIZE(float);
    PRINT_TYPE_SIZE(double);
    PRINT_TYPE_SIZE(long double);
    ASSERT_GE(sizeof(double), sizeof(float));
    ASSERT_GE(sizeof(long double), sizeof(double));

    PRINT_LIMIT_VALUE(calib::floating_point<float>::DEFAULT_DECIMAL_PLACE_COUNT);
    PRINT_LIMIT_VALUE(calib::floating_point<float>::MAX_DECIMAL_PLACE_COUNT);
    PRINT_LIMIT_VALUE(calib::floating_point<float>::CHAR_COUNT_OF_MAX_VALUE);
    PRINT_FLOATING_POINT_VALUE(calib::floating_point<float>::MAX_VALUE, calib::floating_point<float>::MAX_DECIMAL_PLACE_COUNT);
    PRINT_FLOATING_POINT_VALUE(calib::floating_point<float>::MIN_VALUE, calib::floating_point<float>::MAX_DECIMAL_PLACE_COUNT);
    PRINT_FLOATING_POINT_VALUE(calib::floating_point<float>::EPSILON, calib::floating_point<float>::MAX_DECIMAL_PLACE_COUNT);
    ASSERT_GT(calib::floating_point<float>::DEFAULT_DECIMAL_PLACE_COUNT, 0);
    ASSERT_GE(calib::floating_point<float>::MAX_DECIMAL_PLACE_COUNT, calib::floating_point<float>::DEFAULT_DECIMAL_PLACE_COUNT);
    ASSERT_GT(calib::floating_point<float>::CHAR_COUNT_OF_MAX_VALUE, calib::floating_point<float>::MAX_DECIMAL_PLACE_COUNT);

    PRINT_LIMIT_VALUE(calib::floating_point<double>::DEFAULT_DECIMAL_PLACE_COUNT);
    PRINT_LIMIT_VALUE(calib::floating_point<double>::MAX_DECIMAL_PLACE_COUNT);
    PRINT_LIMIT_VALUE(calib::floating_point<double>::CHAR_COUNT_OF_MAX_VALUE);
    PRINT_FLOATING_POINT_VALUE(calib::floating_point<double>::MAX_VALUE, calib::floating_point<double>::MAX_DECIMAL_PLACE_COUNT);
    PRINT_FLOATING_POINT_VALUE(calib::floating_point<double>::MIN_VALUE, calib::floating_point<double>::MAX_DECIMAL_PLACE_COUNT);
    PRINT_FLOATING_POINT_VALUE(calib::floating_point<double>::EPSILON, calib::floating_point<double>::MAX_DECIMAL_PLACE_COUNT);
    ASSERT_GE(calib::floating_point<double>::DEFAULT_DECIMAL_PLACE_COUNT, calib::floating_point<float>::DEFAULT_DECIMAL_PLACE_COUNT);
    ASSERT_GE(calib::floating_point<double>::MAX_DECIMAL_PLACE_COUNT, calib::floating_point<double>::DEFAULT_DECIMAL_PLACE_COUNT);
    ASSERT_GE(calib::floating_point<double>::MAX_DECIMAL_PLACE_COUNT, calib::floating_point<float>::MAX_DECIMAL_PLACE_COUNT);
    ASSERT_GT(calib::floating_point<double>::CHAR_COUNT_OF_MAX_VALUE, calib::floating_point<double>::MAX_DECIMAL_PLACE_COUNT);
    ASSERT_GE(calib::floating_point<double>::CHAR_COUNT_OF_MAX_VALUE, calib::floating_point<float>::CHAR_COUNT_OF_MAX_VALUE);

    PRINT_LIMIT_VALUE(calib::floating_point<long double>::DEFAULT_DECIMAL_PLACE_COUNT);
    PRINT_LIMIT_VALUE(calib::floating_point<long double>::MAX_DECIMAL_PLACE_COUNT);
    PRINT_LIMIT_VALUE(calib::floating_point<long double>::CHAR_COUNT_OF_MAX_VALUE);
    PRINT_FLOATING_POINT_VALUE(calib::floating_point<long double>::MAX_VALUE, calib::floating_point<long double>::MAX_DECIMAL_PLACE_COUNT);
    PRINT_FLOATING_POINT_VALUE(calib::floating_point<long double>::MIN_VALUE, calib::floating_point<long double>::MAX_DECIMAL_PLACE_COUNT);
    PRINT_FLOATING_POINT_VALUE(calib::floating_point<long double>::EPSILON, calib::floating_point<long double>::MAX_DECIMAL_PLACE_COUNT);
    ASSERT_GE(calib::floating_point<long double>::DEFAULT_DECIMAL_PLACE_COUNT, calib::floating_point<double>::DEFAULT_DECIMAL_PLACE_COUNT);
    ASSERT_GE(calib::floating_point<long double>::MAX_DECIMAL_PLACE_COUNT, calib::floating_point<long double>::DEFAULT_DECIMAL_PLACE_COUNT);
    ASSERT_GE(calib::floating_point<long double>::MAX_DECIMAL_PLACE_COUNT, calib::floating_point<double>::MAX_DECIMAL_PLACE_COUNT);
    ASSERT_GT(calib::floating_point<long double>::CHAR_COUNT_OF_MAX_VALUE, calib::floating_point<long double>::MAX_DECIMAL_PLACE_COUNT);
    ASSERT_GE(calib::floating_point<long double>::CHAR_COUNT_OF_MAX_VALUE, calib::floating_point<double>::CHAR_COUNT_OF_MAX_VALUE);

    SET_G_RET_SUCCESS();
}

template<typename FT>
static void test_default_constructor(
    FT tested_num,
    int decimal_place_count)
{
    SET_G_RET_FAILURE();

    if (IS_NOT_FLOATING_POINT_TYPE(FT))
    {
        try
        {
            calib::floating_point<FT> invalid_type_demo;
        }
        catch(const char *e_what)
        {
            printf("%s\n", e_what);
        }
        ASSERT_FALSE(false);
    }

    calib::floating_point<FT> demo;
    calib::floating_point_accessor<FT> accessor(demo);

    printf("Type of demo: %s\n", demo.type());
    printf("Bytes for type[%s]: %d\n", demo.type(), demo.type_size());
    PRINT_FLOATING_POINT_VALUE(demo.value(), calib::floating_point<FT>::MAX_DECIMAL_PLACE_COUNT);
    PRINT_DECIMAL_PLACE_COUNT(demo.decimal_place_count());
    if (typeid(float) == typeid(FT))
    {
        ASSERT_STREQ(FLOAT_TYPE_NAME, demo.type());
        ASSERT_EQ(sizeof(float), static_cast<unsigned int>(demo.type_size()));
    }
    else if (typeid(double) == typeid(FT))
    {
        ASSERT_STREQ(DOUBLE_TYPE_NAME, demo.type());
        ASSERT_EQ(sizeof(double), static_cast<unsigned int>(demo.type_size()));
    }
    else
    {
        ASSERT_STREQ(LONG_DOUBLE_TYPE_NAME, demo.type());
        ASSERT_EQ(sizeof(long double), static_cast<unsigned int>(demo.type_size()));
    }
    ASSERT_EQ(demo.value(), accessor.get_value());
    ASSERT_EQ(calib::floating_point<FT>::DEFAULT_DECIMAL_PLACE_COUNT, demo.decimal_place_count());
    ASSERT_EQ(demo.decimal_place_count(), accessor.get_decimal_place_count());
    ASSERT_EQ(NULL, accessor.get_string());

    SET_G_RET_SUCCESS();
}

template<typename FT>
static void test_other_constructors(
    FT tested_num,
    int decimal_place_count)
{
    SET_G_RET_FAILURE();

    calib::floating_point<FT> demo_default_decimal_place(tested_num);
    calib::floating_point_accessor<FT> accessor_default(demo_default_decimal_place);

    PRINT_FLOATING_POINT_VALUE(demo_default_decimal_place.value(), calib::floating_point<FT>::MAX_DECIMAL_PLACE_COUNT);
    PRINT_DECIMAL_PLACE_COUNT(demo_default_decimal_place.decimal_place_count());
    ASSERT_EQ(demo_default_decimal_place.value(), accessor_default.get_value());
    ASSERT_EQ(calib::floating_point<FT>::DEFAULT_DECIMAL_PLACE_COUNT, demo_default_decimal_place.decimal_place_count());
    ASSERT_EQ(demo_default_decimal_place.decimal_place_count(), accessor_default.get_decimal_place_count());
    ASSERT_EQ(NULL, accessor_default.get_string());

    calib::floating_point<FT> demo_specific_decimal_place(tested_num, decimal_place_count);
    calib::floating_point_accessor<FT> accessor_specific(demo_specific_decimal_place);

    PRINT_FLOATING_POINT_VALUE(demo_specific_decimal_place.value(), calib::floating_point<FT>::MAX_DECIMAL_PLACE_COUNT);
    PRINT_DECIMAL_PLACE_COUNT(demo_specific_decimal_place.decimal_place_count());
    ASSERT_EQ(demo_specific_decimal_place.value(), accessor_specific.get_value());
    ASSERT_EQ(decimal_place_count, demo_specific_decimal_place.decimal_place_count());
    ASSERT_EQ(demo_specific_decimal_place.decimal_place_count(), accessor_specific.get_decimal_place_count());
    ASSERT_EQ(NULL, accessor_specific.get_string());

    SET_G_RET_SUCCESS();
}

template<typename FT>
static void test_copy_constructor(
    FT tested_num,
    int decimal_place_count)
{
    SET_G_RET_FAILURE();

    calib::floating_point<FT> demo_src(tested_num);
    calib::floating_point_accessor<FT> accessor_src(demo_src);
    calib::floating_point<FT> demo_copy(demo_src);
    calib::floating_point_accessor<FT> accessor_copy(demo_copy);

    PRINT_FLOATING_POINT_VALUE(demo_src.value(), calib::floating_point<FT>::MAX_DECIMAL_PLACE_COUNT);
    PRINT_DECIMAL_PLACE_COUNT(demo_src.decimal_place_count());
    PRINT_FLOATING_POINT_VALUE(demo_copy.value(), calib::floating_point<FT>::MAX_DECIMAL_PLACE_COUNT);
    PRINT_DECIMAL_PLACE_COUNT(demo_copy.decimal_place_count());
    ASSERT_TRUE(demo_src.value() == demo_copy.value());
    ASSERT_EQ(demo_src.value(), demo_copy.value());
    ASSERT_EQ(demo_src.decimal_place_count(), demo_copy.decimal_place_count());
    ASSERT_EQ(calib::floating_point<FT>::DEFAULT_DECIMAL_PLACE_COUNT, demo_src.decimal_place_count());
    ASSERT_EQ(NULL, accessor_src.get_string());
    ASSERT_EQ(NULL, accessor_copy.get_string());

    SET_G_RET_SUCCESS();
}

template<typename FT>
static void test_constructors(
    FT tested_num,
    int decimal_place_count)
{
    ASSERT_TEST_OK(test_default_constructor<FT>(tested_num, decimal_place_count));
    ASSERT_TEST_OK(test_other_constructors<FT>(tested_num, decimal_place_count));
    ASSERT_TEST_OK(test_copy_constructor<FT>(tested_num, decimal_place_count));
}

template<typename FT>
static void test_assignment_operators(
    FT tested_num,
    int decimal_place_count)
{
    FT src_num = tested_num;
    FT modified_num = src_num;
    calib::floating_point<FT> for_raw_num;
    calib::floating_point_accessor<FT> accessor_for_raw(for_raw_num);
    calib::floating_point<FT> for_floating_class;
    calib::floating_point_accessor<FT> accessor_for_class(for_floating_class);

    for_raw_num = src_num;
    PRINT_DECIMAL_PLACE_COUNT(for_raw_num.decimal_place_count());
    ASSERT_EQ(calib::floating_point<FT>::DEFAULT_DECIMAL_PLACE_COUNT, for_raw_num.decimal_place_count());
    ASSERT_EQ(NULL, accessor_for_raw.get_string());
    calib::floating_point<FT>::change_precision(for_raw_num.decimal_place_count(), modified_num, NULL, NULL);
    ASSERT_TRUE(modified_num == for_raw_num.value());

    for_floating_class = for_raw_num;
    PRINT_DECIMAL_PLACE_COUNT(for_floating_class.decimal_place_count());
    ASSERT_EQ(calib::floating_point<FT>::DEFAULT_DECIMAL_PLACE_COUNT, for_floating_class.decimal_place_count());
    ASSERT_EQ(NULL, accessor_for_class.get_string());
    ASSERT_TRUE(for_floating_class.value() == for_raw_num.value());
}

TEST(floating_point, AllInOne)
{
    ASSERT_TEST_OK(test_limit_definitions());

    const float FLOAT_NUM = 123456.123456;
    const int FLOAT_DECIMAL_PLACE_COUNT = 5;
    const double DOUBLE_NUM = 123456789012345.123456789012345;
    const int DOUBLE_DECIMAL_PLACE_COUNT = 8;
    const long double LONG_DOUBLE_NUM = 123456789012345678.123456789012345678;
    const int LONG_DOUBLE_DECIMAL_PLACE_COUNT = 15;

    //ASSERT_TEST_OK(test_constructors<InvalidType>());
    ASSERT_TEST_OK(test_constructors<float>(FLOAT_NUM, FLOAT_DECIMAL_PLACE_COUNT));
    ASSERT_TEST_OK(test_constructors<double>(DOUBLE_NUM, DOUBLE_DECIMAL_PLACE_COUNT));
    ASSERT_TEST_OK(test_constructors<long double>(LONG_DOUBLE_NUM, LONG_DOUBLE_DECIMAL_PLACE_COUNT));

    ASSERT_TEST_OK(test_assignment_operators<float>(FLOAT_NUM, FLOAT_DECIMAL_PLACE_COUNT));
    ASSERT_TEST_OK(test_assignment_operators<double>(DOUBLE_NUM, DOUBLE_DECIMAL_PLACE_COUNT));
    ASSERT_TEST_OK(test_assignment_operators<long double>(LONG_DOUBLE_NUM, LONG_DOUBLE_DECIMAL_PLACE_COUNT));

    // TODO: Unfinished. Continue doing it while I'm not tired...
}

