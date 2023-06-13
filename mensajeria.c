// Redes de Computadoras - Curso 1er Semestre 2023
// Tecnologo en Informatica FIng - CETP
//
// Entrega 1  - Programacion con Sockets
// Sistema basico de Mensajeria

// Integrantes:
//	Federico Maciel - 51913844
//	Axel Lois - 51874430
//	Michael Ramos - 52515362
//	Juan Castaño - 54778697

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>

#define MAX_LARGO_MENSAJE 255
#define MAX_NOMBRE 25

using namespace std;

void manejadorSenhales(int signal)
{

	if (signal == SIGINT)
	{
		cout << "\33[46m\33[31m[" << getpid() << "]"
			 << " SIGINT CTRL+C recibido\33[00m\n";
	}
	if (signal == SIGTERM)
	{
		cout << "\33[46m\33[31m[" << getpid() << "]"
			 << " SIGTERM Terminacion de programa\33[00m\n";
	}
	if (signal == SIGSEGV)
	{
		cout << "\33[46m\33[31m[" << getpid() << "]"
			 << " SIGSEGV violacion de segmento\33[00m\n";
	}
	if (signal == SIGCHLD)
	{
		cout << "\33[46m\33[31m["
			 << "]"
			 << " SIGCHLD \33[00m\n";
	}
	if (signal == SIGPIPE)
	{
		cout << "\33[46m\33[31m[" << getpid() << "]"
			 << " SIGPIPE \33[00m\n";
	}
	if (signal == SIGKILL)
	{
		cout << "\33[46m\33[31m[" << getpid() << "]"
			 << " SIGKILL \33[00m\n";
	}
	exit(1);
}

void resetString(char *&s)
{

	s[0] = '\0';
}

char *agregarCero(char *cad, int num)
{

	char *aux = new char[25];
	resetString(aux);
	strcat(aux, "0");
	sprintf(cad, "%d", num);
	if (num < 10)
	{
		strcat(aux, cad);
		return aux;
	}
	else
	{
		delete[] aux;
		return cad;
	}
}

char *getTiempo()
{

	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	int hh = timeinfo->tm_hour;
	int mm = timeinfo->tm_min;
	int ss = timeinfo->tm_sec;
	int DD = timeinfo->tm_mday;
	int MM = (timeinfo->tm_mon) + 1;	   // xq enero = 0
	int YYYY = (timeinfo->tm_year) + 1900; // xq el año es desde 1900
	char *s = new char[19];
	resetString(s);
	char *cad = new char[25];
	resetString(cad);
	// chequeo si alguna es menor q 10 para concatenarle un '0'
	strcat(s, "[");
	cad = agregarCero(cad, YYYY);
	strcat(s, cad);
	strcat(s, ".");
	cad = agregarCero(cad, MM);
	strcat(s, cad);
	strcat(s, ".");
	cad = agregarCero(cad, DD);
	strcat(s, cad);
	strcat(s, " ");
	cad = agregarCero(cad, hh);
	strcat(s, cad);
	strcat(s, ":");
	cad = agregarCero(cad, mm);
	strcat(s, cad);
	strcat(s, ":");
	cad = agregarCero(cad, ss);
	strcat(s, cad);
	strcat(s, "]");
	return s;
}

void readPasswords(char password[])
{
	/*
	Lee contraseñas de un archivo de texto y las almacena en un arreglo.

	Argumentos:
		password (char[]): El arreglo donde se almacenarán las contraseñas leídas.

	*/

	int numPasswords = 0;
	int i = 0;

	ifstream file("passwords.txt");

	if (file.is_open())
	{
		while (!file.eof())
		{
			file >> password[i];
			i++;
			numPasswords++;
		}
		file.close();
	}
}

void extractFilePath(string *filePath, char message[])
{
	/*
	Extrae la ruta de archivo de un mensaje y la almacena en un string.

	Argumentos:
		filePath (string*): Puntero al string donde se almacenará la ruta de archivo extraída.
		message (char[]): El mensaje del cual se extraerá la ruta de archivo.

	*/

	int position = 0;
	bool copyPath = false;

	while (position < MAX_LARGO_MENSAJE - 2 && message[position] != '\0')
	{
		if (message[position] == '&')
		{
			copyPath = true;
			position += 6;
		}

		if (copyPath)
		{
			if (message[position] != ' ')
			{
				*filePath += message[position];
			}
		}

		position++;
	}
}

