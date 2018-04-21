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

#include "command_line.h"

#include <stdlib.h>

#include "private/debug.h"

CA_LIB_NAMESPACE_BEGIN

DEFINE_CLASS_NAME(command_line);

command_line::command_line()
    : m_has_parsed(false)
{
    ;
}

command_line::command_line(const char *usage_header, const char *usage_format)
    : m_usage_header(usage_header)
    , m_usage_format(usage_format)
    , m_has_parsed(false)
{
    ;
}

command_line::~command_line()
{
    for (option_name_name_map::iterator option_iter = m_option_name_relations.begin();
        option_iter != m_option_name_relations.end();
        ++option_iter)
    {
        free(option_iter->first);
        free(option_iter->second);
    }
}

#define IS_LETTER(c)    (( ((c) >= 'a') && ((c) <= 'z') ) || ( ((c) >= 'A') && ((c) <= 'Z') ))

#define SET_IS_SPECIFIED(_option_)  do{\
    option_value_t* op_val = const_cast<option_value_t*>(get_option_value(_option_)); \
    if (NULL == op_val) \
    {\
        cerror("unknown short option: -%s\n", _option_); \
        return CA_RET(UNKNOWN_CMD_LINE_OPTION); \
    }\
    if (!op_val->is_specified) \
    {\
        op_val->is_specified = true; \
        op_val->values.clear(); \
    }\
}while(0)

int command_line::parse(int argc, const char **argv)
{
    if (NULL == argv)
        return CA_RET(NULL_PARAM);

    if (argc < 0)
        return CA_RET(VALUE_OUT_OF_RANGE);

    if (m_has_parsed)
        return CA_RET(OPERATION_ALREADY_DONE);

    m_has_parsed = true;

    char *program_name = strrchr((char*)argv[0], '/');

    if (NULL == program_name)
    {
        m_program_name = argv[0];
        m_program_directory = ".";
    }
    else
    {
        m_program_directory.assign(argv[0], program_name - argv[0]);
        m_program_name = (++program_name);
    }

    char *short_option = NULL;
    char *long_option = NULL;
    std::string long_option_tmp;

    #define ADD_VALUE(_option_, _value_) do{\
        int add_value_ret = add_value(_option_, _value_); \
        if (CA_RET(UNKNOWN_CMD_LINE_OPTION) == add_value_ret) \
            return CA_RET(UNKNOWN_CMD_LINE_OPTION);\
        if (CA_RET(EXCESS_OBJECT_COUNT) == add_value_ret) \
        {\
            short_option = NULL; \
            long_option = NULL; \
            add_value(NULL, _value_); \
        }\
    }while(0)

    for (int i = 1; i < argc; ++i)
    {
        if (NULL == argv[i])
            continue;

        char *arg_value = const_cast<char*>(argv[i]);
        int arg_len = strlen(arg_value);

        if (arg_len <= 0)
            continue;
        else if (1 == arg_len)
        {
            ADD_VALUE(short_option, arg_value); // "-" is acceptable too. e.g: cd -
        }
        else if (2 == arg_len)
        {
            /*
             * It's the value of an option, or a single parameter.
             */
            if ('-' != arg_value[0])
            {
                ADD_VALUE(short_option, arg_value);
                continue;
            }

            if ('-' == arg_value[1]) // invalid
                continue;

            /*
             * It's the key of a short option.
             */
            if (IS_LETTER(arg_value[1]))
            {
                short_option = arg_value + 1;
                SET_IS_SPECIFIED(short_option);
                continue;
            }

            /*
             * It's the value of an option, or a single parameter.
             */
            ADD_VALUE(short_option, arg_value + 1);
        } // else if (arg_len == 2)
        else
        {
            /*
             * It's the value of an option, or a single parameter.
             */
            if ('-' != arg_value[0])
            {
                ADD_VALUE(short_option, arg_value);
                continue;
            }

            if ('-' != arg_value[1])
            {
                /*
                 * It's the value of an option, or a single parameter.
                 */
                if (!IS_LETTER(arg_value[1]))
                {
                    ADD_VALUE(short_option, arg_value/* + 1 NOTE: In case that it's a negative number.*/);
                    continue;
                }

                /*
                 * It's a short options cluster, like: tar -zxvf foo.tar.gz
                 */

                std::string short_option_piece;

                for (int i = 1; i < arg_len; ++i)
                {
                    short_option_piece = arg_value[i];
                    SET_IS_SPECIFIED(short_option_piece.c_str());
                }

                short_option = arg_value + arg_len - 1;

                continue;
            }

            /*
             * It's the key of a long option.
             */

            char *option_start = arg_value + 2;
            char *equal_sign_pos = strchr(option_start, '=');

            if (NULL == equal_sign_pos)
                long_option = option_start;
            else if (0 == equal_sign_pos - option_start
                || arg_value + arg_len - 1 == equal_sign_pos)
            {
                cerror("invalid long option format: %s\n", arg_value);
                return CA_RET(INVALID_CMD_LINE_FORMAT);
            }
            else
            {
                long_option_tmp.clear();
                long_option_tmp.assign(option_start, equal_sign_pos - option_start);
                long_option = (char*)long_option_tmp.c_str();
            }

            option_name_name_map::iterator option_iter = m_option_name_relations.find(long_option);

            if (m_option_name_relations.end() == option_iter)
            {
                cerror("unknown long option: --%s\n", long_option);
                return CA_RET(UNKNOWN_CMD_LINE_OPTION);
            }
            else
                short_option = option_iter->second;

            SET_IS_SPECIFIED(short_option);

            if (NULL != equal_sign_pos)
            {
                int values_len = arg_len - (equal_sign_pos - arg_value) + 1;
                std::vector<std::string> long_option_values;

                str::split(equal_sign_pos + 1, values_len, "|", long_option_values);
                for (size_t value_count = 0; value_count < long_option_values.size(); ++value_count)
                {
                    add_value(short_option, long_option_values[value_count]);
                }
            }
        } // else (arg_len > 2)
    } // for ( i in [1, argc) )

    for (option_name_value_map::iterator op_val_iter = m_option_entries.begin();
        op_val_iter != m_option_entries.end();
        ++op_val_iter)
    {
        option_value_t &op_val = op_val_iter->second;

        if (op_val.is_specified
            && op_val.values.size() < (size_t)op_val.least_value_count)
        {
            cerror("Parameters inefficient, target option: -%s\n", op_val_iter->first);
            return CA_RET(CMD_LINE_PARAM_INSUFFICIENT);
        }
    }

    return CA_RET_OK;
}

