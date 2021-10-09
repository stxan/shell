%: %.c
	gcc $@.c -o $@ -Wall -lm
	cpplint --filter=-legal/copyright $@.c
