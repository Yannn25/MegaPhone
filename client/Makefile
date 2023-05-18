SRC_DIR			=	sources/

TEST_DIR		=	tests/

LIB_DIR			=	lib/sm_libc/

SRC_FILES		=	client.c			\

SRC_FILES_MAIN	=	main.c			\

TEST_FILES		=				\

SRC				=	$(addprefix $(SRC_DIR), $(SRC_FILES))

SRC_TEST		=	$(addprefix $(TEST_DIR), $(TEST_FILES))

SRC_MAIN		=	$(addprefix $(SRC_DIR), $(SRC_FILES_MAIN))

OBJ				=	$(SRC:.c=.o) $(SRC_MAIN:.c=.o)	

NAME			=	client

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
	@$(PRNT) -e "\e[0;31m Compiling ${NAME}... \e[0m"
	$(CC) -o $(NAME) $(OBJ) $(OBJ_MAIN)

make_lib:
	@$(PRNT) -e "\e[0;31m Making lib $(LIB_DIR)... \e[0m"

clean:
	@$(PRNT) -e "\e[0;31m Deleting .o and .gc objects... \e[0m"
	$(RM) $(OBJ) $(OBJ_MAIN)
	$(RM) *.gc*


fclean: clean
	@$(PRNT) -e "\e[0;31m Deleting execs... \e[0m"
	$(RM) $(NAME) $(TEST_NAME)

unit_test: fclean
	@$(PRNT) -e "\e[0;31m Compiling tests for $(NAME)... \e[0m"
	$(CC) -o $(TEST_NAME) $(SRC) $(SRC_TEST) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) $(TEST_LIB)

tests_run: unit_test
	@$(PRNT) -e "\e[0;31m Running tests for $(NAME)... \e[0m"
	./$(TEST_NAME)
	$(COVERAGE) --exclude $(TEST_DIR)
	$(COVERAGE) -b --exclude $(TEST_DIR)

re: fclean $(NAME)

.PHONY:	all make_lib clean fclean unit_test tests_run re