string getPathFromMessage(char message[])
{
	/*
	Obtiene la ruta de archivo de un mensaje y la devuelve como un string.

	Argumentos:
		message (char[]): El mensaje del cual se extraerá la ruta de archivo.
	*/

	int position = 0;
	bool copyPath = false;
	string pathArchivo;

	while (position < MAX_LARGO_MENSAJE - 2 && message[position] != '\0')
	{
		if (message[position] == '&')
		{
			copyPath = true;
			position += 6;
		}

		if (copyPath)
		{
			if (message[position] != ' ')
			{
				pathArchivo += message[position];
			}
		}

		position++;
	}

	return pathArchivo;
}

bool checkFileInMessage(char message[])
{
	/*
	Verifica si un mensaje contiene un carácter '&' y devuelve true en caso afirmativo, de lo contrario, devuelve false.

	Argumentos:
		message (char[]): El mensaje a verificar.
	*/

	int position = 0;
	char letter = message[0];

	while (letter != '\n' && position < MAX_LARGO_MENSAJE - 2)
	{
		letter = message[position];

		if (letter == '&')
		{
			return true;
		}
		position++;
	}
	return false;
}

void readWrittenMessage(char message[])
{
	/*
	Lee un mensaje escrito por el usuario desde la entrada estándar y lo guarda en un arreglo de caracteres.

	Argumentos:
		message (char[]): El arreglo de caracteres donde se almacenará el mensaje.
	*/

	int position = 0;
	char letter;

	letter = getchar();
	while (letter != '\n' && position < MAX_LARGO_MENSAJE - 2)
	{
		message[position] = letter;
		position++;
		letter = getchar();
	}

	message[position] = '\0';
}

void setupSignals()
{
	/*
	Configura el manejo de señales para el programa.

	Establece los manejadores de señales para SIGINT, SIGTERM, SIGPIPE, SIGSEGV y SIGKILL.
	Ignora la señal SIGALRM.
	*/

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &manejadorSenhales;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGKILL, &sa, NULL);
	signal(SIGALRM, SIG_IGN);
}

void sendFile(FILE *file, int fd, struct sockaddr_in server)
{
	/*
	Envía un archivo línea por línea a través de un socket UDP.

	Lee cada línea del archivo y la envía al servidor especificado mediante el socket UDP.
	Después de enviar todas las líneas del archivo, envía un mensaje indicando el fin del archivo.

	Argumentos:
		file: Puntero al archivo que se va a enviar.
		fd: Descriptor de archivo del socket UDP.
		server: Estructura que contiene la información del servidor.
	*/

	char buffer[MAX_LARGO_MENSAJE];
	unsigned int sin_size = sizeof(struct sockaddr_in);

	while (fread(buffer, sizeof(char), MAX_LARGO_MENSAJE, file) != 0)
	{
		if (sendto(fd, buffer, MAX_LARGO_MENSAJE, 0, (struct sockaddr *)&server, sin_size) == -1)
		{
			cout << "\33[46m\33[31m[ERROR]:"
				 << " ERROR: enviando archivo.\33[00m\n";
			exit(1);
		}
		bzero(buffer, MAX_LARGO_MENSAJE);
	}

	// Le digo al servidor que termino el envio del archivo
	strcpy(buffer, "finArchivo");
	sendto(fd, buffer, MAX_LARGO_MENSAJE, 0, (struct sockaddr *)&server, sin_size);
	bzero(buffer, MAX_LARGO_MENSAJE);
}

