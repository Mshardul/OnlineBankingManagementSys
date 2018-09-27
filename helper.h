#ifndef helper_H
#define helper_H

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define file "accounts.txt"
#define users "users.txt"
#define admin_username "1234567890"
#define admin_pass "!@#$%^&*()"

#define inputLength 10
// #define toHex(a) "%x", a
// #define toNum(a) "%lld", a

int id;
int no_of_users;
int bal_int;

struct account
{
	char username[10];
	char password[10];
	long long int balance;
	// char balance[8]; //4*10^9
	char active;
	// int active;
};

struct account acc;

void printAcc(){
	printf("%s, %s, %lld\n", acc.username, acc.password, acc.balance);
}

void GetInfo(int id){
	char info[50], str[10];
	int fd = open(file, O_RDONLY);
	lseek(fd, id*sizeof(acc), SEEK_SET);
	read(fd, &acc, sizeof(acc));
	printAcc();
	close(fd);
}

void CreateFile(){
	int fd = open(file, O_CREAT|O_RDWR, 0777);
	printf("File Created : %s\n", strerror(errno));
	lseek(fd, 0L, SEEK_END);
	struct stat st;
	fstat(fd, &st);
	int sz = st.st_size;
	no_of_users = sz/sizeof(acc);
	close(fd);
}

int Search(char * user){
	struct account temp;
	int fd = open(file, O_RDONLY);
	for(int i=0; i<no_of_users; i++){
		printf("Searching: %d\n", i);
		lseek(fd, i*sizeof(temp), SEEK_SET);
		read(fd, &temp, sizeof(temp));
		printf("User %s at id %d\n", temp.username, i);
		if(!(strcmp(temp.username, user))){
			printf("Username %s already exists\n", user);
			acc = temp;
			return i;
		}
	}
	close(fd);
	return -1;
}

int AddUser(char * user, char * pass){
	int id=Search(user);
	int fd;
	printf("%s has id: %d\n", user, id);
	if(id>-1 && acc.active=='0'){ //only for disabled account
		fd = open(file, O_RDWR);
		printf("Reactivating user @ %d.\n", id);
		lseek(fd, id*(sizeof(acc)), SEEK_SET);
	}
	else if(id==-1) //for new user
		fd = open(file, O_WRONLY|O_APPEND);
	/*
	// lseek(fd, (input-1)*sizeof(acc), SEEK_SET);
	// read(fd, &acc, sizeof(acc));
	// lseek(fd, 0, SEEK_END); 
	// lseek(fd, 0, SEEK_SET);
	*/
	printf("Adding %s\n", user);
	strcpy(acc.username, user);
	strcpy(acc.password, pass);
	// sprintf(acc.balance, toHex(1000));
	acc.balance = 1000;
	acc.active = '1';
	int rt = write(fd, &acc, sizeof(acc));
	if(rt!=sizeof(struct account)){
		printf("Some error occured: %lu: %s\n", sizeof(struct account), strerror(errno));
	}
	close(fd);
	no_of_users++;
	if(errno)
		printf("%s", strerror(errno));
	return 1;
}

/*add :{1: 'deposit', 2: 'withdraw', -1: 'modify directly'}*/
int UpdateBal(char *user, int amt, int add){
	int id;
	if((id = Search(user))==-1)
		return 0;
	int fd = open(file, O_RDWR);
	lseek(fd, id*sizeof(acc), SEEK_SET);
	read(fd, &acc, sizeof(acc));
	lseek(fd, -1*sizeof(acc), SEEK_CUR);
	if(!add){
		if(amt<acc.balance){
			acc.balance-=amt;
		}
		else{
			return -1;
		}
	}
	else if(add==1)
		acc.balance+=amt;
	else if(add==-1)
		acc.balance=amt;
	int rt = write(fd, &acc, sizeof(acc));
	close(fd);
	return 1;
}

int ChangePass(char *user, char *pass){
	int id;
	if((id = Search(user))==-1)
		return 0;
	int fd = open(file, O_RDWR);
	lseek(fd, id*sizeof(acc), SEEK_SET);
	read(fd, &acc, sizeof(acc));
	lseek(fd, -1*sizeof(acc), SEEK_CUR);
	strcpy(acc.password, pass);
	int rt = write(fd, &acc, sizeof(acc));
	close(fd);
	return 1;
}

int DeleteUser(char * user){
	int id;
	if((id=Search(user))==-1)
		return 0;
	int fd = open(file, O_RDWR);
	lseek(fd, id*sizeof(acc), SEEK_SET);
	read(fd, &acc, sizeof(acc));
	lseek(fd, -1*sizeof(acc), SEEK_CUR);
	acc.active='0';
	int rt = write(fd, &acc, sizeof(acc));
	close(fd);
	no_of_users--;
	return 1;
}

int VerifyUser(char * user, char * pass){
	int id = Search(user);
	if(!(strcmp(acc.password, pass)) && (acc.active=='1'))
		return 1;
	return 0;
}

int VerifyAdmin(char * user, char * pass){
	if(!(strcmp(user, admin_username)) && !(strcmp(pass, admin_pass))){
		return 1;
	}
	return 0;
}

long long int GetBal(char * user){
	int id;
	if((id=Search(user))<0)
		return 0;
	GetInfo(id);
	return acc.balance;
}

int ModifyUser(char * user, char *pass, long long int bal, int ind){
	int retVal = 0;
	if(ind==1)
		retVal = ChangePass(user, pass);
	else if(ind==2)
		retVal = UpdateBal(user, bal, -1);
	else
		retVal = (ChangePass(user, pass) && UpdateBal(user, bal, -1));
	return retVal;
}

// char * GetAllAcc()
#endif