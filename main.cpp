#include "ccl.hpp"
#include <iostream>

/**
 * This file is part of CCL which is licenced under
 * the MIT licence (See LICENCE)
 * github.com/univrsal/ccl
 */


/*
   Example program for CCL
   Creates a config file in the same directory named test.ini
   Adds a couple of different test values
   modifies one and reads out two.
*/
int main(int argc, char **argv)
{
    ccl_config* cfg = new ccl_config("./test.ini", "test config");

    if (cfg->is_empty())
    {
        cfg->add_int("test_int", "Test integer 1", 123);
        cfg->add_int("test_int2", "Test integer 2", -1234);
        cfg->add_int("test_int3", "Test integer 3", 1337);

        cfg->add_float("test_float", "Test float", 0.483f);
        cfg->add_string("test_string", "Test string", "Hello World!");
        cfg->add_bool("test_bool", "Test bool", false);
        std::cout << "Test bool: " << cfg->get_bool("test_bool") << std::endl;
        cfg->write();
    }
    else
    {
        std::cout << "Test bool: " << cfg->get_bool("test_bool") << std::endl;
        cfg->set_bool("test_bool", true);
        std::cout << "Test string: " << cfg->get_string("test_string") << std::endl;
        std::cout << "Test float: " << cfg->get_float("test_float") << std::endl;
        std::cout << "Test bool: " << cfg->get_bool("test_bool") << std::endl;
        getchar();
        cfg->write();
    }

    cfg->add_bool("test_bool", "none" , false);
    cfg->get_int("test_float");
    cfg->get_int("test_string");
    cfg->get_float("none_existent");
    cfg->add_float("test_string", "", 0.f);

    if (cfg->has_errors())
        std::cout << cfg->get_error_message().c_str();
    
    getchar();

    delete cfg;
    return 0;
}