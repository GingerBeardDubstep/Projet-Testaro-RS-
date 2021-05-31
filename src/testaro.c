#include "testaro.h"
pid_t pid; //PID du fils
int lineNumber = 1; //Numero de ligne
int timer_amount = 5; //Timer
int pre = 1; // 1 si output avant commande, 0 si apres commande
int main(int argc,char** argv){
	char* file_name = malloc(sizeof(char)*128);
	if(argc != 2){
		free(file_name);
		printf("Vous n'avez pas entré le bon nombre d'arguments !\n%d entré mais 1 attendu\n",argc-1);
		if(DEBUG){
			printf("Code 1\n");
		}
		exit(1);
	}else{
		file_name = argv[1];
	}
	
	FILE* file = fopen(file_name,"r");
	
	if(file == NULL){
		printf("Problème de lecture : fichier \"%s\" introuvable\n",file_name);
		if(DEBUG){
			printf("Code 1\n");
		}
		exit(1);
	}	
	
	struct sigaction nvt,old;
	memset(&nvt, 0, sizeof(nvt));
	nvt.sa_handler = time_handler;
	sigaction(SIGALRM, &nvt,&old);
	
	char* buffer = malloc(sizeof(char)*256);
	size_t len = 256;
	int tstCont = 0;
	int tstAcont = 0;
	int tstACommand = 0;
	int tstLast = 0;
	int cmdIndex = 0;
	int inpIndex = 0;
	int outIndex = 0;
	int pIndex = 0;
	int state = -1; //-1 init/erreur, 0 commande, 1 stdin, 2 stdout
	char* toStdin = malloc(sizeof(char)*BUFSIZE);
	char* expectedStdout = malloc(sizeof(char)*BUFSIZE);
	char* command = malloc(sizeof(char)*BUFSIZE);
	char* currentBuffer = malloc(sizeof(char)*BUFSIZE);
	while(getline(&buffer,&len,file) != -1) {
		if(buffer != NULL && !isSpace(buffer) && !isComment(buffer) && !isChangeTimer(buffer) && !isChangeOutput(buffer,command,toStdin,expectedStdout,&tstACommand)){
			tstLast = isLastLine(buffer);
			tstCont = isCont(buffer);
			if((cmdIndex = isCommand(buffer)) != 0){
				state = 0;	
			}else if((inpIndex = isInput(buffer)) != 0){
				state = 1;
			}else if((outIndex = isOutput(buffer)) != 0){
				state = 2;
			}else if((pIndex = isP(buffer)) != 0){
				state = 3;	
			}
			if(!tstCont){
				// Si la ligne ne comporte pas le symbole '\'
				if(state == 0){
					//Si $
					if(!tstLast){
						buffer[strlen(buffer)-1] = '\0';
					}
					if(tstAcont){
						strcat(currentBuffer,buffer);
						tstAcont = 0;
					}else{
						strcpy(currentBuffer,&buffer[cmdIndex]);	
					}
					state = -1;
					if(pre){
						if(DEBUG){
							printf("Executer la commande : %s avec en entrée : %s\nOn attend en sortie : %s (taille %ld)\n",currentBuffer,toStdin,expectedStdout,strlen(expectedStdout));
						}
						executeCommand(currentBuffer,toStdin,expectedStdout,lineNumber);
						toStdin[0] = '\0';
						expectedStdout[0] = '\0';
					}else{
						if(tstACommand){
							if(DEBUG){
								printf("Executer la commande : %s avec en entrée : %s\nOn attend en sortie : %s (taille %ld)\n",command,toStdin,expectedStdout,strlen(expectedStdout));
							}
							
							executeCommand(command,toStdin,expectedStdout,lineNumber);
							toStdin[0] = '\0';
							expectedStdout[0] = '\0';
							strcpy(command,currentBuffer);
						}else{
							strcpy(command,currentBuffer);
							tstACommand = 1;
						}
					}
					
				}else if(state == 1){
					//Si <
					/*if(!tstLast){
						buffer[strlen(buffer)-1] = '\0';	
					}*/
					if(tstAcont){
						strcat(currentBuffer,buffer);
						tstAcont = 0;
					}else{
						strcpy(currentBuffer,&buffer[inpIndex]);	
					}
					state = -1;
					strcat(toStdin,currentBuffer); //Accumulateur <
					currentBuffer[0] = '\0';
				}else if(state == 2){
					//Si >
					/*if(!tstLast){
						buffer[strlen(buffer)-1] = '\0';
					}*/
					if(tstAcont){
						strcat(currentBuffer,buffer);
						tstAcont = 0;
					}else{
						strcpy(currentBuffer,&buffer[outIndex]);	
					}
					state = -1;
					strcat(expectedStdout,currentBuffer); //Accumulateur >
					currentBuffer[0] = '\0';
				}else if(state == 3){
					if(!tstLast){
						buffer[strlen(buffer)-1] = '\0';
					}
					if(tstAcont){
						strcat(currentBuffer,buffer);
						tstAcont = 0;
					}else{
						strcpy(currentBuffer,&buffer[pIndex]);	
					}
					state = -1;
					if(DEBUG){
						printf("Afficher %s\n",currentBuffer);	
					}
					printf("%s\n",currentBuffer);
					currentBuffer[0] = '\0';
				}else{
					//Non reconnu
					state = -1;
					printf("Non reconnu ligne %d : %s",lineNumber,buffer);
					if(DEBUG){
						printf("Code 1\n");
					}
					exit(1);
				}
			}else{
				//La ligne comporte le symbole '\'
				if(!tstAcont){
					switch(state){
						case 0 :
							strcpy(currentBuffer,&buffer[cmdIndex]);
							break;
						case 1 :
							strcpy(currentBuffer,&buffer[inpIndex]);
							break;
						case 2:
							strcpy(currentBuffer,&buffer[outIndex]);
							break;
						case 3 :
							strcpy(currentBuffer,&buffer[pIndex]);
							break;
						default :
							break;
					}
					tstAcont = 1;
				}else{
					strcat(currentBuffer,buffer);
				}
				tstCont=0;
			}
			lineNumber ++;
		}else{
			//Ligne vide ou commentaire
			lineNumber++;
		}
	}
	if(tstACommand){
		if(DEBUG){
			printf("Executer la commande : %s avec en entrée : %s\nOn attend en sortie : %s (taille %ld)\n",command,toStdin,expectedStdout,strlen(expectedStdout));
		}
		executeCommand(command,toStdin,expectedStdout,lineNumber);
	}
	fclose(file);
	exit(0);
}

