%: %.c
<<<<<<< HEAD
	gcc $@.c -o $@ -Wall -Werror -lm -fsanitize=address,leak
=======
	gcc $@.c -o $@ -Wall -lm -fsanitize=address,leak
>>>>>>> dev-main
	cpplint --filter=-legal/copyright $@.c
