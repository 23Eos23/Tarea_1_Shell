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



int main(){
	struct sigaction sa; // Estructura para acciones
	sa.sa_handler = SIG_IGN; // Ignorar la señal
	sigaction(SIGINT, &sa, NULL); // Aplicar la acción a la señal

	gethostname(user, HOST_NAME_MAX);
	
	while(1){
		string command;
		shell_print(user);
		getline(cin, command);

		if (command.compare("exit") == 0)
		{
				exit(1);
		}
	}



	return 0;
}