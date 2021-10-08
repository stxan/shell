#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

char *get_word(char *end) {
	int bytes, n = 0;
	char *tmp = NULL;
	char ch = getchar();
	while (ch != ' ' && ch != '\n' && ch != '\t') {
		bytes = (n + 1) * sizeof(char);
		tmp = realloc(tmp, bytes);
		tmp[n] = ch;
		n++;
		ch = getchar();
	}
	bytes = (n + 1) * sizeof(char);
	tmp = realloc(tmp, bytes);
	tmp[n] = '\0';
	*end = ch;
	return tmp;
}

char **get_list() {
	char **list = NULL;
	char *word = NULL;
	char word_last_char;
	int bytes, n = 0;
	do {
		word = get_word(&word_last_char);
		bytes = (n + 1) * sizeof(char *);
		list = realloc(list, bytes);
		list[n] = word;
		n++;
	}while (word_last_char != '\n');
	return list;
}

int main(int argc, char **argv) {
	char **list = get_list();
	return 0;
}