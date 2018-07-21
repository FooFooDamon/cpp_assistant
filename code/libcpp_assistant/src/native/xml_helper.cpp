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

#include "xml_helper.h"

#include <stdarg.h>

#include "ca_string.h"
#include "private/debug.h"

CA_LIB_NAMESPACE_BEGIN

/*
 * This function requires:
 *     @depth_recorder = -1,
 *     @result is empty
 * at first call.
 *
 * A key point is that @depth_recorder increments when going to upper level
 * (that is: entering this function), and decrements when going to lower level
 * (that is: exiting this function).
 *
 * TODO: Replace it with non-recursive version in future!
 */
static int __recursively_find_nodes(const xml::element_t *cur_node,
    const std::vector<std::string> &node_names,
    int &depth_recorder,
    std::vector<xml::element_t*> &result)
{
    ++depth_recorder;

    if (nullptr == cur_node)
    {
        --depth_recorder;
        nserror(xml, "null xpath or xml node pointer or path pointer\n");
        return CA_RET(NULL_PARAM);
    }

    int t = cur_node->Type();

    /*
     * Checks current node name and level(or depth) here, and:
     * 1) if current node has an invalid type or wrong name, or
     * 2) if the final level of path has been reached,
     * then the deeper searching to this node should be aborted and be switched to its siblings,
     * and the current node should be recorded if condition 2) is satisfied and it's name should
     * be validated.
     */

    const char *expected_node_name = node_names[depth_recorder].c_str();
    int expected_name_len = strlen(expected_node_name);
    const char *cur_node_name = cur_node->Value();
    int cur_name_len = strlen(cur_node_name);
    bool name_satisfied = ((expected_name_len == cur_name_len) &&
        (0 == strncmp(expected_node_name, cur_node_name, expected_name_len)));
    bool type_invalid = (TiXmlNode::TINYXML_ELEMENT != t);
    bool reaches_final_level = (int)(node_names.size() - 1) == depth_recorder;

    /*nsdebug(xml, "searching level %d: expected node name: %s, actual node name: %s\n",
        depth_recorder, expected_node_name, cur_node_name);*/
    if (type_invalid ||
        !name_satisfied ||
        reaches_final_level)
    {
        /*
         * KEY OPERATION: Adds current node into result set
         * when the final level of path has been reached
         */
        if (reaches_final_level &&
            name_satisfied)
        {
            try
            {
                result.push_back(const_cast<xml::element_t*>(cur_node));
            }
            catch (std::exception& e)
            {
                nserror(xml, "result.push_back() failed: %s\n", e.what());
                result.clear();
                --depth_recorder;
                return CA_RET(STL_ERROR);
            }
            /*nsdebug(xml, "reached the final level and the bottom node has been successfully"
                " added to result set, ptr = %p, depth = %d\n", cur_node, depth_recorder);*/
        }

        --depth_recorder;

        //return reaches_final_level ? CA_RET_OK : CA_RET_GENERAL_FAILURE;
        return CA_RET_OK;
    }

    //nsdebug(xml, "~ ~ ~ ~ ~ node %s validated, depth = %d\n", cur_node_name, depth_recorder);

    /*
     * Until now, node names are OK and the final level of path has not been reached,
     * keeps searching its recursive children for more info.
     */

    xml::element_t *child_node = nullptr;

    for (child_node = const_cast<xml::element_t*>(cur_node->FirstChildElement()); nullptr != child_node; child_node = child_node->NextSiblingElement())
    {
        int find_ret = __recursively_find_nodes(child_node, node_names, depth_recorder, result);

        if (0 != find_ret)
        {
            --depth_recorder;
            return find_ret;
        }
    }

    --depth_recorder;

    return CA_RET_OK;
}

static int __generally_find_nodes(const xml::element_t *cur_node,
    const char *path, /* relative or absolute */
    bool path_contains_cur_node,
    std::vector<xml::element_t*> &result)
{
    int path_len = strlen(path);
    std::vector<std::string> node_names;
    int ret = str::split(path, path_len, "/", node_names);
    int node_count = node_names.size();
    int find_ret = -1;

    if (0 != ret || 0 == node_count)
    {
        nserror(xml, "str::split() failed or invalid path string, original path string = %s,"
            " ret = %d, result size = %d\n", path, ret, node_count);
        return (ret < 0) ? ret : CA_RET_GENERAL_FAILURE;
    }
    //nsdebug(xml, "path[%s] has been split in to %d part(s) successfully\n", path, node_count);

    if (path_contains_cur_node)
    {
        int depth_recorder = -1; // Initial depth MUST be -1

        find_ret = __recursively_find_nodes(cur_node, node_names, depth_recorder, result);
        if (0 != find_ret)
            return find_ret;
    }
    else
    {
        xml::element_t *child_node = nullptr;

        for (child_node = const_cast<xml::element_t*>(cur_node->FirstChildElement()); nullptr != child_node; child_node = child_node->NextSiblingElement())
        {
            int depth_recorder = -1; // Initial depth MUST be -1. Re-declare it on every call to __recursively_find_nodes() for safety!

            find_ret = __recursively_find_nodes(child_node, node_names, depth_recorder, result);
            if (0 != find_ret)
                return find_ret;
        }
    }

    return result.empty() ? CA_RET(OBJECT_DOES_NOT_EXIST) : CA_RET_OK;
}

