SRCS	=	srcs/main.cpp srcs/rpl.cpp srcs/server.cpp srcs/client.cpp

OBJS	=	${SRCS:.cpp=.o}

FLAGS	=	-Wall -Wextra -Werror -std=c++98

NAME	=	irc

${NAME}:	${OBJS}
				c++ ${FLAGS} ${OBJS} -o ${NAME}

all:		${NAME}

clean:
				rm -rf ${OBJS}

fclean:		clean
				rm -rf ${OBJS} ${NAME}

re:		fclean all

.PHONY:		all clean fclean re