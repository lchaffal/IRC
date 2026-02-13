#include "Server.hpp"

void	Server::pass(User *asking, std::string parameters)
{
	if (asking->is_registered())
	{
		asking->send_message(ERR_ALREADYREGISTERED + " " + asking->get_nickname() + " :You may not reregister\r\n");
		return;
	}
	if (parameters.empty())
	{
		asking->send_message(ERR_NEEDMOREPARAMS + " * PASS :Not enough parameters\r\n");
		return;
	}
	if (parameters == _password)
	{
		asking->set_unlocked();
		std::stringstream ss;
		ss << "Password accepted for socket " << asking->get_socket();
		log(INFO, ss.str());
	}
	else
	{
		asking->send_message(ERR_PASSWDMISMATCH + " * :Password incorrect\r\n");
		log(WARNING, "Password mismatch for socket");
	}
}

void	Server::nick(User *asking, std::string parameters)
{
	if (!asking->is_unlocked())
	{
		asking->send_message("ERROR :You must send PASS first\r\n");
		return;
	}
	trim_spaces(parameters);
	if (parameters.empty())
	{
		if (!asking->get_nickname().empty())
			asking->send_message(ERR_NONICKNAMEGIVEN + " " + asking->get_nickname() + " :No nickname given\r\n");
		else
		{
			asking->send_message(ERR_NONICKNAMEGIVEN + " unknown :No nickname given\r\nERROR\r\n");
			asking->set_delete();
		}
		return;
	}
	if (!check_nickname(parameters))
	{
		if (!asking->get_nickname().empty())
			asking->send_message(ERR_ERRONEUSNICKNAME + " " + asking->get_nickname() + " " + parameters + " :Erroneus nickname\r\n");
		else
		{
			asking->send_message(ERR_ERRONEUSNICKNAME + " unknown " + parameters + " :Erroneus nickname\r\n");
			asking->send_message("ERROR\r\n");
			asking->set_delete();
		}
		return;
	}
	if (nick_taken(_users, parameters))
	{
		if (!asking->get_nickname().empty())
			asking->send_message(ERR_NICKNAMEINUSE + " " + asking->get_nickname() + " " + parameters + " :Nickname is already in use\r\n");
		else
		{
			asking->send_message(ERR_NICKNAMEINUSE + " unknown " + parameters + " :Nickname is already in use\r\n");
			asking->send_message("ERROR\r\n");
			asking->set_delete();
		}
		return;
	}
	if (!asking->get_nickname().empty())
		asking->send_message(asking->source() + " NICK " + parameters + "\r\n");
	asking->set_nickname(parameters);
}

void	Server::user(User *asking, std::string parameters)
{
	if (!asking->is_unlocked())
	{
		asking->send_message("ERROR :You must send PASS first\r\n");
		return;
	}
	if (asking->is_registered())
	{
		asking->send_message(ERR_ALREADYREGISTERED + " :You may not reregister\r\n");
		return;
	}
	std::istringstream iss(parameters);
	std::string username, mode, unused;
	iss >> username >> mode >> unused;
	if (username.empty())
	{
		asking->send_message(ERR_NEEDMOREPARAMS + " USER :Not enough parameters\r\n");
		return;
	}
	if (!asking->get_nickname().empty())
	{
		asking->set_username(username);
		asking->set_registered();
		asking->send_message(welcome_message(*asking));
		log(INFO, "User registered: " + asking->get_nickname());
	}
}

