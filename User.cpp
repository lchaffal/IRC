#include "User.hpp"

User::User(int client_socket, sockaddr_in client_socket_addr):
	_username(""),
	_nickname(""),
	_read_buffer(""),
	_write_buffer(""),
	_socket_addr(client_socket_addr),
	_socket(client_socket),
	_delete(false),
	_unlocked(false),
	_registered(false),
	_pad0(0)
{
	_pad0 = 0;
}

User::~User(void)
{
	if (_socket >= 0)
	{
		close(_socket);
	}
}

bool User::operator==(std::string nickname)
{
	return (nickname == _nickname);
}

std::string User::get_username(void) const
{
	return (_username);
}

void User::set_username(std::string username)
{
	_username = username;
}

std::string User::get_nickname(void) const
{
	return (_nickname);
}

void User::set_nickname(std::string nickname)
{
	_nickname = nickname;
}

int User::get_socket(void) const
{
	return (_socket);
}

std::string User::get_ip(void) const
{
	return std::string(inet_ntoa(_socket_addr.sin_addr));
}

unsigned short User::get_port(void) const
{
	return ntohs(_socket_addr.sin_port);
}

size_t User::get_write_buffer_size(void) const
{
	return _write_buffer.size();
}

size_t User::get_read_buffer_size(void) const
{
	return _read_buffer.size();
}

std::string	User::source(void) const
{
	return (":" + _nickname + "!" + _username + "@" + get_ip());
}





bool User::is_delete(void) const
{
	return (_delete);
}

void User::set_delete(void)
{
	_delete = true;
}

bool User::is_unlocked(void) const
{
	return (_unlocked);
}

void User::set_unlocked(void)
{
	_unlocked = true;
}

bool User::is_registered(void) const
{
	return (_registered);
}

void User::set_registered(void)
{
	_registered = true;
}





void User::send_outgoing_data(void)
{
    if (_delete || _write_buffer.empty())
    {
        return;
    }

    ssize_t bytes = send(_socket, _write_buffer.c_str(), _write_buffer.size(), 0);

    if (bytes < 0)
    {
        std::cerr << "Error sending data to " << _nickname << ". Disconnecting.\n";
        set_delete();
        return;
    }
    else if (bytes == 0)
    {
        return;
    }

    _write_buffer.erase(0, static_cast<size_t>(bytes));
}

void User::send_message(const std::string &message)
{
    if (_delete)
    {
        return;
    }

    if (_write_buffer.size() + message.size() > MAX_BUFFER_LIMIT)
    {
        std::cerr << "Client " << _nickname << " write buffer exceeded. Disconnecting.\n";
        set_delete();
        return;
    }

    _write_buffer.append(message);
}





std::string User::recv_data(void)
{
	if (_delete)
	{
		return "";
	}

	size_t pos = _read_buffer.find('\n');
	if (pos != std::string::npos)
	{
		std::string line = _read_buffer.substr(0, pos + 1);
		if (string_good(line.c_str()) == 0)
		{
			_read_buffer.erase(0, pos + 1);
			log(WARNING, TEXT_StringBad);
			send_message(ERR_UNKNOWNERROR + " " + _nickname + " " + TEXT_StringBad + "\r\n");
			return "";
		}
		_read_buffer.erase(0, pos + 1);
		return line;
	}

	if (_read_buffer.size() > MAX_BUFFER_LIMIT)
	{
		std::cerr << "Client " << _nickname << " exceeded buffer limit. Disconnecting.\n";
		set_delete();
		return "";
	}

	char temp[MAX_IO_SIZE];
	ssize_t bytes = recv(_socket, temp, MAX_IO_SIZE, 0);

	if (bytes > 0)
	{
		_read_buffer.append(temp, static_cast<size_t>(bytes));
	}
	else if (bytes == 0)
	{
		set_delete();
		return "";
	}

	pos = _read_buffer.find('\n');
	if (pos != std::string::npos)
	{
		std::string line = _read_buffer.substr(0, pos + 1);
		if (string_good(line.c_str()) == 0)
		{
			_read_buffer.erase(0, pos + 1);
			log(WARNING, TEXT_StringBad);
			send_message(ERR_UNKNOWNERROR + " " + _nickname + " " + TEXT_StringBad + "\r\n");
			return "";
		}
		_read_buffer.erase(0, pos + 1);
		return line;
	}

	return "";
}
