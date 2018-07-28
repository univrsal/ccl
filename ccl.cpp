#include "ccl.hpp"

/**
 * This file is part of CCL which is licensed under
 * the MIT licence (See LICENCE)
 * github.com/univrsal/ccl
 */

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
	m_type = CCL_TYPE_INT;
	m_value.clear();
	m_value = std::to_string(val);
}

void ccl_data::set_float(float val)
{
	m_type = CCL_TYPE_FLOAT;
	m_value.clear();
	m_value = std::to_string(val);
}

void ccl_data::set_bool(bool val)
{
	m_type = CCL_TYPE_BOOL;
	m_value.clear();
	m_value = std::to_string(val);
}

void ccl_data::set_string(std::string val)
{
	set_value(val, CCL_TYPE_STRING);
}

void ccl_data::set_value(std::string val, DATA_TYPE t)
{
	m_type = t;
	m_value.clear();
	m_value = val;
}

void ccl_data::set_comment(std::string comment)
{
	m_comment = comment;
}

void ccl_data::free(void)
{
	m_id.clear();
	m_value.clear();
	m_comment.clear();
	m_next = nullptr;
}

ccl_config::ccl_config()
{
	m_empty = true;
	m_first_node = nullptr;
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
	m_first_node = nullptr;
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
	m_first_node = nullptr;
	m_header = header;
	m_path = path;
	load();
}
#endif

ccl_config::~ccl_config()
{
	if (!m_empty)
		free_nodes();
	m_empty = true;
	m_header = "";
	m_path.clear();
}

void ccl_config::free_nodes(void)
{
	ccl_data* node = m_first_node;
	ccl_data* next = nullptr;

	while (node)
	{
		next = node->get_next();
		delete node;
		node = next;
	}
	m_first_node = nullptr;
	m_empty = true;
	node = nullptr;
}

template<typename ... Args>
std::string format(const char * format, Args ... args)
{
	size_t size = snprintf(nullptr, 0, format, args ...) + 1;
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format, args ...);
	return std::string(buf.get(), buf.get() + size - 1);
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
		int line_index = 2;

		if (!line.empty())
		{
			if (line.at(0) == '#')
				m_header = line.erase(0, 2);
			else
				add_error("Line one in config should be a header comment! First value skipped!", CCL_ERROR_NORMAL);
		}
		else
		{
#if _DEBUG /* This error is redundant */
			add_error("First line was empty!", CCL_ERROR_NORMAL);
#endif
		}

		while (std::getline(f, line))
		{
			std::string comment; // Skip all comments and save last one

			if (line.empty())
			{
				add_error(format("Line %i was empty! Skipping.", line_index), CCL_ERROR_NORMAL);
				continue;
			}

			while (line.at(0) == '#')
			{
				comment = line;
				std::getline(f, line);
				line_index++;
			}

			DATA_TYPE type = util_parse_type(line.at(0)); // Read in the value type

			if (type == CCL_TYPE_INVALID)
			{
				add_error(format("Invalid type '%c' at line %i", line.at(0), line_index), CCL_ERROR_NORMAL);
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
				add_error(format("Invalid value at line %i. No '=' found", line_index), CCL_ERROR_NORMAL);
				continue;
			}

			ccl_data* new_node = new ccl_data(segments[0], comment.erase(0, 2), segments[1], type);
			add_node(new_node);
			line_index++;
		}

		if (m_first_node)
		{
			m_empty = false;
		}
	}
	else
	{
		add_error(format("File %s does not exist or cannot be accessed", m_path), CCL_ERROR_NORMAL);
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
		ccl_data * next = m_first_node;
		fs << "# " << m_header.c_str() << std::endl;

		while (next)
		{
			if (!next->get_comment().empty())
				fs << "# " << next->get_comment().c_str() << std::endl;

			fs << next->get_type() << "_" << next->get_id().c_str() << "=" << next->get_value().c_str() << std::endl;
			next = next->get_next();
		}

		fs.close();
	}
	else
	{
		add_error(format("Couldn't write to %s", m_path.c_str()), CCL_ERROR_FATAL);
	}
}