void	Server::join(User *asking, std::string parameters)
{
	std::string			name;
	std::string			password;
	std::stringstream	stream(parameters);

	if (!asking->is_registered())
	{
		asking->send_message(ERR_NOTREGISTERED + " :You have not registered\r\n");
		return;
	}
	stream >> name;
	std::stringstream	channel_names(name);
	name.clear();
	stream >> name;
	std::stringstream	password_list(name);
	name.clear();
	stream >> name;
	while (std::getline(channel_names, name, ',')) 
	{ 
		if (name.empty() || name[0] != '#')
			continue;
		std::getline(password_list, password, ',');
		trim_spaces(password);
		try
		{
			Channel *chan = select_channel(name);
			chan->join(asking, password);
		}
		catch (std::string &e)
		{
			if (e == ERR_NOSUCHCHANNEL)
				create_channel(asking, name, password);
			else if (e == ERR_INVITEONLYCHAN)
				asking->send_message(e + " " + asking->get_nickname() + " " + name + " :This channel is in invite mode.\r\n");
			else if (e == ERR_CHANNELISFULL)
				asking->send_message(e + " " + asking->get_nickname() + " " + name + " :Channel is full\r\n");
			else if (e == ERR_BADCHANNELKEY)
				asking->send_message(e + " " + asking->get_nickname() + " " + name + " :Password is incorrect\r\n");
			else if (e == ERR_USERONCHANNEL)
				asking->send_message(e + " " + asking->get_nickname() + " " + name + " :You're already on channel\r\n");
		}
	}
}

void	Server::privmsg(User *asking, std::string parameters)
{
	std::string	target_name;
	std::string	msg;
	std::istringstream	iss(parameters);

	if (!asking->is_registered())
	{
		asking->send_message(ERR_NOTREGISTERED + " :You have not registered\r\n");
		return;
	}
	iss >> target_name;
	std::getline(iss, msg);
	trim_spaces(msg);
	if (!msg.empty() && msg[0] == ':')
		msg = msg.substr(1);
	if (msg.empty())
	{
		asking->send_message(ERR_NOTEXTTOSEND + " :No text to send\r\n");
		return;
	}
	try
	{
		if (target_name[0] == '#')
		{
			Channel *chan = select_channel(target_name);
			chan->privmsg(asking, asking->source() + " PRIVMSG " + target_name + " :" + msg + "\r\n");
		}
		else
		{
			User *target = select_user(target_name);
			target->send_message(asking->source() + " PRIVMSG " + target_name + " :" + msg + "\r\n");
		}
	}
	catch (std::string &e)
	{
		if (e == ERR_NOSUCHCHANNEL)
			asking->send_message(e + " " + asking->get_nickname() + " " + target_name + " :No such channel\n");
		else if (e == ERR_CANNOTSENDTOCHAN)
			asking->send_message(e + " " + asking->get_nickname() + " " + target_name + " :Cannot send to channel\n");
		else if (e == ERR_NOSUCHNICK) asking->send_message(ERR_NOSUCHNICK + " " + asking->get_nickname() + " " + target_name + " :No such nickname\n"); }
}

void	Server::kick(User *asking, std::string parameters)
{
	std::string			channel_name, username, message;
	std::stringstream	stream(parameters);
	bool				empty;

	if (!asking->is_registered())
	{
		asking->send_message(ERR_NOTREGISTERED + " :You have not registered\r\n");
		return;
	}
	stream >> channel_name;
	stream >> username;
	
	try
	{
		if (channel_name.empty() || username.empty())
			throw (ERR_NEEDMOREPARAMS);
		Channel	*chan = select_channel(channel_name);
		User	*target = select_user(username);
		std::getline(stream, message);
		if (!message.empty() && message[0] == ' ')
			message.erase(0, 1);
		if (!message.empty() && message[0] == ':')
			message.erase(0, 1);
		message += "\r\n";
		empty = chan->kick(asking, target, message);
		if (empty)
			channel_delete(chan);
	}
	catch (std::string &e)
	{
		if (e == ERR_NEEDMOREPARAMS)
			asking->send_message(e + " " + asking->get_nickname() + " KICK :Not enough parameters\r\n");
		else if (e == ERR_NOSUCHCHANNEL)
			asking->send_message(e + " " + asking->get_nickname() + " " + channel_name + " :No such channel\r\n");
		else if (e == ERR_NOTONCHANNEL)
			asking->send_message(e + " " + asking->get_nickname() + " " + channel_name + " :You're not on that channel\r\n");
		else if (e == ERR_CHANOPRIVSNEEDED)
			asking->send_message(e + " " + asking->get_nickname() + " " + channel_name + " :You're not channel operator\r\n");
		else if (e == ERR_USERNOTINCHANNEL)
			asking->send_message(e + " " + asking->get_nickname() + " " + username + " " + channel_name + " :They aren't on that channel\r\n");
		else if (e == ERR_NOSUCHNICK)
			asking->send_message(e + " " + asking->get_nickname() + " " + username + " :No such nick\r\n");
	}
}

