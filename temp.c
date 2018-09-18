#include <stdio.h>

print(char **x){
	printf("%s\n", *x);
	*x="changed it!!";
}
int main(int argc, char const *argv[])
{
	char *x;
	x = "hi there";
	print(&x);
	printf("%s\n", x);
	return 0;
}