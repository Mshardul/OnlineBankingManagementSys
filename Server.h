#ifndef Server_H
#define Server_H

short CreateSocket(void);
int BindSocket(short);
void SendToClient(int, char * , short);
void RecvFromClient(int, char * , short);
int VerifyCredentials(char *, char *);
void ShowLoginMenu(int, char *, char *);
void LoginOption(int, char*, char*);
void Deposit(int, char *);
void Withdraw(int, char *);
void BalEnquiry(int, char *);
void ChangePass(int, char *);
void ViewDet(int, char *);
int ShowBankingOptions(int, char *);
void SendAck(int, int);

#endif