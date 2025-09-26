# Tarea 1: Desarrollar una Shell

Breve descripción sobre cómo compilar y ejecutar comandos dentro de la shell

# Compilación y ejecución

Para la ejecución y uso de la consola de comando se necesita un entorno o sistema Unix:

'''
g++ tarea1.cpp
./a.out
'''

Luego de compilar el programa y ejecultarlo se deberá mostrar por pantalla el prompt.

# Funcionamiento

La Shell implementada cumple con un funcionamiento similar a las shell's de los sistemas en general.
Cumpliendo con los siguientes requerimientos:

1. Comandos internos
2. Uso de pipes indefinidos

# miprof

La Shell cuenta con un comando personalizado llamado *miprof*. El cual consiste en poder ejecutar cualquier comando o programa y captura la información respecto al tiempo
de ejecución en tiempos de usuario, sistema y real, junto con el peak de memoria máxima residente.

# 1. Ejecutar comandos
'''
miprof ejec comando args
'''
Muestra los datos en pantalla
'''
miprof ejecsave archivo comando args
'''
Ejecuta y guarda la salida en *archivo*.
Si el archivo existe, se agrega al final.

# 2. Con Sort

Ejemplo para analizar rendimiento con distintos tamaños de archivo:
'''
miprof ejec sort archivo.txt
'''

# 3. Limite de tiempo

'''
miprof maxtiempo comando args
'''
Ejecuta con un tiempo máximo (maxtiempo en segundos).
Si se excede, el proceso se termina.

