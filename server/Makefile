SRC_DIR			=	sources/

TEST_DIR		=	tests/

LIB_DIR			=	lib/sm_libc/

SRC_FILES		=	server.c			\

SRC_FILES_MAIN	=	main.c			\

TEST_FILES		=				\

SRC				=	$(addprefix $(SRC_DIR), $(SRC_FILES))

SRC_TEST		=	$(addprefix $(TEST_DIR), $(TEST_FILES))

SRC_MAIN		=	$(addprefix $(SRC_DIR), $(SRC_FILES_MAIN))

OBJ				=	$(SRC:.c=.o) $(SRC_MAIN:.c=.o)	

NAME			=	server

##############################################################

CFLAGS			=	-W -Wall -Wextra

CPPFLAGS		=	-I./includes

TEST_LIB		=	--coverage -lcriterion -lgcov

##############################################################

CC				=	gcc

PRNT			=	echo

COVERAGE		= gcovr

TEST_NAME		=	unit_test


##############################################################

all: $(NAME)

$(NAME): $(OBJ) $(OBJ_MAIN)
	$(CC) -o $(NAME) $(OBJ) $(OBJ_MAIN)


clean:
	$(RM) $(OBJ) $(OBJ_MAIN)
	$(RM) *.gc*


fclean: clean
	$(RM) $(NAME) $(TEST_NAME)

unit_test: fclean
	$(CC) -o $(TEST_NAME) $(SRC) $(SRC_TEST) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) $(TEST_LIB)

tests_run: unit_test
	./$(TEST_NAME)
	$(COVERAGE) --exclude $(TEST_DIR)
	$(COVERAGE) -b --exclude $(TEST_DIR)

re: fclean $(NAME)

.PHONY:	all clean fclean unit_test tests_run re