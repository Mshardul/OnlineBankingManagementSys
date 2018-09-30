/*
accType = {0: invalid input, 1: normal, 2: joint, 3: admin}
*/
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "server.h"
#include "helper.h"
#include <stdlib.h>

int main(int argc, char const *argv[])
{
	int sock = GetSocket();

	if(!sock)
		exit(0);

	printf("%s\n", strerror(errno));
	int exists, n;

	while(1){
		int cSock = AcceptFromSocket(sock);
		printf("%s\n", strerror(errno));

		printf("got Socket\n");
		pid_t pid = fork();
		printf("=====%s\n", strerror(errno));
		
		char username[100]={'\0'};
		char password[100]={'\0'};

		int accType;

		if(!pid){
			CreateFile();
			if(!(accType = GetAccountType(cSock)))
				break;
			printf("---------------\n");
			printf("adding user to %d\n", getpid());
			if(!(GetUser(cSock, username, password, accType)))
				break;
			printf("---------------\n");
			if(!(ShowBankingMenu(cSock, username, accType)))
				continue;
			printf("---------------\n");
			UserLogout(id);
		}
		else{
			close(cSock);
		}

	}
	return 0;
}