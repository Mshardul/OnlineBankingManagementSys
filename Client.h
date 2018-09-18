#ifndef Client_H
#define Client_H

short CreateSocket();
int ConnectToServer(int);
void SendToServer(int, char *, short);
void RecvFromServer(int, char *, short);
void VerifyCredentials(int);
void BankingOptions(int);
int ReceiveAck(int);

#endif