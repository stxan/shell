#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define INPUT 0
#define OUTPUT 1

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
		if (word[0] != ' ' && word[0] != '\t' && word[0] != '\n'){
			bytes = (n + 1) * sizeof(char *);
			list = realloc(list, bytes);
			list[n] = word;
			n++;
		}	
	}while (word_last_char != '\n');
	bytes = (n + 1) * sizeof(char *);
	list = realloc(list, bytes);
	list[n] = NULL;
	return list;
}

void memfree(char **list) {
	int i = 0;
	while (list[i] != NULL) {
		free(list[i]);
		i++;
	}
	free(list[i]);
	free(list);
}

int check_for_inp_outp(char **list, int *rdct_pos) {
	for (int i = 0; list[i] != NULL; i++) {
		for (int j = 0; list[i][j] != '\0'; j++) {
			if (list[i][j] == '>') {
				*rdct_pos = ++i;
				return 1; // change output
			}
			if (list[i][j] == '<') {
				*rdct_pos = ++i;
				return 0; //change input
			}
		}
	}
	return -1; //no changes
}

char **listcut(char **list) {
	char **tmp = list;
	for (int i = 0; list[i] != NULL; i++) {
		for (int j = 0; list[i][j] != '\0'; j++) {
			if (list[i][j] == '>' || list[i][j] == '<') {
				for (int k = i; list[k] != NULL; k++) {
					free(list[k]);
				}
				list[i] = NULL;
				return tmp;
			}
		}
	}
	return tmp;
}

void redirect(char **list, int direction, int pos) {

	int fd;
	if (direction == 1) {
		fd = open(list[pos], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
	}
	if (direction == 0) {
		fd = open(list[pos], O_RDONLY|O_CREAT, S_IRUSR|S_IWUSR);
	}
	list = listcut(list);
	if (fork() == 0) {
		dup2(fd, direction);
		if(execvp(list[0], list) < 0) {
			perror("exec failed");
			return;
		}
	}
	wait(NULL);
	close(fd);
}

int start_shell() {
	int direction, redirect_position;
	char **list = NULL;
	printf("\e[1;36m%s\033[0m", "> ");
	list = get_list();
	while (strcmp(list[0], "quit") && strcmp(list[0], "exit")) {
		direction = check_for_inp_outp(list, &redirect_position);
		if (direction >= 0) { //change output
			redirect(list, direction, redirect_position);
		}
		else {
			if (fork() == 0) {
				if (execvp(list[0], list) < 0) {
					perror("exec failed: ");
					return 1;
				}
			}	
			wait(NULL);
		}
		memfree(list);
		printf("\e[1;36m%s\033[0m", "> ");
		list = get_list();
	}
	memfree(list);
	return 0;
}


int main(int argc, char **argv) {
	int shell_status = start_shell();
	if (shell_status == 0) {
		puts("\e[0;32mshell finished successfully, congrats!\033[0m");
	}
	else {
		puts("\e[0;31mshell failed :( \033[0m");
	}
	return 0;
}
