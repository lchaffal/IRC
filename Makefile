# release / debug makefile :3

#NAME = ircserv_debug

#CXX = c++

#CXXFLAGS = -std=c++98 -pedantic -Wall -Wextra -Werror -Weffc++ -Wpadded -fsanitize=address,undefined,leak
#LDFLAGS  = -Og -fsanitize=address,undefined,leak



NAME = ircserv

CXX = c++

CXXFLAGS = -std=c++98 -pedantic -Wall -Wextra -Werror -Weffc++ -Wpadded -flto
LDFLAGS  = -O3 -flto



SRC = Channel.cpp Channel_commands.cpp Server.cpp Server_commands.cpp User.cpp utils.cpp \
		main.cpp Parsing.cpp Signal.cpp Log.cpp

OBJ = $(SRC:%.cpp=%.o)
DEP = $(SRC:%.cpp=%.d)



all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(LDFLAGS) $^ -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -o $@ -c $<

clean:
	rm -f $(OBJ) $(DEP)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEP)

.PHONY: all clean fclean re
