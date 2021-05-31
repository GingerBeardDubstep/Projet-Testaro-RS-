#ifndef TESTARO
#define TESTARO
#define BUFSIZE 512
#define DEBUG 0

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>


int isSpace(char* buffer);
int isComment(char* buffer);
int isCommand(char* buffer);
int isInput(char* buffer);
int isOutput(char* buffer);
int isCont(char* buffer);
int isP(char* buffer);
int isCd(char* buffer);
int isChangeTimer(char* buffer);
int isChangeOutput(char* buffer,char* command,char* input,char* output,int* tstACommand);
void executeCommand(char* comd,char* stdin,char* stdout,int lineNumber);
int isLastLine(char* buffer);
void time_handler(int sig);
void gestion_signaux(int sig);
char* sub2(char* buf);

#endif