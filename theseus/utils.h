#pragma once

#include <stdexcept>
#include <string>

namespace Utils
{
	struct InvalidGraphException : std::runtime_error
	{
		InvalidGraphException(const char* c);
		InvalidGraphException(std::string c);
	};

}