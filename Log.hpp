// /// /// //
// PERFECT //
// /// /// //

#ifndef LOG_HPP
# define LOG_HPP

# include <string> // string
# include <iostream> // cout cerr

# define COLOR_Info "\033[37m"
# define COLOR_Error "\033[31m"
# define COLOR_Create "\033[32m"
# define COLOR_Delete "\033[35m"
# define COLOR_Warning "\033[33m"
# define COLOR_Reset "\033[0m"

# define LABEL_Info "[INFO]"
# define LABEL_Error "[ERROR]"
# define LABEL_Create "[+]"
# define LABEL_Delete "[-]"
# define LABEL_Warning "[WARN]"

enum LogLevel
{
	INFO, ERROR, CREATE, DELETE, WARNING
};

void log(LogLevel level, const std::string& text);

#endif
