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
	int exists, n;

	while(1){
		int cSock = AcceptFromSocket(sock);
		pid_t pid = fork();
		
		char username[100]={'\0'};
		char password[100]={'\0'};

		int accType;

		if(!pid){
			CreateFile();
			if(!(accType = GetAccountType(cSock)))
				break;
			if(!(GetUser(cSock, username, password, accType)))
				break;
			if(!(ShowBankingMenu(cSock, username, accType)))
				continue;
			UserLogout(id);
		}
		else{
			close(cSock);
		}

	}
	return 0;
}