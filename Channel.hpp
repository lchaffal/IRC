#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "utils.hpp"
# include "User.hpp"
# include "typedef.hpp"
# include "numerics.hpp"
# include <algorithm>
# include <iostream>
# include <sstream>
# include <cctype>
# include <stdexcept>
# include "numerics.hpp"
# include "typedef.hpp"
# include <climits>

class Channel
{
	public:
		bool	kick(User *asking, User *target, std::string message);
		void	invite(User *asking, User *target);
		void	join(User *asking, const std::string password);
		void	topic(User *asking, std::string new_topic);
		void	mode(User *asking, std::string parameters);
		void	privmsg(const User *asking, std::string message);
		bool	part(const User *asking, std::string message);

		void		send_everyone(std::string message);
		std::string	get_name(void) const;
		std::string	get_topic(void) const;

		Channel(const std::string name, const std::string password, User *creator);
		Channel(const Channel &other);
		Channel	&operator=(const Channel &other);
		bool operator==(const std::string &name) const;
		~Channel(void);

	private:
		User	*select_user(std::string nickname);

		bool	is_in_users(const User &someone) const;
		bool	is_in_ops(const User &someone) const;

		void		send_chan_infos(User *target);
		std::string	channel_infos(const User *asking) const;

		// Updated signature: removed static logic, added bool &plus_minus
		void		mode_selection(char c, std::stringstream &stream, std::string &answer, std::string &options, bool &plus_minus);

		void		manage_password(bool plus_minus, std::stringstream &stream, std::string &answer, std::string &options);
		void		manage_operators(bool plus_minus, std::stringstream &stream, std::string &answer, std::string &options);
		void		manage_limit(bool plus_minus, std::stringstream &stream, std::string &answer, std::string &options);

		std::string     _name;
		std::string     _password;
		std::string     _topic;

		User_vector     _users;
		User_vector     _operators;

		unsigned int    _user_limit;

		bool            _i;
		bool            _t;
		bool            _l;

		char            _pad0;
};

std::ostream& operator<<(std::ostream& os, const Channel &channel);

#endif
