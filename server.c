#include <stdio.h>
#include "server.h"
#include "helper.h"

int main(int argc, char const *argv[])
{
	int sock = getSocket();
	int exists, pid;

	while(1){
		int cSock = AcceptFromSocket(sock);

		printf("got Socket\n");
		pid = fork();
		
		char username[100]={'\0'};
		char password[100]={'\0'};

		int accType;

		if(!pid){
			if(!(accType = GetAccountType(cSock)))
				continue;

			if(!(GetUser(cSock, username, password)))
				continue;

			if(!(ShowBankingMenu(cSock, username)))
				continue;
		}
		else{
			close(cSock);
		}

	}
	return 0;
}