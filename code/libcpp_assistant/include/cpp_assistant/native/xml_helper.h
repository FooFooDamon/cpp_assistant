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

/*
 * xml_helper.h
 *
 *  Created on: 2016/10/13
 *      Author: wenxiongchang
 * Description: Utilities based on TinyXML library.
 */

#ifndef __CPP_ASSISTANT_XML_HELPER_H__
#define __CPP_ASSISTANT_XML_HELPER_H__

#include <string>
#include <vector>
#include <map>

#include "base/ca_inner_necessities.h"

class TiXmlElement;
class TiXmlDocument;

CA_LIB_NAMESPACE_BEGIN

class xml : public no_instance
{
public:

    typedef TiXmlElement element_t; // XML node as a whole

    typedef struct node_t
    {
        //std::string node_name;
        std::string node_value;
        std::map<std::string, std::string> attributes; // <name, value>
    }node_t; // XML node which has been parsed into several parts

    typedef TiXmlDocument document_t;

    // Finds nodes with a relative path specified by @relative_path, starting from a node
    // specified by @parent_node, and saves them to a holder specified by @result.
    // NOTE: Pointers within @result are valid only if @parent_node is valid.
    static int find_nodes(const element_t *parent_node,
        const char *relative_path,
        std::vector<element_t*> &result);

    // Finds nodes with an absolute path specified by @relative_path from a document handle
    // specified by @doc, and saves them to a holder specified by @result.
    // NOTE: Pointers within @result are valid only if @doc is valid.
    static int find_nodes(const document_t &doc,
        const char *absolute_path,
        std::vector<element_t*> &result);

    static int find_and_parse_nodes(const element_t *parent_element,
        const char *relative_xpath,
        size_t expected_node_count,
        std::vector<node_t> &result,
        bool allows_missing_attributes = false,
        const char *first_attr = nullptr,
        .../* Must end with nullptr!!! */);

    static int find_and_parse_nodes(const document_t &file,
        const char *absolute_xpath,
        size_t expected_node_count,
        std::vector<node_t> &result,
        bool allows_missing_attributes = false,
        const char *first_attr = nullptr,
        .../* Must end with nullptr!!! */);
}; // class xml

CA_LIB_NAMESPACE_END

#endif /* __CPP_ASSISTANT_XML_HELPER_H__ */
