#ifndef TYPDEF_HPP
# define TYPDEF_HPP

# include <string>
# include <vector>

# include <netinet/in.h> /* sockaddr_in */

# define CMD_NBR 14
//for number of chans
# define MID_LIMIT 64
//for whole messages
# define BIG_LIMIT 512

class Channel;
class User;
class Server;

typedef std::vector<Channel*>		Channel_vector;
typedef std::vector<User*>		User_vector;
struct Command
{
	const char *name;
	void (Server::*function)(User*, std::string);
};

#endif

