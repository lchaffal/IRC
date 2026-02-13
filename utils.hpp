/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gromit <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 12:36:43 by gromit            #+#    #+#             */
/*   Updated: 2026/01/30 12:13:25 by gromit           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include "typedef.hpp"
# include "User.hpp"

void	trim_spaces(std::string &s);
bool	is_printable(std::string &s);
bool	check_nickname(std::string &s);
bool	nick_taken(User_vector &users, std::string nick);

#endif
