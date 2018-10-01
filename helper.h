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

/*global variables*/
int id; //used to store the AccNo of current user
int no_of_users; //total no of users in the file
struct flock login_lock; //used for normal acc - lock on login
struct flock func_lock; //used for joint acc - lock on particular action
int fd; //file descriptor

struct account //holds user info - so that, can be accessed anywhere
{
	char username[11];
	char password[11];
	int balance;
	char active; //{'1': active acc, '0': deleted acc} - deleted acc doesnt actually gets deleted
	char normalAcc; //{'1': normal acc, '0': joint acc}
}acc;

/*function declaration*/
int SetFuncLock(int id, int lock); //locks and unlocks func_lock - joint acc
void CreateFile(); //creates (and opens) file
int SearchUser(char * user); //returns id of user
int SearchActiveUser(char * user); //returns id only if user is active
void InitializeAcc(int id); //gets value of current user from the file
int AddUser(char * user, char * pass, int accType); //adds user to file, simultaneously storing values in 'acc'
int UpdateBal(char *user, int amt, int draw); //updates balance draw={0: deposit, 1: withdraw, 2: modify directly}
int ChangePass(char *user, char *pass); //updates password - no need to know the original password
int DeleteUser(char * user); //daletes user - sets 'active' variable of structure to 0 (actually, not deleted) - no need to know the original password
int LockUser(int id); //for normal user - write lock on login_lock
int UserLogin(char * user, char * pass, int accType); //log-in user
void UserLogout(int id); //for normal user - unlocks login_lock
int AdminLogin(char * user, char * pass); //log-in admin - username and password are hard-coded
int GetBal(char * user); //returns current balance of given user
int ModifyUser(char * user, char *pass, int bal, int ind); //for modify functionality to admin - ind={1: change password, 2: update balance, 3: change both(not functional yet)}

/*functions - (just for testing)*/
int TotalNoOfAcc(); //returns total no of users
void printAcc(); //prints details of current user
char * GetInfo(int id); //reads details of current user from file
void GetAllAcc(); //prints details of all users, stored in the file

/*function definitions*/
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
	return 1;
}

void CreateFile(){
	int sz;
	struct stat st; //uses to get the size of file. hence, is used to get total number of users
	fd = open(file, O_CREAT|O_RDWR, 0777); //opens file in read-write mode; if not exists, creates the file
	lseek(fd, 0L, SEEK_END);
	fstat(fd, &st);
	sz = st.st_size;
	no_of_users = sz/sizeof(acc);
}

int SearchUser(char * user){
	struct account temp;
	for(int id=0; id<no_of_users; id++){
		lseek(fd, id*sizeof(temp), SEEK_SET);
		SetFuncLock(id, 1);
		read(fd, &temp, sizeof(temp));
		SetFuncLock(id, 0);
		if(!(strcmp(temp.username, user))){ //user already exists
			acc = temp;
			return id;
		}
	}
	return -1;
}

int SearchActiveUser(char * user){
	int id;
	if((id=SearchUser(user))>-1)
		if(acc.active=='1') //if users exists, checks if it is active
			return id;
	return -1;
}

void InitializeAcc(int id){
	lseek(fd, id*sizeof(acc), SEEK_SET);
	SetFuncLock(id, 1);
	read(fd, &acc, sizeof(acc));
	SetFuncLock(id, 0);
}

int AddUser(char * user, char * pass, int accType){
	int id=SearchUser(user);
	int rt, tr=0;
	char norm=(accType==1)?'1':'0';

	if(id>-1){
		if(acc.active=='0') //only for disabled account
			lseek(fd, id*(sizeof(acc)), SEEK_SET);
		else
			return 0;
	}
	else if(id==-1){ //for new user
		lseek(fd, no_of_users*(sizeof(acc)), SEEK_SET);
		no_of_users++;
	}
	else
		return 0;

	strcpy(acc.username, user);
	strcpy(acc.password, pass);
	acc.balance = 1000;
	acc.active = '1';
	acc.normalAcc = norm;

	SetFuncLock(id, 2);
	rt = write(fd, &acc, sizeof(acc));
	SetFuncLock(id, 0);	

	return 1;
}

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
	
	if(!draw){ //deposit(amt) - if 'int' limit is exceeded, equal it to the limit
		if(acc.balance+amt>2147483647)
			amt=2147483647-acc.balance;
		acc.balance+=amt;
	}
	else if(draw==1){ //withdraw(amt) - doesnot work if withdrawing amount is greater than current balance
		if(amt<=acc.balance)
			acc.balance-=amt;
		else
			return -1;
	}
	else if(draw==2){ //directly modify balance
		if(amt>2147483647)
			acc.balance=2147483647;
		else
			acc.balance=amt;
	}

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

int DeleteUser(char * user){ //sets user's 'active' variable to 0
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
	if(acc.normalAcc=='0') //no login lock for joint accs
		return 1;

	lseek(fd, id*sizeof(acc), SEEK_SET);
	struct flock login_lock;
	login_lock.l_type = F_WRLCK;
	login_lock.l_whence = SEEK_SET;
	login_lock.l_start = id*sizeof(acc);
	login_lock.l_len = sizeof(acc);
	login_lock.l_pid = getpid();
	fcntl(fd, F_SETLKW, &login_lock); //sets waiting write lock

	return 1;
}

int UserLogin(char * user, char * pass, int accType){
	int id;
	id = SearchUser(user);
	if(!(strcmp(acc.password, pass)) && (acc.active=='1')){ //valid credentials
		if(((acc.normalAcc=='0')&&(accType==2)) || ((acc.normalAcc=='1')&&(accType==1))){
			LockUser(id);
			return id;
		}
	}
	return -1;
}

void UserLogout(int id){ //unlock login lock

	InitializeAcc(id);

	login_lock.l_type = F_UNLCK;
	login_lock.l_whence = SEEK_SET;
	login_lock.l_start = id*sizeof(acc);
	login_lock.l_len = sizeof(acc);
	fcntl(fd, F_SETLKW, &login_lock);
	
	close(fd); //finally, close file descriptor
}

int AdminLogin(char * user, char * pass){
	if(!(strcmp(user, admin_username)) && !(strcmp(pass, admin_pass))) //hard-coded admin username and password
		return 1;
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
	if(ind==1) //change password of 'user' to 'pass'
		retVal = ChangePass(user, pass);
	else if(ind==2) //update balance depending on 'ind'
		retVal = (UpdateBal(user, bal, 2)==1)?1:0;
	else //change password and update balance - not given to admin yet
		retVal = (ChangePass(user, pass) && UpdateBal(user, bal, 2));
	return retVal;
}

/*testing function*/
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

#endif