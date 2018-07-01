//#define __x86_64__
#define __USE_GNU
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <stdio.h>
#include <ucontext.h>
#include <sys/types.h>
#include <bits/sigcontext.h>
#include <features.h>
#include <stdbool.h>
#include <setjmp.h>
#include <sys/mman.h>
#include <errno.h>
void reverse(char s[])
 {
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }
void itoa(int n, char s[], bool offset)
 {
	int i, sign;

	if ((sign = n) < 0)  /* записываем знак */
	 n = -n;          /* делаем n положительным числом */
	i = 0;
	do {       /* генерируем цифры в обратном порядке */
	 s[i++] = n % 10 + '0';   /* берем следующую цифру */
	} while ((n /= 10) > 0);     /* удаляем */
	if (sign < 0)
	 s[i++] = '-';
	if(offset == true) {
		s[i++] = '\n';	
	}
	s[i] = '\0';
	reverse(s);
}
enum
{
  REG_R8 = 0,
# define REG_R8		REG_R8
  REG_R9,
# define REG_R9		REG_R9
  REG_R10,
# define REG_R10	REG_R10
  REG_R11,
# define REG_R11	REG_R11
  REG_R12,
# define REG_R12	REG_R12
  REG_R13,
# define REG_R13	REG_R13
  REG_R14,
# define REG_R14	REG_R14
  REG_R15,
# define REG_R15	REG_R15
  REG_RDI,
# define REG_RDI	REG_RDI
  REG_RSI,
# define REG_RSI	REG_RSI
  REG_RBP,
# define REG_RBP	REG_RBP
  REG_RBX,
# define REG_RBX	REG_RBX
  REG_RDX,
# define REG_RDX	REG_RDX
  REG_RAX,
# define REG_RAX	REG_RAX
  REG_RCX,
# define REG_RCX	REG_RCX
  REG_RSP,
# define REG_RSP	REG_RSP
  REG_RIP,
# define REG_RIP	REG_RIP
  REG_EFL,
# define REG_EFL	REG_EFL
  REG_CSGSFS,		/* Actually short cs, gs, fs, __pad0.  */
# define REG_CSGSFS	REG_CSGSFS
  REG_ERR,
# define REG_ERR	REG_ERR
  REG_TRAPNO,
# define REG_TRAPNO	REG_TRAPNO
  REG_OLDMASK,
# define REG_OLDMASK	REG_OLDMASK
  REG_CR2
# define REG_CR2	REG_CR2
};
char * letter[16] = {
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"a",
	"b",
	"c",
	"d",
	"e",
	"f"
};

void fall()
{
  char * s = "short_text";
  sprintf(s,"This is very long text");
}
void write_converted_hex(unsigned char c) {
	unsigned char first, second;
	first = c / 16;
	second = c % 16;
	write(1, letter[first], 1);
	write(1, letter[second], 1);
	write(1, " ", 1);
}

sigjmp_buf j_buffer;
void sig_handler_inside(int signum) {
	siglongjmp(j_buffer, 123);
}

void dump_memory(unsigned long long address, int length) {
	if(address == 0) {
		char * temp = "address of memory fault is 0";
		write(1, temp, strlen(temp));
		return;
	}
	struct sigaction new_action;
	memset(&new_action, 0, sizeof(new_action));
	new_action.sa_handler = sig_handler_inside;
	new_action.sa_flags = SA_NODEFER;
	if(sigaction(SIGSEGV, &new_action, NULL) == -1) {
		write(1, strerror(errno), strlen(strerror(errno)));
		exit(0);
	}
	int i;
    unsigned char buff[17];
    //unsigned char *pc = (unsigned char*)addr;

    // Process every byte in the data.
    for (i = 0; i < length; i++) {

		//write(1 , "pass", 4);
    	if(i % 16 == 0) {
    		char new_line[1024];
    		itoa(i / 16, new_line, false);
    		write(1, "index = ", 8);
    		write(1, new_line, strlen(new_line));
    		write(1, " ", 1);
    		write(1, "value = ", 8);
    	}
    	if(sigsetjmp(j_buffer, 0)) {
    		write(1, "xz ", 3);
    	} else {
    		unsigned char * pc = (unsigned char *) address;
        	write_converted_hex(pc[i]);
    	}

        if(i % 16 == 15 && i != 0) {
        	write(1, "\n", 1);
        }
    }
}
void sig_handler(int signum, siginfo_t * si, void * arg) {
	char * capture = "Capture signal fault\n\n";
	write(1, capture, strlen(capture));
	ucontext_t context, *cp = &context;
	getcontext(cp);
	
	unsigned long long address = (unsigned long long)si->si_addr;
	write(1, "Memory dump start \n", 19);
	dump_memory(address, 32);
	write(1, "Memory dump end \n", 17);
	write(1, "\n", 1);
	write(1, "Registres dump start \n", 22);
	for(int i = REG_R8;i != REG_CR2;i++) {
		char temp_string[1024];
		itoa(cp->uc_mcontext.gregs[i], temp_string, true);
		write(1, temp_string, strlen(temp_string));
	}
	write(1, "\nRegistres dump end \n", 21);
	exit(0);
}

int main(int argc, char * argv[]) {
	struct sigaction action;
	memset(&action, 0, sizeof(action));

	action.sa_sigaction = sig_handler;
	action.sa_flags = SA_SIGINFO | SA_NODEFER;
	
	if(sigaction(SIGSEGV, &action, NULL) == -1) {
		write(1, strerror(errno), strlen(strerror(errno)));
		exit(0);
	} 
	//fall();
	char * mem = (mmap(NULL, 4096, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1,0));
	mem[0] = 'x';
	//int * x;
	//int y = *x;
	//int * x = NULL;
	//*x = 123;
	return 0;
}