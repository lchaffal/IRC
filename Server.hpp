#ifndef SERVER_HPP
# define SERVER_HPP

# include "Log.hpp" // log

# include <iostream>
# include <string>
# include <vector>
# include <algorithm>
# include <cstdlib>
# include <cstring>
# include <fcntl.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <netinet/in.h>
# include <netinet/tcp.h> // For TCP_NODELAY
# include <arpa/inet.h>
# include <stdexcept>

# include "Channel.hpp"
# include "User.hpp"
# include "numerics.hpp"
# include "typedef.hpp"
# include "utils.hpp"

class Server
{
	public:
		Server(unsigned short port, std::string password);
		~Server(void);

		void		routine(void);

		// Public helpers
		void		channel_delete(Channel *target);
		std::string	welcome_message(User &target) const;

	private:
		/* Core Logic */
		User	*select_user(std::string nickname);
		User	*select_username(std::string username);
		Channel	*select_channel(std::string name);

		void    select_server_scan(void);
		void	select_client_read(void);
		void	select_client_write(void);
		void	select_client_delete(void);

		void	cleanup_user_from_channels(User *asking, std::string reason);

		void	create_channel(User *asking, std::string name, std::string password);

		/* Commands */
		void	kick(User *asking, std::string parameters);
		void	invite(User *asking, std::string parameters);
		void	join(User *asking, std::string parameters);
		void	topic(User *asking, std::string parameters);
		void	mode(User *asking, std::string parameters);
		void	part(User *asking, std::string parameters);
		void	cap(User *asking, std::string parameters);
		void	nick(User *asking, std::string parameters);
		void	user(User *asking, std::string parameters);
		void	ping(User *asking, std::string parameters);
		void	privmsg(User *asking, std::string parameters);
		void	quit(User *asking, std::string parameters);
		void	pass(User *asking, std::string parameters);
		void	who(User *asking, std::string parameters);

		/* Data */
		static const Command 	_commands[CMD_NBR];

		std::vector<Channel*>   _channels;
		std::vector<User*>      _users;

		std::string             _password;

		fd_set                  _read_fds;
		fd_set                  _write_fds;

		sockaddr_in             _socket_addr;

		int                     _max_fd;
		int                     _socket;

		unsigned short          _port;

		char                    _pad0[6];
};

#endif
