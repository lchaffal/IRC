#ifndef USER_HPP
# define USER_HPP

# include <iostream>
# include <string>
# include <unistd.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <cerrno>
# include <cstring>
# include <vector>
# include <algorithm>
# include <fcntl.h>

# include "Text.hpp"
# include "Parsing.hpp"
# include "Log.hpp"
# include "numerics.hpp"

# define MAX_IO_SIZE 65535
# define MAX_BUFFER_LIMIT 524288

class User
{
	public:
		User(int client_socket, sockaddr_in client_socket_addr);
		~User(void);

		bool			operator==(std::string nickname);

		/* Getters / Setters */
		std::string		get_username(void) const;
		void			set_username(std::string username);

		std::string		get_nickname(void) const;
		void			set_nickname(std::string nickname);

		int				get_socket(void) const;
		std::string		get_ip(void) const;
		unsigned short	get_port(void) const;

		size_t			get_write_buffer_size(void) const;
		size_t			get_read_buffer_size(void) const;

		std::string		source(void) const;

		/* Status */
		bool			is_delete(void) const;
		void			set_delete(void);

		bool			is_unlocked(void) const;
		void			set_unlocked(void);

		bool			is_registered(void) const;
		void			set_registered(void);

		/* Networking */
		void			send_outgoing_data(void);
		void 			send_message(const std::string &message);

		std::string		recv_data(void);

	private:
		std::string         _username;
		std::string         _nickname;

		std::string         _read_buffer;
		std::string         _write_buffer;

		sockaddr_in         _socket_addr;

		int                 _socket;

		bool                _delete;
		bool                _unlocked;
		bool                _registered;

		char                _pad0;
};

#endif
