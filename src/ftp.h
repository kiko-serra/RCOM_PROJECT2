#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_LENGTH 256

typedef struct FTP{
    int control_socket_fd; // file descriptor to control socket
    int data_socket_fd; // file descriptor to data socket
} ftp;

typedef struct URL {
	char user[MAX_LENGTH]; // string to user
	char password[MAX_LENGTH]; // string to password
	char host[MAX_LENGTH]; // string to host
	char ip[MAX_LENGTH]; // string to IP
	char path[MAX_LENGTH]; // string to path
	char filename[MAX_LENGTH]; // string to filename
	int port; // integer to port
} url;

//Parsing functions
void initURL(url* url);
int parseURL(url* url, const char* str); // Parse a string with the url to create the URL structure
int getIpByHost(url* url); // gets an IP by host name
char* processElementUntilChar(char* str, char chr);

//FTP functions
int ftpConnect(ftp* ftp, const char* ip, int port);
int ftpLogin(ftp* ftp, const char* user, const char* password);
int ftpCWD(ftp* ftp, const char* path);
int ftpPasv(ftp* ftp);
int ftpRetr(ftp* ftp, const char* filename);
int ftpDownload(ftp* ftp, const char* filename);
int ftpDisconnect(ftp* ftp);

int ftpSend(ftp* ftp, const char* str, size_t size);
int ftpRead(ftp* ftp, char* str, size_t size);
