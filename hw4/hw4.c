#include <stdio.h>
#include <dlfcn.h>
int plus(int a, int b);
int xor(int a, int b);

int mult(int a, int b);


int main(int argc, char * argv[]) {
	int a = 3;
	int b = 2;
	//
	void * library = dlopen("lib_dyn_rt.so", RTLD_LAZY);
	if(library == NULL) {
		fprintf(stderr, "Failed to get lib_dyn_rt.so\n");
	}
	int (*divide)(int, int) = dlsym(library, "div");
	if(divide == NULL) {
		fprintf(stderr, "Failed to get lib function %s\n", dlerror());
	}
	//
	printf("Static library functions\n");
	printf("%d + %d = %d", a , b, plus(a, b));
	printf("\n");
	printf("%d ^ %d = %d", a , b, xor(a, b));
	printf("\n");
	printf("Dynamic library functions\n");
	printf("%d * %d = %d", a, b, mult(a, b));
	printf("\n");
	printf("Dynamic library runtime connected functions\n");
	printf("%d / %d = %d", a, b, divide(a, b));
	printf("\n");
	if(dlclose(library) != 0) {
		fprintf(stderr,"Failed to close library\n");
	}

	return 0;
}