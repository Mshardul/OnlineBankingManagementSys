#ifndef helper_H
#define helper_H

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define file "accounts.txt"
#define admin_username "1234567890"
#define admin_pass "!@#$%^&*()"

#define inputLength 10

struct account
{
	int accountNo;
	char username[10];
	char password[10];
	char balance[10];
}acc;

void printAcc(){
	printf("%s, %s, %s\n", acc.username, acc.password, acc.balance);
}

int GetAccInfo(int id){
	char info[50], str[10];
	int fd = open(file, O_RDONLY);
	int i=0, rd, j=0, bal;
	char ch;
	char buff[100];

	while(1){
		if((rd = read(fd, &ch, 1))<1)
			break;
		else if(ch=='\n'){
			buff[i]='\0';
			j+=1;
			if(j==id){
				strcpy(acc.username, strtok(buff,","));
				strcpy(acc.password, strtok(NULL,","));
				// strcpy(str, strtok(NULL,","));// sscanf(str, "%d", &bal);// acc.balance = bal;
				strcpy(acc.balance, strtok(NULL,","));
				// printAcc();
				return 1;
			}
			i=0;
		}
		else
			buff[i++] = ch;
	}
	close(fd);
	return 0;
}

void CreateFile(){
	open(file, O_CREAT|O_RDWR, 0777);
}

int Search(char * user){ //sends line number if account found
	// printf("Searching %s\n", user);
	int fd = open(file, O_RDWR);
	int i=0, j=0;
	char ch;
	char buff[100];
	while(1){
		int rd = read(fd, &ch, 1);
		if(rd<=0)
			break;
		else if(ch=='\n'){
			j+=1;
			buff[i]='\0';
			char *uname = strtok(buff, ",");
			if(!strcmp(uname, user))
				return j;
			i=0;
		}
		else{
			buff[i++] = ch;
		}
	}
	return 0;
}

int addUser(char * user, char * pass){
	char * response;
	char wrbuffer[200], rdbuffer[200];
	
	if(Search(user)){
		response = "Account already exists.";
		return 0;
	}

	int fd = open(file, O_RDWR|O_APPEND);

	sprintf(wrbuffer, "%s,%s,%d,\n", user, pass, 1000);
	write(fd, wrbuffer, strlen(wrbuffer));
	close(fd);

	return 1;
}

int VerifyUser(char * user, char *pass){ //returns id of user in the file(ie lining number(starting from 0))
	int id = Search(user);
	printf("id is :%d\n", id);

	if(!GetAccInfo(id)){
		printf("Account not found\n");
	}
	if(!(strcmp(acc.password,pass)))
		return 1;
	return 0;
}

int VerifyAdmin(char * user, char * pass){
	if(!(strcmp(user, admin_username)) && !(strcmp(pass, admin_pass))){
		return 1;
	}
	return 0;
}

void GetString(int ind, char * replace){ //to replace content on the file, eg update
	char buff[30], ch;
	int i=0, j;
	if(ind==1){
		while((ch=acc.password[i])!='\0')
			buff[i] = ch;
		buff[i]=';';
	}
	j=i;
	i=0;
	while((ch=acc.balance[i++])!='\0')
		buff[j+i]= ch;
	buff[j+i]=';';
	buff[j+i+1]='\n';
	strcpy(replace, buff);
}

int UpdateAccInfo(int ind, int prev_len){
	printAcc();
	char replace[30];
	int fd = open(file, O_RDWR);
	char ch;
	int rd, j=0, curr_ind=0, id=acc.accountNo;
	while(j<id){
		if((rd = read(fd, &ch, 1))<1)
			return 0;
		else if(ch=='\n')
			j+=1;
	}
	printf("Got to corrct line: %d\n", j);
	while(1){
		if((rd = read(fd, &ch, 1))<1)
			return 0;
		else if(ch==','){
			curr_ind++;
			if(curr_ind==ind){
				printf("Got the correct ind: %d\n", curr_ind);
				if(ind==1){
					write(fd, acc.password, strlen(acc.password));
					write(fd,",",1);
				}
				write(fd, acc.balance, strlen(acc.balance));
				write(fd,",\n",2);
			}
		}
		else if(ch=='\n'){
			break;
		}
	}
	close(fd);
	return 1;
}

int DeleteAccount(int id){
	int fd = open(file, O_RDWR);
	int i=0, rd;
	char c;
	while(1){
		if((rd=read(fd, &c, 1))<0)
			break;
		else if(c=='\n'){

		}
	}
}
int Deposit(char * user, char * pass, int amt){ //replacing with amt, instead of adding
	int prev_len
	printf("In deposit\n");
	int balance = atoi(acc.balance);
	char bal[10];
	amt += balance;
	sprintf(bal, "%d", amt);
	printf("bal converted: %s\n", bal);
	if(!(VerifyUser(user, pass)))
		return 0;
	strcpy(acc.balance, bal);
	return UpdateAccInfo(2);
}

// void Login(char * user, char * pass){
// 	int verify;
// 	if((verify = VerifyUser(user, pass)))

// }
#endif