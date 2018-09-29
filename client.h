#ifndef server_H
#define server_H

#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>

#define max 100
#define sPort 55555

/*function declaration*/
void DieWithError(int sock, char *err);
int ReceiveIntFromServer(int sock);
void SendIntToServer(int sock, int n);
int SendStringToServer(int sock, char * msg);
int ReceiveStringFromServer(int sock, char * msg);
int AccountType(int sock);
void SendPassword(int sock, char * pass);
void SendUsername(int sock, char * user);
void SendUserPass(int sock, char * user, char * pass);
int VerifyUser(int sock);
int AdminMenu(int sock);

/*function defination*/

void SendBalToServer(int sock){
	printf("c..sending bal to server\n");
	char msg[50];
	int bal;
	int x=ReceiveStringFromServer(sock, msg);
	printf("%s\n", msg);
	scanf("%d", &bal);
	SendIntToServer(sock, bal);
}

int ReceiveAck(int sock){
	int ack = ReceiveIntFromServer(sock);
	printf("c..receiving ack from server as %d\n", ack);
	if(ack==-100){
		DieWithError(sock, "Something went wrong!!");
	}
	return ack;
}

int UserMenu(int sock){
	printf("s..user menu\n");
	return 1;
}

int AdminMenu(int sock){
	printf("c..Admin Menu\n");
	char msg[100], user[11], pass[11];
	int ch, x, choice;
	x=ReceiveStringFromServer(sock, msg);
	printf("%s\n", msg);
	scanf("%d", &ch);
	SendIntToServer(sock, ch);
	switch(ch){
		case 1: //add user
			SendUserPass(sock, user, pass);
			break;
		case 2: //del user
			SendUsername(sock, user);
			break;
		case 3: //modify user
			printf("1.To change password\n2. To modify balance\n");
			scanf("%d", &choice);
			if(choice==1)
				SendPassword(sock, pass);
			else if(choice==2)
				SendBalToServer(sock);
			else
				x=ReceiveAck(sock);
			break;
		case 4:
			SendUsername(sock, user);
			break;
		default:
			x=ReceiveAck(sock);
	}

	int ack=ReceiveAck(sock);
	return ack;
}

int VerifyUser(int sock){
	printf("c..VerifyUser\n");
	char msg[100], user[11], pass[11];
	int x, ack;

	SendUserPass(sock, user, pass);

	ack = ReceiveAck(sock);

	x = ReceiveStringFromServer(sock, msg);

	printf("%s\n", msg);

	return 1;
}


void SendUserPass(int sock, char * user, char * pass){
	printf("c..SendUserPass\n");
	int x, y;
	char resp[100];
	SendUsername(sock, user);
	SendPassword(sock, pass);
	int ack=ReceiveAck(sock);
	x=ReceiveStringFromServer(sock, resp);
	printf("%s\n", resp);
}

void SendUsername(int sock, char * user){
	printf("c..SendUsername\n");
	int x, ack=0;
	char msg[50];

	x=ReceiveStringFromServer(sock, msg);
	printf("%s\n", msg);
	scanf("%s", user);
	SendStringToServer(sock, user);
}

void SendPassword(int sock, char * pass){
	printf("c..SendPassword\n");
	int x, ack=0;
	char msg[50];

	x=ReceiveStringFromServer(sock, msg);
	printf("%s\n", msg);
	scanf("%s", pass);
	SendStringToServer(sock, pass);
}

int AccountType(int sock){
	printf("c..AccountType\n");
	char msg[max];
	int ch;
	ReceiveStringFromServer(sock, msg);
	printf("\n%s\n", msg);
	scanf("%d", &ch);
	SendIntToServer(sock, ch);
	int ack = ReceiveIntFromServer(sock);
	if(!ack)
		DieWithError(sock, "Wrong Input");
	return ch;
}
void DieWithError(int sock, char *err){
	printf("c..DieWithError\n");
	printf("%s\n%s\n", err, strerror(errno));
	close(sock);
	exit(1);
}

int ReceiveIntFromServer(int sock){
	printf("c..ReceiveIntFromServer\n");
	int n, out, rcv;
	if((rcv = read(sock, (char*)&n, sizeof(n)))<0)
		DieWithError(sock, "Error in receiving ACK");
	out = ntohl(n);
	printf("received %d as %d: %s\n", out, n, strerror(errno));
	return out;
}

void SendIntToServer(int sock, int n){
	printf("c..SendIntToServer\n");
	int toSend = htonl(n);
	int snd;
	if((snd = write(sock, (char*)(&toSend), sizeof(n)))<0)
		DieWithError(sock, "Error in sending int");
	// snd = send(sock, &ack, sizeof(ack), 0);
	printf("int sent %d as %d: %s\n", n, toSend, strerror(errno));
}

int SendStringToServer(int sock, char * msg){
	printf("c..SendStringToServer\n");
	int len = strlen(msg);
	struct timeval tv;
	tv.tv_sec = 20;
	tv.tv_usec = 0;
	if(setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(tv)) < 0)
		DieWithError(sock, "Time out while sending");
	SendIntToServer(sock, len);
	write(sock, msg, len);
	return 1;
}

int ReceiveStringFromServer(int sock, char * msg){
	printf("c..ReceiveStringFromServer\n");
	struct timeval tv;
	tv.tv_sec=20;
	tv.tv_usec=0;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(tv)) < 0)
		DieWithError(sock, "Time out while receiving");
	int len = ReceiveIntFromServer(sock);
	read(sock, msg, len);
	return 1;
}

int connectToServerSocket(){
	int sock;
	struct sockaddr_in server;

	if((sock = socket(AF_INET, SOCK_STREAM, 0))<0)
		DieWithError(sock, "Cannot create socket");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1"); //Local Host. Change.
	server.sin_port = htons(sPort);

	if(connect(sock, (struct sockaddr *)&server, sizeof(struct sockaddr_in))<0)
		DieWithError(sock, "Cannot connect to socket");

	return sock;
}

#endif