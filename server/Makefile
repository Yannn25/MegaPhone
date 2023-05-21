SRC_DIR			=	sources/

SRC_FILES		=	server.c			\

SRC_FILES_MAIN	=	main.c			\

SRC				=	$(addprefix $(SRC_DIR), $(SRC_FILES))

SRC_MAIN		=	$(addprefix $(SRC_DIR), $(SRC_FILES_MAIN))

OBJ				=	$(SRC:.c=.o) $(SRC_MAIN:.c=.o)	

NAME			=	server

##############################################################

CFLAGS			=	-W -Wall -Wextra

CPPFLAGS		=	-I./includes

##############################################################

CC				=	gcc

PRNT			=	echo

##############################################################

all: $(NAME)

$(NAME): $(OBJ) $(OBJ_MAIN)
	$(CC) -o $(NAME) $(OBJ) $(OBJ_MAIN)


clean:
	$(RM) $(OBJ) $(OBJ_MAIN)
	$(RM) *.gc*


fclean: clean
	$(RM) $(NAME) $(TEST_NAME)

re: fclean $(NAME)

.PHONY:	all clean fclean re