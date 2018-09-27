#ifndef client_H
#define client_H

#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>


void DieWithError(int sock, char *err){
	printf("%s\n%s\n", err, strerror(errno));
	close(sock);
	exit(1);
}

int connectToServerSocket(){
	int sock, sPort = 90190;
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

void SendIntToServer(int sock, int x){
	int snd;
	snd = send(sock, &x, sizeof(x), 0);
	printf("sent: %d, %d, %s\n", x, snd, strerror(errno));
}

int SendStringToServer(int sock, char * msg){
	int sendBytes;
	struct timeval tv;
	tv.tv_sec = 20;
	tv.tv_usec = 0;
	if(setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(tv)) < 0)
		DieWithError(sock, "Time out");
	if((sendBytes=send(sock, msg, strlen(msg), 0))<0){
		DieWithError(sock, "unable to send data to server");
	}
	return 1;
}

int ReceiveStringFromServer(int sock, char * msg){
	int recvBytes;
	struct timeval tv;
	tv.tv_sec=20;
	tv.tv_usec=0;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(tv)) < 0)
		DieWithError(sock, "Time out");
	if((recvBytes = recv(sock, msg, 200, 0))<0){
		DieWithError(sock, "unable to receive data from server");
	}
	return 1;
}

int ReceiveAck(int sock){
	int rcv, ack;
	rcv = recv(sock, &ack, 4, 0);
	printf("ACK => %d, %d, %s\n", ack, rcv, strerror(errno));
	if(ack==-100)
		DieWithError(sock, "Thank you for using services!!");
	return ack;
}

void VerifyUser(int sock){

	char msg[1000], username[100], password[100];
	int x;

	x = ReceiveStringFromServer(sock, msg);
	printf("%s\n", msg);
	scanf("%s", username);
	x = SendStringToServer(sock, username);

	x = ReceiveStringFromServer(sock, msg);
	printf("%s\n", msg);
	scanf("%s", password);
	x = SendStringToServer(sock, password);

	int ack = ReceiveAck(sock);
	x = ReceiveStringFromServer(sock, msg);

	if(!ack)
		DieWithError(sock, msg);
	else
		printf("%s\n", msg);
}

void Deposit(int sock){
	char msg[200];
	int x = ReceiveStringFromServer(sock, msg);
	printf("%s\n", msg);
}

void Withdraw(int sock){
	char msg[200];
	int x = ReceiveStringFromServer(sock, msg);
	printf("%s\n", msg);
}

void BalanceEnquiry(int sock){
	char msg[200];
	int x = ReceiveStringFromServer(sock, msg);
	printf("%s\n", msg);
}

void ChangePassword(int sock){
	char msg[200];
	int x = ReceiveStringFromServer(sock, msg);
	printf("%s\n", msg);
}

void ViewDetails(int sock){
	char msg[200];
	int x = ReceiveStringFromServer(sock, msg);
	printf("%s\n", msg);
}

void GetBankingMenu(int sock){
	printf("Getting banking menu\n");
	char msg[100] = {0}, msg2[100]={0};
	int ch, x;

	x = ReceiveStringFromServer(sock, msg);
	printf("****************\n");
	printf("\n%s\n", msg);
	scanf("%d", &ch);
	SendIntToServer(sock, ch);

	switch(ch){
		case 1:
			Deposit(sock);
			break;
		case 2:
			Withdraw(sock);
			break;
		case 3:
			BalanceEnquiry(sock);
			break;
		case 4:
			ChangePassword(sock);
			break;
		case 5:
			ViewDetails(sock);
			break;
	}

	int ack = ReceiveAck(sock);
	printf("received ack as => %d\n", ack);
	x = ReceiveStringFromServer(sock, msg2);
	printf("%d => Received msg as => %s\n", x, msg2);

	if(ack==1)
		GetBankingMenu(sock);
	else
		DieWithError(sock, msg2);
}

void UserType(int sock){
	printf("UserType\n");
	char msg[100]={'\0'};
	int x, ch;

	x = ReceiveStringFromServer(sock, msg);
	printf("%s\n", msg);

	scanf("%d", &ch);
	SendIntToServer(sock, ch);

	int ack = ReceiveAck(sock);
	if(!ack)
		DieWithError(sock, "Wrong input");
}

#endif