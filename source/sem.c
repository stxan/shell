#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

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
	bytes = (n + 1) * sizeof(char *);
	list = realloc(list, bytes);
	list[n] = NULL;
	return list;
}

void memfree(char **list) {
	int i = 0;
	while (list != NULL) {
		free(list[i]);
		i++;
	}
	free(list);
}

int start_shell() {
	char **list = NULL;
	printf("%s", "> ");
	list = get_list();
	while (strcmp(list[0], "quit") && strcmp(list[0], "exit")) {
		if (fork() == 0) {
			if (execvp(list[0], list) < 0) {
				perror("exec failed: ");
				return 1;
			}
			return 0;
		}
		wait(NULL);
		printf("%s", "> ");
		list = get_list();
	}
	memfree(list);
	return 0;
}


int main(int argc, char **argv) {
	int shell_status = start_shell();
	if (shell_status == 0) {
		puts("shell finished succesfully, congrats!");
	}
	else {
		puts("shell failed :( ");
	}
	return 0;
}
