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

/*admin username and password*/
#define admin_username "12345678"
#define admin_pass "!@#$%^&*"


#define inputLength 10

int id;
int no_of_users;
int bal_int;
struct flock login_lock, func_lock;
int fd;

struct account
{
	char username[11];
	char password[11];
	int balance;
	char active; //{'1': active acc, '0': deleted acc}
	char normalAcc; //{'1': normal acc, '0': joint acc}
};

struct account acc;

int SetFuncLock(int id, int lock){
	lseek(fd, id*sizeof(acc), SEEK_SET);
	struct flock func_lock;
	if(!lock)
		func_lock.l_type = F_UNLCK;
	else if(lock==1)
		func_lock.l_type = F_RDLCK;
	else
		func_lock.l_type = F_WRLCK;
	func_lock.l_whence = SEEK_SET;
	func_lock.l_start = id*sizeof(acc);
	func_lock.l_len = sizeof(acc);
	func_lock.l_pid = getpid();
	fcntl(fd, F_SETLKW, &func_lock);
	printf("user with id = %d is locked as %d\n", id, lock);
	return 1;
}

int TotalNoOfAcc(){
	return no_of_users;
}
void printAcc(){
	printf("%s, %s, %d, %c\n", acc.username, acc.password, acc.balance, acc.active);
}

char * GetInfo(int id){
	char info[50], str[10];
	lseek(fd, id*sizeof(acc), SEEK_SET);
	SetFuncLock(id, 1);
	read(fd, &acc, sizeof(acc));
	SetFuncLock(id, 0);
	printAcc();
	return acc.username;
}

void CreateFile(){
	int sz;
	struct stat st;

	fd = open(file, O_CREAT|O_RDWR, 0777);
	printf("File Created : %s\n", strerror(errno));
	lseek(fd, 0L, SEEK_END);
	fstat(fd, &st);
	sz = st.st_size;
	no_of_users = sz/sizeof(acc);
	printf("%d\n", no_of_users);
}

int SearchUser(char * user){
	struct account temp;
	for(int id=0; id<no_of_users; id++){
		printf("Searching: %d\n", id);
		lseek(fd, id*sizeof(temp), SEEK_SET);
		SetFuncLock(id, 1);
		read(fd, &temp, sizeof(temp));
		SetFuncLock(id, 1);
		printf("User %s at id %d\n", temp.username, id);
		if(!(strcmp(temp.username, user))){
			printf("Username %s already exists\n", user);
			acc = temp;
			return id;
		}
	}
	return -1;
}

int SearchActiveUser(char * user){
	int id;
	if((id=SearchUser(user))>-1)
		if(acc.active=='1')
			return id;
	return -1;
}

void InitializeAcc(int id){
	lseek(fd, id*sizeof(acc), SEEK_SET);
	SetFuncLock(id, 1);
	read(fd, &acc, sizeof(acc));
	SetFuncLock(id, 0);
}

void GetAllAcc(){
	struct account temp;
	for(int id=0; id<no_of_users; id++){
		lseek(fd, id*sizeof(temp), SEEK_SET);
		SetFuncLock(id, 1);
		read(fd, &temp, sizeof(temp));
		SetFuncLock(id, 0);
		printf("%s, %s, %d, %c, %c\n", temp.username, temp.password, temp.balance, temp.active, temp.normalAcc);
	}
}

