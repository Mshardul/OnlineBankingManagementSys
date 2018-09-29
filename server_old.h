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

void SendAck(int sock, int ack){
	int snd;
	snd = send(sock, &ack, sizeof(ack), 0);
	printf("ACK => %d, %d, %s\n", ack, snd, strerror(errno));
}

int KillSocketWithError(int sock, char *err){
	printf("%s\n%s\n", err, strerror(errno));
	close(sock);
	return 0;
}

void DieWithError(int sock, char *err){
	printf("%s\n%s\n", err, strerror(errno));
	SendAck(sock, -100);
	close(sock);
	exit(1);
}

int GetSocket(){
	int sock, port = 90190;
	struct sockaddr_in server;

	if((sock = socket(AF_INET, SOCK_STREAM, 0))<0)
		DieWithError(sock, "Cannot create socket");

	server.sin_family = AF_INET; 
	server.sin_addr.s_addr = htonl(INADDR_ANY); 
	server.sin_port = htons(port); 

	if(bind(sock, (struct sockaddr *)&server, sizeof(server))<0)
		DieWithError(sock, "Cannot bind socket");

	return sock;
}

int AcceptFromSocket(int sock){
	struct sockaddr_in client;
	int cSock, sockaddr_size = sizeof(struct sockaddr_in);
	if(listen(sock, 5)<0)
		DieWithError(sock, "Cannot listen through socket");

	if((cSock = accept(sock, (struct sockaddr *)&client, (socklen_t*)(&sockaddr_size)))<0)
		DieWithError(sock, "Cannot connect through socket");

	return cSock;
}

int SendStringToClient(int sock, char * msg){
	int sendBytes, len = strlen(msg);
	struct timeval tv;
	tv.tv_sec = 20;
	tv.tv_usec = 0;
	if(setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(tv)) < 0)
		DieWithError(sock, "Time out while sending");
	sendBytes = send(sock, msg, 100, 0);
	if(sendBytes==-1){
		DieWithError(sock, "unable to send data to client");
	}
	return 1;
}

int ReceiveStringFromClient(int sock, char * msg){
	int recvBytes;
	struct timeval tv;
	tv.tv_sec=20;
	tv.tv_usec=0;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(tv)) < 0)
		DieWithError(sock, "Time out while receiving");
	recvBytes = recv(sock, msg, 100, 0);
	if(recvBytes==-1){
		DieWithError(sock, "unable to receive data from client");
	}
	return 1;
}

int ReceiveIntFromClient(int sock){
	int rcv, ch;
	rcv = recv(sock, &ch, 4, 0);
	printf("received: %d, %d, %s\n", ch, rcv, strerror(errno));
	return ch;
}

int Verification(char * username, char * password){
	return 1;
} //use helper function VerifyUser(char * user, char *pass)

int GetUser(int sock, char user[11], char pass[11]){
	int x, ack=0;
	char msg[50] = "Login Screen:\nUsername: ";
	x = SendStringToClient(sock, msg);
	x = ReceiveStringFromClient(sock, user);
	
	strcpy(msg, "Password: ");
	x = SendStringToClient(sock, msg);
	x = ReceiveStringFromClient(sock, pass);

	printf("%s & %s\n", user, pass);

	if((strlen(user)<11) && (strlen(pass)<11))
		ack = Verification(user, pass);
	SendAck(sock, ack);

	if(ack)
		strcpy(msg,"Login Successful");
	else
		strcpy(msg, "Wrong credentials");
	
	SendStringToClient(sock, msg);

	if(!(ack)){
		KillSocketWithError(sock, msg);
	}

	return ack;
}

void Deposit(int sock, char * username){
	char * msg;
	msg = "in Deposit()";
	SendStringToClient(sock, msg);
}

void Withdraw(int sock, char * username){
	char * msg;
	msg = "in Withdraw()";
	SendStringToClient(sock, msg);
}

void BalanceEnquiry(int sock, char * username){
	char * msg;
	msg = "in BalanceEnquiry()";
	SendStringToClient(sock, msg);
}

void ChangePassword(int sock, char * username){
	char * msg;
	msg = "in ChangePassword()";
	SendStringToClient(sock, msg);
}

void ViewDetails(int sock, char * username){
	char * msg;
	msg = "in ViewDetails()";
	SendStringToClient(sock, msg);
}

int BankingOptions(int sock, char * user, int ch){
	int ack = 1;
	switch(ch){
		case 1:
			Deposit(sock, user);
			break;
		case 2:
			Withdraw(sock, user);
			break;
		case 3:
			BalanceEnquiry(sock, user);
			break;
		case 4:
			ChangePassword(sock, user);
			break;
		case 5:
			ViewDetails(sock, user);
			break;
		case 6:
			ack = -100;
			break;
		default:
			ack = 0;
	}
	return ack;
}

int ShowBankingMenu(int sock, char * user){
	char menu[100];
	strcpy(menu, "1) Deposit.\n2) Withdraw.\n3) Balance Enquiry.\n4) Change Password.\n5) View Details.\n6) Exit.");

	printf("Showing bank menu\n");
	int ack, exit = 0, x, ch, i=0;
	char response[100];

	x = SendStringToClient(sock, menu);
	ch = ReceiveIntFromClient(sock);
	ack = BankingOptions(sock, user, ch); //1, 0, -100

	printf("%d\n", ack);
	SendAck(sock, ack);
	
	if(ack==1)
		strcmp(response, "Redirecting to banking menu..");
	else{
		if(!ack)
			strcmp(response, "Wrong input. You will be logged out..");
		else
			strcmp(response, "Thanks for using internet banking.");
		SendStringToClient(sock, response);
		DieWithError(sock, response);
		return 0;
	}

	SendStringToClient(sock, response);

	return ShowBankingMenu(sock, user);
}

int GetAccountType(int sock){
	char * menu = "1) Individual Account.\n2) Joint Account.\n3) Administration";

	printf("Asking for accType\n");

	int x, ch, ack = 0;
	x = SendStringToClient(sock, menu);
	ch = ReceiveIntFromClient(sock);
	if(ch>=1 && ch<=3)
		ack = 1;
	SendAck(sock, ack);
	return ack;
}
#endif