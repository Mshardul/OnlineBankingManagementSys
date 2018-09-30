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
#include <time.h>

#define file "accounts.txt"
#define transaction "transaction.txt"
#define admin_username "12345678"
#define admin_pass "!@#$%^&*"

#define inputLength 10

int id;
int no_of_users;
int no_of_transactions;
int bal_int;
struct flock lock;

struct account
{
	char username[11];
	char password[11];
	int balance;
	char active;
};

struct transact
{
	int year, month, date, hour, min;
	int ind;
	char username[11];
	int amt;
	int balance;
};

struct account acc;

int TotalNoOfAcc(){
	return no_of_users;
}
void printAcc(){
	printf("%s, %s, %d\n", acc.username, acc.password, acc.balance);
}

char * GetInfo(int id){
	char info[50], str[10];
	int fd = open(file, O_RDONLY);
	lseek(fd, id*sizeof(acc), SEEK_SET);
	read(fd, &acc, sizeof(acc));
	printAcc();
	close(fd);
	return acc.username;
}

void CreateFile(){
	int fd, sz;
	struct stat st;

	fd = open(file, O_CREAT|O_RDWR, 0777);
	printf("File Created : %s\n", strerror(errno));
	lseek(fd, 0L, SEEK_END);
	fstat(fd, &st);
	sz = st.st_size;
	no_of_users = sz/sizeof(acc);
	close(fd);

	struct transact trans;
	fd=open(transaction, O_CREAT|O_WRONLY, 0777);
	printf("File Created : %s\n", strerror(errno));
	lseek(fd, 0L, SEEK_END);
	fstat(fd, &st);
	sz=st.st_size;
	no_of_transactions = sz/sizeof(trans);
	close(fd);
}

int SearchUser(char * user){
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
			close(fd);
			return i;
		}
	}
	close(fd);
	return -1;
}

void InitializeAcc(int id){
	int fd=open(file, O_RDONLY);
	lseek(fd, id*sizeof(acc), SEEK_SET);
	read(fd, &acc, sizeof(acc));
	close(fd);
}

void GetAllAcc(){
	struct account temp;
	int fd = open(file, O_RDONLY);
	for(int i=0; i<no_of_users; i++){
		lseek(fd, i*sizeof(temp), SEEK_SET);
		read(fd, &temp, sizeof(temp));
		printf("%s%s%d%c\n", temp.username, temp.password, temp.balance,temp.active);
	}
	close(fd);
}

void GetAllTransact(){
	struct transact temp;
	int fd = open(file, O_RDONLY);
	for (int i = 0; i < no_of_transactions; ++i)
	{
		lseek(fd, i*sizeof(temp), SEEK_SET);
		read(fd, &temp, sizeof(temp));
		printf("%d-%d-%d %d:%d %d %s %d %d\n", temp.year, temp.month, temp.date, temp.hour, temp.min, temp.ind, temp.username, temp.amt, temp.balance);
	}
}
struct transact GetTransaction(int id, int lim){
	struct transact trans;
	int fd=open(transaction, O_RDONLY);
	int i=no_of_transactions;
	int num=0;
	while(i>0){
		lseek(fd, i*sizeof(trans), SEEK_SET);
		read(fd, &trans, sizeof(trans));
		if(id==-1)
			num+=1;
		else if(trans.ind==id)
			num+=1;
		if(lim==num)
			return trans;
	}
	struct transact temp;
	return temp;
}
int WriteTransactions(int ind, char * user, int amt, int bal){
	time_t t=time(NULL);
	struct tm tm=*localtime(&t);
	struct transact trans;
	trans.year=tm.tm_year;
	trans.month=tm.tm_mon;
	trans.date=tm.tm_mday;
	trans.hour=tm.tm_hour;
	trans.min=tm.tm_min;
	trans.ind=ind;
	strcpy(trans.username, user);
	trans.amt=amt;
	trans.balance=bal;

	int fd = open(transaction, O_WRONLY|O_APPEND);
	int rt = write(fd, &trans, sizeof(trans));
	close(fd);
	if(rt!=sizeof(struct transact))
		printf("Some error occured: %s\n", strerror(errno));
	return 1;
}
int AddUser(char * user, char * pass){
	int id=SearchUser(user);
	int fd, rt, tr=0;
	printf("%s has id: %d\n", user, id);

	if(id>-1 && acc.active=='0'){ //only for disabled account
		fd = open(file, O_RDWR);
		printf("Reactivating user @ %d.\n", id);
		lseek(fd, id*(sizeof(acc)), SEEK_SET);
		tr=1;
	}
	else if(id==-1){ //for new user
		fd = open(file, O_WRONLY|O_APPEND);
		tr=1;
	}
	printf("Adding %s\n", user);
	strcpy(acc.username, user);
	strcpy(acc.password, pass);
	acc.balance = 1000;
	acc.active = '1';
	rt = write(fd, &acc, sizeof(acc));
	if(rt!=sizeof(struct account))
		printf("Some error occured: %lu: %s\n", sizeof(struct account), strerror(errno));
	close(fd);
	if(tr)
		rt=WriteTransactions(0, user, id, 1000);

	no_of_users++;
	if(errno)
		printf("%s", strerror(errno));
	return 1;
}

