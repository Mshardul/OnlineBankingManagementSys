#ifndef server_H
#define server_H

/*header files*/
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>

/*macros*/
#define max 100
#define sPort 55555
#define sIP "127.0.0.1" //local-host. change it.

int id; //id of user in the accounts.txt(at server)

/*function declaration*/
int connectToServerSocket(); //connects socket to server
int UserMenu(int sock); // works on menu designed for user (both, normal and joint)
int AdminMenu(int sock); //works on menu designed for admin
int VerifyUser(int sock); //sends username and password to server, and gets acknowledgement(depending on whether credentials are correct)
void DieWithError(int sock, char *err); //closes connection after showing 'err'

/*functions sending to server*/
void SendBalToServer(int sock); //sends balance to server
void SendUserPass(int sock, char * user, char * pass); //sends username and password to server
void SendUsername(int sock, char * user); //sends username to server
void SendPassword(int sock, char * pass); //sends password to server
int AccountType(int sock); //sends account type to server - account type = {1: normal, 2: joint, 3: admin}
void SendIntToServer(int sock, int n); //sends integer value to server - useful in (sending balance, len(string_to_send))
int SendStringToServer(int sock, char * msg); //sends 'msg' to server - sends len(msg) first

/*functions receiving from server*/
int ReceiveAck(int sock); //receives ack, closes connection if special ack (-100, here) is received
int ReceiveIntFromServer(int sock); //receives integer value to server - useful in (receiving ack, len(string_to_receive))
int ReceiveStringFromServer(int sock, char * msg); //receives 'msg' form server - receives len(msg) first


/*function definition*/
void SendBalToServer(int sock){
	char msg[50]={'\0'};
	int bal;
	int x=ReceiveStringFromServer(sock, msg);
	printf("%s\n", msg);
	scanf("%d", &bal);
	SendIntToServer(sock, bal);
}

int ReceiveAck(int sock){
	int ack = ReceiveIntFromServer(sock);
	if(ack==-100){
		char msg[100];
		SendIntToServer(sock, id);
		int x=ReceiveStringFromServer(sock, msg);
		DieWithError(sock, msg);
	}
	return ack;
}

int UserMenu(int sock){
	char req[100]={'\0'}, pass[11]={'\0'}, user[11]={'\0'}, msg[100]={'\0'}, resp[100]={'\0'};
	int x, ch, amt;
	x=ReceiveStringFromServer(sock, req);
	printf("%s\n", req);
	scanf("%d", &ch);
	SendIntToServer(sock, ch);
	switch(ch){
		case 1: //Deposit Amount
			x=ReceiveStringFromServer(sock, msg);
			printf("%s\n", msg);
			scanf("%d", &amt);
			SendIntToServer(sock, amt);
			x=ReceiveStringFromServer(sock, resp);
			printf("%s\n", resp);
			break;
		case 2: //Withdraw amount
			x=ReceiveStringFromServer(sock, msg);
			printf("%s\n", msg);
			scanf("%d", &amt);
			SendIntToServer(sock, amt);
			x=ReceiveStringFromServer(sock, resp);
			printf("%s\n", resp);
			break;
		case 3: //Balance Enquiry
			x=ReceiveStringFromServer(sock, msg);
			printf("%s", msg);
			x=ReceiveIntFromServer(sock);
			if(x)
				printf("%d\n", x);
			break;
		case 4: //Change Password
			x=ReceiveStringFromServer(sock, msg);
			printf("%s\n", msg);
			scanf("%s", pass);
			x=SendStringToServer(sock, pass);
			x=ReceiveStringFromServer(sock,resp);
			printf("%s\n", resp);
			break;
		case 5: //View Details
			x=ReceiveIntFromServer(sock);
			printf("Id is %d\n", x);
			x=ReceiveStringFromServer(sock, user);
			printf("Username is %s\n", user);
			x=ReceiveStringFromServer(sock, pass);
			printf("Password is %s\n", pass);
			x=ReceiveIntFromServer(sock);
			printf("Balance is %d\n", x);
			break;
		case 6:
			x=ReceiveAck(sock);
			break;
		default:
			x=ReceiveAck(sock);
			break;
	}
	int ack=ReceiveAck(sock);
	printf("%s\n", strerror(errno));
	return ack;
}

