#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

void help() {
	printf("This function xor 3 and 1\n");
}
int main(int argc, char ** argv) {
	int change_num;
	help();
	printf("Input number to xor with 3\n");
	scanf("%d", &change_num);
	unsigned char machine_code[] = {0x55,0x48,0x89,0xe5,0x89,0x7d,0xfc,0x8b,0x45,0xfc,0x83,0xf0,0x01,0x5d,0xc3};
	void * stored = mmap(NULL, sizeof(machine_code), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if(stored == MAP_FAILED) {
		printf("%s\n", strerror(errno));
	}
	machine_code[12] = change_num;
	printf("Here comes change of code : machine_code[12] = change_num;\n");
	memcpy(stored, machine_code, sizeof(machine_code));
	if(mprotect(stored, sizeof(machine_code), PROT_WRITE | PROT_EXEC) == -1) {
		fprintf(stderr, "%s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	int res;
	printf("Now function xores something with 3\n");
	res = ((int(*)(int))stored)(3);
	
	if(munmap(stored, sizeof(machine_code)) == -1) {
		printf("%s\n", strerror(errno));
	}
	printf("%d\n", res);
	return 0;
}