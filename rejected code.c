
/*server.c -  multiple credential verifications */
int GetUser(int sock, char * user, char * pass){
	int x, i=0;
	char * msg;
	int validUser;
	do{
		validUser = 1;

		msg = "Login Screen:\nUsername: ";
		x = SendStringToClient(sock, msg);
		x = ReceiveStringFromClient(sock, user);
		
		msg = "Password: ";
		x = SendStringToClient(sock, msg);
		x = ReceiveStringFromClient(sock, pass);

		printf("%s & %s\n", user, pass);

		if(!(VerifyUser(user, pass))){
			if(i!=2){
				printf("hey1");
				validUser = 0;
				msg = "Entered details are incorrect. Plz try again";
			}
			else{
				printf("hey2");
				validUser = -1;
				msg = "You have entered wrong credentials 3 times";
			}
		}
		else{
			printf("hey3");
			msg = "Login successful!!";
			i = -1;
		}
		printf("%s\n", msg);
		SendAck(sock, validUser);
		x = SendStringToClient(sock, msg);
		i+=1;
	}while(i<3 && (validUser!=1));
	return validUser;
}


/*client.c - multiple credential verifications*/
int VerifyUser(int sock){

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

	if(ack==-1){
		DieWithError(sock, msg);
		return 0;
	}
	else {
		printf("%s\n", msg);
		if(ack==0)
			return VerifyUser(sock);
		else
			return 1;
	}
}

/*server.c - banking options - multiple times*/ 
int ShowBankingMenu(int sock, char * user){
	char * menu = "1) Deposit.\n2) Withdraw.\n3) Balance Enquiry.\n4) Change Password.\n5) View Details.\n6) Exit.";
	int ack, exit = 0, x, ch, i=0;
	do{
		printf("menu => %s\n", menu);
		char * response;
		x = SendStringToClient(sock, menu);
		ch = ReceiveIntFromClient(sock);
		ack = BankingOptions(sock, user, ch); //1, 0, -100
		printf("%d\n", ack);
		SendAck(sock, ack);
		if(ack==-100){
			response = "Thanks for using.";
			KillSocketWithError(sock, response);
			return 0;
		}
		if(ack){
			i=-1;
			response = "Redirecting to banking menu..";
		}
		else if(i==2){
			response = "3 wrong inputs.";
			KillSocketWithError(sock, response);
			return 0;
		}
		else{
			response = "wrong input. try again";
		}
		SendStringToClient(sock, response);
		i+=1;
	}while(i<3);
	return 1;
}

/*client.c - banking options - multiple times*/
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
	if(ack==-100){
		DieWithError(sock, msg2);
	}
	else{ //banking menu in both cases - after some operation from menu, or wrong input
		printf("%s\n", msg2);
		GetBankingMenu(sock);
	}
}
