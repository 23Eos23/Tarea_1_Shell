#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <limits.h> //Para obtener el limite del nombre del host
#include <signal.h>
#include <vector>
#include <sstream>

using namespace std;
char user[HOST_NAME_MAX+1];
#define MAX_COMMAND_SUPPORTED 3 // cd, exit y miprof

void shell_print(char* user)
{
	printf("\033[1;36m%s\033[0m:\033[1;31m~\033[0m$ ",user);
}

void leer_input(string &input)
{
	getline(cin, input);
}

void manejar_ctrl_c(int sig)
{
	printf("\n");
	shell_print(user);
	fflush(stdout);
}

int manejar_comandos(vector<string> comandos){
	if(comandos[0].compare("exit") == 0)
	{
		cout << "Saliendo de la shell..." << endl;
		exit(0); 
	}
	else if(comandos[0].compare("cd") == 0)
	{
		if(comandos.size() < 2)
		{
			cout << "cd: falta el argumento del directorio" << endl;
			return 1;
		}
		if(chdir(comandos[1].c_str()) != 0)
		{
			perror("Error al cambiar de directorio");
		}
		return 1;
	}
	else if(comandos[0].compare("miprof") == 0)
	{
		// ojo -> miprof parte 2 de la tarea
		return 1; 
	}
	return 0; 
}

// Se implementa la lógica de solo un pipe
void proceso(vector<string> &comando)
{
	int pid = fork();

	if(pid == 0)
	{
		
	}
	else 
	{
		perror("Error al crear proceso hijo en fork");
	}
}

// Se implementa la lógica de multiples pipes
void procesar_multiples_pipes(vector<vector<string>> &comandos)
{
	
}

// Funcion para separar los comandos por pipes
void separar_comandos(string &input, vector<vector<string>> &comandos)
{
	stringstream ss(input);
	string comando;
	
	while(getline(ss, comando, '|'))
	{
		comandos.push_back({});

		stringstream comando_ss(comando);
		string argumento;

		while( comando_ss >> argumento)
		{
			comandos.back().push_back(argumento);
		}
	}
}


// Revisa si el comando es interno
int procesar_input_usuario(string &input, vector<vector<string>> &comandos)
{
	separar_comandos(input, comandos);
	
	if(comandos.size() > 0 && comandos[0].size() > 0)
	{
		return manejar_comandos(comandos[0]);
	}
	
	return 0;
}

int main()
{
	vector<vector<string>> comandos;
	signal(SIGINT, manejar_ctrl_c); // Manejar Ctrl+C
	gethostname(user, HOST_NAME_MAX);
	
	while(1)
	{
		string input;
		shell_print(user);
		leer_input(input);

		// Saltar si la entrada está vacía
		if (input.empty() || input.find_first_not_of(" \t\n\r") == string::npos)
		{
			comandos.clear();
			continue;
		}

		if(!procesar_input_usuario(input, comandos)){
			//Ver si hay pipes
			cout << "Comando no reconocido: " << input << endl;
		}
		comandos.clear();
	}



	return 0;
}