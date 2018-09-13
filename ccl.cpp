#include "ccl.hpp"
#include <utility>
#include <string>
#include <memory>
#include <cstdio>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>

#ifdef WINDOWS
#include <io.h>
#include <windows.h>
#endif

/**
 * This file is part of CCL which is licensed under
 * the MIT license (See LICENSE)
 * github.com/univrsal/ccl
 */

template <typename ... Args>
std::string format(const char* format, Args ... args)
{
    const size_t size = snprintf(nullptr, 0, format, args ...) + 1;
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format, args ...);
    return std::string(buf.get(), buf.get() + size - 1);
}

ccl_data::ccl_data()
{
    m_type_ = ccl_type_invalid;
    m_next_ = nullptr;
}

ccl_data::ccl_data(std::string id, std::string comment, std::string val,
    const data_type type)
{
    m_comment_ = std::move(comment);
    m_id_ = std::move(id);
    m_type_ = type;
    m_value_ = std::move(val);
    m_next_ = nullptr;
}

ccl_data::ccl_data(std::string id, std::string comment, const int value)
{
    m_comment_ = std::move(comment);
    m_id_ = std::move(id);
    m_type_ = ccl_type_int;
    m_value_ = std::to_string(value);
    m_next_ = nullptr;
}

ccl_data::ccl_data(std::string id, std::string comment, const float value)
{
    m_comment_ = std::move(comment);
    m_id_ = std::move(id);
    m_type_ = ccl_type_float;
    m_value_ = std::to_string(value);
    m_next_ = nullptr;
}

ccl_data::ccl_data(std::string id, std::string comment, const bool value)
{
    m_comment_ = std::move(comment);
    m_id_ = std::move(id);
    m_type_ = ccl_type_bool;
    m_value_ = std::to_string(value);
    m_next_ = nullptr;
}

ccl_data::ccl_data(std::string id, std::string comment, std::string value)
{
    m_comment_ = std::move(comment);
    m_id_ = std::move(id);
    m_type_ = ccl_type_string;
    m_value_ = std::move(value);
    m_next_ = nullptr;
}

ccl_data::~ccl_data()
{
    free();
}

ccl_data* ccl_data::get_next() const
{
    return m_next_;
}

std::string ccl_data::get_id() const
{
    return m_id_;
}

std::string ccl_data::get_value() const
{
    return m_value_;
}

std::string ccl_data::get_comment() const
{
    return m_comment_;
}

data_type ccl_data::get_type() const
{
    return m_type_;
}

void ccl_data::set_next(ccl_data* next)
{
    m_next_ = next;
}

void ccl_data::set_int(const int val)
{
    m_type_ = ccl_type_int;
    m_value_.clear();
    m_value_ = std::to_string(val);
}

void ccl_data::set_float(const float val)
{
    m_type_ = ccl_type_float;
    m_value_.clear();
    m_value_ = std::to_string(val);
}

void ccl_data::set_bool(const bool val)
{
    m_type_ = ccl_type_bool;
    m_value_.clear();
    m_value_ = std::to_string(val);
}

void ccl_data::set_string(std::string val)
{
    set_value(std::move(val), ccl_type_string);
}

void ccl_data::set_value(std::string val, const data_type t)
{
    m_type_ = t;
    m_value_.clear();
    m_value_ = std::move(val);
}

void ccl_data::set_comment(std::string comment)
{
    m_comment_ = std::move(comment);
}

void ccl_data::free()
{
    m_id_.clear();
    m_value_.clear();
    m_comment_.clear();
    m_next_ = nullptr;
}

ccl_config::ccl_config()
{
    m_empty_ = true;
    m_first_node_ = nullptr;
    m_header_ = "";
#ifdef _MSC_VER
    m_path_ = L"";
#else
	m_path = "";
#endif
}

ccl_config::ccl_config(const std::string& path, std::string header)
{
    m_empty_ = true;
    m_first_node_ = nullptr;
    m_header_ = std::move(header);
#ifdef _MSC_VER
    m_path_ = to_utf_16(path);
#else
	m_path = path;
#endif

    load();
}