void command_line::reset(void)
{
    for (option_name_value_map::iterator op_val_iter = m_option_entries.begin();
        op_val_iter != m_option_entries.end();
        ++op_val_iter)
    {
        op_val_iter->second.values.clear();
        op_val_iter->second.is_specified = false;
    }
    m_single_parameters.clear();
    m_has_parsed = false;
}

int command_line::learn_option(const char *name_combination,
    const char *desc,
    int least_value_count/* = 0*/,
    const char *assign_expression/* = ""*/,
    const char *default_values/* = NULL*/,
    bool value_count_is_fixed/* = true*/)
{
    if (NULL == name_combination || NULL == desc)
        return CA_RET(NULL_PARAM);

    if (least_value_count < 0)
        return CA_RET(VALUE_OUT_OF_RANGE);

    std::vector<std::string> option_names;
    int ret = str::split(name_combination, strlen(name_combination), ",", option_names);

    if (0 != ret)
    {
        cerror("invalid option name format.\n");
        return CA_RET(INVALID_PARAM_VALUE);
    }
    /*cdebug("original option name: %s, fragments after split(): %lu\n",
        name, option_names.size());*/

    char *short_name = const_cast<char*>(option_names[0].c_str());
    int short_name_len = option_names[0].length();

    if (m_option_entries.end() != m_option_entries.find(short_name))
    {
        cerror("short option \"%s\" already exists\n", short_name);
        return CA_RET(OBJECT_ALREADY_EXISTS);
    }
    //cdebug("short option name: %s\n", short_name);

    short_name = (char*)malloc(short_name_len + 1);
    if (NULL == short_name)
    {
        cerror("failed to allocate memory for short option\n");
        return CA_RET(MEMORY_ALLOC_FAILED);
    }
    strncpy(short_name, option_names[0].c_str(), short_name_len + 1);

    int long_name_index = (option_names.size() > 1) ? 1 : 0;
    char *long_name = const_cast<char*>(option_names[long_name_index].c_str());
    int long_name_len = (option_names.size() > 1) ? option_names[1].length()
        : option_names[0].length();

    if (m_option_name_relations.end() != m_option_name_relations.find(long_name))
    {
        cerror("long option \"%s\" already exists\n", long_name);
        free(short_name);
        return CA_RET(OBJECT_ALREADY_EXISTS);
    }
    //cdebug("long option name: %s\n", long_name);

    long_name = (char*)malloc(long_name_len + 1);
    if (NULL == long_name)
    {
        free(short_name);
        cerror("failed to allocate memory for long option\n");
        return CA_RET(MEMORY_ALLOC_FAILED);
    }
    strncpy(long_name, option_names[long_name_index].c_str(), long_name_len + 1);

    if (!m_option_name_relations.insert(std::make_pair(long_name, short_name)).second)
    {
        free(short_name);
        free(long_name);
        cerror("failed to create a new relation map item for long option[%s]"
            " and short option[%s]\n", long_name, short_name);
        return CA_RET(STL_ERROR);
    }

    option_value_t value = {
        /*.description = */desc,
        /*.assign_expression = */(NULL != assign_expression) ? assign_expression : "",
        /*.least_value_count = */least_value_count,
        /*.value_count_is_fixed = */value_count_is_fixed,
        /*.is_specified = */false,
        /*.values = */std::vector<std::string>()
    };

    if (NULL != default_values)
        str::split(default_values, strlen(default_values), " ", value.values);

    if (!m_option_entries.insert(std::make_pair(short_name, value)).second)
    {
        m_option_name_relations.erase(long_name);
        free(short_name);
        free(long_name);
        cerror("failed to create a new map item for short option[%s]\n", short_name);
        return CA_RET(STL_ERROR);
    }

    /*option_value_t &value_ref = m_options[short_name];

    value_ref.description = desc;
    if (NULL != assign_expression)
        value_ref.assign_expression = assign_expression;
    value_ref.least_value_count = least_value_count;
    value_ref.is_specified = false;
    if (NULL != default_values)
        str::split(default_values, strlen(default_values), " ", value_ref.values);*/

    return CA_RET_OK;
}