/*add :{1: 'deposit', 2: 'withdraw', -1: 'modify directly'}*/
int UpdateBal(char *user, int amt, int draw){
	int id;
	if((id = SearchUser(user))==-1)
		return 0;
	int fd, rt;
	fd = open(file, O_RDWR);
	lseek(fd, id*sizeof(acc), SEEK_SET);
	read(fd, &acc, sizeof(acc));
	lseek(fd, -1*sizeof(acc), SEEK_CUR);
	if(!draw){
		if(acc.balance+amt>2147483647)
			amt=2147483647-acc.balance;
		acc.balance+=amt;
	}
	else if(draw==1){
		if(amt<acc.balance){
			acc.balance-=amt;
		}
		else{
			close(fd);
			return -1;
		}
	}
	else if(draw==2)
		acc.balance=amt;
	rt = write(fd, &acc, sizeof(acc));
	close(fd);

	rt = WriteTransactions(draw+1, user, amt, acc.balance);
	return 1;
}

int ChangePass(char *user, char *pass){
	int id;
	if((id = SearchUser(user))==-1)
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
	if((id=SearchUser(user))==-1)
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
	int id = SearchUser(user);
	if(!(strcmp(acc.password, pass)) && (acc.active=='1')){
		return 1;
	}
	return 0;
}

int LockUser(int id){
	int fd = open(file, O_RDONLY);
	lseek(fd, id*sizeof(acc), SEEK_SET);
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = id*sizeof(acc);
	lock.l_len = sizeof(acc);
	lock.l_pid = getpid();
	fcntl(fd, F_SETLKW, &lock);
	close(fd);
	return 1;
}

int UserLogin(char * user, char * pass){ //should replace VerifyUser(char * user, char * pass)
	int id;
	id = SearchUser(user);
	if(!(strcmp(acc.password, pass)) && (acc.active=='1')){
		LockUser(id);
		return id;
	}
	return -1;
}

void UserLogout(int id){
	InitializeAcc(id);
	int fd = open(file, O_RDWR);
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = id*sizeof(acc);
	lock.l_len = sizeof(acc);
	fcntl(fd, F_SETLK, &lock);
	close(fd);
}

int AdminLogin(char * user, char * pass){
	if(!(strcmp(user, admin_username)) && !(strcmp(pass, admin_pass))){
		return 1;
	}
	return 0;
}

int GetBal(char * user){
	int id;
	if((id=SearchUser(user))<0)
		return 0;
	InitializeAcc(id);
	return acc.balance;
}

int ModifyUser(char * user, char *pass, int bal, int ind){
	int retVal = 0;
	if(ind==1)
		retVal = ChangePass(user, pass);
	else if(ind==2)
		retVal = UpdateBal(user, bal, 2);
	else
		retVal = (ChangePass(user, pass) && UpdateBal(user, bal, 2));
	return retVal;
}

#endif