int isSpace(char* buffer){
	int ret = 1;
	for(int i=0;i<strlen(buffer);i++){
		if(buffer[i] == '\n'){
			break;
		}else if(buffer[i] != ' ' && buffer[i] != '\t'){
			ret = 0;	
		}
	}
	return(ret);
}

int isLastLine(char* buffer){
	if(buffer[strlen(buffer)-1] != '\n'){
		return(1);
	}else{
		return(0);	
	}
}
	
int isComment(char* buffer){
	int tst = 0;
	for(int i=0;i<strlen(buffer);i++){
		if(buffer[i] != ' ' && buffer[i] != '\t'){
			if(buffer[i] == '#'){
				tst = 1;
				break;
			}else{
				tst = 0;
				break;
			}
		}
	}
	return(tst);
}

int isCommand(char* buffer){
	int tst = 0;
	for(int i=0;i<strlen(buffer);i++){
		if(buffer[i] != ' ' && buffer[i] != '\t'){
			if(buffer[i] == '$'){
				if(i+1 < strlen(buffer) && buffer[i+1] == ' '){
					tst = i + 2;	
				}else if(strlen(buffer) == 1 || (strlen(buffer)==2 && buffer[1] == ' ')){
					tst = 1;
				}
				break;
			}else{
				tst = 0;
				break;
			}
		}
	}
	return(tst);
}

int isInput(char* buffer){
	int tst = 0;
	for(int i=0;i<strlen(buffer);i++){
		if(buffer[i] != ' ' && buffer[i] != '\t'){
			if(buffer[i] == '<'){
				if(i+1<strlen(buffer) && buffer[i+1] == ' '){
					tst = i + 2;	
				}else if(strlen(buffer) == 1 || (strlen(buffer)==2 && buffer[1] == ' ')){	
					tst = 1;
				}
				break;
			}else{
				tst = 0;
				break;
			}
		}
	}
	return(tst);
}

