# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: rahmed <rahmed@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/06/22 22:59:14 by rahmed            #+#    #+#              #
#    Updated: 2022/09/09 22:17:17 by abittel          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME			=	webserv



SRC_DIR			=	.
INC_DIR			=	.
CC				=	clang++
RM				=	rm -f
AR				=	ar rcs

UNAME_S			:=	$(shell uname -s)

ifeq ($(UNAME_S),Darwin) #FOR MAC
OSFLAG			=	-DMACOS
CFLAGS			=	-std=c++98 -g
ARG				=	conf/config_test_rab.cfg #[configuration file]
SRCS			=	main.cpp \
					parsing.cpp \
					Response.cpp \
					Request.cpp \
					utils.cpp \
					Cgi.cpp \
					new_Server.cpp
endif

ifeq ($(UNAME_S),Linux) #FOR LINUX
OSFLAG			=	-DLINUX
CFLAGS			=	-Wall -Wextra -Werror -std=c++98 -g
ARG				=	conf/config_test2.cfg #[configuration file]
SRCS			=	main.cpp \
					parsing.cpp \
					Response.cpp \
					Request.cpp \
					utils.cpp \
					Cgi.cpp \
					Server.cpp
endif

HEADER		=	-I${INC_DIR}
LDFLAGS		=	-L${SRC_DIR}
FSFLAGS		=	-fsanitize=address

OBJS			=	${SRCS:.cpp=.o}
%.o				:	%.cpp
					@echo "${TXT_BLUE}"
					@echo "~~~~~~~ MAKE PROJECT ~~~~~~~~"
					${CC} -c ${CFLAGS} ${OSFLAG} -o $@ $< ${HEADER}
					@echo "${FANCY_RESET}"

${NAME}			:	${OBJS}
					@echo "${TXT_YELLOW}"
					@echo "~~~~~~~ COMPILATION ~~~~~~~~~"
					@echo "${TXT_GREEN}"
#					${CC} ${CFLAGS} ${LDFLAGS} -o ${NAME} ${OBJS}
					${CC} ${CFLAGS} ${LDFLAGS} ${FSFLAGS} -o ${NAME} ${OBJS}
					@echo "${FANCY_RESET}"

all			:	${NAME}

clean :
				@echo "${TXT_YELLOW}"
				@echo "~~~~~~~~~~ CLEAN ~~~~~~~~~~~~"
				@echo "${TXT_RED}"
				${RM}  ${OBJS}
				@echo "$(FANCY_RESET)"

fclean		:	clean
				@echo "${TXT_YELLOW}"
				@echo "~~~~~~~~~~ FCLEAN ~~~~~~~~~~"
				@echo "${TXT_MAGENTA}"
				${RM} ${NAME}
				@echo "$(FANCY_RESET)"

re			:	fclean all

.PHONY		:	all clean fclean re
############### PARTIE TEST ###############
test		:
				make
				@echo "${TXT_YELLOW}"
				@echo "\n~~~~~~~~~~   CPP   ~~~~~~~~~~~"
				@echo "${FANCY_RESET}"
				./${NAME} ${ARG}

norm		:
				@echo "${TXT_YELLOW}"
				@echo "\n~~~~~ CHECK PROJECT NORM ~~~~~~~"
				@echo "${TXT_CYAN}"
				norminette ${SRC_DIR}/*.c -R CheckForbiddenSourceHeader
				@echo "${TXT_GREEN}"
				norminette ${INC_DIR}/*.h -R CheckDefine
				@echo "${FANCY_RESET}"

flush		:	fclean
				@echo "${TXT_YELLOW}"
				@echo "\n~~~~~~~ Extra-CLEANING  ~~~~~~~~"
				@echo "${TXT_RED}"
				${RM} .DS_STORE
				${RM} *.out
				${RM} *.a
				${RM} ${SRC_DIR}/*.o
				${RM} -R *.dSYM
				@echo "${FANCY_RESET}"

# Set COLORS
TXT_RED		=	\033[1;31m
TXT_GREEN	=	\033[1;32m
TXT_YELLOW	=	\033[1;33m
TXT_BLUE	=	\033[1;34m
TXT_MAGENTA	=	\033[1;35m
TXT_CYAN	=	\033[1;36m
BCK_RED		=	\033[0;41m
BCK_GREEN	=	\033[0;42m
BCK_YELLOW	=	\033[0;43m
BCK_BLUE	=	\033[0;44m
BCK_MAGENTA	=	\033[0;45m
BCK_CYAN	=	\033[0;46m
FANCY_RESET	=	\033[0m
###########################################
