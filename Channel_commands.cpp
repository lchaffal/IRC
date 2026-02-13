#include "Channel.hpp"
#include "numerics.hpp"

bool	Channel::kick(User *asking, User *target, std::string reason)
{
	std::string	message;

	if (!is_in_users(*asking))
		throw (ERR_NOTONCHANNEL);
	if (!is_in_users(*target))
		throw (ERR_USERNOTINCHANNEL);
	if (!is_in_ops(*asking))
		throw (ERR_CHANOPRIVSNEEDED);
	std::vector<User*>::iterator it_op = std::find(_operators.begin(), _operators.end(), target);
	if (it_op != _operators.end())
		_operators.erase(it_op);
	message = asking->source() + " KICK " + _name + " " + target->get_nickname();
	if (reason == ":\n" || reason.empty())
		message += " :No reason specified\r\n";
	else
		message += " " + reason;
	send_everyone(message);
	std::vector<User*>::iterator it = std::find(_users.begin(), _users.end(), target);
	if (it != _users.end())
		_users.erase(it);
	if (_operators.size() == 0)
		return (true);
	return (false);
}

void	Channel::invite(User *asking, User *target)
{
	if (!is_in_users(*asking))
		throw (ERR_NOTONCHANNEL);
	if (is_in_users(*target))
		throw (ERR_USERONCHANNEL);
	if (_i && !is_in_ops(*asking))
		throw (ERR_CHANOPRIVSNEEDED);
	if (_l && _users.size() >= _user_limit)
		throw(ERR_CHANNELISFULL);
	asking->send_message(RPL_INVITING + " " + asking->get_nickname() + " " + target->get_nickname() + " " + _name + "\r\n");
	_users.push_back(target);
	target->send_message(asking->source() + " INVITE " + target->get_nickname() + " " + _name + "\r\n");
	send_everyone(target->source() + " JOIN " + _name + "\r\n");
	send_chan_infos(target);
}

void	Channel::join(User *asking, const std::string password)
{
	if (is_in_users(*asking))
		throw (ERR_USERONCHANNEL);
	if (_i)
		throw (ERR_INVITEONLYCHAN);
	if (_l && _users.size() >= _user_limit)
		throw (ERR_CHANNELISFULL);
	if (_password.empty() || (!_password.empty() && _password.compare(password) == 0))
	{
		_users.push_back(asking);
		send_everyone(asking->source() + " JOIN " + _name + "\r\n");
		send_chan_infos(asking);
	}
	else
		throw (ERR_BADCHANNELKEY);
}

void	Channel::topic(User *asking, std::string new_topic)
{
	if (!is_in_users(*asking))
		throw (ERR_NOTONCHANNEL);
	if (new_topic.empty())
	{
		if (_topic.empty())
			asking->send_message(RPL_NOTOPIC + " " + asking->get_nickname() + " " + _name + " :No topic is set\r\n");
		else
			asking->send_message(RPL_TOPIC + " " + asking->get_nickname() + " " + _name + " :" + _topic + "\r\n");
	}
	else if (_t && !is_in_ops(*asking))
		throw (ERR_CHANOPRIVSNEEDED);
	else
	{
		_topic = new_topic;
		send_everyone(asking->source() +  " TOPIC " + _name + " :" + _topic + "\r\n");
	}
}

void	Channel::mode(User *asking, std::string parameters)
{
	std::stringstream stream(parameters);
	std::string modstring, options, answer;
	bool plus_minus = true;

	if (parameters.empty())
	{
		asking->send_message(channel_infos(asking));
		return;
	}
	else if (parameters == "b")
	{
		asking->send_message(RPL_BANLIST + " " + asking->get_nickname() + " " + _name + "\r\n");
		asking->send_message(RPL_ENDOFBANLIST + " " + asking->get_nickname() + " " + _name + " :End of channel ban list\r\n");
		return;
	}
	if (!(is_in_users(*asking)))
		throw (ERR_NOTONCHANNEL);
	if (!(is_in_ops(*asking)))
		throw (ERR_CHANOPRIVSNEEDED);
	stream >> modstring;
	answer = asking->source() + " MODE " + _name + " ";
	for (size_t i = 0; i < modstring.size(); i++)
		mode_selection(modstring[i], stream, answer, options, plus_minus);
	if (options.length() > 0)
		answer += options;
	answer += "\r\n";
	send_everyone(answer);
}

void	Channel::privmsg(const User *asking, std::string message)
{
	if (is_in_users(*asking))
	{
		for (std::vector<User*>::const_iterator it = _users.begin(); it != _users.end(); it++)
			if (*it != asking)
				(*it)->send_message(message);
	}
	else
		throw (ERR_CANNOTSENDTOCHAN);
}

bool	Channel::part(const User *asking, std::string message)
{
	if (!is_in_users(*asking))
		throw (ERR_NOTONCHANNEL);
	send_everyone(asking->source() + " PART " + _name + " " + message + "\r\n");
	std::vector<User*>::iterator it_op = std::find(_operators.begin(), _operators.end(), asking);
	if (it_op != _operators.end())
		_operators.erase(it_op);
	std::vector<User*>::iterator it = std::find(_users.begin(), _users.end(), asking);
	if (it != _users.end())
		_users.erase(it);
	if (_operators.empty())
		return true;
	else
		return false;
}