int isChangeTimer(char* buffer){
	char* newBuff = NULL;
	newBuff = malloc(sizeof(char)*BUFSIZE);
	newBuff=strstr(buffer,"! set timout ");
	int tst = 0;
	/*if(!isLastLine(buffer)){
		newBuff[strlen(newBuff)-1] = '\0';
	}*/
	if(newBuff != NULL){
		int timer;
		tst = 1;
		char* argument = &buffer[13];
		if(argument[0] == '0'){
			timer_amount = 0;
			if(DEBUG){
				printf("Timer désactivé\n");
			}
		}else{
			timer = atoi(argument);
			if(timer == 0){
				printf("Mauvais argument pour le timer ligne %d : %s",lineNumber,argument);
				exit(1);
			}else{
				timer_amount = timer;
				if(DEBUG){
					printf("Timer mis à %ds\n",timer);
				}
			}
			
		}
	}
	return(tst);
}

int isCont(char* buffer){
	int tstCont = 0;
	for(int i=0;i<strlen(buffer);i++){
		if(buffer[i]=='\\'){
			if(i+1<strlen(buffer) && buffer[i+1] == '\\'){
					buffer[i] = '\0';
					strcat(buffer,&buffer[i+1]);
			}else{
				buffer[i] = '\0';
				tstCont = i+1;
				break;
			}
		}
	}
	return(tstCont);
}

int isChangeOutput(char* buffer,char* commande,char* input,char* output,int* tstAcommand){
	char* newBuffPre = NULL;
	newBuffPre = malloc(sizeof(char)*BUFSIZE);
	newBuffPre = strstr(buffer,"! set output pre");
	
	char* newBuffPost = NULL;
	newBuffPost = malloc(sizeof(char)*BUFSIZE);
	newBuffPost = strstr(buffer,"! set output post");
	
	int tst = 0;

	if(newBuffPre != NULL){
		tst = 1;
		if(!pre && *tstAcommand){
			*tstAcommand = 0;
			if(DEBUG){
				printf("Executer la commande : %s avec en entrée : %s\nOn attend en sortie : %s (taille %ld)\n",commande,input,output,strlen(output));
			}
			executeCommand(commande,input,output,lineNumber);
			input[0] = '\0';
			output[0] = '\0';
			commande[0] = '\0';
		}
		pre = 1;
		if(DEBUG){
			printf("Mode pré output activé\n");
		}
	}else if(newBuffPost != NULL){
		tst = 1;
		pre = 0;
		if(DEBUG){
			printf("Mode post output activé\n");
		}
	}else{
		tst = 0;
	}
	return(tst);
}

int isOutput(char* buffer){
	int tst = 0;
	for(int i=0;i<strlen(buffer);i++){
		if(buffer[i] != ' ' && buffer[i] != '\t'){
			if(buffer[i] == '>'){
				if(i + 1<strlen(buffer) && buffer[i + 1] == ' '){
					tst = i + 2;	
				}else if(strlen(buffer) == 1 || (strlen(buffer)==2 && buffer[1] == ' ')){
					tst = 1;
				}
				break;
			}else{
				tst = 0;
				break;
			}
		}
	}
	return(tst);
}

int isCd(char* buffer){
	int ret = 0;
	int tstC = 0;
	int tstD = 0;
	for(int i=0;i<strlen(buffer);i++){
		if(buffer[i] == ' '){
			if(tstC && tstD){
				ret = i;
				break;
			}
		}else if(buffer[i] == 'c'){
			tstC = 1;	
		}else if(buffer[i] == 'd' && tstC){
			tstD = 1;
		}else{
			return(0);	
		}
	}
	return(ret);
}

int isP(char* buffer){
	int tst = 0;
	for(int i=0;i<strlen(buffer);i++){
		if(buffer[i] != ' ' && buffer[i] != '\t'){
			if(buffer[i] == 'p'){
				if(i + 1 < strlen(buffer) && buffer[i+1] == ' '){
					tst = i + 2;
					break;
				}else if(strlen(buffer) == 1 || (strlen(buffer)==2 && buffer[1] == ' ')){
					tst = 1;
					break;
				}
			}else{
				tst = 0;
				break;
			}
		}
	}
	return(tst);
}

