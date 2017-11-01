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

/*
 * command_line.h
 *
 *  Created on: 2017-09-23
 *      Author: wenxiongchang
 * Description: Command line parser.
 */

#ifndef __CPP_ASSISTANT_COMMAND_LINE_H__
#define __CPP_ASSISTANT_COMMAND_LINE_H__

#include "base/ca_inner_necessities.h"
#include "base/stl_adapters/vector.h"
#include "base/stl_adapters/map.h"
#include "ca_string.h"
#include "system.h"

CA_LIB_NAMESPACE_BEGIN

class CommandLine
{
/* ===================================
 * constructors:
 * =================================== */
public:
    CommandLine();
    CommandLine(const char *usage_header, const char *usage_format);

/* ===================================
 * copy control:
 * =================================== */
private:
    CommandLine(const CommandLine& src);
    CommandLine& operator=(const CommandLine& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    ~CommandLine();

/* ===================================
 * types:
 * =================================== */
public:
    typedef struct OptionValue
    {
        std::string description;
        std::string assign_expression;
        int least_value_count;
        bool value_count_is_fixed;
        bool is_specified;
        std::vector<std::string> values;
    }OptionValue;

    typedef struct UserOption
    {
        const char *name; // "<short option>,<long option>", e.g: "h,help"
        const char *description;
        int least_value_count;
        bool value_count_is_fixed;
        const char *assign_expression;
        const char *default_values;
    }UserOption;

    enum
    {
        VALUE_COUNT_NOT_FIXED = false,
        VALUE_COUNT_IS_FIXED = true
    };

protected:
    typedef std::map<char*, OptionValue, CharLessOperator > OptionValueMap;
    typedef std::map<char*, char*, CharLessOperator > OptionOptionMap;

/* ===================================
 * abilities:
 * =================================== */
public:
    // Parses options from command line.
    // It should be called after all expected options are learned,
    // and it will fail if one or more than one invalid option(s) occur(s).
    int Parse(int argc, const char **argv);

    // Resets the instance so that it can parse command line again.
    void Reset(void);

    // Learns a new option to know how it should be like.
    // @name: Contains the short option name, or the short and long option name
    //     dividing by a comma. For example: "h" which gives a short option "-h"
    //     and a long option "--h", or "h,help" which gives a short option "-h"
    //     and a long option "--help".
    // @desc: Description of the new option.
    // @least_value_count: The least values needed.
    // @assign_expression: Shows the format and the meaning of the key-value pair.
    //     It's just a hint for readable usage information.
    //     For example: To the option "-f,--file", this can be "=FILE", or " FILE".
    //     To the option "-F,--files", this can be "=\"FILE1|FILE2\"", or " FILE1 FILE2".
    // @default_values: Default values of the option.
    //     For example: "default_file.txt", or "file1.txt file2.txt".
    // @value_count_is_fixed: If it's true, then @least_value_count is definitely the value
    //     count that this option needs, neither greater nor less than it.
    int LearnOption(const char *name,
        const char *desc,
        int least_value_count = 0,
        const char *assign_expression = "",
        const char *default_values = NULL,
        bool value_count_is_fixed = true);

    // Much like LearnOption() above, except that this function learns options in bulk.
    int LearOptions(const UserOption *option_array, int option_count);

    // Shows result after parsing in terminal if @holder is NULL, or saves the result to @holder.
    void ShowParsingResult(std::string *holder = NULL) const;

/* ===================================
 * attributes:
 * =================================== */
public:
    DEFINE_CLASS_NAME_FUNC()

    inline const char* program_directory(void) const
    {
        return m_program_directory.empty() ? NULL : m_program_directory.c_str();
    }

    inline const char* program_name(void) const
    {
        return m_program_name.empty() ? NULL : m_program_name.c_str();
    }

    inline void set_usage_header(const char *header)
    {
        if (NULL == header)
            return;

        m_usage_header = header;
    }

    inline void set_usage_format(const char *format)
    {
        if (NULL == format)
            return;

        m_usage_format = format;
    }

    // Displays usage info on terminal (if @holder is null)
    // or saves it to @holder string (if @holder is not null).
    void usage(std::string *holder = NULL) const;

    // Finds the value(s) of option with name @option_name,
    // and @option_name can be the short name or the long name.
    const OptionValue* option(const char *option_name) const;

    /*inline int option_count(void) const
    {
        return m_options
    }*/

    // Gets all single parameters which are not relative to any options.
    // They are usually files or targets.
    inline const std::vector<std::string>& single_parameters(void) const
    {
        return m_single_parameters;
    }

/* ===================================
 * status:
 * =================================== */
public:
    bool has_parsed(void) const
    {
        return m_has_parsed;
    }

/* ===================================
 * operators:
 * =================================== */
public:

/* ===================================
 * private methods:
 * =================================== */
protected:
    // Adds a value to the specified option holder if @option is not null,
    // or to the single parameters holder if @option is null.
    int AddValue(const char *option, const std::string &value);

/* ===================================
 * data:
 * =================================== */
protected:
    DECLARE_CLASS_NAME_VAR();
    std::string m_program_directory;
    std::string m_program_name;
    std::string m_usage_header;
    std::string m_usage_format;
    OptionValueMap m_options; // map<short name, value>
    OptionOptionMap m_option_relation; // map<long name, short name>
    std::vector<std::string> m_single_parameters;
    bool m_has_parsed;
};

typedef CommandLine CmdLine;

CA_LIB_NAMESPACE_END

#endif // __CPP_ASSISTANT_COMMAND_LINE_H__

