#include "utils.h"

namespace Utils
{
	InvalidGraphException::InvalidGraphException(const char* c) : std::runtime_error(c)
	{
	}

	InvalidGraphException::InvalidGraphException(std::string c) : std::runtime_error(c)
	{
	}
}