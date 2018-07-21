/*
 * Copyright (c) 2017-2018, Wen Xiongchang <udc577 at 126 dot com>
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

#include "floating_point.h"

CA_LIB_NAMESPACE_BEGIN

DEFINE_TEMPLATE_CLASS_NAME(floating_point, FT);

template<> const int floating_point<float>::DEFAULT_DECIMAL_PLACE_COUNT = 4;
template<> const int floating_point<float>::MAX_DECIMAL_PLACE_COUNT = FLT_DIG;
template<> const int floating_point<float>::CHAR_COUNT_OF_MAX_VALUE = 63;
template<> const float floating_point<float>::MAX_VALUE = FLT_MAX;
template<> const float floating_point<float>::MIN_VALUE = FLT_MIN;
template<> const float floating_point<float>::EPSILON = FLT_EPSILON;

template<> const int floating_point<double>::DEFAULT_DECIMAL_PLACE_COUNT = 6;
template<> const int floating_point<double>::MAX_DECIMAL_PLACE_COUNT = DBL_DIG;
template<> const int floating_point<double>::CHAR_COUNT_OF_MAX_VALUE = 511;
template<> const double floating_point<double>::MAX_VALUE = DBL_MAX;
template<> const double floating_point<double>::MIN_VALUE = DBL_MIN;
template<> const double floating_point<double>::EPSILON = DBL_EPSILON;

template<> const int floating_point<long double>::DEFAULT_DECIMAL_PLACE_COUNT = 8;
template<> const int floating_point<long double>::MAX_DECIMAL_PLACE_COUNT = LDBL_DIG;
template<> const int floating_point<long double>::CHAR_COUNT_OF_MAX_VALUE = 5 * 1024 - 1;
template<> const long double floating_point<long double>::MAX_VALUE = LDBL_MAX;
template<> const long double floating_point<long double>::MIN_VALUE = LDBL_MIN;
template<> const long double floating_point<long double>::EPSILON = LDBL_EPSILON;

/*
 * NOTE: Definitions below just provide default values in order to avoid compile errors
 *     due to using invalid types(floating_point<unsigned int> for example) in application code.
 *     Invalid types should be used in testing code only.
 */
template<typename FT> const int floating_point<FT>::DEFAULT_DECIMAL_PLACE_COUNT = floating_point<float>::DEFAULT_DECIMAL_PLACE_COUNT;
template<typename FT> const int floating_point<FT>::MAX_DECIMAL_PLACE_COUNT = floating_point<float>::MAX_DECIMAL_PLACE_COUNT;
template<typename FT> const int floating_point<FT>::CHAR_COUNT_OF_MAX_VALUE = floating_point<float>::CHAR_COUNT_OF_MAX_VALUE;
template<typename FT> const FT floating_point<FT>::MAX_VALUE = floating_point<float>::MAX_VALUE;
template<typename FT> const FT floating_point<FT>::MIN_VALUE = floating_point<float>::MIN_VALUE;
template<typename FT> const FT floating_point<FT>::EPSILON = floating_point<float>::EPSILON;

CA_LIB_NAMESPACE_END
