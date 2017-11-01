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

#include "base/ca_return_code.h"
#define CA_USE_STL 1 // gtest uses STL, thus it's difficult to test the version without STL.
//#define TIXML_USE_STL "YES"
#include "xml_helper.h"

enum
{
    // Being greater than 1 is acceptable to TinyXML (TinyXML just takes the first root node and its sub-nodes)
    // but not to a web browser.
    ROOT_NODE_COUNT = 1,

    // This constant CANNOT be too big, otherwise it may lead to exponential explosion!
    MAX_NODE_LEVEL = 5,
};

#define XML_FILE_NAME               "test_xml_helper.xml"
#define PRETEST_CONTENTS            "中文：这是热身测试。English: this is pretest."

static void make_node_header(int node_level)
{
    char cmd[4096] = {0};
    char tabs[2048] = {0};

    for (int i = 0; i < node_level; ++i)
    {
        strncat(tabs, "\\t", 2);
    }
    snprintf(cmd, sizeof(cmd), "printf \"%s<level%d> this is level %d.\\n\" >> "XML_FILE_NAME,
        tabs, node_level, node_level);
    system(cmd);
}

static void make_node_footer(int node_level)
{
    char cmd[4096] = {0};
    char tabs[2048] = {0};

    for (int i = 0; i < node_level; ++i)
    {
        strncat(tabs, "\\t", 2);
    }
    snprintf(cmd, sizeof(cmd), "printf \"%s</level%d><!-- end of level%d -->\\n\" >> "XML_FILE_NAME,
        tabs, node_level, node_level);
    system(cmd);
}

static void make_nodes(int node_level)
{
    if (node_level < MAX_NODE_LEVEL)
    {
        for (int i = 0; i < node_level; ++i)
        {
            make_node_header(node_level);

            make_nodes(node_level + 1);

            make_node_footer(node_level);
        }
    }
    else
    {
        for (int i = 0; i < MAX_NODE_LEVEL; ++i)
        {
            make_node_header(node_level);
            make_node_footer(node_level);
        }
    }
}

static void make_xml(void)
{
    system("echo \"<?xml version=\\\"1.0\\\"?>\" > "XML_FILE_NAME);
    for (int root_count = 0; root_count < ROOT_NODE_COUNT; ++root_count)
    {
        system("echo \"<root>\" >> "XML_FILE_NAME);
        if (0 == root_count)
            system("printf \"\\t<pretest id=\\\"0\\\"> 中文：这是热身测试。English: this is pretest. </pretest>\\n\" >> "XML_FILE_NAME);

        make_nodes(1);

        system("echo \"</root>\" >> "XML_FILE_NAME);
    }
    //system("sed -i '/^\\-e/s///g' "XML_FILE_NAME);
}

static int calc_absolute_node_count(int node_level)
{
    if (node_level <= 0 || node_level > MAX_NODE_LEVEL)
        return 0;

    int count = 1;

    for (int i = 2; i <= node_level; ++i)
    {
        count *= i;
    }

    return count; // * ROOT_NODE_COUNT;
}

static int calc_relative_node_count(int self_node_level, int parent_node_level)
{
    if (self_node_level <= 0 || self_node_level > MAX_NODE_LEVEL)
        return 0;

    int count = calc_absolute_node_count(self_node_level);

    if (self_node_level <= 1)
        return count;

    for (int i = parent_node_level; i > 0; --i)
    {
        count /= i;
    }

    return count;
}

TEST(Xml, InvalidConditions)
{
    std::vector<TiXmlElement*> result;
    TiXmlDocument unused_doc;
    const TiXmlElement *root = unused_doc.RootElement();
    const TiXmlElement *INVALID_NODE = reinterpret_cast<const TiXmlElement *>(0x12345);
    const char *EMPTY_PATH = "\0";
    const char *ABSOLUTE_PATH = "/root/level1";
    const char *RELATIVE_PATH = "root/level1";

    ASSERT_EQ(NULL, root);

    /*
     * For find_nodes() with absolute path.
     */
    ASSERT_EQ(CA_RET(NULL_PARAM), calib::xml::find_nodes(unused_doc, NULL, result));
    ASSERT_EQ(0, result.size());
    ASSERT_EQ(CA_RET(INVALID_PATH), calib::xml::find_nodes(unused_doc, EMPTY_PATH, result));
    ASSERT_EQ(0, result.size());
    ASSERT_EQ(CA_RET(INVALID_PATH), calib::xml::find_nodes(unused_doc, RELATIVE_PATH, result));
    ASSERT_EQ(0, result.size());
    ASSERT_EQ(CA_RET(OBJECT_DOES_NOT_EXIST), calib::xml::find_nodes(unused_doc, ABSOLUTE_PATH, result));
    ASSERT_EQ(0, result.size());

    /*
     * For find_nodes() with relative path.
     */
    ASSERT_EQ(CA_RET(NULL_PARAM), calib::xml::find_nodes(NULL, NULL, result));
    ASSERT_EQ(0, result.size());
    ASSERT_EQ(CA_RET(NULL_PARAM), calib::xml::find_nodes(INVALID_NODE, NULL, result));
    ASSERT_EQ(0, result.size());
    ASSERT_EQ(CA_RET(INVALID_PATH), calib::xml::find_nodes(INVALID_NODE, EMPTY_PATH, result));
    ASSERT_EQ(0, result.size());
    ASSERT_EQ(CA_RET(INVALID_PATH), calib::xml::find_nodes(INVALID_NODE, ABSOLUTE_PATH, result));
    ASSERT_EQ(0, result.size());
}

