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

// Se implementa la lógica de solo un comando (sin pipes)
void proceso(vector<string> &comando)
{
	pid_t pid = fork();

	if(pid == 0)
	{
		// Proceso hijo: ejecutar el comando
		// Convertir vector<string> a char* array para execvp
		vector<char*> args;
		args.reserve(comando.size() + 1);
		for(size_t i = 0; i < comando.size(); i++)
		{
			args.push_back(const_cast<char*>(comando[i].c_str()));
		}
		args.push_back(nullptr);

		// Ejecutar el comando
		if(execvp(args[0], args.data()) == -1)
		{
			perror("Error al ejecutar comando");
			exit(1);
		}
	}
	else if(pid > 0)
	{
		// Padre: esperar al hijo
		int status;
		waitpid(pid, &status, 0);
	}
	else 
	{
		perror("Error al crear proceso hijo en fork");
	}
}

// Se implementa la lógica de multiples pipes
void procesar_multiples_pipes(vector<vector<string>> &comandos)
{
	int num_comandos = comandos.size();
	
	// Si solo hay un comando, usar la función proceso simple
	if(num_comandos == 1)
	{
		proceso(comandos[0]);
		return;
	}

	// Array para almacenar los pipes
	int pipes[num_comandos - 1][2];
	
	// Crear todos los pipes necesarios
	for(int i = 0; i < num_comandos - 1; i++)
	{
		if(pipe(pipes[i]) == -1)
		{
			perror("Error al crear pipe");
			return;
		}
	}
	
	// Crear procesos para cada comando
	for(int i = 0; i < num_comandos; i++)
	{
		pid_t pid = fork();
		
		if(pid == 0)
		{
			// Proceso hijo
			
			// Configurar entrada (stdin)
			if(i > 0) // No es el primer comando
			{
				dup2(pipes[i-1][0], STDIN_FILENO);
			}
			
			// Configurar salida (stdout)
			if(i < num_comandos - 1) // No es el último comando
			{
				dup2(pipes[i][1], STDOUT_FILENO);
			}
			
			// Cerrar todos los pipes en el proceso hijo
			for(int j = 0; j < num_comandos - 1; j++)
			{
				close(pipes[j][0]);
				close(pipes[j][1]);
			}
			
			// Convertir comando a formato execvp
			char* args[comandos[i].size() + 1];
			for(size_t k = 0; k < comandos[i].size(); k++)
			{
				args[k] = const_cast<char*>(comandos[i][k].c_str());
			}
			args[comandos[i].size()] = nullptr;
			
			// Ejecutar comando
			if(execvp(args[0], args) == -1)
			{
				perror("Error al ejecutar comando");
				exit(1);
			}
		}
		else if(pid < 0)
		{
			perror("Error al crear proceso hijo");
			return;
		}
	}
	
	// Proceso padre: cerrar todos los pipes
	for(int i = 0; i < num_comandos - 1; i++)
	{
		close(pipes[i][0]);
		close(pipes[i][1]);
	}
	
	// Esperar a que terminen todos los procesos hijos
	for(int i = 0; i < num_comandos; i++)
	{
		wait(nullptr);
	}
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
			// No es comando interno, ejecutar como comando externo
			procesar_multiples_pipes(comandos);
		}
		comandos.clear();
	}



	return 0;
}