#ifdef _MSC_VER
ccl_config::ccl_config(std::wstring path, std::string header)
{
    m_empty_ = true;
    m_first_node_ = nullptr;
    m_header_ = std::move(header);
    m_path_ = std::move(path);
    load();
}
#endif

ccl_config::~ccl_config()
{
    if (!m_empty_)
        free_nodes();
    m_empty_ = true;
    m_header_ = "";
    m_path_.clear();
}

void ccl_config::free_nodes()
{
    auto node = m_first_node_;
    ccl_data* next = nullptr;

    while (node)
    {
        next = node->get_next();
        delete node;
        node = next;
    }
    m_first_node_ = nullptr;
    m_empty_ = true;
    node = nullptr;
}

void ccl_config::load()
{
    if (!can_load())
    {
        return;
    }

    std::ifstream f(m_path_.c_str());

    if (f.good())
    {
        std::string line;
        std::getline(f, line);
        auto line_index = 2;

        if (!line.empty())
        {
            if (line.at(0) == '#')
            {
                m_header_ = line.erase(0, 2);
            }
            else
            {
                add_error(
                    "Line one in config should be a header comment! \
First value skipped!",
                    ccl_error_normal);
            }
        }
        else
        {
#if _DEBUG /* This error is redundant */
            add_error("First line was empty!", ccl_error_normal);
#endif
        }

        while (std::getline(f, line))
        {
            /* Skip all comments and save last one */
            std::string comment; 
            if (line.empty())
            {
                add_error(format(
                    "Line %i was empty! Skipping.", line_index),
                    ccl_error_normal);
                continue;
            }

            while (line.at(0) == '#')
            {
                comment = line;
                std::getline(f, line);
                line_index++;
            }

            /* Read in the value type */
            const auto type = util_parse_type(line.at(0)); 

            if (type == ccl_type_invalid)
            {
                add_error(format(
                    "Invalid type '%c' at line %i", line.at(0), line_index),
                    ccl_error_normal);
                continue;
            }

            line = line.erase(0, 2);

            if (line.find("\\n") != std::string::npos)
            {
                size_t start_pos = 0;
                while ((start_pos = line.find("\\n", start_pos)) !=
                    std::string::npos)
                {
                    line.replace(start_pos, 2, "\n");
                    start_pos += 1;
                }
            }

            std::string segment;
            std::vector<std::string> segments;
            std::stringstream stream(line);

            while (std::getline(stream, segment, '='))
            {
                segments.push_back(segment);
            }

            if (segments.empty())
            {
                add_error(format(
                    "Invalid value at line %i. No '=' found", line_index),
                    ccl_error_normal);
                continue;
            }

            const auto new_node = new ccl_data(
                segments[0], comment.erase(0, 2), segments[1], type);
            add_node(new_node);
            line_index++;
        }

        if (m_first_node_)
        {
            m_empty_ = false;
        }
    }
    else
    {
        add_error(format(
            "File %s does not exist or cannot be accessed", m_path_),
            ccl_error_normal);
    }
}

void ccl_config::write()
{
    if (m_path_.empty())
    {
        return;
    }

#ifdef _MSC_VER
    DeleteFileW(m_path_.c_str());
#else
	std::remove(m_path.c_str());
#endif
    std::ofstream fs(m_path_.c_str());

    if (fs)
    {
        auto next = m_first_node_;
        fs << "# " << m_header_.c_str() << std::endl;

        while (next)
        {
            if (!next->get_comment().empty())
            {
                fs << "# " << next->get_comment().c_str() << std::endl;
            }
                
            fs  << next->get_type() << "_" 
                << next->get_id().c_str()  << "=" 
                << next->get_value().c_str() << std::endl;
            next = next->get_next();
        }

        fs.close();
    }
    else
    {
        add_error(format("Couldn't write to %s", m_path_.c_str()),
            ccl_error_fatal);
    }
}

bool ccl_config::is_empty() const
{
    return m_empty_;
}

bool ccl_config::can_write() const
{
    auto result = false;
#ifdef WINDOWS
    result = _waccess(m_path_.c_str(), W_OK) == 0;
#else
	int code = access(m_path.c_str(), W_OK);
	result = code != EACCES && code != ENOENT && code != EROFS;
#endif
    return result;
}

