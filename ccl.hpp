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


#ifdef _MSC_VER
enum DATA_TYPE;
#else
enum DATA_TYPE {
    CCL_TYPE_INVALID,
    CCL_TYPE_INT,
    CCL_TYPE_STRING,
    CCL_TYPE_BOOL,
    CCL_TYPE_FLOAT
};
#endif

// Data class that holds config entries
// CCL Will create these automatically
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

// Class holding all information and data of a config file
class ccl_config
{
public:
    ccl_config();
    // Constructor for CCL configs
    // Supports unicode paths
    ccl_config(std::string path, std::string header);
    #ifdef _MSC_VER
    ccl_config::ccl_config(std::wstring path, std::string header);
    #endif
    
    ~ccl_config();

    void free(void);
    void load(void);
    void write(void);

    ccl_data* get_first(void);

    // True if the file didn't exist our couldn't be loaded
    bool is_empty(void);
    // True if file exists and can be accessed
    bool can_load(void);
    // True if config value could be found by identifier
    bool node_exists(std::string id);
    // Get data node by identifier
    ccl_data* get_node(std::string id);

    // Adds a new data node (Use type specific methods instead)
    void add_node(ccl_data* node);
    // Adds a new value of type int
    void add_int(std::string id, std::string comment, int val);
    // Adds a new value of type float
    void add_float(std::string id, std::string comment, float val);
    // Adds a new value of type boolean
    void add_bool(std::string id, std::string comment, bool val);
    // Adds a new value of type string
    void add_string(std::string id, std::string comment, std::string val);

    // Sets an entry to a new value if it exists
    void set_int(std::string id, int val);
    // Sets an entry to a new value if it exists
    void set_float(std::string id, float val);
    // Sets an entry to a new value if it exists
    void set_bool(std::string id, bool val);
    // Sets an entry to a new value if it exists
    void set_string(std::string id, std::string val);
    
    // Reads out a value if it exists
    int get_int(std::string id);
    // Reads out a value if it exists
    float get_float(std::string id);
    // Reads out a value if it exists
    bool get_bool(std::string id);
    // Reads out a value if it exists
    std::string get_string(std::string id);

private:
    
    DATA_TYPE util_parse_type(char c);

    bool m_empty;
    ccl_data* m_first_node;
    #ifdef _MSC_VER
    std::wstring m_path;
    #else
    std::string m_path;
    #endif

    std::string m_header;
};

#ifdef _MSC_VER
#include <Windows.h>
std::wstring to_utf_16(std::string str);
#endif

#endif // CCL_HPP