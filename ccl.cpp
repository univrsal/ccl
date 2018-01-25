#include "ccl.hpp"

#ifdef WINDOWS
enum DATA_TYPE {
    CCL_TYPE_INVALID,
    CCL_TYPE_INT,
    CCL_TYPE_STRING,
    CCL_TYPE_BOOL,
    CCL_TYPE_FLOAT
};
#endif


ccl_data::ccl_data()
{
}

ccl_data::ccl_data(std::string id, std::string comment, std::string val, DATA_TYPE type)
{
    m_comment = comment;
    m_id = id;
    m_type = type;
    m_value = val;
}

ccl_data::ccl_data(std::string id, std::string comment, int value)
{
    m_comment = comment;
    m_id = id;
    m_type = CCL_TYPE_INT;
    m_value = std::to_string(value);
}

ccl_data::ccl_data(std::string id, std::string comment, float value)
{
    m_comment = comment;
    m_id = id;
    m_type = CCL_TYPE_FLOAT;
    m_value = std::to_string(value);
}

ccl_data::ccl_data(std::string id, std::string comment, bool value)
{
    m_comment = comment;
    m_id = id;
    m_type = CCL_TYPE_BOOL;
    m_value = std::to_string(value);
}

ccl_data::ccl_data(std::string id, std::string comment, std::string value)
{
    m_comment = comment;
    m_id = id;
    m_type = CCL_TYPE_STRING;
    m_value = value;
}

ccl_data::~ccl_data()
{
    free();
}

ccl_data * ccl_data::get_next(void)
{
    return m_next;
}

std::string ccl_data::get_id(void)
{
    return m_id;
}

std::string ccl_data::get_value(void)
{
    return m_value;
}

std::string ccl_data::get_comment(void)
{
    return m_comment;
}

DATA_TYPE ccl_data::get_type(void)
{
    return m_type;
}

void ccl_data::set_next(ccl_data * next)
{
    m_next = next;
}

void ccl_data::set_int(int val)
{
    m_value.clear();
    m_value = std::to_string(val);
}

void ccl_data::set_float(float val)
{
    m_value.clear();
    m_value = std::to_string(val);
}

void ccl_data::set_bool(bool val)
{
    m_value.clear();
    m_value = std::to_string(val);
}

void ccl_data::set_string(std::string val)
{
    m_value.clear();
    m_value = val;
}

void ccl_data::free(void)
{
    m_id.clear();
    m_value.clear();
    m_comment.clear();
    m_next = NULL;
}

ccl_config::ccl_config()
{
    m_empty = true;
    m_first_node = NULL;
    m_header = "";
    #ifdef _MSC_VER
    m_path = L"";
    #else
    m_path = "";
    #endif
}

ccl_config::ccl_config(std::string path, std::string header)
{
    m_empty = true;
    m_first_node = NULL;
    m_header = header;
    #ifdef _MSC_VER
    m_path = to_utf_16(path);
    #else
    m_path = path;
    #endif

    load();
}

#ifdef _MSC_VER
ccl_config::ccl_config(std::wstring path, std::string header)
{
    m_empty = true;
    m_first_node = NULL;
    m_header = header;
    m_path = path;
    load();
}
#endif

ccl_config::~ccl_config()
{
    free();
    m_empty = true;
    m_header = "";
    m_path.clear();
}

void ccl_config::free(void)
{
    ccl_data* node = m_first_node;
    ccl_data* next = NULL;

    while (node)
    {
        next = node->get_next();
        delete node;
        node = next;
    }

    node = NULL;
}

void ccl_config::load(void)
{
    if (!can_load())
    {
        return;
    }

    std::ifstream f(m_path.c_str());

    if (f.good())
    {
        std::string line;
        std::getline(f, line);
        m_header = line.erase(0, 2);

        while (std::getline(f, line))
        {
            std::string comment; // Skip all comments and save last one
            while (line.at(0) == '#')
            {
                comment = line;
                std::getline(f, line);
            }

            DATA_TYPE type = util_parse_type(line.at(0)); // Read in the value type
            
            if (type == CCL_TYPE_INVALID)
            {
                continue;
            }

            line = line.erase(0, 2);
            
            std::string segment;
            std::vector<std::string> segments;
            std::stringstream stream(line);

            while (std::getline(stream, segment, '='))
            {
                segments.push_back(segment);
            }

            if (segments.size() < 1)
            {
                continue;
            }

            ccl_data* new_node = new ccl_data(segments[0], comment, segments[1], type);
            add_node(new_node);
        }

        if (m_first_node)
        {
            m_empty = false;
        }
    }
}

