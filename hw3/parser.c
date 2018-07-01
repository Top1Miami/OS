#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
int main(int argc, char ** argv) {
	FILE * f = fopen("disassemble.ds", "r+");
	char line[256];
	FILE * f1 = fopen("text.txt","w");
	while(fscanf(f, "%s", line) == 1) {
		if(strlen(line) == 2) {
			if((isdigit(line[0]) || (line[0] >= 'a' && line[0] <= 'f')) && (isdigit(line[1]) || (line[1] >= 'a' && line[1] <= 'f'))) {
				fprintf(f1, "0x%s,", line);
			}
		}
	}
}