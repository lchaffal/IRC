// /// /// //
// PERFECT //
// /// /// //

#ifndef SIGNAL_HPP
# define SIGNAL_HPP

# include <csignal> // struct sigaction, sigemptyset, sigaction, SIGINT, SIGQUIT, SIGPIPE, SIG_IGN
# include <stdexcept> // runtime_error

# include "Text.hpp" // TEXT

extern volatile int g_sig;

void signal_routine(); // throw

#endif