bool ccl_config::is_empty(void)
{
	return m_empty;
}

bool ccl_config::can_write(void)
{
	bool result = false;
#ifdef WINDOWS
	result = _waccess(m_path.c_str(), W_OK) == 0;
#else
	int code = access(m_path.c_str(), W_OK);
	result = code != EACCES && code != ENOENT && code != EROFS;
#endif
	return result;
}

bool ccl_config::can_load(void)
{
	bool result = false;

#ifdef WINDOWS
	result = _waccess(m_path.c_str(), R_OK) == 0;
#else
	int code = access(m_path.c_str(), R_OK);
	result = code != EACCES && code != ENOENT;
#endif
	return result;
}

bool ccl_config::node_exists(std::string id, bool silent)
{
	bool flag = get_node(id, silent) != nullptr;
	if (flag && !silent)
		add_error(format("Value with id '%s' already exists", id.c_str()), CCL_ERROR_NORMAL);
	return flag;
}

ccl_data* ccl_config::get_first(void)
{
	return m_first_node;
}

ccl_data * ccl_config::get_node(std::string id, bool silent)
{
	if (m_empty)
	{
		return nullptr;
	}

	ccl_data * node = m_first_node;

	while (node)
	{
		if (node->get_id().compare(id.c_str()) == 0)
		{
			return node;
		}

		node = node->get_next();
	}

	if (!silent)
		add_error(format("Value with id '%s' does not exist", id.c_str()), CCL_ERROR_NORMAL);
	return nullptr;
}

void ccl_config::add_node(ccl_data * node, bool replace)
{
	if (node)
	{
		if (node_exists(node->get_id(), replace))
		{
			if (replace)
			{
				ccl_data * existing = get_node(node->get_id(), replace);
				if (node->get_type() != CCL_TYPE_INVALID)
				{
					existing->set_value(node->get_value(), node->get_type());
					existing->set_comment(node->get_comment());
				}
				else
				{
					add_error(format("Can't replace node '%s', because\
						replacement has invalid type '%i'",
						node->get_id().c_str(), node->get_type()), CCL_ERROR_NORMAL);
				}
			}
			else
			{
				return;
			}
		}

		node->set_next(m_first_node);
		m_first_node = node;
	}
}

void ccl_config::add_int(std::string id, std::string comment, int val, bool replace)
{
	add_node(new ccl_data(id, comment, val), replace);
}

void ccl_config::add_float(std::string id, std::string comment, float val, bool replace)
{
	add_node(new ccl_data(id, comment, val), replace);
}

void ccl_config::add_bool(std::string id, std::string comment, bool val, bool replace)
{
	add_node(new ccl_data(id, comment, val), replace);
}

void ccl_config::add_string(std::string id, std::string comment, std::string val, bool replace)
{
	add_node(new ccl_data(id, comment, val), replace);
}

void ccl_config::set_int(std::string id, int val)
{
	ccl_data * node = get_node(id);

	if (node && node->get_type() == CCL_TYPE_INT)
	{
		node->set_int(val);
	}
	else
	{
		add_error(format("Cannot set value of '%s' to '%i'. Doesn't exist or type mismatch", id.c_str(), val), CCL_ERROR_NORMAL);
	}
}

void ccl_config::set_float(std::string id, float val)
{
	ccl_data * node = get_node(id);

	if (node && node->get_type() == CCL_TYPE_FLOAT)
	{
		node->set_float(val);
	}
	else
	{
		add_error(format("Cannot set value of '%s' to '%f'. Doesn't exist or type mismatch", id.c_str(), val), CCL_ERROR_NORMAL);
	}
}

void ccl_config::set_bool(std::string id, bool val)
{
	ccl_data * node = get_node(id);

	if (node && node->get_type() == CCL_TYPE_BOOL)
	{
		node->set_bool(val);
	}
	else
	{
		add_error(format("Cannot set value of '%s' to '%i'. Doesn't exist or type mismatch", id.c_str(), val), CCL_ERROR_NORMAL);
	}
}

