// /// /// //
// PERFECT //
// /// /// //

#include "Signal.hpp"

volatile int g_sig = 0;

static void SIG_HANDLE(int sig)
{
	(void)sig;
	g_sig = 1;
}

void signal_routine()
{
	struct sigaction sa = {};
	struct sigaction sa_pipe = {};

	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_HANDLE;
	sa.sa_flags = SA_RESTART; // works with (recv, send, accept) does not work with (select)

	sigemptyset(&sa_pipe.sa_mask);
	sa_pipe.sa_handler = SIG_IGN;
	sa_pipe.sa_flags = SA_RESTART; // works with (recv, send, accept) does not work with (select)

	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		throw std::runtime_error(TEXT_SignalIntBad);
	}

	if (sigaction(SIGQUIT, &sa, NULL) == -1)
	{
		throw std::runtime_error(TEXT_SignalQuitBad);
	}

	if (sigaction(SIGPIPE, &sa_pipe, NULL) == -1)
	{
		throw std::runtime_error(TEXT_SignalPipeBad);
	}
}