int AddUser(char * user, char * pass, int accType){
	printf("----->Adding user %s, %s, %d\n", user, pass, accType);
	int id=SearchUser(user);
	printf("----->user found at %d\n", id);
	int rt, tr=0;
	char norm=(accType==1)?'1':'0';
	printf("----->%s has id: %d\n", user, id);

	if(id>-1 && acc.active=='0'){ //only for disabled account
		printf("Reactivating user @ %d.\n", id);
		lseek(fd, id*(sizeof(acc)), SEEK_SET);
	}
	else if(id==-1){ //for new user
		printf("----->adding new user at %d\n", no_of_users);
		lseek(fd, no_of_users*(sizeof(acc)), SEEK_SET);
		no_of_users++;
	}
	printf("Adding %s\n", user);
	strcpy(acc.username, user);
	strcpy(acc.password, pass);
	acc.balance = 1000;
	acc.active = '1';
	acc.normalAcc = norm;
	SetFuncLock(id, 2);
	rt = write(fd, &acc, sizeof(acc));
	SetFuncLock(id, 0);	
	printf("----->%s added successfully\n", user);
	if(rt!=sizeof(struct account))
		printf("Some error occured: %lu: %s\n", sizeof(struct account), strerror(errno));

	if(errno)
		printf("%s", strerror(errno));
	return 1;
}

/*add :{1: 'deposit', 2: 'withdraw', -1: 'modify directly'}*/
int UpdateBal(char *user, int amt, int draw){
	int id;
	if((id = SearchUser(user))==-1)
		return 0;
	int rt;
	lseek(fd, id*sizeof(acc), SEEK_SET);
	SetFuncLock(id, 1);
	read(fd, &acc, sizeof(acc));
	SetFuncLock(id, 0);
	lseek(fd, -1*sizeof(acc), SEEK_CUR);
	if(!draw){
		if(acc.balance+amt>2147483647)
			amt=2147483647-acc.balance;
		acc.balance+=amt;
	}
	else if(draw==1){
		if(amt<acc.balance)
			acc.balance-=amt;
		else
			return -1;
	}
	else if(draw==2)
		acc.balance=amt;
	SetFuncLock(id, 2);
	rt = write(fd, &acc, sizeof(acc));
	SetFuncLock(id, 0);
	return 1;
}

int ChangePass(char *user, char *pass){
	int id;
	if((id = SearchUser(user))==-1)
		return 0;
	lseek(fd, id*sizeof(acc), SEEK_SET);
	SetFuncLock(id, 1);
	read(fd, &acc, sizeof(acc));
	SetFuncLock(id, 0);
	lseek(fd, -1*sizeof(acc), SEEK_CUR);
	strcpy(acc.password, pass);
	SetFuncLock(id, 2);
	int rt = write(fd, &acc, sizeof(acc));
	SetFuncLock(id, 0);
	return 1;
}

int DeleteUser(char * user){
	struct account temp;
	int id;
	if((id=SearchUser(user))==-1)
		return 0;
	lseek(fd, id*sizeof(temp), SEEK_SET);
	SetFuncLock(id, 1);
	read(fd, &temp, sizeof(temp));
	SetFuncLock(id, 1);
	lseek(fd, -1*sizeof(temp), SEEK_CUR);
	temp.active='0';
	SetFuncLock(id, 2);
	int rt = write(fd, &temp, sizeof(temp));
	SetFuncLock(id, 20);
	return 1;
}

int LockUser(int id){
	if(acc.normalAcc=='0')
		return 1;
	lseek(fd, id*sizeof(acc), SEEK_SET);
	struct flock login_lock;
	login_lock.l_type = F_WRLCK;
	login_lock.l_whence = SEEK_SET;
	login_lock.l_start = id*sizeof(acc);
	login_lock.l_len = sizeof(acc);
	login_lock.l_pid = getpid();
	fcntl(fd, F_SETLKW, &login_lock);
	printf("user with id = %d is locked\n", id);
	return 1;
}

int UserLogin(char * user, char * pass){
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
	login_lock.l_type = F_UNLCK;
	login_lock.l_whence = SEEK_SET;
	login_lock.l_start = id*sizeof(acc);
	login_lock.l_len = sizeof(acc);
	fcntl(fd, F_SETLKW, &login_lock);
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
	if(SearchActiveUser(user)==-1)
		return retVal;
	if(ind==1)
		retVal = ChangePass(user, pass);
	else if(ind==2)
		retVal = UpdateBal(user, bal, 2);
	else
		retVal = (ChangePass(user, pass) && UpdateBal(user, bal, 2));
	return retVal;
}

#endif