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

/*
 * command_line.h
 *
 *  Created on: 2017-09-23
 *      Author: wenxiongchang
 * Description: Command line parser.
 */

#ifndef __CPP_ASSISTANT_COMMAND_LINE_H__
#define __CPP_ASSISTANT_COMMAND_LINE_H__

#include <vector>
#include <map>

#include "base/ca_inner_necessities.h"
#include "ca_string.h"
#include "system.h"

CA_LIB_NAMESPACE_BEGIN

class command_line
{
/* ===================================
 * constructors:
 * =================================== */
public:
    command_line();
    command_line(const char *usage_header, const char *usage_format);

/* ===================================
 * copy control:
 * =================================== */
private:
    command_line(const command_line& src);
    command_line& operator=(const command_line& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    ~command_line();

/* ===================================
 * types:
 * =================================== */
public:
    typedef char* option_name_t;

    typedef struct option_value_t
    {
        std::string description;
        std::string assign_expression;
        int least_value_count;
        bool value_count_is_fixed;
        bool is_specified;
        std::vector<std::string> values;
    }option_value_t; // generally used after parsing

    typedef struct user_option
    {
        const char *full_name; // "<short option>,<long option>", e.g: "h,help"
        const char *description;
        int least_value_count;
        bool value_count_is_fixed;
        const char *assign_expression;
        const char *default_values;
    }user_option; // generally used before learning

    enum
    {
        VALUE_COUNT_NOT_FIXED = false,
        VALUE_COUNT_IS_FIXED = true
    };

protected:
    typedef std::map<option_name_t, option_value_t, char_lt_op > option_name_value_map;
    typedef option_name_value_map opt_name_val_map;
    typedef std::map<option_name_t, option_name_t, char_lt_op > option_name_name_map;
    typedef option_name_name_map option_names_map;
    typedef option_name_name_map opt_names_map;

/* ===================================
 * abilities:
 * =================================== */
public:
    // Parses options from command line.
    // It should be called after all expected options are learned,
    // and it will fail if one or more than one invalid option(s) occur(s).
    int parse(int argc, const char **argv);

    // Resets the instance so that it can parse command line again.
    void reset(void);

    // Learns a new option to know how it should be like.
    // @name_combination: Combinations of short option name and long option name.
    //     It may contains the short option name only, or the long name only, or both together
    //     dividing by a comma. For example:
    //     1) "h," which gives a short option "-h" and a long option "--h";
    //     2) "h,help" which gives a short option "-h" and a long option "--help".
    //     3) ",help" which gives a short option "--help" and a long option "--help" (TODO: not supported yet),
    //        but the short one can not be used.
    //        Think about a program has used all alphabets (lower case ones and upper case ones)
    //        as short option names, and then it wants a new option. It's pity that the new option
    //        only gets a long name.
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
    int learn_option(const char *name_combination,
        const char *desc,
        int least_value_count = 0,
        const char *assign_expression = "",
        const char *default_values = nullptr,
        bool value_count_is_fixed = true);

    // Much like learn_option() above, except that this function learns options in bulk.
    int learn_options(const user_option *option_array, int option_count);

    // Shows result after parsing in terminal if @holder is nullptr, or saves the result to @holder.
    void get_parsing_result(std::string *holder = nullptr) const;

/* ===================================
 * attributes:
 * =================================== */
public:
    //DEFINE_CLASS_NAME_FUNC()

    // Returns the directory where the host program is in.
    inline const char* program_directory(void) const
    {
        return m_program_directory.empty() ? nullptr : m_program_directory.c_str();
    }

    // Returns the host program name.
    inline const char* program_name(void) const
    {
        return m_program_name.empty() ? nullptr : m_program_name.c_str();
    }

    // Generally sets the info that shows what the host program can do.
    // For example: ls - list directory contents.
    inline void set_usage_header(const char *header)
    {
        if (nullptr == header)
            return;

        m_usage_header = header;
    }

    // Generally sets the info that shows how the host program should be executed.
    // For example: ls [OPTION]... [FILE]...
    inline void set_usage_format(const char *format)
    {
        if (nullptr == format)
            return;

        m_usage_format = format;
    }

    // Displays usage info on terminal (if @holder is null)
    // or saves it to @holder string (if @holder is not null).
    // NOTE: The usage info includes usage header, usage format and options details.
    void usage(std::string *holder = nullptr) const;

    // Finds the value(s) of option with name @option_name,
    // and @option_name can be the short name or the long name.
    const option_value_t* get_option_value(const char *option_name) const;

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

    // Returns map<short name, value>.
    inline const opt_name_val_map& all_option_entries(void) const
    {
        return m_option_entries;
    }

    // Returns map<long name, short name>.
    inline const opt_names_map& all_option_name_relations(void) const
    {
        return m_option_name_relations;
    }

/* ===================================
 * status:
 * =================================== */
public:
    // Checks if the current command line object has done the parsing job.
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
    int add_value(const char *option, const std::string &value);

/* ===================================
 * data:
 * =================================== */
protected:
    //DECLARE_CLASS_NAME_VAR();
    std::string m_program_directory;
    std::string m_program_name;
    std::string m_usage_header;
    std::string m_usage_format;
    option_name_value_map m_option_entries; // map<short name, value>
    option_name_name_map m_option_name_relations; // map<long name, short name>
    std::vector<std::string> m_single_parameters;
    bool m_has_parsed;
};

typedef command_line cmdline;

CA_LIB_NAMESPACE_END

#endif // __CPP_ASSISTANT_COMMAND_LINE_H__

