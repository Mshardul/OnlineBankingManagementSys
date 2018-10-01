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
int GetSocket(); //binds socket to ip and port - create and bind
int AcceptFromSocket(int sock); //accepts connection from client - listen and connect
int ShowBankingMenu(int cSock, char * user, int accType); //shows menu for admin or user, depending upon 'accType'
int ShowAdminMenu(int cSock); //menu designed for admin
void ShowUserMenu(int cSock, char * user); //menu designed for user
void DieWithError(int cSock, char *err); //closes connection after showing 'err'

/*function sending to client*/
void SendAck(int cSock, int ack); //sends ack to client - closes connection in case of special integer value (-100, here)
void SendIntToClient(int cSock, int n); //send integer value to client - useful in (sending ack, len(string_to_Send))
int SendStringToClient(int cSock, char * msg);

/*function receiving from client*/
int GetBalFromClient(int cSock); //gets balance from client
int GetUserPass(int cSock, char * user, char * pass); //gets username and password form client
int GetUsername(int cSock, char * user); //get username from client
int GetPassword(int cSock, char * pass); //get password from client
int GetUser(int cSock, char * user, char * pass, int accType); //login admin or user depending upon 'accType'
int GetAccountType(int cSock); //get accType from client
int ReceiveIntFromClient(int cSock); //receive integer value from client - useful in (receiving bal, len(string_to_receive))
int ReceiveStringFromClient(int cSock, char * msg); //receive 'msg' from client

/*funcion definitions*/

int GetBalFromClient(int cSock){
	int x=SendStringToClient(cSock, "Balance: ");
	int bal=ReceiveIntFromClient(cSock);
	return bal;
}

void SendAck(int cSock, int ack){
	SendIntToClient(cSock, ack);
}

int ShowAdminMenu(int cSock){
	char user[11]={'\0'}, pass[11]={'\0'};
	int x, ch, ack=0;
	int bal=0;
	
	x=SendStringToClient(cSock, "Please choose correct input:\n1. Add\n2. Delete\n3. Modify\n4. Search\n5. ViewAll");
	ch=ReceiveIntFromClient(cSock);

	switch(ch){
		case 1: //Add User
			SendStringToClient(cSock, "AccType of User:\n1. Individual\n2. Joint");
			int accType = ReceiveIntFromClient(cSock);
			if(!(accType==1 || accType==2))
				DieWithError(cSock, "Wrong Input");
			else
				SendAck(cSock, 1);
			if(!(GetUserPass(cSock, user, pass))){
				SendStringToClient(cSock, "Invalid Credentials");
			}
			else{
				if((ack=AddUser(user, pass, accType)))
					SendStringToClient(cSock, "User Added Successfully");
				else
					SendStringToClient(cSock, "User addition failed. See manual for possible errors");
			}
			break;
		case 2: //Delete User
			x=GetUsername(cSock, user);
			if((ack=DeleteUser(user))!=0)
				x=SendStringToClient(cSock, "User Deleted Successfully");
			else
				x=SendStringToClient(cSock, "User deletion failed. See manual for possible errors");
			break;
		case 3: //Modify User Details
			x=GetUsername(cSock, user);
			SendStringToClient(cSock, "1.To change password\n2. To modify balance");
			x=ReceiveIntFromClient(cSock);

			if(x==1){
				ack=GetPassword(cSock,pass);
			}
			else if(x==2){
				bal=GetBalFromClient(cSock);
				ack=1;
			}
			else{
				DieWithError(cSock, "Wrong Input");
			}

			if(ack){
				ack=ModifyUser(user, pass, bal, x);
				SendAck(cSock, ack);
				if(ack)
					SendStringToClient(cSock, "Details Modidied successfully");
				else
					SendStringToClient(cSock, "Details modification faile. See manual for possible errors");
			}
			else
				DieWithError(cSock, "Wrong Input");
			break;
		case 4: //Search User
			x=GetUsername(cSock, user);
			id=SearchActiveUser(user);
			if(id==-1){
				ack=0;
				x=SendStringToClient(cSock, "User not found\n");
			}
			else
				x=SendStringToClient(cSock, "User has id: ");
			SendIntToClient(cSock, id);
			break;
		case 5: //View All Users
			x=TotalNoOfAcc();
			SendIntToClient(cSock, x);
			for (int i = 0; i < x; ++i)
				SendStringToClient(cSock, GetInfo(i));
			ack=1;
			break;
		default:
			DieWithError(cSock, "Wrong choice");
	}

	SendAck(cSock, ack);
	return ack;
}

