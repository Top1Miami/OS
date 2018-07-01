#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define DELIMS " \t\n\a\r"
#define STANDARD_BF_SIZE 64

char ** splitter(char * inp) {
	int buffer_s = STANDARD_BF_SIZE;
	int pos = 0;
	char ** tokens = malloc(buffer_s * sizeof(char *));
	char * token;

	token = strtok(inp, DELIMS);
	while(token) {
		tokens[pos] = token;
		pos++;
		if(pos >= buffer_s) {
			buffer_s += STANDARD_BF_SIZE;
			tokens = realloc(tokens, buffer_s * sizeof(char *));
			if(!tokens) {
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, DELIMS);
	}
	tokens[pos] = NULL;
	return tokens;
}

int launcher(char ** arg) {
	extern char ** environ;
	pid_t pid;
	int status;
	if(arg[0] == NULL) {
		return 1;
	}
	pid = fork();
	if(pid == 0) {
		if(execve(arg[0],arg, environ) == -1) {
			printf("Error occured %s\n", strerror(errno));
		}
		free(arg);
		exit(EXIT_FAILURE);
	} else if(pid < 0) {
		printf("\n Failed to process \n");
	} else {
		do {
			if(wait(&status) == -1) {
				printf("Error in wait occured %s\n", strerror(errno));
			}
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	printf("Exit code: %d \n", status); 
	return 1;
}




int main(int argc, char ** argv) {
	char * inp;
	char ** tokens;
	int status = 1;
	while(status) {
		size_t buffer_s = 0;
		if(getline(&inp, &buffer_s, stdin) == -1) {
			free(inp);
			if(errno == 0)  {
				exit(EXIT_FAILURE);
			}
		} else {
			tokens = splitter(inp);
			if(tokens[0] != NULL && strcmp("exit", tokens[0]) != 0) {
				status = launcher(tokens);
			} else if(tokens[0] != NULL){
				status = 0;
			}
		}
		free(inp);
		free(tokens);
	}
	return 0;
}