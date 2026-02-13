// /// /// //
// PERFECT //
// /// /// //

#include "Log.hpp"

void log(LogLevel level, const std::string& text)
{
	switch (level)
	{
		case INFO:
			std::cout << COLOR_Info << LABEL_Info << " " << text << COLOR_Reset << std::endl;
			break;
		case ERROR:
			std::cerr << COLOR_Error << LABEL_Error << " " << text << COLOR_Reset << std::endl;
			break;
		case CREATE:
			std::cout << COLOR_Create << LABEL_Create << " " << text << COLOR_Reset << std::endl;
			break;
		case DELETE:
			std::cout << COLOR_Delete << LABEL_Delete << " " << text << COLOR_Reset << std::endl;
			break;
		case WARNING:
			std::cout << COLOR_Warning << LABEL_Warning << " " << text << COLOR_Reset << std::endl;
			break;
		default:
			break;
	}
}
