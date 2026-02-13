/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gromit <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 12:35:50 by gromit            #+#    #+#             */
/*   Updated: 2026/01/30 12:12:58 by gromit           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

void	trim_spaces(std::string &s)
{
	s.erase(s.find_last_not_of(" \f\n\r\t\v") + 1);
	s.erase(0, s.find_first_not_of(" \f\n\r\t\v"));
}

__attribute__((pure))
bool	is_printable(std::string &s)
{
	for (std::string::const_iterator it = s.begin(); it != s.end(); it++)
	{
		if (!std::isprint(*it))
			return (false);
	}
	return (true);
}

bool	check_nickname(std::string &s)
{
	std::string allowed_chars;

	allowed_chars = "[]{}\\|";
	for (std::string::const_iterator it = s.begin(); it != s.end(); it++)
	{
		if (!std::isalnum(*it) && allowed_chars.find(*it) == allowed_chars.npos && *it != '_')
			return (false);
	}
	return (true);
}

bool	nick_taken(User_vector &users, std::string nick)
{
	for (User_vector::iterator it = users.begin(); it != users.end(); it++)
	{
		if (nick == (*it)->get_nickname())
			return (true);
	}
	return (false);
}