void receiveFile(int fd, struct sockaddr_in server, char mensaje[])
{
	/*
	Recibe un archivo línea por línea a través de un socket UDP.

	Recibe cada línea del archivo del servidor especificado mediante el socket UDP y lo guarda en un archivo local.
	La función continúa recibiendo y guardando las líneas del archivo hasta que reciba un mensaje indicando el fin del archivo.

	Argumentos:
		fd: Descriptor de archivo del socket UDP.
		server: Estructura que contiene la información del servidor.
		mensaje: Mensaje que contiene información adicional.
	*/

	char *filename;
	char buffer[MAX_LARGO_MENSAJE];
	string strPathArchivo;
	FILE *redesFile;

	strPathArchivo = getPathFromMessage(mensaje);

	filename = (char *)strPathArchivo.c_str();
	redesFile = fopen(filename, "w");

	while (true)
	{
		unsigned int sin_size = sizeof(struct sockaddr_in);

		if (recvfrom(fd, buffer, MAX_LARGO_MENSAJE, 0, (struct sockaddr *)&server, &sin_size) == -1)
		{
			cout << "\33[46m\33[31m[ERROR]:"
				 << " ERROR: Imposible hacer recvfrom() para recepcion.\33[00m\n";
			exit(1);
		}

		if (strcmp(buffer, "finArchivo") == 0)
		{
			fclose(redesFile);
			break;
		}

		fwrite(buffer, sizeof(char), MAX_LARGO_MENSAJE, redesFile);
		bzero(buffer, MAX_LARGO_MENSAJE);
	}
}

void authenticateUser(char usuario[MAX_NOMBRE], char clave[MAX_NOMBRE], char *argv[4])
{
	/*
	Autentica al usuario mediante un servidor remoto.

	Genera una cadena de autenticación del usuario y la contraseña proporcionados.
	Establece una conexión con el servidor remoto utilizando TCP/IP.
	Envía la cadena de autenticación al servidor y recibe la respuesta.
	Si la autenticación es exitosa, muestra un mensaje de bienvenida.

	Argumentos:
		usuario: Nombre de usuario.
		clave: Contraseña del usuario.
		argv: Argumentos del programa, incluyendo la dirección IP y el puerto del servidor.
	*/

	char buf[MAX_LARGO_MENSAJE];
	char userPass[MAX_LARGO_MENSAJE];
	char passMD5[MAX_NOMBRE];
	char echo[MAX_NOMBRE];

	struct sockaddr_in server;
	struct hostent *he;

	int numbytes;
	int fd;
	int error;

	echo[0] = 0;
	strcat(echo, "echo -n ");
	strcat(echo, "'");
	strcat(echo, clave);
	strcat(echo, "'");
	strcat(echo, " | md5sum > passwords.txt");

	system(echo);
	readPasswords(passMD5);
	strtok(passMD5, "-");

	userPass[0] = 0;
	strcat(userPass, usuario);
	strcat(userPass, "-");
	strcat(userPass, passMD5);
	strcat(userPass, "\r\n");

	if ((he = gethostbyname(argv[2])) == NULL)
	{
		cout << "\33[46m\33[31m[ERROR]:"
			 << " gethostbyname()\33[00m\n";
		exit(-1);
	}

	fd = socket(AF_INET, SOCK_STREAM, 0); // socket TCP
	if (fd == -1)
	{
		cout << "\33[46m\33[31m[ERROR]:"
			 << " socket()\33[00m\n";
		exit(-1);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[3]));
	server.sin_addr = *((struct in_addr *)he->h_addr);
	bzero(&(server.sin_zero), 8);

	error = connect(fd, (struct sockaddr *)&server, sizeof(struct sockaddr));
	if (error == -1)
	{
		cout << "\33[46m\33[31m[ERROR]:"
			 << " connect()\33[00m\n";
		exit(-1);
	}

	numbytes = recv(fd, buf, MAX_LARGO_MENSAJE, 0);
	if (numbytes == -1)
	{
		cout << "\33[46m\33[31m[ERROR]:"
			 << " recv()\33[00m\n";
		exit(-1);
	}
	buf[numbytes - 2] = '\0';

	send(fd, userPass, strlen(userPass), 0);

	numbytes = recv(fd, buf, MAX_LARGO_MENSAJE, 0);
	if (numbytes == -1)
	{
		cout << "\33[46m\33[31m[ERROR]: recv()\33[00m\n";
		exit(-1);
	}
	buf[numbytes - 2] = '\0';

	if (strcmp(buf, "NO") == 0)
	{
		cout << "\33[46m\33[31m[ERROR]: Imposible autenticar, usuario no valido.\33[00m\n";
		exit(-1);
	}
	else if (strcmp(buf, "SI") == 0)
	{
		numbytes = recv(fd, buf, MAX_LARGO_MENSAJE, 0);
		if (numbytes == -1)
		{
			cout << "\33[46m\33[31m[ERROR]:"
				 << " ERROR: Al conectar.\33[00m\n";
			exit(-1);
		}
		buf[numbytes - 2] = '\0';
		cout << "Bienvenid@ " << buf << endl;
	}
	else
	{
		cout << "\33[46m\33[31m[ERROR]: Error en protocolo de autenticacion.\33[00m\n";
		exit(-1);
	}

	close(fd);
}

