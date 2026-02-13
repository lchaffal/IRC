#include "Channel.hpp"

void	Channel::send_everyone(std::string message)
{
	for (std::vector<User*>::const_iterator it = _users.begin(); it != _users.end(); it++)
		(*it)->send_message(message);
}

std::string	Channel::get_name(void) const
{
	return (_name);
}

std::string	Channel::get_topic(void) const
{
	return (_topic);
}

static void	correct_chan_name(const std::string name)
{
	for (std::string::const_iterator it = name.begin(); it != name.end(); it++)
	{
		if (!std::isprint(*it) || *it == ',' || *it == ' ')
			throw (static_cast<std::string>("Name character"));
	}
}

Channel::Channel(const std::string name, const std::string password, User *creator):
	_name("Default name"),
	_password(password),
	_topic("Default topic"),
	_users(),
	_operators(),
	_user_limit(0),
	_i(false),
	_t(false),
	_l(false),
	_pad0(0)
{
	correct_chan_name(name);
	_name = name;
    if (creator)
	{
        _users.push_back(creator);
        _operators.push_back(creator);
    }
}

Channel::Channel(const Channel &other):
	_name(other._name),
	_password(other._password),
	_topic(other._topic),
	_users(other._users),
	_operators(other._operators),
	_user_limit(other._user_limit),
	_i(other._i),
	_t(other._t),
	_l(other._l),
	_pad0(0)
{}

Channel	&Channel::operator=(const Channel &other)
{
	if (this != &other)
	{
		_i = other._i;
		_t = other._t;
		_l = other._l;
		_user_limit = other._user_limit;
		_name = other._name;
		_password = other._password;
		_topic = other._topic;
		_users = other._users;
		_operators = other._operators;
	}
	return (*this);
}

__attribute__((pure))
bool Channel::operator==(const std::string &name) const
{
	return (_name == name);
}

Channel::~Channel(void)
{
	for (std::vector<User*>::iterator it = _users.begin(); it != _users.end(); it++)
		(*it)->send_message("KICK " + _name + " " + (*it)->get_nickname() + " :Channel closed by server\r\n");
	_operators.clear();
	_users.clear();
}

User	*Channel::select_user(std::string nickname)
{
	for (std::vector<User*>::iterator it = _users.begin(); it != _users.end(); it++)
	{
		if ((*it)->operator==(nickname))
			return (*it);
	}
	throw (ERR_NOSUCHNICK);
}

__attribute__((pure))
bool	Channel::is_in_users(const User &someone) const
{
	return (std::find(_users.begin(), _users.end(), &someone) != _users.end());
}

__attribute__((pure))
bool	Channel::is_in_ops(const User &someone) const
{
	return (std::find(_operators.begin(), _operators.end(), &someone) != _operators.end());
}

void	Channel::send_chan_infos(User *asking)
{
	if (!_topic.empty())
		asking->send_message(RPL_TOPIC + " " + asking->get_nickname() + " " + _name + " :" + _topic + "\r\n");
	std::string list = RPL_NAMREPLY + " " + asking->get_nickname() + " = " + _name + " :";
	for (size_t i = 0; i < _users.size(); ++i)
	{
		if (is_in_ops(*_users[i]))
			list += "@";
		list += _users[i]->get_nickname();
		if (i + 1 < _users.size())
			list += " ";
	}
	asking->send_message(list + "\r\n");
	asking->send_message(RPL_ENDOFNAMES + " " + asking->get_nickname() + " " + _name + " :End of /NAMES list\r\n");
}

std::string	Channel::channel_infos(const User *asking) const
{
	std::string	answer = RPL_CHANNELMODEIS + " " + asking->get_nickname() + " " + _name + " ";
	if (_i)
		answer += "i";
	if (_t)
		answer += "t";
	if (!_password.empty())
		answer += "k";
	if (_l)
		answer += "l";
	for (unsigned long i = _operators.size(); i != 0; i--)
		answer += "o";
	if (!_password.empty())
		answer += " " + _password;
	if (_l)
	{
		std::stringstream ss;
		ss << _user_limit;
		answer += " " + ss.str();
	}
	for (User_vector::const_iterator it = _operators.begin(); it != _operators.end(); it++)
		answer += " " + (*it)->get_nickname();
	answer += "\r\n";
	return (answer);
}

void	Channel::mode_selection(char c, std::stringstream &stream, std::string &answer, std::string &options, bool &plus_minus)
{
	switch (c)
	{
		case '+':
			{
				plus_minus = true;
				break;
			}
		case '-':
			{
				plus_minus = false;
				break;
			}
		case 'i':
			{
				_i = plus_minus;
				if (plus_minus)
					answer += "+i";
				else
					answer += "-i";
				break;
			}
		case 't':
			{
				_t = plus_minus;
				if (plus_minus)
					answer += "+t";
				else
					answer += "-t";
				break;
			}
		case 'k':
			{
				manage_password(plus_minus, stream, answer, options);
				break;
			}
		case 'o':
			{
			   manage_operators(plus_minus, stream, answer, options);
			   break;
			}
		case 'l':
			{
			   manage_limit(plus_minus, stream, answer, options);
			   break;
			}
		default:
			 break;
	}
}

void	Channel::manage_password(bool plus_minus, std::stringstream &stream, std::string &answer, std::string &options)
{
	std::string		tmp;

	if (plus_minus)
	{
		if (stream >> tmp)
		{
			_password = tmp;
			answer += "+k";
			options += " " + tmp;
		}
	}
	else
	{
		_password.clear();
		answer += "-k";
	}
}

void	Channel::manage_operators(bool plus_minus, std::stringstream &stream, std::string &answer, std::string &options)
{
	std::string tmp;
	User *target;

	if (!(stream >> tmp))
		return;
	try
	{
		target = select_user(tmp);
		if (plus_minus == false && _operators.size() > 1 && is_in_ops(*target))
		{
			answer += "-o";
			options += " " + tmp;
			std::vector<User*>::iterator it = std::find(_operators.begin(), _operators.end(), target);
			if (it != _operators.end())
				_operators.erase(it);
		}
		else if (plus_minus == true && !is_in_ops(*target))
		{
			answer += "+o";
			options += " " + tmp;
			_operators.push_back(target);
		}
	}
	catch (std::string &e)
	{}
}

void	Channel::manage_limit(bool plus_minus, std::stringstream &stream, std::string &answer, std::string &options)
{
	std::string			user_limit;
	std::stringstream	ss;
	double			limit_value;

	if (plus_minus && stream >> user_limit)
	{
		limit_value = strtod(user_limit.c_str(), NULL);
		if ((0 <= limit_value && limit_value <= INT_MAX) &&
			(_users.size() <= static_cast<unsigned long>(limit_value)))
		{
			_user_limit = static_cast<unsigned int>(limit_value);
			_l = true;
			answer += "+l";
			ss << limit_value;
			options += " " + ss.str() + " ";
		}
	}
	else
	{
		_l = false;
		answer += "-l";
	}
}

std::ostream& operator<<(std::ostream& os, const Channel &channel)
{
	os << channel.get_name() << ": topic: " << channel.get_topic();
	return (os);
}
