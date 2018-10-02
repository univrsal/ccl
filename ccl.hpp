/**
 * This file is part of CCL which is licensed under
 * the MIT license (See LICENSE)
 * github.com/univrsal/ccl
 */

#pragma once

#ifdef _WIN32
#define WINDOWS
#define W_OK 6
#define R_OK 4
#else
#define LINUX
#endif

/*
	Amount of errors to report
	Any errors exceeding this number will
	be reported with a message stating
	that there's more errors hidden
*/
#define MAX_ERROR_REPORT 5

#include <string>
#include <map>
#include <vector>
#include <winerror.h>

#ifdef LINUX
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#else
#endif

enum data_type
{
    ccl_type_invalid = -1,
    ccl_type_int,
    ccl_type_string,
    ccl_type_bool,
    ccl_type_float,
    ccl_type_point,
    ccl_type_rect,
};

enum error_level
{
    ccl_error_normal,
    ccl_error_fatal
};

struct ccl_point
{
    int x = 0, y = 0;
};

struct ccl_rect
{
    int x = 0, y = 0,
        w = 0, h = 0;
};

/* Data class that holds config entries
   CCL Will create these automatically */
class ccl_data
{
public:

    ccl_data() = default;
    ccl_data(const std::string& id, const std::string& comment, const std::string& val,
             data_type type);
    ccl_data(const std::string& id, const std::string& comment, int value);
    ccl_data(const std::string& id, const std::string& comment, float value);
    ccl_data(const std::string& id, const std::string& comment, bool value);
    ccl_data(const std::string& id, const std::string& comment, const std::string& value);
    ccl_data(const std::string& id, const std::string& comment, int x, int y);
    ccl_data(const std::string& id, const std::string& comment, int x, int y, int w, int h);

    ~ccl_data();

    ccl_data* get_next() const;
    std::string get_id() const;
    std::string get_value() const;
    std::string get_comment() const;
    data_type get_type() const;

    void set_next(ccl_data* next);
    void set_int(int val);
    void set_float(float val);
    void set_bool(bool val);
    void set_string(const std::string& val);
    void set_value(const std::string& val, data_type t);
    void set_comment(const std::string& comment);
    void set_point(int x, int y);
    void set_rect(int x, int y, int w, int h);
private:
    void free();

    data_type m_type_ = ccl_type_invalid;
    std::string m_id_;
    std::string m_value_;
    std::string m_comment_;
    ccl_data* m_next_ = nullptr;
};

/* Class holding all information and data of a config file */
class ccl_config
{
public:
    ccl_config();
    /* Constructor for CCL config
       Supports Unicode paths */
    ccl_config(const std::string& path, std::string header);
#ifdef _MSC_VER
    ccl_config(std::wstring path, std::string header);
#endif

    ~ccl_config();

    void free_nodes(); /* Deletes ALL nodes */
    void load();
    void write(bool comments = true);

    ccl_data* get_first() const;

    /* True if the file didn't exist our couldn't be loaded */
    bool is_empty() const;
    /* True if the current path can be written to */
    bool can_write() const;
    /* True if file exists and can be accessed */
    bool can_load() const;
    /* True if config value could be found by identifier */
    bool node_exists(const std::string& id, bool silent = false);
    /* Get data node by identifier */
    ccl_data* get_node(const std::string& id, bool silent = false);

    /* Adds a new data node (Use type specific methods instead) */
    void add_node(ccl_data* node, bool replace = false);
    /* Adds a new value of type int */
    void add_int(const std::string& id, const std::string& comment, int val,
                 bool replace = false);
    /* Adds a new value of type float */
    void add_float(const std::string& id, const std::string& comment, float val,
                   bool replace = false);
    /* Adds a new value of type boolean */
    void add_bool(const std::string& id, const std::string& comment, const bool val,
                  bool replace = false);
    /* Adds a new value of type string */
    void add_string(const std::string& id, const std::string& comment, const std::string& val,
                    bool replace = false);
    /* Adds a new value of type point (x and y values) */
    void add_point(const std::string& id, const std::string& comment, int x, int y,
                   bool replace = false);
    /* Adds a new value of type rect (x, y, w and h values) */
    void add_rect(const std::string& id, const std::string& comment,
                    int x, int y, int w, int h, bool replace = false);

    /* Sets an entry to a new value if it exists */
    void set_int(const std::string& id, int val);
    /* Sets an entry to a new value if it exists */
    void set_float(const std::string& id, float val);
    /* Sets an entry to a new value if it exists */
    void set_bool(const std::string& id, bool val);
    /* Sets an entry to a new value if it exists */
    void set_string(const std::string& id, const std::string& val);
    /* Sets an entry to a new value if it exists */
    void set_point(const std::string& id, int x, int y);
    /* Sets an entry to a new value if it exists */
    void set_rect(const std::string& id, int x, int y, int w, int h);

    /* Reads out a value if it exists */
    int get_int(const std::string& id, bool silent = false);
    /* Reads out a value if it exists */
    int get_hex(const std::string& id, bool silent = false);
    /* Reads out a value if it exists */
    float get_float(const std::string& id, bool silent = false);
    /* Reads out a value if it exists */
    bool get_bool(const std::string& id, bool silent = false);
    /* Reads out a value if it exists */
    std::string get_string(const std::string& id, bool silent = false);
    /* Reads out a value if it exists */
    int get_point_x(const std::string& id, bool silent = false);
    /* Reads out a value if it exists */
    int get_point_y(const std::string& id, bool silent = false);
    /* Reads out a value if it exists */
    int get_rect_x(const std::string& id, bool silent = false);
    /* Reads out a value if it exists */
    int get_rect_y(const std::string& id, bool silent = false);
    /* Reads out a value if it exists */
    int get_rect_w(const std::string& id, bool silent = false);
    /* Reads out a value if it exists */
    int get_rect_h(const std::string& id, bool silent = false);
    /* Reads out a value if it exists */
    ccl_point get_point(const std::string& id, bool silent = false);
     /* Reads out a value if it exists */
    ccl_rect get_rect(const std::string& id, bool silent = false);
    
    /* Errors 
       True if any errors were reported
       Anything that impacts loading of the file or values
       is considered an error */
    bool has_errors() const;

    /* True if fatal errors were reported
      which prevent CCL from working correctly */
    bool has_fatal_errors() const;

    /* If any errors were reported they'll
       be formatted into one string here
       This only contains the last five errors and
       the amount of total errors */
    std::string get_error_message();

private:
    void add_error(const std::string& error_msg, error_level lvl);

    static const char* error_to_string(error_level lvl);

    static data_type util_parse_type(char c);

    std::map<std::string, error_level> m_errors_;
    bool m_empty_ = true;
    bool m_fatal_errors_ = false;
    ccl_data* m_first_node_;
    std::string m_header_;

#ifdef _MSC_VER
    std::wstring m_path_;
#else
	std::string m_path;
#endif
};

#ifdef _MSC_VER
std::wstring to_utf_16(const std::string& str);
std::string to_utf8(std::wstring str);
#endif