void receiveMessages(int puerto)
{
	/*
	Escucha y procesa mensajes entrantes en un puerto especificado.

	Crea un socket UDP para recibir mensajes.
	Enlaza el socket al puerto especificado.
	Espera y procesa mensajes entrantes.
	Si el mensaje es un archivo, lo recibe.
	Muestra información sobre el mensaje recibido.

	Argumentos:
		puerto: Puerto en el que se va a escuchar.
	*/

	int fd;
	int numbytes;
	char buffer[MAX_LARGO_MENSAJE];
	struct sockaddr_in server;
	struct sockaddr_in client;

	if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		cout << "\33[46m\33[31m[ERROR]:"
			 << " ERROR: Imposible crear socket UDP.\33[00m\n";
		exit(1);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(puerto);
	server.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server.sin_zero), 8);

	if (bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
	{
		cout << "\33[46m\33[31m[ERROR]:"
			 << " ERROR: Imposible hacer bind() para recepcion.\33[00m\n";
		exit(1);
	}

	unsigned int sin_size = sizeof(struct sockaddr_in);

	while (true)
	{
		if ((numbytes = recvfrom(fd, buffer, MAX_LARGO_MENSAJE, 0, (struct sockaddr *)&client, &sin_size)) == -1)
		{
			cout << "\33[46m\33[31m[ERROR]:"
				 << " ERROR: Imposible hacer recvfrom() para recepcion.\33[00m\n";
			exit(1);
		}

		buffer[numbytes - 2] = '\0';

		if (checkFileInMessage(buffer))
		{
			printf("%s %s %s\n", getTiempo(), inet_ntoa(client.sin_addr), buffer);
			receiveFile(fd, client, buffer);
		}
		else
		{
			printf("%s %s %s\n", getTiempo(), inet_ntoa(client.sin_addr), buffer);
		}
	}

	close(fd);
}

void sendMessages(int puerto, char usuario[MAX_NOMBRE])
{
	/*
	Envía mensajes desde el cliente al servidor en un puerto especificado.

	Crea un socket UDP para enviar mensajes.
	Lee el mensaje escrito por el usuario.
	Verifica si el mensaje es un archivo.
	Si es un archivo, envía el nombre del archivo y luego el contenido.
	Si no es un archivo, envía el mensaje como texto.
	Puede enviar mensajes de forma individual o utilizando broadcast.

	Argumentos:
		puerto: Puerto en el que se va a enviar el mensaje.
		usuario: Nombre de usuario que envía el mensaje.
	*/

	int fd;
	int broadcast;

	char mensaje[MAX_LARGO_MENSAJE];
	char buffer[MAX_LARGO_MENSAJE];
	char filePath[MAX_LARGO_MENSAJE];
	string strfilePath;
	FILE *redesFile;

	struct hostent *he;

	struct sockaddr_in server;

	while (true)
	{
		if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		{ // socket UDP
			cout << "\33[46m\33[31m[ERROR]:"
				 << " ERROR: Imposible abrir socket UDP para envio.\33[00m\n";
			exit(1);
		}

		unsigned int sin_size = sizeof(struct sockaddr_in);

		char ip[25];
		cin >> ip;

		if (strcmp(ip, "*") == 0)
		{ // broadcast
			broadcast = 1;

			server.sin_family = AF_INET;
			server.sin_port = htons(puerto);
			server.sin_addr.s_addr = INADDR_BROADCAST;
			bzero(&(server.sin_zero), 8);

			if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) == -1)
			{
				cout << "\33[46m\33[31m[ERROR]:"
					 << " ERROR: Imposible hacer setsockopt() para envio.\33[00m\n";
				exit(1);
			}
		}
		else
		{ // no broadcast
			if ((he = gethostbyname(ip)) == NULL)
			{
				cout << "\33[46m\33[31m[ERROR]:"
					 << " gethostbyname()\33[00m\n";
				exit(-1);
			}

			server.sin_family = AF_INET;
			server.sin_port = htons(puerto);
			server.sin_addr = *((struct in_addr *)he->h_addr);
			bzero(&(server.sin_zero), 8);
		}

		readWrittenMessage(mensaje);

		if (checkFileInMessage(mensaje) == true)
		{
			buffer[0] = '\0';

			buffer[0] = '\0';
			strcpy(buffer, usuario);
			strcat(buffer, " [Envio de archivo] ");
			strcat(buffer, mensaje);
			strcat(buffer, "\0");

			if (sendto(fd, buffer, MAX_LARGO_MENSAJE, 0, (struct sockaddr *)&server, sin_size) == -1)
			{
				cout << "\33[46m\33[31m[ERROR]:"
					 << " ERROR: sendto().\33[00m\n";
				exit(1);
			}

			extractFilePath(&strfilePath, mensaje);
			strcpy(filePath, strfilePath.c_str());
			redesFile = fopen(filePath, "rb");

			if (redesFile == NULL)
			{
				cout << "\33[46m\33[31m[ERROR]:"
					 << " ERROR: Imposible leer el archivo.\33[00m\n";
				exit(-1);
			}

			sendFile(redesFile, fd, server);
			fclose(redesFile);
		}
		else
		{
			buffer[0] = '\0';
			strcpy(buffer, usuario);
			strcat(buffer, " Dice: ");
			strcat(buffer, mensaje);
			strcat(buffer, "\0");

			if (sendto(fd, buffer, MAX_LARGO_MENSAJE, 0, (struct sockaddr *)&server, sin_size) == -1)
			{
				cout << "\33[46m\33[31m[ERROR]:"
					 << " ERROR: sendto().\33[00m\n";
				exit(1);
			}
		}
	}

	close(fd);
}