int AdminMenu(int sock){
	char req[100]={'\0'}, user[11]={'\0'}, pass[11]={'\0'}, msg[100]={'\0'}, msg2[100]={'\0'};
	int ch, x, n, bal;
	x=ReceiveStringFromServer(sock, req);
	printf("%s\n", req);
	scanf("%d", &ch);
	SendIntToServer(sock, ch);
	switch(ch){
		case 1: //Add User
			x=ReceiveStringFromServer(sock, msg);
			printf("%s\n", msg);
			scanf("%d", &ch);
			SendIntToServer(sock, ch);
			x=ReceiveAck(sock);
			SendUserPass(sock, user, pass);
			ReceiveStringFromServer(sock, msg2);
			printf("%s\n", msg2);
			break;
		case 2: //Delete User
			SendUsername(sock, user);
			x=ReceiveStringFromServer(sock, msg);
			printf("%s\n", msg);
			break;
		case 3: //Modify User Details
			SendUsername(sock, user);
			x=ReceiveStringFromServer(sock, msg);
			printf("%s\n", msg);
			scanf("%d", &n);
			SendIntToServer(sock, n);
			if(n==1)
				SendPassword(sock, pass);
			else if(n==2)
				SendBalToServer(sock);
			x=ReceiveAck(sock);
			x=ReceiveStringFromServer(sock, msg2);
			printf("%s\n", msg2);
			break;
		case 4: //Search User
			SendUsername(sock, user);
			x=ReceiveStringFromServer(sock, msg);
			printf("%s", msg);
			id=ReceiveIntFromServer(sock);
			if(id!=-1)
				printf("%d\n", id);
			break;
		case 5: //View All Users
			n=ReceiveIntFromServer(sock);
			for (int i = 0; i < n; ++i)
			{
				char userDet[11];
				x=ReceiveStringFromServer(sock, userDet);
				printf("%s\n", userDet);
			}
			break;
		default:
			x=ReceiveAck(sock);
	}

	int ack=ReceiveAck(sock);
	return ack;
}

int VerifyUser(int sock){
	char msg[100]={'\0'}, user[11]={'\0'}, pass[11]={'\0'};
	int x;
	SendUserPass(sock, user, pass);
	x = ReceiveAck(sock);
	return 1;
}


void SendUserPass(int sock, char * user, char * pass){
	int x;
	char resp[100]={'\0'};
	SendUsername(sock, user);
	SendPassword(sock, pass);
	x=ReceiveAck(sock);
}

void SendUsername(int sock, char * user){
	int x;
	char msg[50]={'\0'};
	x=ReceiveStringFromServer(sock, msg);
	printf("%s\n", msg);
	scanf("%s", user);
	SendStringToServer(sock, user);
}

void SendPassword(int sock, char * pass){
	int x;
	char msg[50]={'\0'};
	x=ReceiveStringFromServer(sock, msg);
	printf("%s\n", msg);
	scanf("%s", pass);
	SendStringToServer(sock, pass);
}

int AccountType(int sock){
	char msg[max]={'\0'};
	int ch, ack;
	ReceiveStringFromServer(sock, msg);
	printf("\n%s\n", msg);
	scanf("%d", &ch);
	SendIntToServer(sock, ch);
	ack=ReceiveAck(sock);
	return ch;
}
void DieWithError(int sock, char *err){
	printf("%s\n", err);
	close(sock);
	exit(1);
}

int ReceiveIntFromServer(int sock){
	int n, out, rcv;
	if((rcv = read(sock, (char*)&n, sizeof(n)))<0)
		DieWithError(sock, "Error in receiving ACK");
	out = ntohl(n);
	return out;
}

void SendIntToServer(int sock, int n){
	int toSend = htonl(n);
	int snd;
	if((snd = write(sock, (char*)(&toSend), sizeof(n)))<0)
		DieWithError(sock, "Error in sending int");
}

int SendStringToServer(int sock, char * msg){
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
	server.sin_addr.s_addr = inet_addr(sIP);
	server.sin_port = htons(sPort);
	if(connect(sock, (struct sockaddr *)&server, sizeof(struct sockaddr_in))<0)
		DieWithError(sock, "Cannot connect to socket");
	return sock;
}

#endif