#include "error.hpp"
#include <iostream>
#include <stdexcept>

void DumpLog(const std::string& log)
{
    std::cerr 
        << "Dumping Log:" << std::endl
        << log << std::endl;
}

void FatalError(const std::string& message)
{
    throw std::runtime_error("fatal error: " + message);
}
