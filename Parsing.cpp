// /// /// //
// PERFECT //
// /// /// //

#include "Parsing.hpp"

void string_nr_to_n(const std::string& input, std::string& output)
{
	std::size_t loop = 0;

	output.clear();

	while (input[loop] != 0)
	{
		char c = input[loop];
		if (c == '\n')
		{
			output.push_back('\n');
		}
		else if (c == '\r')
		{
			// skip
		}
		else
		{
			output.push_back(c);
		}
		loop += 1;
	}
}

void string_n_to_nr(const std::string& input, std::string& output)
{
	std::size_t loop = 0;

	output.clear();

	while (input[loop] != 0)
	{
		char c = input[loop];
		if (c == '\n')
		{
			output.push_back('\n');
			output.push_back('\r');
		}
		else if (c == '\r')
		{
			// skip
		}
		else
		{
			output.push_back(c);
		}
		 loop += 1;
	}
}

int string_good(const char *input)
{
	std::size_t loop = 0;

	while (input[loop] != 0)
	{
		if (!std::isalnum(input[loop]) && !std::ispunct(input[loop]) && input[loop] != ' ' && input[loop] != '\n' && input[loop] != '\r')
		{
			return (0);
//			throw std::string(TEXT_StringBad);
		}
		input += 1;
	}
	return (1);
}

unsigned short port_parse(char *input)
{
	char *ptr = NULL;
	long tmp;

	string_good(input);

	tmp = std::strtol(input, &ptr, 10);

	if (input == ptr || *ptr != '\0')
	{
		throw std::runtime_error(TEXT_PortBad);
	}

	if (tmp < 0 || tmp > 65536)
	{
		throw std::runtime_error(TEXT_PortOutOfRange);
	}

	return static_cast<unsigned short>(tmp);
}

char *password_parse(char *input)
{
	string_good(input);

	if (input[0] == 0)
	{
		throw std::runtime_error(TEXT_PasswordEmpty);
	}
	return input;
}

