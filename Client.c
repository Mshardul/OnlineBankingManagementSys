#include <stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h> 
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "Client.h"

short CreateSocket(){
	short sock;
	printf("Creating the socket\n");
	sock = socket(AF_INET, SOCK_STREAM, 0); //socket(domain, type, protocol)
	return sock;
}

int ConnectToServer(int sock){
	int conn=-1;
	int sPort=90190;
	struct sockaddr_in comm={0}; //communication
	comm.sin_family = AF_INET;
	comm.sin_addr.s_addr = inet_addr("127.0.0.1"); //Local Host. Change.
	comm.sin_port = htons(sPort);
	conn = connect(sock , (struct sockaddr *)&comm , sizeof(struct sockaddr_in));
	return conn;
}

void SendToServer(int sock, char * msg, short len){
	int snd;
	struct timeval tv; //timeout of 20 sec
	tv.tv_sec=20;
	tv.tv_usec=0;
	if(setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(tv)) < 0){
		printf("Time Out\n");
		close(sock);
		return;
	}
	snd = send(sock , msg , len , 0);
	if(snd<0){
		printf("Some error occured while sending to server\n");
		close(sock);
	}
}

void RecvFromServer(int sock, char * msg, short len){
	int rcv;
	struct timeval tv; //timeout of 20 sec
	tv.tv_sec=20;
	tv.tv_usec=0;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(tv)) < 0){
		printf("Time Out\n");
		close(sock);
		return;
	}
	rcv = recv(sock, msg, len, 0);
	if(rcv<0){
		printf("Some error occured while receiving from server\n");
		close(sock);
	}
}

void VerifyCredentials(int sock){
	char msg[100], username[20], password[20];
	int x;
	RecvFromServer(sock, msg, 200);
	printf("%s", msg);
	scanf("%s", username);
	printf("%s => %lu\n", username, strlen(username));
	SendToServer(sock, username, strlen(username));
	RecvFromServer(sock, msg, 200);
	printf("%s", msg);
	scanf("%s", password);
	SendToServer(sock, password, strlen(password));
	printf("%s => %lu\n", password, strlen(password));

	x = ReceiveAck(sock);

	if(x==-1){
		printf("3 times wrong credential input\n");
		close(sock);
		exit(0);
	}

	if(x==0){
		VerifyCredentials(sock);
	}
}

void BankingOptions(int sock){
	char msg[200], ch[200], msg2[100];
	int choice;
	RecvFromServer(sock, msg, 200);
	printf("%s\n", msg);
	scanf("%s", ch);
	SendToServer(sock, ch, 100);
	// RecvFromServer(sock, msg2, 200);
	if(!ReceiveAck(sock))
		BankingOptions(sock);
}

int ReceiveAck(int sock){
	int ack = 0, x;
	int converted_ack;
	x = recv(sock, &converted_ack, sizeof(converted_ack), 0);
	ack = ntohl(converted_ack);
	printf("ACK ==> %d\n", ack);
	if (x < 0) {
		printf("Some error occured while receiving ack\n");
	}
	printf("\n");
	return ack;
}

int main(int argc, char const *argv[])
{
	int sock, len, x;
	// struct sockaddr_in server;
	char msg[100];
	/*create socket*/
	sock = CreateSocket();
	if(sock==-1){
		printf("Socket could not be created.\n");
		return 1;
	}
	else
		printf("Socket created\n");

	/*connect to server*/
	if(ConnectToServer(sock)<0){
		printf("Connection failed.\n");
		return 1;
	}
	else
		printf("Connection to server established\n");

	/*user verification*/
	VerifyCredentials(sock);

	/*banking options*/
	BankingOptions(sock);
	printf("Out of banking options.\n");

	close(sock);

	return 0;
}
