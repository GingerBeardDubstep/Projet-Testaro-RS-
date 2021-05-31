#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char** argv){
	if(argc != 2){
		printf("No argument\n");
		exit(1);
	}else{
		int pid = atoi(argv[1]);
		if(pid != 0){
			printf("On kill le process %d\n",pid);
			kill(pid,17);
		}else{
			printf("Not Integer\n");
			exit(1);
		}		
	}
}