bool ccl_config::can_load() const
{
    auto result = false;

#ifdef WINDOWS
    result = _waccess(m_path_.c_str(), R_OK) == 0;
#else
	int code = access(m_path.c_str(), R_OK);
	result = code != EACCES && code != ENOENT;
#endif
    return result;
}

bool ccl_config::node_exists(const std::string& id, const bool silent)
{
    const auto flag = get_node(id, silent) != nullptr;
    if (flag && !silent)
        add_error(format(
            "Value with id '%s' already exists", id.c_str()),
            ccl_error_normal);
    return flag;
}

ccl_data* ccl_config::get_first() const
{
    return m_first_node_;
}

ccl_data* ccl_config::get_node(const std::string& id, const bool silent)
{
    if (m_empty_)
    {
        return nullptr;
    }

    auto node = m_first_node_;

    while (node)
    {
        if (node->get_id() == id)
        {
            return node;
        }

        node = node->get_next();
    }

    if (!silent)
        add_error(format(
            "Value with id '%s' does not exist", id.c_str()),
            ccl_error_normal);
    return nullptr;
}

void ccl_config::add_node(ccl_data* node, const bool replace)
{
    if (node)
    {
        if (node_exists(node->get_id(), replace))
        {
            if (replace)
            {
                auto existing = get_node(node->get_id(), replace);
                if (node->get_type() != ccl_type_invalid)
                {
                    existing->set_value(node->get_value(), node->get_type());
                    existing->set_comment(node->get_comment());
                }
                else
                {
                    add_error(format("Can't replace node '%s', because \
replacement has invalid type '%i'",
                        node->get_id().c_str(), node->get_type()),
                        ccl_error_normal);
                }
            }
            else
            {
                return;
            }
        }

        node->set_next(m_first_node_);
        m_first_node_ = node;
    }
}

void ccl_config::add_int(std::string id, std::string comment,
    const int val, const bool replace)
{
    add_node(new ccl_data(std::move(id), std::move(comment), val), replace);
}

void ccl_config::add_float(std::string id, std::string comment,
    const float val, const bool replace)
{
    add_node(new ccl_data(std::move(id), std::move(comment), val), replace);
}

void ccl_config::add_bool(std::string id, std::string comment,
    const bool val, const bool replace)
{
    add_node(new ccl_data(std::move(id), std::move(comment), val), replace);
}

void ccl_config::add_string(std::string id, std::string comment,
    std::string val, const bool replace)
{
    add_node(new ccl_data(std::move(id), std::move(comment),
        std::move(val)), replace);
}

void ccl_config::set_int(const std::string& id, const int val)
{
    auto node = get_node(id);

    if (node && node->get_type() == ccl_type_int)
    {
        node->set_int(val);
    }
    else
    {
        add_error(format(
            "Cannot set value of '%s' to '%i'. Doesn't exist or type mismatch",
            id.c_str(), val), ccl_error_normal);
    }
}

void ccl_config::set_float(const std::string& id, const float val)
{
    auto node = get_node(id);

    if (node && node->get_type() == ccl_type_float)
    {
        node->set_float(val);
    }
    else
    {
        add_error(format(
            "Cannot set value of '%s' to '%f'. Doesn't exist or type mismatch",
            id.c_str(), val), ccl_error_normal);
    }
}

void ccl_config::set_bool(const std::string& id, const bool val)
{
    auto node = get_node(id);

    if (node && node->get_type() == ccl_type_bool)
    {
        node->set_bool(val);
    }
    else
    {
        add_error(format(
            "Cannot set value of '%s' to '%i'. Doesn't exist or type mismatch",
            id.c_str(), val), ccl_error_normal);
    }
}

void ccl_config::set_string(const std::string& id, const std::string& val)
{
    auto node = get_node(id);

    if (node && node->get_type() == ccl_type_string)
    {
        node->set_string(val);
    }
    else
    {
        add_error(format(
            "Cannot set value of '%s' to '%s'. Doesn't exist or type mismatch",
            id.c_str(), val), ccl_error_normal);
    }
}

int ccl_config::get_int(const std::string& id, const bool silent)
{
    const auto node = get_node(id);

    if (node && node->get_type() == ccl_type_int)
    {
        return std::stoi(node->get_value());
    }

    if (!silent)
        add_error(format(
            "Cannot get int value of '%s'. Doesn't exist or type mismatch",
            id.c_str()), ccl_error_normal);

    return 0;
}