TEST(Xml, ReadingAndWriting)
{
    make_xml();

    TiXmlDocument doc(XML_FILE_NAME);
    bool load_ok = doc.LoadFile();

    ASSERT_TRUE(load_ok);

    const TiXmlElement *root_element = doc.RootElement();
    const TiXmlElement *first_element = doc.FirstChildElement();
    const TiXmlNode *first_node = doc.FirstChild();

    printf("root element: name = %s, ptr = %p\n", root_element->Value(), root_element);
    printf("first element: name = %s, ptr = %p\n", first_element->Value(), first_element);
    printf("first node: name = %s, ptr = %p\n", first_node->Value(), first_node);
    if (first_node->ToElement() != first_element)
        printf("first node != first element\n");
    if (&doc != root_element->Parent()->ToDocument())
        printf("doc != root_element->Parent()->ToDocument()\n");
    if (&doc != first_element->Parent()->ToDocument())
        printf("doc != first_element->Parent()->ToDocument()\n");
    if (&doc != first_node->Parent()->ToDocument())
        printf("doc != first_node->Parent()->ToDocument()\n");

    std::vector<TiXmlElement*> outer_nodes;
    std::vector<TiXmlElement*> inner_nodes;
    int find_ret = -1;
    const char *PRETEST_PATH = "/root/pretest";
    const int EXPECTED_PRETEST_COUNT = 1;
    int actual_pretest_count = 0;

    find_ret = calib::xml::find_nodes(doc, PRETEST_PATH, outer_nodes);
    actual_pretest_count = outer_nodes.size();
    printf("pretest: ret = %d, expected node count = %d, actual node count = %d\n",
        find_ret, EXPECTED_PRETEST_COUNT, actual_pretest_count);
    ASSERT_EQ(CA_RET_OK, find_ret);
    ASSERT_EQ(EXPECTED_PRETEST_COUNT, actual_pretest_count);
    ASSERT_TRUE(NULL != outer_nodes[0]);
    printf("contents of [%s]: %s\n", PRETEST_PATH, outer_nodes[0]->GetText());
    ASSERT_STREQ(PRETEST_CONTENTS, outer_nodes[0]->GetText());

    for (int root_count = 0; root_count < MAX_NODE_LEVEL; ++root_count)
    {
        for (int i = 1; i <= MAX_NODE_LEVEL; ++i)
        {
            char absolute_path[1024] = {0};
            char node_contents[1024] = {0};
            const int kExpectedAbsoluteNodeCount = calc_absolute_node_count(i);
            int actual_node_count = 0;

            snprintf(absolute_path, sizeof(absolute_path), "%s", "/root");
            for (int level = 1; level <= i; ++level)
            {
                char level_str[32] = {0};

                snprintf(level_str, sizeof(level_str), "/level%d", level);
                strcat(absolute_path, level_str);
            }
            find_ret = calib::xml::find_nodes(doc, absolute_path, outer_nodes);
            actual_node_count = outer_nodes.size();
            printf("absolute path[%s]: ret = %d, expected node count = %d, actual node count = %d\n",
                absolute_path, find_ret, kExpectedAbsoluteNodeCount, actual_node_count);
            ASSERT_EQ(CA_RET_OK, find_ret);
            ASSERT_EQ(kExpectedAbsoluteNodeCount, actual_node_count);
            snprintf(node_contents, sizeof(node_contents), "this is level %d.", i);
            for (int j = 0; j < kExpectedAbsoluteNodeCount; ++j)
            {
                ASSERT_TRUE(NULL != outer_nodes[j]);
                ASSERT_STREQ(node_contents, outer_nodes[j]->GetText());

                if (i >= MAX_NODE_LEVEL)
                    continue;

                char relative_path[1024] = {0};

                for (int k = i + 1; k <= MAX_NODE_LEVEL; ++k)
                {
                    char level_str[32] = {0};
                    const int kExpectedRelativeNodeCount = calc_relative_node_count(k, i);
                    char inner_node_contents[1024] = {0};

                    snprintf(level_str, sizeof(level_str), "level%d/", k);
                    strcat(relative_path, level_str);

                    find_ret = calib::xml::find_nodes(outer_nodes[j], relative_path, inner_nodes);
                    actual_node_count = inner_nodes.size();
                    printf("sub-node[%s] of node[%s|%p]: ret = %d, expected node count = %d, actual node count = %d\n",
                        relative_path, absolute_path, outer_nodes[j], find_ret, kExpectedRelativeNodeCount, actual_node_count);
                    ASSERT_EQ(CA_RET_OK, find_ret);
                    ASSERT_EQ(kExpectedRelativeNodeCount, actual_node_count);
                    snprintf(inner_node_contents, sizeof(inner_node_contents), "this is level %d.", k);
                    for (int l = 0; l < kExpectedRelativeNodeCount; ++l)
                    {
                        ASSERT_TRUE(NULL != inner_nodes[l]);
                        ASSERT_STREQ(inner_node_contents, inner_nodes[l]->GetText());
                    }
                }
            }
        }
    }

    // TODO: tests for very long paths.
}