int command_line::learn_options(const user_option *option_array, int option_count)
{
    if (NULL == option_array || option_count <= 0)
        return CA_RET(INVALID_PARAM_VALUE);

    for (int i = 0; i < option_count; ++i)
    {
        const user_option &option_info = option_array[i];

        int learn_ret = learn_option(option_info.full_name,
            option_info.description,
            option_info.least_value_count,
            option_info.assign_expression,
            option_info.default_values);

        if (learn_ret < 0)
        {
            cerror("Failed to learn option[%s], ret = %d\n",
                option_info.full_name, learn_ret);
            return learn_ret;
        }
    }

    return CA_RET_OK;
}

void command_line::get_parsing_result(std::string *holder/* = NULL*/) const
{
    if (NULL != holder)
        holder->clear();

    std::string tmp;
    std::string *result_ptr = (NULL != holder) ? holder : &tmp;
    char tmp_str[32];

    if (!m_has_parsed)
    {
        *result_ptr = "Command line parsing has not been executed yet.";
        if (NULL == holder)
            printf("%s\n", result_ptr->c_str());
        return;
    }

    result_ptr->append("Command line parsing result:\n");
    result_ptr->append("Program directory: ").append(program_directory()).append("\n");
    result_ptr->append("Program name: ").append(program_name()).append("\n");
    result_ptr->append("=========================\n");

    int option_count = 0;
    int single_param_count = 0;

    result_ptr->append(">>> values of each option:\n");
    for (option_name_value_map::const_iterator op_val_iter = m_option_entries.begin();
        op_val_iter != m_option_entries.end();
        ++op_val_iter)
    {
        const option_value_t &value = op_val_iter->second;

        if (!value.is_specified)
            continue;

        ++option_count;

        result_ptr->append("********** -")
            .append(op_val_iter->first)
            .append(" **********\n");
        memset(tmp_str, 0, sizeof(tmp_str));
        snprintf(tmp_str, sizeof(tmp_str), "%d", value.least_value_count);
        result_ptr->append("least value count: ")
            .append(tmp_str)
            .append("\n");
        memset(tmp_str, 0, sizeof(tmp_str));
        snprintf(tmp_str, sizeof(tmp_str), "%lu", value.values.size());
        result_ptr->append(tmp_str).append(" (default)value(s):");
        for (size_t i = 0; i < value.values.size(); ++i)
        {
            result_ptr->append(" [").append(value.values[i]).append("]");
        }
        result_ptr->append("\n");
    }
    memset(tmp_str, 0, sizeof(tmp_str));
    snprintf(tmp_str, sizeof(tmp_str), "%d", option_count);
    result_ptr->append(">>> summary: ").append(tmp_str).append(" option(s) in total\n");
    result_ptr->append("=========================\n");

    result_ptr->append(">>> single parameter(s):");
    for (size_t i = 0; i < m_single_parameters.size(); ++i)
    {
        result_ptr->append(" [").append(m_single_parameters[i]).append("]");
        ++single_param_count;
    }
    result_ptr->append("\n");
    memset(tmp_str, 0, sizeof(tmp_str));
    snprintf(tmp_str, sizeof(tmp_str), "%d", single_param_count);
    result_ptr->append(">>> summary: ").append(tmp_str).append(" single parameter(s) in total\n");
    result_ptr->append("=========================");

    if (NULL == holder)
        printf("%s\n", result_ptr->c_str());
}

