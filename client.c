#include <stdio.h>
#include "client.h"

int main(int argc, char const *argv[])
{
	int sock, x;

	sock = connectToServerSocket();
	printf("Socket connected @ %d\n", sock);

	int accType = AccountType(sock);
	printf("---------------\n");
	VerifyUser(sock);
	printf("---------------\n");
	if(accType==3)
		x=AdminMenu(sock);
	else
		x=UserMenu(sock);
	printf("---------------\n");
	return 0;
}