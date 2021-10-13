all:
	gcc source/sem.c -o bin/sem -Wall -Werror -lm -fsanitize=address,leak