void command_line::usage(std::string *holder/* = NULL*/) const
{
    if (NULL != holder)
        holder->clear();

    const char *program_name = m_program_name.empty() ? "[Unknown program name]"
        : m_program_name.c_str();

    if (!m_usage_header.empty())
    {
        if (NULL != holder)
        {
            *holder += program_name;
            *holder += " - ";
            *holder += m_usage_header;
            *holder += "\n";
        }
        else
            printf("%s - %s\n", program_name, m_usage_header.c_str());
    }

    const char *usage_format = m_usage_format.empty() ? "[options ...] [targets ...]"
        : m_usage_format.c_str();

    if (NULL != holder)
    {
        *holder += "Format: ";
        *holder += program_name;
        *holder += " ";
        *holder += usage_format;
        *holder += "\n";
    }
    else
        printf("Format: %s %s\n", program_name, usage_format);

    for (option_name_name_map::const_iterator option_iter = m_option_name_relations.begin();
        option_iter != m_option_name_relations.end();
        ++option_iter)
    {
        option_name_value_map::const_iterator op_val_iter = m_option_entries.find(option_iter->second);

        if (NULL != holder)
        {
            *holder += "-";
            *holder += op_val_iter->first;
            *holder += ", --";
            *holder += option_iter->first;
            *holder += op_val_iter->second.assign_expression;
            *holder += ": ";
            *holder += op_val_iter->second.description;
            *holder += "\n";
        }
        else
        {
            printf("-%s, --%s%s: %s\n", op_val_iter->first, option_iter->first,
                op_val_iter->second.assign_expression.c_str(),
                op_val_iter->second.description.c_str());
        }
    }
}

const command_line::option_value_t* command_line::get_option_value(const char *option_name) const
{
    if (NULL == option_name)
        return NULL;

    option_name_value_map::const_iterator op_val_iter = m_option_entries.end();
    option_name_name_map::const_iterator option_iter = m_option_name_relations.find((option_name_t)option_name);

    if (m_option_name_relations.end() == option_iter)
        op_val_iter = m_option_entries.find((option_name_t)option_name); // @option_name may be a short name.
    else
        op_val_iter = m_option_entries.find(option_iter->second); // @option_name is a long name.

    if (m_option_entries.end() == op_val_iter)
        return NULL;

    return &(op_val_iter->second);
}

int command_line::add_value(const char *option, const std::string &value)
{
    if (NULL == option)
    {
        m_single_parameters.push_back(value);
        //cdebug("%s added into m_single_parameters successfully\n", value.c_str());
        return CA_RET_OK;
    }

    option_name_value_map::iterator op_val_iter = m_option_entries.find((char*)option);

    if (m_option_entries.end() == op_val_iter)
    {
        cwarn("unknown short option: -%s\n", option);
        return CA_RET(UNKNOWN_CMD_LINE_OPTION);
    }

    option_value_t &op_val = op_val_iter->second;

    if (op_val.value_count_is_fixed
        && op_val.values.size() >= (size_t)op_val.least_value_count)
    {
        /*cwarn("short option[-%s]: value count has reached to fixed limit[%d],"
            " can not add new value to it, try to treat this value as a single parameter.\n",
            option, op_val.least_value_count);*/
        return CA_RET(EXCESS_OBJECT_COUNT);
    }

    op_val.values.push_back(value);
    //cdebug("%s added into option[-%s] successfully\n", value.c_str(), op_val_iter->first);

    return CA_RET_OK;
}

CA_LIB_NAMESPACE_END
