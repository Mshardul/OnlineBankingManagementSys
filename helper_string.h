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
	long long int balance;
}acc;

void printAcc(){
	printf("%s, %s\n", acc.username, acc.password, acc.balance);
}

long long int ConvertToNum(char * x){
	char ch;
	int i;
	long long int res;
	while((ch=x[i++])!='\0'){
		res = (res*10)+(int)ch;
	}
	return res;
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
				return 1;
			}
			i=0;
		}
		else
			buff[i++] = ch;
	}
	close(fd);

	char * name = NULL;
	char str_id[100];
	sprintf(str_id, "%d", id);
	asprintf(&name, "%s_%s", name, str_id);

	char buff2[100];
	int fd2 = open(file, O_RDONLY);
	while(1){
		if((rd = read(fd, &ch, 1))<0)
			break;
		else if(ch=='\n'){
			buff2[i]='\0';
			i=0;
		}
		else{
			buff2[i++]=ch;
		}
	}
	acc.balance = ConvertToNum(buff2);
	printAcc();
	return 0;
}

void CreateFile(){
	open(file, O_CREAT|O_RDWR, 0777);
}

void CreateAccFile(int id){
	char * name = NULL;
	char str_id[100];
	sprintf(str_id, "%d.txt", id);
	asprintf(&name, "%s_%s", "acc", str_id);
	printf("%s file created", str_id);
	open(name, O_CREAT|O_RDWR, 0777);
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
			char *uname = strtok(buff, ",");//tab as delimeter
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

	int id = Search(user);
	printf("id of %s is %d", user, id);
	CreateAccFile(id);

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

int VerifyAdmin(char * user, char * pass){ //hard-coded username and password for admin
	if(!(strcmp(user, admin_username)) && !(strcmp(pass, admin_pass))){
		return 1;
	}
	return 0;
}

// int Transact(char * user, int amt, int add){

// }


#endif