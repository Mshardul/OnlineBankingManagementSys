#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <sys/time.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "Server.h"


short CreateSocket(){
	short sock;
	printf("Creating the socket\n");
	sock = socket(AF_INET, SOCK_STREAM, 0); //socket(domain, type, protocol)
	return sock;
}

int BindSocket(short sock){
	int conn=-1;
	int port = 90190; //some random port
	struct sockaddr_in  comm={0};
	comm.sin_family = AF_INET; //internet address family
	comm.sin_addr.s_addr = htonl(INADDR_ANY); //local interface; change.
	comm.sin_port = htons(port); //local port
	conn = bind(sock,(struct sockaddr *)&comm,sizeof(comm));//bind(sockfd, struct sockaddr *addr, socklen_t addrlen);
	return conn;
}

void SendToClient(int sock, char * msg, short len){
	int snd;
	struct timeval tv; //timeout of 20 sec
	tv.tv_sec=20;
	tv.tv_usec=0;
	if(setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(tv)) < 0){
		printf("Time Out\n");
		return;
	}
	snd = send(sock , msg , len , 0);
	if(snd<0){
		printf("Some error occured while sending to server\n");
		close(sock);
	}
}

void RecvFromClient(int sock, char * msg, short len){
	int rcv;
	struct timeval tv; //timeout of 20 sec
	tv.tv_sec=20;
	tv.tv_usec=0;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(tv)) < 0){
		printf("Time Out\n");
		return;
	}
	rcv = recv(sock, msg, len, 0);
	if(rcv<0){
		printf("Some error occured while receiving from server\n");
		close(sock);
	}
}

int VerifyCredentials(char *username, char *password){
	return 1;
}

void ShowLoginMenu(int cSock, char *username, char *password){
	char *msg;
	int x;
	msg = "Welcome to the ABC Bank.\nPlease provide your login details.\nUsername: ";
	printf("%s => %lu\n", msg, strlen(msg));
	SendToClient(cSock, msg, strlen(msg));
	printf("Message sent. asking for credentials..\n");
	RecvFromClient(cSock, username, 20);
	msg = "\nPassword:\0";
	SendToClient(cSock, msg, strlen(msg));
	RecvFromClient(cSock, password, 20);
	printf("Username: %s\nPassword: %s\n", username, password);
}

void LoginOption(int cSock, char* user, char* pass){
	for(int i=0; i<3; i++){
		ShowLoginMenu(cSock, user, pass);
		printf("%s & %s\n", user, pass);
		if(VerifyCredentials(user, pass)){
			SendAck(cSock, 1);
			break;
		}
		else{
			if(i==2){
				SendAck(cSock, -1);
				close(cSock);
			}
			else
				SendAck(cSock, 0);
		}
	}
}

void Deposit(int cSock, char * username){
	char *msg;
	msg = "in Deposit";
	printf("%s", msg);
	SendToClient(cSock, msg, strlen(msg));
}
void Withdraw(int cSock, char * username){
	char *msg;
	msg = "in Withdraw";
	printf("%s", msg);
	SendToClient(cSock, msg, strlen(msg));
}
void BalEnquiry(int cSock, char * username){
	char *msg;
	msg = "in BalEnquiry";
	printf("%s", msg);
	SendToClient(cSock, msg, strlen(msg));
}
void ChangePass(int cSock, char * username){
	char *msg;
	msg = "in ChangePass";
	printf("%s", msg);
	SendToClient(cSock, msg, strlen(msg));
}
void ViewDet(int cSock, char * username){
	char *msg;
	msg = "in ViewDet";
	printf("%s", msg);
	SendToClient(cSock, msg, strlen(msg));
}
int ShowBankingOptions(int cSock, char *username){
	printf("Showing banking option\n");
	// char * msg = malloc(200*sizeof(char));
	char * msg;
	char ch[200];
	int i=0, correctInput;
	msg = "Press\n1 for Deposit.\n2 for Withdrawl.\n3 for Balance Inquiry.\n4 for password change.\n5 for View Details.\n6 for Exit.\n";

	do{
		correctInput=1;
		SendToClient(cSock, msg, strlen(msg));
		RecvFromClient(cSock, ch, 100);
		printf("%s\n", ch);
		if(!strcmp(ch, "1"))
			Deposit(cSock, username);
		else if(!strcmp(ch, "2"))
			Withdraw(cSock, username);
		else if(!strcmp(ch, "3"))
			BalEnquiry(cSock, username);
		else if(!strcmp(ch, "4"))
			ChangePass(cSock, username);
		else if(!strcmp(ch, "5"))
			ViewDet(cSock, username);
		else if(!strcmp(ch, "6"))
			return 0;
		else
			correctInput = 0;
		SendAck(cSock, correctInput);
		printf("Ack sent\n");
		i++;
		msg = "Wrong input. Please try again.";
		printf("%s\n", msg);
		printf("======%d %d\n", i, correctInput);
	}while(i<4 && !(correctInput));
	return correctInput;
}

void SendAck(int cSock, int ack){
	int converted_ack = htonl((uint32_t)ack);
	printf("Sending %d\n", ack);
	int snd = send(cSock , &converted_ack , sizeof(converted_ack), 0);
}

int main(int argc, char const *argv[])
{
	short sock;
	int cSock, cLen, x, len, ch, socketOpen = 1;
	struct sockaddr_in server, client;
	char msg[100];

	/*create socket*/
	sock = CreateSocket();
	if(sock==-1){
		printf("Socket not created properly. %s\n", strerror(errno));
		return 1;
	}
	else
		printf("Socket created.\n");

	/*bind the socket just created*/
	if(BindSocket(sock)<0){
		printf("Socket binding unsuccessful. %s\n", strerror(errno));
		return 1;
	}
	else
		printf("Socket binding successful.\n");

	while(1){
		char user[20]={"\0"};
		char pass[20]={"\0"};

		int loginSuccessful = 0;

		cLen = sizeof(struct sockaddr_in);
		if(listen(sock, 10) == -1){
			printf("Failed to listen\n");
			return 1;
		}
		
		/*accept the connection*/
		cSock = accept(sock, (struct sockaddr *)&client, (socklen_t*)&cLen);

		if(cSock<0){
			printf("Accept Failed. %s\n", strerror(errno));
			return 1;
		}
		else{
			printf("Connection accepted.\n");
		}

		LoginOption(cSock, user, pass);

		/*banking option*/ 
		int ci = ShowBankingOptions(cSock, user);
		printf("Done with banking options\n");

		
		close(cSock);
	}
	return 0;
}
