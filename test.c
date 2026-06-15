#include <stdio.h>

int func(){
 	printf("Here");
	return 0;
}

int main(){
	
	int * f() = &func();
	f();
 	return 0;
}
