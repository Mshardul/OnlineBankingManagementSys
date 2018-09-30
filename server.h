#ifndef server_H
#define server_H

#include "helper.h"

#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>

#define port 55555
#define size_s 11
#define size_m 50
#define size_l 100


/*function declaration*/

int AcceptFromSocket(int sock);
int GetSocket();
int ReceiveStringFromClient(int cSock, char * msg);
int SendStringToClient(int cSock, char * msg);
int ReceiveIntFromClient(int cSock);
void SendIntToClient(int cSock, int n);
void DieWithError(int cSock, char *err);
int GetAccountType(int cSock);
int GetUser(int sock, char * user, char * pass, int accType);
int GetUserPass(int cSock, char * user, char * pass);
int GetPassword(int cSock, char * pass);
int GetUsername(int cSock, char * user);
int ShowBankingMenu(int cSock, char * user, int accType);
int ShowAdminMenu(int cSock);

/*funcion definations*/

int GetBalFromClient(cSock){
	printf("s..Getting balance from client\n");
	int x=SendStringToClient(cSock, "Balance: ");
	int bal=ReceiveIntFromClient(cSock);
	return bal;
}

void SendAck(int cSock, int ack){
	printf("s..Sending ack\n");
	SendIntToClient(cSock, ack);
}

int ShowAdminMenu(int cSock){
	printf("s..sending admin menu\n");
	char req[100], resp[100], user[11], pass[11];
	int x, ch, ack;
	int bal=0;
	
	strcpy(req, "Please choose correct input:\n1. Add\n2. Delete\n3. Modify\n4. Search\n5. ViewAll");
	x=SendStringToClient(cSock, req);
	ch=ReceiveIntFromClient(cSock);

	switch(ch){
		case 1:
			GetUserPass(cSock, user, pass);
			ack=AddUser(user, pass);
			break;
		case 2:
			GetUsername(cSock, user);
			ack=DeleteUser(user);
			break;
		case 3:
			GetUsername(cSock, user);
			x=ReceiveIntFromClient(cSock);
			if(x==1)
				GetPassword(cSock,pass);
			else if(x==2)
				bal=GetBalFromClient(cSock);
			else
				DieWithError(cSock, "Wrong Input");
			ack=ModifyUser(user, pass, bal, x);
			break;
		case 4:
			GetUsername(cSock, user);
			if((ack=SearchUser(user))==-1)
				ack=0;
			break;
		case 5:
			x=TotalNoOfAcc();
			SendIntToClient(cSock, x);
			for (int i = 0; i < x; ++i)
				SendStringToClient(cSock, GetInfo(i));
			GetAllAcc();
			ack=1;
			break;
		// case 6:
		// 	x=ReceiveIntFromClient(cSock);
		// 	for (int i = 0; i < x; ++i)
		// 		SendStringToClient(cSock, GetTransactions(-1, i));
		default:
			DieWithError(cSock, "Wrong choice");
	}

	SendAck(cSock, ack);
	return ack;
}

void ShowUserMenu(int cSock, char * user){
	printf("s..showing user menu\n");
	char req[50], pass[11];
	int x, ch, ack=0;
	// strcpy(req, "1. Deposit\n2. Withdraw\n3. Balance Enquiry\n4. Password Change\n5. View Details\n6. Exit");
	x=SendStringToClient(cSock, "1. Deposit\n2. Withdraw\n3. Balance Enquiry\n4. Password Change\n5. View Details\n6. Exit");
	ch=ReceiveIntFromClient(cSock);

	switch(ch){
		case 1:
			x=ReceiveIntFromClient(cSock);
			if((ack=UpdateBal(user, x, 0))>0)
				ack=1;
			break;
		case 2:
			x=ReceiveIntFromClient(cSock);
			if((ack=UpdateBal(user, x, 1))>0)
				ack=1;
			break;
		case 3:
			x=GetBal(user);
			SendIntToClient(cSock, x);
			ack=1;
			break;
		case 4:
			x=ReceiveStringFromClient(cSock, pass);
			ack=ChangePass(user, pass);
			break;
		case 5:
			InitializeAcc(id);
			SendIntToClient(cSock, id);
			SendStringToClient(cSock, acc.username);
			SendStringToClient(cSock, acc.password);
			SendIntToClient(cSock, acc.balance);
			ack=1;
			break;
		case 6:
			DieWithError(cSock, "Thanks for using");
		default:
			DieWithError(cSock, "Wrong choice");
	}
	SendAck(cSock, ack);
}

int ShowBankingMenu(int cSock, char * user, int accType){
	printf("s..showing banking menu\n");
	if(accType==3)
		ShowAdminMenu(cSock);
	else
		ShowUserMenu(cSock, user);
	return 1;
}