void executeCommand(char* comd,char* stdin,char* stdout,int lineNumber){
	int entree[2];
	int sortie[2];
	pipe(entree);
	pipe(sortie);
	char* output = malloc(sizeof(char)*512);
	int tstCd = isCd(comd);
	if(tstCd > 0){ // la commande est cd
		if(DEBUG){
			printf("cd %s\n",&comd[tstCd+1]);
		}
		if(chdir(&comd[tstCd+1]) != 0){ //On rentre dans la boucle si le cd echoue
			printf("Erreur no %d ligne %d\n",errno,lineNumber);
			if(DEBUG){
				printf("Code %d\n",errno + 40);
			}
			exit(errno + 40);
		}
	}else{
		pid = fork();
		if(pid == -1){
			printf("Erreur appel systeme fork ligne %d\n",lineNumber);
			exit(4);
		}else if(pid >0){
			if(DEBUG){
				printf("Exec PID : %d\n",pid);	
			}
			alarm(timer_amount); //On enclenche le timer
			if(strlen(stdin)>0){
				//Si stdin à mettre
				close(entree[0]);
				write(entree[1],sub2(stdin),strlen(sub2(stdin)));
				close(entree[1]);
				close(sortie[1]);
			}else{
				//Sans stdin
				close(entree[0]);
				close(entree[1]);
				close(sortie[1]);
			}
			
			int status;
			waitpid(-1,&status,0);
			alarm(0); //On desarme le timer une fois que le fils a terminé
			if(WIFSIGNALED(status)){
				//Execution terminée par un signal
				printf("Processus fils terminé par le signal %d ligne %d\n",WTERMSIG(status),lineNumber);
				if(DEBUG){
					printf("Code %d\n",WTERMSIG(status) + 4);	
				}
				exit(WTERMSIG(status) + 4);
			}
			if(WIFEXITED(status) != 0 && WEXITSTATUS(status) != 0){
				//Execution terminée avec code d'erreur
				printf("Erreur no %d ligne %d\n",WEXITSTATUS(status),lineNumber);
				if(DEBUG){
					printf("Code %d\n",WEXITSTATUS(status) + 40);	
				}
				exit(WEXITSTATUS(status) + 40);
			}
			
			int ret;
			do{
				ret = read(sortie[0],output,BUFSIZE);
				assert(ret != -1);
			}while(ret != 0);
			close(sortie[0]);
			
			if(strcmp(output,stdout)==0){
				if(DEBUG){
					printf("OK\n");
				}
				free(output);
			}else{
				printf("La commande %s (input = %s) ligne %d n'a pas retourné la valeur souhaitée :\nAttendue : %s,Obtenue : %s\n",comd,stdin,lineNumber,stdout,output);
				if(DEBUG){
					printf("Code 2\n");	
				}
				free(output);
				exit(2);
			}
		}else{
			struct sigaction nvt,old;
			memset(&nvt, 0, sizeof(nvt));
			nvt.sa_handler = SIG_DFL;
			sigaction(SIGINT, &nvt,&old);
			for(int i=1;i<65;i++){
				if(i != 9 && i != 19){
					struct sigaction old1;
					memset(&nvt, 0, sizeof(nvt));
					nvt.sa_handler = old.sa_handler;
					sigaction(i, &old,&old1);
				}
			}
			/*struct sigaction nvt,old;
			memset(&nvt, 0, sizeof(nvt));
			nvt.sa_handler = time_handler;
			sigaction(SIGCHLD, &nvt,&old);*/
			close(entree[1]);
			close(sortie[0]);
			dup2(entree[0],0); //On duplique l'entree du tube avec stdin
			close(entree[0]);
			dup2(sortie[1],1); // On duplique la sortie du tube avec stdout
			close(sortie[1]);
			free(output);
			//sleep(10);
			execlp("sh" ,"sh","-c",comd,NULL); //On execute la commande
		}
	}
}

void time_handler(int sig){
	printf("La commande ligne %d n'a pas répondu en moins de %ds\n",lineNumber,timer_amount);
	if(DEBUG){
		printf("Code 3\n");	
	}
	kill(pid,SIGKILL); // On kill le processus fils
	exit(3); //On retourne l'erreur 3
}

void gestion_signaux(int sig){
	if(DEBUG){
		printf("signal detecté\n");
	}
	exit(sig + 4 - 40);	
}

char* sub2(char* buf){
	char* res = malloc(sizeof(char)*512);
	for(int i=0;i<strlen(buf);i++){
		if(buf[i]!='\0'){
			res[i] = buf[i];
		}else{
			res[i] = '\0';
			break;
		}
	}
	return(res);
}