void ShowUserMenu(int cSock, char * user){
	char pass[11]={'\0'};
	int x, ch, y, ack=0;
	x=SendStringToClient(cSock, "1. Deposit\n2. Withdraw\n3. Balance Enquiry\n4. Password Change\n5. View Details\n6. Exit");
	ch=ReceiveIntFromClient(cSock);

	switch(ch){
		case 1: //Deposit amount
			x=SendStringToClient(cSock, "Enter amount");
			x=ReceiveIntFromClient(cSock);
			if((ack=UpdateBal(user, x, 0))>0){
				ack=1;
				x=SendStringToClient(cSock, "Balance updated successfully");
			}
			else
				x=SendStringToClient(cSock, "Balance updation failed. See manual for possible errors");
			break;
		case 2: //Withdraw amount
			x=SendStringToClient(cSock, "Enter amount");
			x=ReceiveIntFromClient(cSock);
			if((ack=UpdateBal(user, x, 1))>0){
				ack=1;
				x=SendStringToClient(cSock, "Balance updated successfully");
			}
			else
				x=SendStringToClient(cSock, "Balance updation failed. See manual for possible errors");
			break;
		case 3: //Balance Enquiry
			if(!(x=GetBal(user))){
				y=SendStringToClient(cSock, "User doesnot exist\n");
			}

			else
				y=SendStringToClient(cSock, "The account balance is: ");
			SendIntToClient(cSock, x);
			ack=1;
			break;
		case 4: //Change Password
			x=SendStringToClient(cSock, "Enter new password");
			x=ReceiveStringFromClient(cSock, pass);
			if((ack=ChangePass(user, pass))>0){
				ack=1;
				x=SendStringToClient(cSock, "Password changed successfully");
			}
			else
				x=SendStringToClient(cSock, "Password change failed. See manual for possible errors");

			break;
		case 5: //View Details
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
	if(accType==3)
		ShowAdminMenu(cSock);
	else
		ShowUserMenu(cSock, user);
	return 1;
}

int GetUserPass(int cSock, char * user, char * pass){
	int x, y;
	x=GetUsername(cSock, user);
	y=GetPassword(cSock, pass);
	if(x&&y){
		SendAck(cSock, 1);
	}
	else
		DieWithError(cSock, "Invalid Input. See manual for possible errors");
	return x&&y;
}

int GetUsername(int cSock, char * user){
	int x, ack=0;

	x = SendStringToClient(cSock, "Username: ");
	x = ReceiveStringFromClient(cSock, user);

	if(strlen(user)<11)
		ack=1;
	return ack;
}

int GetPassword(int cSock, char * pass){
	int x, ack=0;

	x = SendStringToClient(cSock, "Password: ");
	x = ReceiveStringFromClient(cSock, pass);

	if(strlen(pass)<11)
		ack=1;

	return ack;
}

int GetUser(int cSock, char * user, char * pass, int accType){
	int admin = (accType==3)?1:0;
	int x, ack=0, id=-1;

	x=GetUserPass(cSock, user, pass);

	if(x){
		if(admin){
			if((id = AdminLogin(user, pass))==1) //{0,1}
				ack=1;
			}
		else{
			if((id = UserLogin(user, pass, accType))>-1) //record lock, here
				ack=1;
		}
	}

	if(ack)
		SendAck(cSock, ack);
	else
		DieWithError(cSock, "Invalid Credentials");

	return ack;
}

int GetAccountType(int cSock){
	int x = SendStringToClient(cSock, "Select type of account:\n1)Individual.\n2)Joint\n3)Admin");
	int ch = ReceiveIntFromClient(cSock);
	int ack=0;
	if(ch>=1 && ch<=3)
		SendAck(cSock, 1);
	else
		DieWithError(cSock, "Wrong input");

	return ch;
}

void DieWithError(int cSock, char *err){
	printf("s..dying with error: %s\n", err);
	SendAck(cSock, -100);
	int id=ReceiveIntFromClient(cSock);
	int x=SendStringToClient(cSock, err);
	UserLogout(id);
	close(cSock);
	exit(1);
}

void SendIntToClient(int cSock, int n){
	int toSend = htonl(n);
	int snd;
	if((snd = write(cSock, (char*)(&toSend), sizeof(n)))<0)
		DieWithError(cSock, "Error in sending int");
}

int ReceiveIntFromClient(int cSock){
	int n, out, rcv;
	if((rcv = read(cSock, (char*)&n, sizeof(n)))<0)
		DieWithError(cSock, "Error in receiving int");
	out = ntohl(n);
	return out;
}

int SendStringToClient(int cSock, char * msg){
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
	return cSock;
}

#endif