void ccl_config::write(void)
{
    if (m_path.empty())
    {
        return;
    }

    #ifdef _MSC_VER
    DeleteFileW(m_path.c_str());
    #else
    std::remove(m_path.c_str());
    #endif
    std::ofstream fs(m_path.c_str());

    if (fs)
    {
        ccl_data* next = m_first_node;
        fs << "# " << m_header.c_str() << std::endl;

        while (next)
        {
            fs << "# " << next->get_comment().c_str() << std::endl;
            fs << next->get_type() << "_" << next->get_id().c_str() << "=" << next->get_value().c_str() << std::endl; 
            next = next->get_next();
        }
        
        fs.close();
    }
}

bool ccl_config::is_empty(void)
{
    return m_empty;
}

bool ccl_config::can_load(void)
{
    std::ifstream f(m_path);
    return !m_path.empty() && f.good();
}

bool ccl_config::node_exists(std::string id)
{
    return get_node(id) != NULL;
}

ccl_data * ccl_config::get_node(std::string id)
{
    if (m_empty)
    {
        return NULL;
    }
        
    ccl_data* node = m_first_node;

    while (node)
    {
        if (node->get_id().compare(id.c_str()) == 0)
        {
            return node;
        }
        
        node = node->get_next();
    }

    return NULL;
}

void ccl_config::add_node(ccl_data * node)
{
    if (node)
    {
        if (node_exists(node->get_id()))
        {
            return;
        }

        node->set_next(m_first_node);
        m_first_node = node;
    }
}

void ccl_config::add_int(std::string id, std::string comment, int val)
{
    add_node(new ccl_data(id, comment, val));
}

void ccl_config::add_float(std::string id, std::string comment, float val)
{
    add_node(new ccl_data(id, comment, val));
}

void ccl_config::add_bool(std::string id, std::string comment, bool val)
{
    add_node(new ccl_data(id, comment, val));
}

void ccl_config::add_string(std::string id, std::string comment, std::string val)
{
    add_node(new ccl_data(id, comment, val));
}

void ccl_config::set_int(std::string id, int val)
{
    ccl_data* node = get_node(id);

    if (node && node->get_type() == CCL_TYPE_INT)
    {
        node->set_int(val);
    }
}

void ccl_config::set_float(std::string id, float val)
{
    ccl_data* node = get_node(id);

    if (node && node->get_type() == CCL_TYPE_FLOAT)
    {
        node->set_float(val);
    }
}

void ccl_config::set_bool(std::string id, bool val)
{
    ccl_data* node = get_node(id);

    if (node && node->get_type() == CCL_TYPE_BOOL)
    {
        node->set_bool(val);
    }
}

void ccl_config::set_string(std::string id, std::string val)
{
    ccl_data* node = get_node(id);

    if (node && node->get_type() == CCL_TYPE_STRING)
    {
        node->set_string(val);
    }
}

int ccl_config::get_int(std::string id)
{
    ccl_data* node = get_node(id);

    if (node && node->get_type() == CCL_TYPE_INT)
    {
        return std::stoi(node->get_value());
    }

    return 0;
}

float ccl_config::get_float(std::string id)
{
    ccl_data* node = get_node(id);

    if (node && node->get_type() == CCL_TYPE_FLOAT)
    {
        return std::stof(node->get_value());
    }

    return 0.0f;
}

bool ccl_config::get_bool(std::string id)
{
    ccl_data* node = get_node(id);

    if (node && node->get_type() == CCL_TYPE_BOOL)
    {
        return std::stoi(node->get_value()) == 1 ? true : false;
    }

    return false;
}

std::string ccl_config::get_string(std::string id)
{
    ccl_data* node = get_node(id);

    if (node)
    {
        return node->get_value();
    }
    return "";
}

DATA_TYPE ccl_config::util_parse_type(char c)
{
    switch (c)
    {
    case '1':
        return DATA_TYPE::CCL_TYPE_INT;
    case '2':
        return DATA_TYPE::CCL_TYPE_STRING;
    case '3':
        return DATA_TYPE::CCL_TYPE_BOOL;
    case '4':
        return DATA_TYPE::CCL_TYPE_FLOAT;
    default:
        return DATA_TYPE::CCL_TYPE_INVALID;
    }
}

#ifdef WINDOWS
std::wstring to_utf_16(std::string str)
{
    std::wstring ret;
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
    if (len > 0)
    {
        ret.resize(len);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &ret[0], len);
    }
    return ret;
}
#endif