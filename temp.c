#include <stdio.h>
#include "helper.h"

int main(int argc, char const *argv[])
{
	int x, y, z;

	CreateFile();

	// GetAllAcc();

	// x = AddUser("User1", "Pass143");
	// printf("User Creation : %d\n", x);

	// x = ChangePass("User1", "Pass");
	// printf("Pass Change: %d\n", x);

	// x = VerifyUser("User1", "Pass143");
	// printf("User Verification: %d\n", x);

	// x = DeleteUser("User1");
	// printf("User deleted: %d\n", x);

	// y = Search("User1");
	// printf("User1 search : %d\n", y);

	// x = UpdateBal("User1", 990, 0);
	// printf("Deposit: %d\n", x);

	// z = VerifyAdmin("ajksdf", "askdj");
	// printf("AdminVerification : %d\n", z);

	// printf("%lld\n", GetBal("User1"));

	// x = ModifyUser("User1", "HeyBabe", 50000, 0);

	GetAllAcc();
	// GetAllTransact();

	return 0;
}