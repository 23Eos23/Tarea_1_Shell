#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <limits.h> //Para obtener el limite del nombre del host
#include <signal.h>
#include <vector>
#include <sstream>

#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <fcntl.h>

using namespace std;
char user[HOST_NAME_MAX+1];
// --- Variable global para el PID del hijo en miprof con timeout ---
pid_t child_pid_global = -1;

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

// --- FUNCIONES PARA MIPROF ---

// Manejador para el timeout de miprof
void manejador_alarma(int sig) {
    if (child_pid_global > 0) {
        cout << "\n[miprof] Tiempo máximo de ejecución excedido. Terminando proceso..." << endl;
        kill(child_pid_global, SIGKILL); // Se termina el proceso hijo a la fuerza
    }
}

// Función principal contiene logica
void ejecutar_miprof(const vector<string> &comandos) {
    if (comandos.size() < 3) {
        cerr << "Uso incorrecto. Formatos:" << endl;
        cerr << "  miprof ejec <comando> [args...]" << endl;
        cerr << "  miprof ejecsave <archivo> <comando> [args...]" << endl;
        cerr << "  miprof ejecutar <maxtiempo> <comando> [args...]" << endl;
        return;
    }

    struct rusage usage;
    struct timespec start_time, end_time;
    string modo = comandos[1];
    vector<string> comando_a_ejecutar;
    string archivo_salida = "";
    int tiempo_maximo = 0;

    // Parsear los argumentos para saber que modo ocupar
    size_t comando_idx = 0;
    if (modo == "ejec") {
        comando_idx = 2;
    } else if (modo == "ejecsave") {
        if (comandos.size() < 4) {
            cerr << "miprof: ejecsave requiere un archivo y un comando." << endl;
            return;
        }
        archivo_salida = comandos[2];
        comando_idx = 3;
    } else if (modo == "ejecutar") {
        if (comandos.size() < 4) {
            cerr << "miprof: ejecutar requiere un tiempo máximo y un comando." << endl;
            return;
        }
        try {
            tiempo_maximo = stoi(comandos[2]);
        } catch (const exception& e) {
            cerr << "miprof: el tiempo máximo debe ser un número entero." << endl;
            return;
        }
        comando_idx = 3;
    } else {
        cerr << "miprof: modo '" << modo << "' no reconocido." << endl;
        return;
    }

    // Copiar el comando que se va a ejecutar
    for (size_t i = comando_idx; i < comandos.size(); ++i) {
        comando_a_ejecutar.push_back(comandos[i]);
    }

    //Medir tiempo y ejecutar
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    pid_t pid = fork();
    child_pid_global = pid; // Guardar PID para el manejador de la alarma

    if (pid == 0) { // Proceso Hijo
        vector<char*> args;
        for (const auto& arg : comando_a_ejecutar) {
            args.push_back(const_cast<char*>(arg.c_str()));
        }
        args.push_back(nullptr);

        if (execvp(args[0], args.data()) == -1) {
            cerr << "miprof: No se pudo ejecutar el comando '" << args[0] << "'" << endl;
            perror("Error de execvp");
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) { // Proceso Padre
        if (tiempo_maximo > 0) {
            signal(SIGALRM, manejador_alarma);
            alarm(tiempo_maximo);
        }

        int status;
        wait4(pid, &status, 0, &usage); // wait4 obtiene los recursos usados

        if (tiempo_maximo > 0) {
            alarm(0); // Cancelar la alarma si el proceso termina tiempo
        }

        clock_gettime(CLOCK_MONOTONIC, &end_time);

        // Calcular y mostrar resultado
        double real_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
        double user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
        double sys_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;
        long mem_peak = usage.ru_maxrss; // En KB en Linux

        stringstream output;
        output << "--- Medicion de '";
        for(const auto& arg : comando_a_ejecutar) output << arg << " ";
        output << "' ---\n";
        output << "Tiempo Real: \t\t" << fixed << real_time << " s\n";
        output << "Tiempo de Usuario (CPU): \t" << fixed << user_time << " s\n";
        output << "Tiempo de Sistema (CPU): \t" << fixed << sys_time << " s\n";
        output << "Peak de Memoria: \t" << mem_peak << " KB\n";
        output << "--------------------------------------\n";

        cout << output.str();

        if (modo == "ejecsave" && !archivo_salida.empty()) {
            FILE* f = fopen(archivo_salida.c_str(), "a");
            if (f) {
                fputs(output.str().c_str(), f);
                fclose(f);
                cout << "Resultados anexados en '" << archivo_salida << "'" << endl;
            } else {
                perror("miprof: no se pudo abrir el archivo de salida");
            }
        }
    } else {
        perror("Error en fork para miprof");
    }
}

// Ahora maneja 3 comandos  internos
int manejar_comandos(vector<string> comandos){
  if(comandos.empty()) return 0;

  if(comandos[0] == "exit")
  {
    cout << "Saliendo de la shell..." << endl;
    exit(0);
  }
  else if(comandos[0] == "cd")
  {
    if(comandos.size() < 2)
    {
      cerr << "cd: falta el argumento del directorio" << endl;
    } else {
      if(chdir(comandos[1].c_str()) != 0)
      {
        perror("Error al cambiar de directorio");
      }
    }
    return 1;
  }
  else if(comandos[0] == "miprof")
  {
    ejecutar_miprof(comandos);
    return 1; 
  }
  return 0; 
}

// Se implementa logica de solo un comando sin pipes
void proceso(vector<string> &comando)
{
  pid_t pid = fork();

  if(pid == 0)
  {
    // Proceso hijo: ejecutar el comando
    vector<char*> args;
    args.reserve(comando.size() + 1);
    for(size_t i = 0; i < comando.size(); i++)
    {
      args.push_back(const_cast<char*>(comando[i].c_str()));
    }
    args.push_back(nullptr);

    if(execvp(args[0], args.data()) == -1)
    {
      cerr << "mishell: comando no encontrado: " << args[0] << endl;
      exit(1); // El hijo debe terminar si execvp falla
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

// Se implementa logica de multiples pipes
void procesar_multiples_pipes(vector<vector<string>> &comandos)
{
  int num_comandos = comandos.size();

  if(num_comandos == 1)
  {
    proceso(comandos[0]);
    return;
  }

  int pipes[num_comandos - 1][2];

  for(int i = 0; i < num_comandos - 1; i++)
  {
    if(pipe(pipes[i]) == -1)
    {
      perror("Error al crear pipe");
      return;
    }
  }

  for(int i = 0; i < num_comandos; i++)
  {
    pid_t pid = fork();

    if(pid == 0)
    {
      if(i > 0)
      {
        dup2(pipes[i-1][0], STDIN_FILENO);
      }
      if(i < num_comandos - 1)
      {
        dup2(pipes[i][1], STDOUT_FILENO);
      }

      for(int j = 0; j < num_comandos - 1; j++)
      {
        close(pipes[j][0]);
        close(pipes[j][1]);
      }

      vector<char*> args;
            for (const auto& arg : comandos[i]) {
                args.push_back(const_cast<char*>(arg.c_str()));
            }
            args.push_back(nullptr);

      if(execvp(args[0], args.data()) == -1)
      {
        cerr << "mishell: comando no encontrado: " << args[0] << endl;
        exit(1);
      }
    }
    else if(pid < 0)
    {
      perror("Error al crear proceso hijo");
      return;
    }
  }

  for(int i = 0; i < num_comandos - 1; i++)
  {
    close(pipes[i][0]);
    close(pipes[i][1]);
  }

  for(int i = 0; i < num_comandos; i++)
  {
    wait(nullptr);
  }
}

// Funcion para separar los comandos por pipes
void separar_comandos(string &input, vector<vector<string>> &comandos)
{
  stringstream ss(input);
  string comando_pipe;

  while(getline(ss, comando_pipe, '|'))
  {
    comandos.push_back({});
    stringstream comando_ss(comando_pipe);
    string argumento;

    while( comando_ss >> argumento)
    {
      comandos.back().push_back(argumento);
    }
    // Evitar agregar vectores vacios si hay espacio 
    if (comandos.back().empty()) {
            comandos.pop_back();
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
  signal(SIGINT, manejar_ctrl_c);
  gethostname(user, HOST_NAME_MAX);

  while(1)
  {
    string input;
    shell_print(user);
    leer_input(input);

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