int GetUserPass(int cSock, char * user, char * pass){
	printf("s..getting username and pass\n");
	int x, y;
	x=GetUsername(cSock, user);
	y=GetPassword(cSock, pass);
	printf("username and Password as %s and %s\n", user, pass);
	printf("x&y => %d, %d\n", x, y);
	if(x&&y){
		printf("X&&y is true\n");
		SendAck(cSock, 1);
		SendStringToClient(cSock, "Successful!!");
	}
	else{
		printf("x&&y is false\n");
		DieWithError(cSock, "Invalid input");
	}
	printf("Done with username and password\n");
	return x&&y;
}

int GetUsername(int cSock, char * user){
	printf("s..getting username\n");
	int x, ack=0;
	char msg[50];

	strcpy(msg, "Username: ");
	x = SendStringToClient(cSock, msg);
	x = ReceiveStringFromClient(cSock, user);

	if(strlen(user)<11)
		ack=1;
	return ack;
}

int GetPassword(int cSock, char * pass){
	printf("s..getting Password\n");
	int x, ack=0;
	char msg[50];

	strcpy(msg, "Password: ");
	x = SendStringToClient(cSock, msg);
	x = ReceiveStringFromClient(cSock, pass);

	if(strlen(pass)<11)
		ack=1;

	return ack;
}

int GetUser(int cSock, char * user, char * pass, int accType){
	int admin = (accType==3)?1:0;
	printf("s..getting username\n");
	int x, ack=0, id=-1;

	x=GetUserPass(cSock, user, pass);
	printf("%s && %s\n", user, pass);

	if(x){
		if(admin){
			if((id = AdminLogin(user, pass))==1) //{0,1}
				ack=1;
			}
		else{
			if((id = UserLogin(user, pass))>-1) //record lock, here
				ack=1;
		}
	}

	if(ack){
		SendAck(cSock, ack);
		SendStringToClient(cSock, "Successful!!");
	}
	else
		DieWithError(cSock, "Invalid Credentials");

	return ack;
}

int GetAccountType(int cSock){
	printf("s..getting user account type\n");
	char menu[size_l];
	strcpy(menu, "Select type of account:\n1)Individual.\n2)Joint\n3)Admin");
	int x = SendStringToClient(cSock, menu);
	int ch = ReceiveIntFromClient(cSock);
	int ack=0;
	if(ch>=1 && ch<=3)
		ack=1;
	else
		ch=0;
	SendIntToClient(cSock, ack);
	return ch;
}

void DieWithError(int cSock, char *err){
	printf("s..dying with error: %s: %s\n", err, strerror(errno));
	SendAck(cSock, -100);
	int id=ReceiveIntFromClient(cSock);
	UserLogout(id);
	close(cSock);
	exit(1);
}

void SendIntToClient(int cSock, int n){
	printf("s..sending int to client\n");
	int toSend = htonl(n);
	int snd;
	if((snd = write(cSock, (char*)(&toSend), sizeof(n)))<0)
		DieWithError(cSock, "Error in sending int");
	printf("int sent %d as %d: %s\n", n, toSend, strerror(errno));
}

int ReceiveIntFromClient(int cSock){
	printf("s..receiving int from client\n");
	int n, out, rcv;
	if((rcv = read(cSock, (char*)&n, sizeof(n)))<0)
		DieWithError(cSock, "Error in receiving int");
	out = ntohl(n);
	printf("received %d as %d: %s\n", out, n, strerror(errno));
	return out;
}

int SendStringToClient(int cSock, char * msg){
	printf("s..sending string to client\n");
	int len = strlen(msg);
	struct timeval tv;
	tv.tv_sec = 20;
	tv.tv_usec = 0;
	if(setsockopt(cSock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(tv)) < 0)
		DieWithError(cSock, "Time out while sending");
	SendIntToClient(cSock, len);
	write(cSock, msg, len);
	return 1;
}

int ReceiveStringFromClient(int cSock, char * msg){
	printf("s..receiving string from client\n");
	struct timeval tv;
	tv.tv_sec=20;
	tv.tv_usec=0;
	if(setsockopt(cSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(tv)) < 0)
		DieWithError(cSock, "Time out while receiving");
	int len = ReceiveIntFromClient(cSock);
	read(cSock, msg, len);
	return 1;
}

int GetSocket(){
	int sock;
	struct sockaddr_in server;
	if((sock = socket(AF_INET, SOCK_STREAM, 0))<0)
		DieWithError(sock, "Cannot create socket");
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);
	if(bind(sock, (void *)&server, sizeof(server))<0)
		DieWithError(sock, "Cannot bind socket");
	return sock;
}

int AcceptFromSocket(int sock){
	struct sockaddr_in client;
	int sockaddr_size = sizeof(client);
	int cSock;
	if(listen(sock, 5)<0)
		DieWithError(sock, "Cannot listen through socket");
	if((cSock = accept(sock, (void *)&client, (socklen_t*)(&sockaddr_size)))<0)
		DieWithError(sock, "Cannot connect through socket");
	printf("%s\n", strerror(errno));
	return cSock;
}

#endif