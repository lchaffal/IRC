// /// /// //
// PERFECT //
// /// /// //

#ifndef TEXT_HPP
# define TEXT_HPP

# define TEXT_Usage "Usage: ./ircserv <port> <password>"

# define TEXT_PortBad "Port unsupported"
# define TEXT_PortOutOfRange "Port out of range (0-65535)"

# define TEXT_PasswordEmpty "Password empty"

# define TEXT_StringBad "String unsupported"

# define TEXT_SignalIntBad "Signal SIGINT unsupported"
# define TEXT_SignalQuitBad "Signal SIGQUIT unsupported"
# define TEXT_SignalPipeBad "Signal SIGPIPE unsupported"

# define TEXT_ServerSocket "[server] create socket failed"
# define TEXT_ServerSetsockopt "[server] setsockopt (rebindable quickly) failed"
# define TEXT_ServerFcntlGet "[server] fcntl (get) failed"
# define TEXT_ServerFcntlSet "[server] fcntl (set non-blocking) failed"
# define TEXT_ServerBind "[server] bind failed"
# define TEXT_ServerGetsockname "[server] getsockname failed"
# define TEXT_ServerListen "[server] listen failed"
# define TEXT_ServerCreate "Successfully create server"
# define TEXT_ServerDelete "Successfully delete server"

# define TEXT_ServerSelect "[server] select failed"

# define TEXT_ClientSocket "[client] create socket failed"
# define TEXT_ClientSetsockopt "[client] setsockopt (rebindable quickly) failed"
# define TEXT_ClientFcntlGet "[client] fcntl (get) failed"
# define TEXT_ClientFcntlSet "[client] fcntl (set non-blocking) failed"
# define TEXT_ClientAllocSuccess "Successfully allocated user"
# define TEXT_ClientAllocFail "Failed to allocate user"
# define TEXT_ClientFreeSuccess "Successfully freed user"

# define TEXT_ChannelFreeSuccess "Successfully freed channel"

#endif