/* static */int xml::find_nodes(const element_t *parent_node,
    const char *relative_path,
    std::vector<element_t*> &result)
{
    if (!result.empty())
        result.clear();

    if (nullptr == relative_path)
    {
        nserror(xml, "null relative path pointer\n");
        return CA_RET(NULL_PARAM);
    }

    if ('\0' == relative_path[0]
        || '/' == relative_path[0])
    {
        nserror(xml, "invalid relative path: %s\n", relative_path);
        return CA_RET(INVALID_PATH);
    }

    return __generally_find_nodes(parent_node, relative_path, false, result);
}

/* static */int xml::find_nodes(const document_t &doc,
    const char *absolute_path,
    std::vector<element_t*> &result)
{
    if (!result.empty())
        result.clear();

    if (nullptr == absolute_path)
    {
        nserror(xml, "null absolute path pointer\n");
        return CA_RET(NULL_PARAM);
    }

    if ('\0' == absolute_path[0]
        || '/' != absolute_path[0])
    {
        nserror(xml, "invalid absolute path: %s\n", absolute_path);
        return CA_RET(INVALID_PATH);
    }

    const element_t *root = doc.RootElement();

    if (nullptr == root)
    {
        nserror(xml, "no root node found\n");
        return CA_RET(OBJECT_DOES_NOT_EXIST);
    }

    return __generally_find_nodes(root, absolute_path, true, result);
}

static int extract_xml_nodes(const std::vector<xml::element_t*> &nodes,
    size_t expected_node_count,
    bool allows_missing_attributes,
    const char *first_attr,
    va_list &other_attr,
    std::vector<xml::node_t> &result)
{
    int node_count = nodes.size();

    if (node_count <= 0)
        return CA_RET(OBJECT_DOES_NOT_EXIST);

    if ((size_t)node_count > expected_node_count)
        node_count = expected_node_count;

    //nsdebug(xml, "node size: %ld, expected node count: %d\n", nodes.size(), node_count);

    xml::node_t node;

    for (int i = 0; i < node_count; ++i)
    {
        xml::element_t *element = nodes[i];
        const char *element_text = element->GetText();

        node.node_value.clear();
        if (nullptr != element_text)
            node.node_value = element_text;
        node.attributes.clear();

        if (nullptr == first_attr)
        {
            result.push_back(node);
            continue;
        }

        va_list argp;
        const char *attribute = first_attr;

        va_copy(argp, other_attr);
        while (nullptr != attribute)
        {
            //nsdebug(xml, "Attribute = %s\n", attribute);
            const char *attr_value = element->Attribute(attribute);

            if (nullptr == attr_value)
            {
                attribute = va_arg(argp, const char*);
                if (allows_missing_attributes)
                    continue;

                nswarn(xml, "Attrbite[%s] of node[%s] does not exist.\n", attribute, element->Value());

                return CA_RET(OBJECT_DOES_NOT_EXIST);
            }

            //if (!node.attributes.insert(std::make_pair(attribute, attr_value)).second) // TODO
            if (!node.attributes.insert(std::make_pair(std::string(attribute), std::string(attr_value))).second)
                return CA_RET(STL_ERROR);

            attribute = va_arg(argp, const char*);
        } /* while (nullptr != attribute) */

        result.push_back(node);
    } /* for (i : element_count)*/

    return node_count;
}

/* static */int xml::find_and_parse_nodes(const element_t *parent_element,
    const char *relative_xpath,
    size_t expected_node_count,
    std::vector<node_t> &result,
    bool allows_missing_attributes/* = false*/,
    const char *first_attr/* = nullptr*/,
    .../* Must end with nullptr!!! */)
{
    result.clear();

    if (0 == expected_node_count)
        return CA_RET(INVALID_PARAM_VALUE);

    std::vector<element_t*> elements;
    int find_ret = find_nodes(parent_element, relative_xpath, elements);

    if (find_ret < 0)
        return find_ret;

    va_list argp;

    va_start(argp, first_attr);

    int extract_ret = extract_xml_nodes(elements,
        expected_node_count,
        allows_missing_attributes,
        first_attr,
        argp,
        result);

    va_end(argp);

    return extract_ret;
}

/* static */int xml::find_and_parse_nodes(const document_t &file,
    const char *absolute_xpath,
    size_t expected_node_count,
    std::vector<node_t> &result,
    bool allows_missing_attributes/* = false*/,
    const char *first_attr/* = nullptr*/,
    .../* Must end with nullptr!!! */)
{
    result.clear();

    if (0 == expected_node_count)
        return CA_RET(INVALID_PARAM_VALUE);

    std::vector<element_t*> elements;
    int find_ret = find_nodes(file, absolute_xpath, elements);

    if (find_ret < 0)
        return find_ret;

    va_list argp;

    va_start(argp, first_attr);

    int extract_ret = extract_xml_nodes(elements,
        expected_node_count,
        allows_missing_attributes,
        first_attr,
        argp,
        result);

    va_end(argp);

    return extract_ret;
}

CA_LIB_NAMESPACE_END
