#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/types.h>

void help() {
    printf("Function \'find\' takes odd number of arguments:\n");
    printf("First argument is a path for search\n");
    printf("\'find\' can take several options:\n");
    printf("\t\'-name\' takes a string which is an absolute path to file\n");
    printf("\t\'-size\' takes a char :\n \t\t = for files with equal size\n\t\t + for files with larger size\n\t\t - for files with fewer size \n");
    printf("\t\'-inum\' takes a positive integer which is inode of file\n");
    printf("\t\'-nlink\' takes a positive integer which is number of hardlinks\n");
    printf("\t\'-exec\' takes a string which is an absolute path to executable file\n");
}

int launch(char * const * argv) {
	extern char ** environ;
	pid_t pid;
	int status;
	pid = fork();
	if(pid == 0) {
		if(execve(argv[0],argv, environ) == -1) {
			printf("Error occured %s\n", strerror(errno));
		}
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
	return 1;
}

typedef struct Convertable {
	bool is_conv;
	ino_t value;
} Convertable;

Convertable convert(char *st) {
	char *x;
	Convertable result;
	for (x = st ; *x ; x++) {
		if (!isdigit(*x)) {
			result.is_conv = false;
			result.value = 0;
			return result;
		}
	}
	result.is_conv = true;
	result.value = strtoul(st, 0L, 10);
	return result;
}

int parse_argv(int argc, char ** argv) {
	if(argc % 2 == 1) {
		help();
		return 0;
	}
	argv++;
	DIR *directory;
	struct dirent *dir;

	if ((directory = opendir(*argv)) == NULL) {
		printf("Failed to open directory%s\n", strerror(errno));
	}
	char directory_path[1024];
	
	if (strcmp(*argv, ".") != 0) {
		strcpy(directory_path, *argv);
		strcat(directory_path, "/");
	} else {
		directory_path[0] = '\0';
	}
	argv++;

	char *name = "";
	char * exec_file = "";
	int size = -1, nlink = -1;
	char sign;
	bool has_inode = false;
	ino_t inode;
	while (*argv) {
		if (strcmp(*argv, "-name") == 0) {
			argv++;
			name = *argv;	
		} else if(strcmp(*argv, "-size") == 0) {
			argv++;
			sign = **argv;
			char * pointer = *argv + 1;
			char * i;
			for(i = pointer; *i;i++) {
				if(!isdigit(*i)) {
					printf("Incorrect input for size\n");
				}
			}
			size = atoi(pointer);
		} else if(strcmp(*argv, "-inum") == 0) {
			has_inode = true;
			argv++;
			Convertable temp = convert(*argv);
			if(temp.is_conv) {
				inode = temp.value;
			} else {
				printf("Incorrect input for inode\n");
			}
		} else if(strcmp(*argv, "-nlinks") == 0) {
			argv++;
			Convertable temp = convert(*argv);
			if(temp.is_conv) {
				nlink = temp.value;
			} else {
				printf("Incorrect input for number of hardlinks\n");
			}
		} else if(strcmp(*argv, "-exec") == 0) {
			argv++;
			exec_file = *argv;
		}
		argv++;
	}
	//printf("input size%d\n", size);
	while ((dir = readdir(directory)) != NULL) {
		struct stat file_stat;
		char file_path[1024];
		strcpy(file_path, directory_path);
		strcat(file_path, dir->d_name);
		if (stat(file_path, &file_stat) == -1) {
			printf("Failed to get files stats%s\n", strerror(errno));
		}
		if (S_ISREG(file_stat.st_mode)) {
			if (strcmp(name, "") != 0 && strcmp(dir->d_name,name) != 0) {
				continue;
			}
			if (has_inode && inode != file_stat.st_ino) {
				continue;
			}
			if(nlink != -1 && nlink != file_stat.st_nlink) {
				continue;
			}

			if(size != -1) {
				if(sign == '=' && size != file_stat.st_size) {
					continue;
				}
				if(sign == '+' && size >= file_stat.st_size) {
					continue;
				}
				if(sign == '-' && size <= file_stat.st_size) {
					continue;
				}
			}

			if(strcmp(exec_file, "") != 0) {
				printf("%s %s\n", exec_file, file_path);
				char * const a[3] = {exec_file, file_path, (char*)0};
				launch(a);
			} else {
				printf("%s\n", file_path);	
			}	
		}
	}

	if (closedir(directory) == -1) {
		printf("Failed to close directory%s\n", strerror(errno));
	}
	return 1;
} 



int main(int argc, char ** argv) {
	parse_argv(argc, argv);
	return EXIT_SUCCESS;
}