int ccl_config::get_hex(const std::string& id, const bool silent)
{
    const auto node = get_node(id);
    if (node && node->get_type() == ccl_type_string)
    {
        auto value = node->get_value();
        if (value.find("0x") != std::string::npos)
        {
            return std::stoul(value, nullptr, 16);
        }
        return static_cast<uint16_t>(0x0);
    }

    if (!silent)
        add_error(format(
            "Cannot get float value of '%s'. Doesn't exist or type mismatch",
            id.c_str()), ccl_error_normal);

    return 0x0;
}

float ccl_config::get_float(const std::string& id, const bool silent)
{
    const auto node = get_node(id);

    if (node && node->get_type() == ccl_type_float)
    {
        return std::stof(node->get_value());
    }

    if (!silent)
        add_error(format(
            "Cannot get float value of '%s'. Doesn't exist or type mismatch",
            id.c_str()), ccl_error_normal);

    return 0.0f;
}

bool ccl_config::get_bool(const std::string& id, const bool silent)
{
    const auto node = get_node(id);

    if (node && node->get_type() == ccl_type_bool)
    {
        return std::stoi(node->get_value()) == 1;
    }

    if (!silent)
        add_error(format(
            "Cannot get bool value of '%s'. Doesn't exist or type mismatch",
            id.c_str()), ccl_error_normal);

    return false;
}

std::string ccl_config::get_string(const std::string& id, const bool silent)
{
    const auto node = get_node(id);

    if (node)
    {
        return node->get_value();
    }

    if (!silent)
        add_error(format(
            "Cannot get string value of '%s'. Doesn't exist or type mismatch",
            id.c_str()), ccl_error_normal);

    return "";
}

bool ccl_config::has_errors() const
{
    return !m_errors_.empty();
}

bool ccl_config::has_fatal_errors() const
{
    return m_fatal_errors_;
}

std::string ccl_config::get_error_message()
{
    if (!has_errors())
        return "No errors reported";
    std::string error;

#ifdef WINDOWS
    error = format("Encountered errors when loading '%s':",
        to_utf8(m_path_).c_str());
#else
	error = format("Encountered errors when loading '%s':",
        m_path.c_str());
#endif
    auto i = 0;
    auto flag = false;

    for (auto const& e : m_errors_)
    {
        error.append(format("\n [%s] %s", error_to_string(e.second),
            e.first.c_str()));
        if (++i > MAX_ERROR_REPORT)
        {
            flag = true;
            break;
        }
    }

    if (flag)
        error.append(format("\n %i more error(s) reported",
            m_errors_.size() - MAX_ERROR_REPORT));
    return error;
}

void ccl_config::add_error(const std::string& error_msg, const error_level lvl)
{
    if (lvl == ccl_error_fatal)
        m_fatal_errors_ = true;
    m_errors_[error_msg] = lvl;
}

const char* ccl_config::error_to_string(const error_level lvl)
{
    switch (lvl)
    {
    case ccl_error_normal:
        return "ERROR";
    case ccl_error_fatal:
        return "FATAL";
    default:
        return "-----";
    }
}

data_type ccl_config::util_parse_type(char c)
{
    switch (c)
    {
    case '1':
        return ccl_type_int;
    case '2':
        return ccl_type_string;
    case '3':
        return ccl_type_bool;
    case '4':
        return ccl_type_float;
    default:
        return ccl_type_invalid;
    }
}

#ifdef WINDOWS
std::wstring to_utf_16(const std::string& str)
{
    std::wstring ret;
    const auto len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(),
        str.length(), nullptr, 0);
    if (len > 0)
    {
        ret.resize(len);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(),
            &ret[0], len);
    }
    return ret;
}

std::string to_utf8(std::wstring str)
{
    if (str.empty()) return std::string();
    const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, &str[0],
        static_cast<int>(str.size()), nullptr, 0, nullptr, nullptr);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &str[0], static_cast<int>(str.size()),
        &strTo[0], size_needed, nullptr, nullptr);
    return strTo;
}
#endif
