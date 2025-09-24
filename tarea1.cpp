#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <limits.h> //Para obtener el limite del nombre del host
#include <signal.h>

using namespace std;
char user[HOST_NAME_MAX+1];


void shell_print(char* user)
{
	printf("\033[1;36m%s\033[0m:\033[1;31m~\033[0m$ ",user);
}

void leer_input(string &command)
{
	getline(cin, command);
}

void manejar_ctrl_c(int sig)
{
	printf("\n");
	shell_print(user);
	fflush(stdout);
}

int main()
{
	signal(SIGINT, manejar_ctrl_c); // Manejar Ctrl+C
	gethostname(user, HOST_NAME_MAX);
	
	while(1)
	{
		string command;
		shell_print(user);
		leer_input(command);

		if (command.compare("exit") == 0)
		{
				exit(1);
		}
	}



	return 0;
}