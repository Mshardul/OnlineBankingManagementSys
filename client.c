#include <stdio.h>
#include "client.h"

int main(int argc, char const *argv[])
{
	int sock, x;

	sock = connectToServerSocket();

	int accType = AccountType(sock);

	id = VerifyUser(sock);

	if(accType==3)
		x=AdminMenu(sock);
	else
		x=UserMenu(sock);

	return 0;
}