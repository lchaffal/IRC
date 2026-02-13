#include "Server.hpp"
#include "typedef.hpp"

#include "Signal.hpp" // g_sig
#include "Text.hpp"



/* lmarion: fini */
const Command Server::_commands[CMD_NBR] = {
	{"KICK", &Server::kick},
	{"INVITE", &Server::invite},
	{"JOIN", &Server::join},
	{"TOPIC", &Server::topic},
	{"MODE", &Server::mode},
	{"PART", &Server::part},
	{"CAP",	 &Server::cap},
	{"NICK", &Server::nick},
	{"USER", &Server::user},
	{"PING", &Server::ping},
	{"PRIVMSG", &Server::privmsg},
	{"QUIT", &Server::quit},
	{"PASS", &Server::pass},
	{"WHO", &Server::who}
};



/* lmarion: fini */
Server::Server(unsigned short port, std::string password):
	_channels(),
	_users(),
	_password(password),
	_read_fds(),
	_write_fds(),
	_socket_addr(),
	_max_fd(0),
	_socket(-1),
	_port(port)
{
	/* [server] configuration */
	std::memset(&_socket_addr, 0, sizeof(_socket_addr));
	_socket_addr.sin_family = AF_INET;
	_socket_addr.sin_addr.s_addr = INADDR_ANY;
	_socket_addr.sin_port = htons(port);

	/* [server] create socket */
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0)
	{
		throw (std::runtime_error(TEXT_ServerSocket));
	}

	/* [server] setsockopt (rebindable quickly) */
	int opt = 1;
	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(_socket);
		throw (std::runtime_error(TEXT_ServerSetsockopt));
	}

	// /* [server] fcntl (get) */
	// int flags = fcntl(_socket, F_GETFL, 0);
	// if (flags < 0)
	// {
	// 	close(_socket);
	// 	throw (std::runtime_error(TEXT_ServerFcntlGet));
	// }

	/* [server] fcntl (set non-blocking) */
	// if (fcntl(_socket, F_SETFL, flags | O_NONBLOCK) < 0)
	if (fcntl(_socket, F_SETFL, O_NONBLOCK) < 0)
	{
		close(_socket);
		throw (std::runtime_error(TEXT_ServerFcntlSet));
	}

	/* [server] bind */
	if (bind(_socket, reinterpret_cast<sockaddr *>(&_socket_addr), sizeof(_socket_addr)) < 0)
	{
		close(_socket);
		throw (std::runtime_error(TEXT_ServerBind));
	}

	/* [server] getsockname */
	socklen_t addr_len = sizeof(_socket_addr);
	if (getsockname(_socket, reinterpret_cast<sockaddr *>(&_socket_addr), &addr_len) < 0)
	{
		close(_socket);
		throw (std::runtime_error(TEXT_ServerGetsockname));
	}
	_port = ntohs(_socket_addr.sin_port);

	/* [server] listen (max) */
	if (listen(_socket, SOMAXCONN) < 0)
	{
		close(_socket);
		throw (std::runtime_error(TEXT_ServerListen));
	}

	log(CREATE, TEXT_ServerCreate);
}



/* lmarion: fini */
std::string Server::welcome_message(User &target) const
{
	std::stringstream message;

	std::string name = "Awoo";
	std::string version = "1.0";
	std::string created = "Not so long ago";
	std::string user_modes = "no user mode";
	std::string chan_modes = "ikolt"; // Invite-only, Password, Operator, Limit, Topic

	message << RPL_WELCOME  << " " << target.get_nickname() << " :" << target.get_nickname() << "!" << target.get_username() << "@" << target.get_ip() << "\r\n";
	message << RPL_YOURHOST << " " << target.get_nickname() << " :" << name << ", version " << version << "\r\n";
	message << RPL_CREATED  << " " << target.get_nickname() << " :" << created << "\r\n";
	message << RPL_MYINFO   << " " << target.get_nickname() << " " << name << " " << version << " " << user_modes << " " << chan_modes << "\r\n";
	message << RPL_ISUPPORT << " " << target.get_nickname() << " " << "CHANMODES=" << chan_modes << " CHANTYPES=#" << " :" << "are supported" << "\r\n";

	return (message.str());
}



/* lmarion: fini */
Server::~Server()
{
	while (!_channels.empty())
	{
		Channel* temp = _channels.back();
		_channels.pop_back();
		delete temp;
	}

	while (!_users.empty())
	{
		User* temp = _users.back();
		_users.pop_back();
		delete temp;
	}

	if (_socket > 0)
	{
		close(_socket);
	}

	log(DELETE, TEXT_ServerDelete);
}



