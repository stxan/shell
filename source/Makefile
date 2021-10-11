%: %.c
	gcc $@.c -o $@ -Wall -lm -fsanitize=address,leak
	cpplint --filter=-legal/copyright $@.c
