#ifndef CCL_HPP
#define CCL_HPP

#ifdef _WIN32
#define WINDOWS
#else
#define LINUX
#endif

#include <string>
#include <fstream>
#include <vector>
#include <sstream>

/**
 * This file is part of CCL which is licenced under
 * the MIT licence (See LICENCE)
 * github.com/univrsal/ccl
 */

#ifdef LINUX
enum DATA_TYPE {
    INVALID,
    INT,
    STRING,
    BOOL,
    FLOAT
};
#else
enum DATA_TYPE;
#endif

class ccl_data
{
public:
    
    ccl_data();
    ccl_data(std::string id, std::string comment, std::string val, DATA_TYPE type);
    ccl_data(std::string id, std::string comment, int value);
    ccl_data(std::string id, std::string comment, float value);
    ccl_data(std::string id, std::string comment, bool value);
    ccl_data(std::string id, std::string comment, std::string value);

    ~ccl_data();

    ccl_data* get_next(void);
    std::string get_id(void);
    std::string get_value(void);
    std::string get_comment(void);
    DATA_TYPE get_type(void);

    void set_next(ccl_data* next);
    void set_int(int val);
    void set_float(float val);
    void set_bool(bool val);
    void set_string(std::string val);

private:
    void free(void);

    DATA_TYPE m_type;
    std::string m_id;
    std::string m_value;
    std::string m_comment;
    ccl_data* m_next;
};

class ccl_config
{
public:
    ccl_config();
    ccl_config(std::string path, std::string header);
    ~ccl_config();

    void free(void);
    void load(void);
    void write(void);

    bool is_empty(void);
    bool can_load(void);
    bool node_exists(std::string id);
    ccl_data* get_node(std::string id);

    void add_node(ccl_data* node);
    void add_int(std::string id, std::string comment, int val);
    void add_float(std::string id, std::string comment, float val);
    void add_bool(std::string id, std::string comment, bool val);
    void add_string(std::string id, std::string comment, std::string val);

    void set_int(std::string id, int val);
    void set_float(std::string id, float val);
    void set_bool(std::string id, bool val);
    void set_string(std::string id, std::string val);
    
    int get_int(std::string id);
    float get_float(std::string id);
    bool get_bool(std::string id);
    std::string get_string(std::string id);

private:
    
    DATA_TYPE util_parse_type(char c);

    bool m_empty;
    ccl_data* m_first_node;
    std::string m_path;
    std::string m_header;
};

#endif // CCL_HPP