/* lmarion: fini */
void Server::routine(void)
{
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 10000; // 10ms

	while (g_sig != 1)
	{
		select_client_delete();

		FD_ZERO(&_read_fds);
		FD_ZERO(&_write_fds);
		FD_SET(_socket, &_read_fds);
		FD_SET(_socket, &_write_fds);
		_max_fd = _socket;

		unsigned int i = 0;
		while (i < _users.size())
		{
			int sd = _users[i]->get_socket();
			if (sd > 0)
			{
				FD_SET(sd, &_read_fds);
				FD_SET(sd, &_write_fds);
				if (sd > _max_fd)
				{
					_max_fd = sd;
				}
			}
			i += 1;
		}

		int ret = select(_max_fd + 1, &_read_fds, &_write_fds, NULL, &timeout);

		if (g_sig == 1)
		{
			break;
		}

		if (ret < 0)
		{
			throw (std::runtime_error(TEXT_ServerSelect));
		}
		else if (ret == 0)
		{
			continue;
		}
		else
		{
			select_server_scan();
			select_client_read();
			select_client_write();
		}
	}
}



/* lmarion: fini */
void Server::select_server_scan()
{
	if (FD_ISSET(_socket, &_read_fds))
	{
		/* [client] configuration */
		sockaddr_in cli_addr;
		std::memset(&cli_addr, 0, sizeof(cli_addr));

		/* [client] create socket */
		socklen_t addr_len = sizeof(cli_addr);
		int cli_sock = accept(_socket, reinterpret_cast<sockaddr *>(&cli_addr), &addr_len);
		if (cli_sock < 0)
		{
			log(WARNING, TEXT_ClientSocket);
			return;
		}

		/* [client] setsockopt (no delay) */
		int opt = 1;
		if (setsockopt(cli_sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0)
		{
			close(cli_sock);
			log(WARNING, TEXT_ClientSetsockopt);
			return;
		}

		/* [client] fcntl (get) */
		// int flags = fcntl(cli_sock, F_GETFL, 0);
		// if (flags < 0)
		// {
		// 	close(cli_sock);
		// 	log(WARNING, TEXT_ClientFcntlGet);
		// 	return;
		// }

		/* [client] fcntl (set non-blocking) */
		// if (fcntl(cli_sock, F_SETFL, flags | O_NONBLOCK) < 0)
		if (fcntl(cli_sock, F_SETFL, O_NONBLOCK) < 0)
		{
			close(cli_sock);
			log(WARNING, TEXT_ClientFcntlSet);
			return;
		}

		try
		{
			/* [client] Successfully allocated */
			_users.push_back(new User(cli_sock, cli_addr));
			log(CREATE, TEXT_ClientAllocSuccess);
		}
		catch (const std::exception& e)
		{
			/* [client] Failed to allocate */
			close(cli_sock);
			log(ERROR, TEXT_ClientAllocFail);
		}
	}
}



/* lmarion: fini */
void Server::select_client_delete(void)
{
	std::vector<User*>::iterator it = _users.begin();
	while (it != _users.end())
	{
		if ((*it)->is_delete())
		{
			/* [client] Successfully freed */
			cleanup_user_from_channels(*it, "Quit");
			delete *it;
			it = _users.erase(it);
			log(DELETE, TEXT_ClientFreeSuccess);
			//return // every one
		}
		else
		{
			it += 1;
		}
	}
}



/* lmarion: fini */
void Server::channel_delete(Channel *target)
{
	std::vector<Channel*>::iterator it = _channels.begin();
	while (it != _channels.end())
	{
		if (*it == target)
		{
			/* [channel] Successfully freed */
			delete *it;
			it = _channels.erase(it);
			log(DELETE, TEXT_ChannelFreeSuccess);
			return; // only one
		}
		else
		{
			it += 1;
		}
	}
}



/* lmarion: fini */
void Server::cleanup_user_from_channels(User *asking, std::string reason)
{
	std::vector<Channel*>::iterator it = _channels.begin();
	while (it != _channels.end())
	{
		bool should_delete = false;

		try {
			if ((*it)->part(asking, reason))
			{
				should_delete = true;
			}
		}
		catch (...)
		{
		}

		if (should_delete)
		{
			/* [channel] Successfully freed */
			delete *it;
			it = _channels.erase(it);
			log(DELETE, TEXT_ChannelFreeSuccess);
		}
		else
		{
			it += 1;
		}
	}
}



User *Server::select_user(std::string nickname)
{
	size_t i = 0;
	while (i < _users.size())
	{
		if (_users[i]->get_nickname() == nickname)
		{
			return _users[i];
		}
		i += 1;
	}
	throw (ERR_NOSUCHNICK);
}

User *Server::select_username(std::string username)
{
	size_t i = 0;
	while (i < _users.size())
	{
		if (_users[i]->get_username() == username)
		{
			return _users[i];
		}
		i += 1;
	}
	throw (ERR_NOSUCHNICK);
}

Channel *Server::select_channel(std::string name)
{
	size_t i = 0;
	while (i < _channels.size())
	{
		if (*_channels[i] == name)
		{
			return _channels[i];
		}
		i += 1;
	}
	throw (ERR_NOSUCHCHANNEL);
}




void Server::select_client_write(void)
{
	std::vector<User*>::iterator it = _users.begin();
	while (it != _users.end())
	{
		if (FD_ISSET((*it)->get_socket(), &_write_fds))
		{
			(*it)->send_outgoing_data();
		}
		it += 1;
	}
}

//For each user, goes through input a line at a time
//For each line, checks if the first word is a command name
//and calls the correct function if it is
void Server::select_client_read(void)
{
	std::vector<User*>::iterator it = _users.begin();
	while (it != _users.end())
	{
		if (FD_ISSET((*it)->get_socket(), &_read_fds))
		{
			while (true)
			{
				std::string line = (*it)->recv_data();

				if (line.empty())
				{
					break;
				}

				if (!line.empty() && line[line.size() - 1] == '\n')
				{
					line.erase(line.size() - 1);
				}
				if (!line.empty() && line[line.size() - 1] == '\r')
				{
					line.erase(line.size() - 1);
				}
				
				if (line.empty() || line.size() > BIG_LIMIT - 2)
				{
					log(WARNING, "Line too long received");
					continue;
				}
				log(INFO, "Received from " + (*it)->get_nickname() + ": " + line);

				std::istringstream stream(line);
				std::string cmd_name;
				stream >> cmd_name;

				std::string params;
				if (line.size() > cmd_name.size())
					params = line.substr(cmd_name.size());

				size_t first = params.find_first_not_of(" \t");
				if (first != std::string::npos) 
					params = params.substr(first);
				else 
					params.clear();

				bool found = false;
				int i = 0;
				while (i < CMD_NBR)
				{
					if (cmd_name == _commands[i].name)
					{
						(this->*_commands[i].function)(*it, params);
						found = true;
						break;
					}
					i++;
				}

				if (!found)
				{
					(*it)->send_message(ERR_UNKNOWNCOMMAND + " " + (*it)->get_nickname() + " " + cmd_name + " :Unknown command\r\n");
					log(WARNING, "Unknown command: " + cmd_name);
				}
			}
		}
		it += 1;
	}
}

void	Server::create_channel(User *asking, std::string name, std::string password)
{
	try
	{
		if (_channels.size() >= MID_LIMIT)
		{
			throw std::string("too many channels");
		}
		Channel *new_chan = new Channel(name, password, asking);
		_channels.push_back(new_chan);
		log(CREATE, "Channel created: " + name);
		asking->send_message(asking->source() + " JOIN " + name + "\r\n");
		asking->send_message(RPL_TOPIC + " " + asking->get_nickname() + " " + name + " :Default topic\r\n");
		asking->send_message(RPL_NAMREPLY + " " + asking->get_nickname() + " = " + name + " :@" + asking->get_nickname() + "\r\n");
		asking->send_message(RPL_ENDOFNAMES + " " + asking->get_nickname() + " " + name + " :End of /NAMES list\r\n");
	}
	catch (std::string &err)
	{
		if (err == "Name character")
		{
			asking->send_message(ERR_UNKNOWNERROR + " " + asking->get_nickname() + " JOIN :Channel name contains incorrect characters\r\n");
		}
		else if (err == "too many channels")
		{
			asking->send_message(ERR_UNKNOWNERROR + " " + asking->get_nickname() + " JOIN :There is too many channels on the server already\r\n");
		}
		asking->send_message(ERR_NOSUCHCHANNEL + " " + asking->get_nickname() + " " + name + " :No such channel\r\n");
	}
}
