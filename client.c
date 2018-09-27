#include <stdio.h>
#include "client.h"

int main(int argc, char const *argv[])
{
	int sock;

	sock = connectToServerSocket();
	printf("Socket connected @ %d\n", sock);

	UserType(sock);
	VerifyUser(sock);
	printf("user verified");
	
	GetBankingMenu(sock);

	return 0;
}