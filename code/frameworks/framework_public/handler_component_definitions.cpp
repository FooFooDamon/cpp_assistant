/*
 * Copyright (c) 2016-2018, Wen Xiongchang <udc577 at 126 dot com>
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

#include "handler_component_definitions.h"

namespace cafw
{

#ifdef USE_JSON_MSG

DECLARE_ALLOC_FUNC(json)
{
    return new Json::Value();
}

DECLARE_ASSEMBLE_OUT_FUNC(minimal)
{
    DECLARE_AND_CAST(in_body, req, Json::Value);
    DECLARE_AND_CAST(out_body, resp, Json::Value);
    SET_RESP_PREFIX((*req), (*resp), retcode);
}

#endif // #ifdef USE_JSON_MSG

} // namespace cafw