void	Server::invite(User *asking, std::string parameters)
{
	std::stringstream stream(parameters);
	std::string channel_name, username;

	if (!asking->is_registered())
	{
		asking->send_message(ERR_NOTREGISTERED + " :You have not registered\r\n");
		return;
	}
	try
	{
		stream >> username;
		stream >> channel_name;
		if (username.empty() || channel_name.empty())
			throw (ERR_NEEDMOREPARAMS);
		Channel *chan = select_channel(channel_name);
		User	*target = select_user(username);
		chan->invite(asking, target);
	}
	catch (std::string &e)
	{
		if (e == ERR_NEEDMOREPARAMS)
			asking->send_message(e + " " + asking->get_nickname() + " INVITE :Not enough parameters\r\n");
		else if (e == ERR_NOSUCHCHANNEL)
			asking->send_message(e + " " + asking->get_nickname() + " " + channel_name + " :No such channel\r\n");
		else if (e == ERR_NOTONCHANNEL)
			asking->send_message(e + " " + asking->get_nickname() + " " + channel_name + " :You're not on that channel\r\n");
		else if (e == ERR_CHANOPRIVSNEEDED)
			asking->send_message(e + " " + asking->get_nickname() + " " + channel_name + " :You're not channel operator\r\n");
		else if (e == ERR_CHANNELISFULL)
			asking->send_message(e + " " + asking->get_nickname() + " " + channel_name + " :Channel is full\r\n");
		else if (e == ERR_USERONCHANNEL)
			asking->send_message(e + " " + asking->get_nickname() + " " + username + " " + channel_name + " :is already on channel\r\n");
		else if (e == ERR_NOSUCHNICK)
			asking->send_message(e + " " + asking->get_nickname() + " " + username + " :No such nick\r\n");
	}
}

void	Server::topic(User *asking, std::string parameters)
{
	std::string	channel_name, new_topic;
	std::stringstream stream(parameters);

	if (!asking->is_registered())
	{
		asking->send_message(ERR_NOTREGISTERED + " :You have not registered\r\n");
		return;
	}
	stream >> channel_name;
	std::getline(stream, new_topic);
	if (!new_topic.empty() && new_topic[0] == ' ')
		new_topic.erase(0, 1);
	if (!new_topic.empty() && new_topic[0] == ':')
		new_topic.erase(0, 1);
	try
	{
		if (channel_name.empty())
			throw (ERR_NEEDMOREPARAMS);
		Channel *chan = select_channel(channel_name);
		chan->topic(asking, new_topic);
	}
	catch (const std::string& e)
	{
		if (e == ERR_NEEDMOREPARAMS)
			asking->send_message(e + " " + asking->get_nickname() + " TOPIC :Not enough parameters\r\n");
		else if (e == ERR_NOSUCHCHANNEL)
			asking->send_message(e + " " + asking->get_nickname() + " " + channel_name + " :No such channel\r\n");
		else if (e == ERR_NOTONCHANNEL)
			asking->send_message(e + " " + asking->get_nickname() + " " + channel_name + " :You're not on that channel\r\n");
		else if (e == ERR_CHANOPRIVSNEEDED)
			asking->send_message(e + " " + asking->get_nickname() + " " + channel_name + " :You're not channel operator\r\n");
	}
}