void ccl_config::set_string(std::string id, std::string val)
{
	ccl_data * node = get_node(id);

	if (node && node->get_type() == CCL_TYPE_STRING)
	{
		node->set_string(val);
	}
	else
	{
		add_error(format("Cannot set value of '%s' to '%s'. Doesn't exist or type mismatch", id.c_str(), val), CCL_ERROR_NORMAL);
	}
}

int ccl_config::get_int(std::string id)
{
	ccl_data * node = get_node(id);

	if (node && node->get_type() == CCL_TYPE_INT)
	{
		return std::stoi(node->get_value());
	}
	else
	{
		add_error(format("Cannot get int value of '%s'. Doesn't exist or type mismatch", id.c_str()), CCL_ERROR_NORMAL);
	}

	return 0;
}

int ccl_config::get_hex(std::string id)
{
	ccl_data * node = get_node(id);
	if (node && node->get_type() == CCL_TYPE_STRING)
	{
		std::string value = node->get_value();
		if (value.find("0x") != std::string::npos)
		{
			return std::stoul(value, nullptr, 16);
		}
		else
		{
			return (uint16_t)0x0;
		}
	}
	return 0x0;
}

float ccl_config::get_float(std::string id)
{
	ccl_data * node = get_node(id);

	if (node && node->get_type() == CCL_TYPE_FLOAT)
	{
		return std::stof(node->get_value());
	}
	else
	{
		add_error(format("Cannot get float value of '%s'. Doesn't exist or type mismatch", id.c_str()), CCL_ERROR_NORMAL);
	}

	return 0.0f;
}

bool ccl_config::get_bool(std::string id)
{
	ccl_data * node = get_node(id);

	if (node && node->get_type() == CCL_TYPE_BOOL)
	{
		return std::stoi(node->get_value()) == 1 ? true : false;
	}
	else
	{
		add_error(format("Cannot get bool value of '%s'. Doesn't exist or type mismatch", id.c_str()), CCL_ERROR_NORMAL);
	}

	return false;
}

std::string ccl_config::get_string(std::string id)
{
	ccl_data * node = get_node(id);

	if (node)
	{
		return node->get_value();
	}
	else
	{
		add_error(format("Cannot get string value of '%s'. Doesn't exist or type mismatch", id.c_str()), CCL_ERROR_NORMAL);
	}

	return "";
}

bool ccl_config::has_errors(void)
{
	return m_errors.size() > 0;
}

bool ccl_config::has_fatal_errors(void)
{
	return m_fatal_errors;
}

std::string ccl_config::get_error_message(void)
{
	if (!has_errors())
		return "No errors reported";
	std::string error;

#ifdef WINDOWS
	error = format("Encountered errors when loading '%s':", to_utf8(m_path).c_str());
#else
	error = format("Encountered errors when loading '%s':", m_path.c_str());
#endif
	int i = 0;
	bool flag = false;

	for (auto const& e : m_errors)
	{
		error.append(format("\n [%s] %s", error_to_string(e.second), e.first.c_str()));
		if (++i > MAX_ERROR_REPORT)
		{
			flag = true;
			break;
		}
	}

	if (flag)
		error.append(format("\n %i more error(s) reported", m_errors.size() - MAX_ERROR_REPORT));
	return error;
}

void ccl_config::add_error(std::string error_msg, ERROR_LEVEL lvl)
{
	if (lvl == CCL_ERROR_FATAL)
		m_fatal_errors = true;
	m_errors[error_msg] = lvl;
}

const char * ccl_config::error_to_string(ERROR_LEVEL lvl)
{
	switch (lvl)
	{
	case CCL_ERROR_NORMAL:
		return "ERROR";
	case CCL_ERROR_FATAL:
		return "FATAL";
	default:
		return "-----";
	}
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
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), nullptr, 0);
	if (len > 0)
	{
		ret.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &ret[0], len);
	}
	return ret;
}

std::string to_utf8(std::wstring str)
{
	if (str.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0, nullptr, nullptr);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &str[0], (int)str.size(), &strTo[0], size_needed, nullptr, nullptr);
	return strTo;
}
#endif
