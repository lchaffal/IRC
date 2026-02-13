// /// /// //
// PERFECT //
// /// /// //

#ifndef PARSING_HPP
# define PARSING_HPP

# include <string> // string
# include <cstddef> // size_t
# include <cctype> // isalnum ispunct
# include <cstdlib> // strtol
# include <stdexcept> // runtime_error

# include "Text.hpp" // TEXT

void string_nr_to_n(const std::string& input, std::string& output);
void string_n_to_nr(const std::string& input, std::string& output);

int string_good(const char *input); // throw (not anymore)

unsigned short port_parse(char *input); // throw
char *password_parse(char *input); // throw

#endif
