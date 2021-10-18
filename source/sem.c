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
	while (*end == ' ' && (ch == ' ' || ch == '\n' || ch == '\t')) {
		if (ch == '\n') {
			bytes = (n + 1) * sizeof(char);
			tmp = realloc(tmp, bytes);
			tmp[n] = '\0';
			*end = ch;
			return tmp;
		}
		else 
			ch = getchar();
	}
	while (ch == ' ' || ch == '\t' || ch == '\n') {
		if (ch == '\n') {
			printf("\e[1;36m%s\033[0m", "> ");
		}
		ch = getchar();
	}
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
	char word_last_char = '0';
	int bytes, n = 0;
	do {
		word = get_word(&word_last_char);
		if (word[0] != ' ' && word[0] != '\t' && word[0] != '\n' && word[0] != '\0'){
			bytes = (n + 1) * sizeof(char *);
			list = realloc(list, bytes);
			list[n] = word;
			n++;
		}
		else {
			free(word);
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

int check_for_input_output(char **list, int *redirect_pos) {
	for (int i = 0; list[i] != NULL; i++) {
		if (strcmp(list[i], "|") == 0) {
			*redirect_pos = ++i;
			return 2; // pipe
		}
	}
	for (int i = 0; list[i] != NULL; i++) {
		if (strcmp(list[i], ">") == 0) {
			*redirect_pos = ++i;
			return 1; // change output
		}
		if (strcmp(list[i], "<") == 0) {
			*redirect_pos = ++i;
			return 0; //change input
		}
	}
	return -1; //no changes
}

char **listcut(char **list, int rdr_pos) {
	for (int i = rdr_pos - 1; list[i] != NULL; i++) {
		free(list[i]);
	}
	list[rdr_pos - 1] = NULL;
	return list;
}

void redirect(char **list, int direction, int pos) {

	int fd;
	if (direction == 1) {
		fd = open(list[pos], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
	}
	if (direction == 0) {
		fd = open(list[pos], O_RDONLY, S_IRUSR);
	}
	list = listcut(list, pos);
	if (fork() == 0) {
		dup2(fd, direction);
		if(execvp(list[0], list) < 0) {
			memfree(list);
			perror("exec failed");
			return;
		}
	}
	wait(NULL);
	close(fd);
}

void redirect_for_pipe(char **list, int direction, int pos) {
	int fd;
	if (direction == 1) {
		fd = open(list[pos], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
	}
	if (direction == 0) {
		fd = open(list[pos], O_RDONLY, S_IRUSR);
	}
	list = listcut(list, pos);
	dup2(fd, direction);
	if (execvp(list[0], list) < 0) {
		memfree(list);
		perror("exec failed");
		return;
	}
}

// ls -l | wc
//создать массив для второй части после пайпа, затем снова запустить чек на редирект
//может, создать указатель на вторую часть списка, чтобы не выделять память

void call_conv(char **list, int redirect_pos) { //redirect_pos - позиция первого слова после символа '|'
	char **sec_part = NULL;
	int i, direction, direction_changer_position;
	sec_part = &list[redirect_pos];
	free(list[redirect_pos - 1]);
	list[redirect_pos - 1] = NULL;
	int fd[2];
	pipe(fd);
	if (fork() == 0) {
		dup2(fd[1], 1);
		close(fd[0]);
		close(fd[1]);
		direction = check_for_input_output(list, &direction_changer_position);
		if (direction >= 0) {
			redirect_for_pipe(list, direction, direction_changer_position);
		}
		else {
			if (execvp(list[0], list) < 0) {
				memfree(list);
				perror("exec failed successfully: ");
				return;

			}
		}
		return;
	}
	if (fork() == 0) {
		dup2(fd[0], 0);
		close(fd[0]);
		close(fd[1]);
		direction = check_for_input_output(sec_part, &direction_changer_position);
		if (direction >= 0) {
			redirect_for_pipe(sec_part, direction, direction_changer_position);
		}
		else {
			if (execvp(sec_part[0], sec_part) < 0) {
				memfree(list);
				perror("exec failed successfully: ");
				return;
			}
		}
		return;
	}
	//dup2(1, fd[1]);
	//dup2(0, fd[0]);
	close(fd[0]);
	close(fd[1]);
	wait(NULL);
	wait(NULL);
	dup2(1, fd[1]);
	dup2(0, fd[0]);
	for (i = redirect_pos; list[i] != NULL; i++) {
		free(list[i]);
	}
	free(list[i]);

}
//	ls -l | wc
//   (list) --> ls -l NULL (sec_part) --> wc NULL
//
int start_shell() {
	int direction, redirect_position;
	char **list = NULL;
	printf("\e[1;36m%s\033[0m", "> ");
	list = get_list();
	while (strcmp(list[0], "quit") && strcmp(list[0], "exit")) {
		direction = check_for_input_output(list, &redirect_position);
		if (direction == 2) {
			call_conv(list, redirect_position);
		}
		else if (direction >= 0) { //change output
			redirect(list, direction, redirect_position);
		}
		else {
			if (fork() == 0) {
				if (execvp(list[0], list) < 0) {
					memfree(list);
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