void	Server::mode(User *asking, std::string parameters)
{
	std::string channel_name;
	std::stringstream stream(parameters);

	if (!asking->is_registered())
	{
		asking->send_message(ERR_NOTREGISTERED + " :You have not registered\r\n");
		return;
	}
	stream >> channel_name;
	if (channel_name.empty())
	{
		asking->send_message(ERR_NEEDMOREPARAMS + " " + asking->get_nickname() + " MODE :Not enough parameters\r\n");
		return;
	}
	std::string remainder;
	std::getline(stream, remainder);
	if (!remainder.empty() && remainder[0] == ' ')
		remainder.erase(0, 1);
	if (channel_name[0] != '#')
	{
		if (channel_name == asking->get_nickname())
			asking->send_message(RPL_UMODEIS + " " + asking->get_nickname() + " standard\r\n");
		return;
	}
	try
	{
		Channel *chan = select_channel(channel_name);
		chan->mode(asking, remainder);
	}
	catch (const std::string& e)
	{
		if (e == ERR_NOSUCHCHANNEL)
			asking->send_message(e + " " + asking->get_nickname() + " " + channel_name + " :No such channel\r\n");
		else if (e == ERR_CHANOPRIVSNEEDED)
			asking->send_message(e + " " + asking->get_nickname() + " " + channel_name + " :You're not channel operator\r\n");
		else if (e == ERR_NOTONCHANNEL)
			asking->send_message(e + " " + asking->get_nickname() + " " + channel_name + " :You're not on that channel\r\n");
	}
}

void	Server::part(User *asking, std::string parameters)
{
	std::string name, message, single_name;
	std::stringstream stream(parameters);
	bool empty;

	if (!asking->is_registered())
	{
		asking->send_message(ERR_NOTREGISTERED + " :You have not registered\r\n");
		return;
	}
	stream >> name;
	std::stringstream channel_names(name);
	std::getline(stream, message);
	if (!message.empty() && message[0] == ' ')
		message.erase(0, 1);
	if (!message.empty() && message[0] == ':')
		message.erase(0, 1);
	if (name.empty())
	{
		asking->send_message(ERR_NEEDMOREPARAMS + " " + asking->get_nickname() + " PART :Not enough parameters\r\n");
		return;
	}
	while (std::getline(channel_names, single_name, ','))
	{
		try
		{
			Channel *chan = select_channel(single_name);
			empty = chan->part(asking, message);
			if (empty)
				channel_delete(chan);
		}
		catch (std::string &e)
		{
			if (e == ERR_NOTONCHANNEL)
				asking->send_message(e + " " + asking->get_nickname() + " " + single_name + " :You're not on that channel\r\n");
			else if (e == ERR_NOSUCHCHANNEL)
				asking->send_message(e + " " + asking->get_nickname() + " " + single_name + " :No such channel\r\n");
		}
	}
}

void	Server::ping(User *asking, std::string parameters)
{
	asking->send_message("PONG " + parameters + "\r\n");
}

void	Server::quit(User *asking, std::string parameters)
{
	cleanup_user_from_channels(asking, ":Quit: " + parameters);
	asking->send_message("ERROR :You exited the server\n");
	asking->set_delete();
}

void	Server::cap(User *asking, std::string parameters)
{
	if (parameters == "LS")
		asking->send_message("CAP * LS :\r\n");
}

void	Server::who(User *asking, std::string parameters)
{
	std::string			channel_name;
	std::stringstream	stream(parameters);

	if (!asking->is_registered())
	{
		asking->send_message(ERR_NOTREGISTERED + " :You have not registered\r\n");
		return;
	}
	stream >> channel_name;
	try
	{
		if (channel_name.empty())
			throw (ERR_NEEDMOREPARAMS);
		select_channel(channel_name);
		asking->send_message(RPL_ENDOFWHO + " " + asking->get_nickname() + " " + channel_name + " \r\n");
	}
	catch (std::string &e)
	{
		if (e == ERR_NEEDMOREPARAMS)
			asking->send_message(e + " " + asking->get_nickname() + " WHO :Not enough parameters\r\n");
		else if (e == ERR_NOSUCHCHANNEL)
			asking->send_message(e + " " + asking->get_nickname() + " " + channel_name + " :No such channel\n");
	}
}