int main(int argc, char *argv[])
{
	// En argc viene la cantidad de argumentos que se pasan,
	// si se llama solo al programa, el nombre es el argumento 0
	// En nuestro caso:
	//      - argv[0] es el string "mensajeria", puede cambiar si se llama con otro path.
	//      - argv[1] es el puerto de escucha de mensajeria.
	//      - argv[2] es el ip del servidor de autenticacion.
	//      - argv[3] es el puerto del servidor de autenticacion.

	if (argc < 4)
	{
		cout << "\33[46m\33[31m[ERROR]:"
			 << " Faltan argumentos: port, ipAuth, portAuth.\33[00m\n";
		exit(1);
	}

	// Estructuras para el manejo de Senhales
	// Deberia haber un manejador de senhales para cada hijo si hacen cosas distintas
	// *********************************
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &manejadorSenhales;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGKILL, &sa, NULL);
	signal(SIGALRM, SIG_IGN);
	// **********************************

	cout << "\33[34mRedes de Computadoras 2023\33[39m: Sistema de Mensajeria.\nEscuchando en el puerto " << argv[1] << ".\nProceso de pid: " << getpid() << ".\n";

	// Antes de iniciar el programa de mensajeria debe autenticarse
	// como especifica la letra

	char usuario[MAX_NOMBRE];
	char clave[MAX_NOMBRE];

	cout << "Usuario: ";
	cin >> usuario;
	cout << "Clave: ";
	cin >> clave;

	authenticateUser(usuario, clave, argv);

	// Una vez autenticado puede comenzar a recibir y empezar el mensajes y archivos.
	// Para esto se debe bifircar el programa.
	// Es indistinto si el padre transmite y el hijo recibe, o viceversa, lo que si
	// al ser distintos porcesos, van a tener distinto pid.
	// Familiarizarse con los comandos de UNIX ps, ps -as, kill, etc.

	int pid = fork();

	int puerto = atoi(argv[1]);

	if (pid < 0)
	{
		cout << "\33[46m\33[31m[ERROR]:"
			 << " Imposible Bifurcar.\33[00m\n";
		exit(1);
	}

	if (pid == 0)
	{
		printf("\33[34mRx\33[39m: Iniciada parte que recepciona mensajes. Pid %d\n", getpid());
		receiveMessages(puerto);
	}

	if (pid > 0)
	{
		printf("\33[34mTx\33[39m: Iniciada parte que transmite mensajes. Pid %d\n", getpid());
		sendMessages(puerto, usuario);
	}
}