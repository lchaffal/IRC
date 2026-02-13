// /// /// //
// PERFECT //
// /// /// //

# include <stdexcept> // exception

# include "Log.hpp" // log INFO ERROR
# include "Text.hpp" // TEXT
# include "Parsing.hpp" // port_parse password_parse
# include "Signal.hpp" // signal_routine
# include "Server.hpp" // Server

int main(int argc, char **argv)
{
	unsigned short port = 0;
	char *password = NULL;

	if (argc != 3)
	{
		log(INFO, TEXT_Usage);
		return 1;
	}

	try
	{
		port = port_parse(argv[1]);
		password = password_parse(argv[2]);

		signal_routine();

		Server server(port, password);
		server.routine();
	}
	catch (const std::exception &exception)
	{
		log(ERROR, exception.what());
		return 1;
	}

	return 0;
}
