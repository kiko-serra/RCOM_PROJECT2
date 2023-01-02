#include "ftp.h"

void initURL(url* url) {
	memset(url->user, 0, MAX_LENGTH);
	memset(url->password, 0, MAX_LENGTH);
	memset(url->host, 0, MAX_LENGTH);
	memset(url->path, 0, MAX_LENGTH);
	memset(url->filename, 0, MAX_LENGTH);
	url->port = 21;
}

const char* regExpression = "ftp://([([A-Za-z0-9])*:([A-Za-z0-9])*@])*([A-Za-z0-9.~-])+/([[A-Za-z0-9/~._-])+";

const char* regExprAnony = "ftp://([A-Za-z0-9.~-])+/([[A-Za-z0-9/~._-])+";

int parseURL(url* url, const char* urlStr) {
	printf("Parsing command line arguments...\n");
	
	char* tempURL, *element, *activeExpression;
	regex_t* regex;
	size_t nmatch = strlen(urlStr);
	regmatch_t pmatch[nmatch];
	int userPassMode;

	element = (char*) malloc(sizeof(char)*(strlen(urlStr) + 1));
	tempURL = (char*) malloc(sizeof(char)*(strlen(urlStr) + 1));

	memcpy(tempURL, urlStr, strlen(urlStr));

	if (tempURL[6] == '[') {
		userPassMode = 1;
		activeExpression = (char*) regExpression;
	} else {
		userPassMode = 0;
		activeExpression = (char*) regExprAnony;
	}

	regex = (regex_t*) malloc(sizeof(regex_t));

	int reti;
	if ((reti = regcomp(regex, activeExpression, REG_EXTENDED)) != 0) {
		printf("URL format is wrong.");
		return 1;
	}

	if ((reti = regexec(regex, tempURL, nmatch, pmatch, REG_EXTENDED)) != 0) {
		printf("URL could not execute.");
		return 1;
	}

	free(regex);

	// removing ftp:// from string
	strcpy(tempURL, tempURL + 6);

	if (userPassMode) {
		//removing [ from string
		strcpy(tempURL, tempURL + 1);

		// saving username
		strcpy(element, processElementUntilChar(tempURL, ':'));
		memcpy(url->user, element, strlen(element));

		//saving password
		strcpy(element, processElementUntilChar(tempURL, '@'));
		memcpy(url->password, element, strlen(element));
		strcpy(tempURL, tempURL + 1);
	}

	//saving host
	strcpy(element, processElementUntilChar(tempURL, '/'));
	memcpy(url->host, element, strlen(element));

	//saving url path
	char* path = (char*) malloc(strlen(tempURL));
	int startPath = 1;
	while (strchr(tempURL, '/')) {
		element = processElementUntilChar(tempURL, '/');

		if (startPath) {
			startPath = 0;
			strcpy(path, element);
		} else {
			strcat(path, element);
		}

		strcat(path, "/");
	}
	strcpy(url->path, path);

	// saving filename
	strcpy(url->filename, tempURL);

	free(tempURL);
	free(element);

	/*printf("\n%s\n%s\n%s\n%s\n%s\n", url->user, url->password, url->host,
	 url->path, url->filename);*/

	return 0;
}

int getIpByHost(url* url) {
	struct hostent* h;

	if ((h = gethostbyname(url->host)) == NULL) {
		herror("gethostbyname");
		return 1;
	}

//	printf("Host name  : %s\n", h->h_name);
//	printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *) h->h_addr)));

	char* ip = inet_ntoa(*((struct in_addr *) h->h_addr));
	strcpy(url->ip, ip);

	return 0;
}

char* processElementUntilChar(char* str, char chr) {
	// using temporary string to process substrings
	char* tempStr = (char*) malloc(strlen(str));

	// calculating length to copy element
	int index = strlen(str) - strlen(strcpy(tempStr, strchr(str, chr)));

	tempStr[index] = '\0'; // termination char in the end of string
	strncpy(tempStr, str, index);
	strcpy(str, str + strlen(tempStr) + 1);

	return tempStr;
}

static int connectSocket(const char* ip, int port) {
	int sockfd;
	struct sockaddr_in server_addr;

	// server address handling
	bzero((char*) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip); /*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port); /*server TCP port must be network byte ordered */

	// open an TCP socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket()");
		return -1;
	}

	// connect to the server
	if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr))< 0) {
		perror("connect()");
		return -1;
	}

	return sockfd;
}

int ftpConnect(ftp* ftp, const char* ip, int port) {
	int socketfd;
	char rd[1024];

	if ((socketfd = connectSocket(ip, port)) < 0) {
		printf("ERROR: Cannot connect socket.\n");
		return 1;
	}

	ftp->control_socket_fd = socketfd;
	ftp->data_socket_fd = 0;

	if (ftpRead(ftp, rd, sizeof(rd))) {
		printf("ERROR: ftpRead failure.\n");
		return 1;
	}

	return 0;
}

int ftpLogin(ftp* ftp, const char* user, const char* password) {
	char sd[1024];

	// username
	sprintf(sd, "USER %s\r\n", user);
	if (ftpSend(ftp, sd, strlen(sd))) {
		printf("ERROR: ftpSend failure.\n");
		return 1;
	}

	if (ftpRead(ftp, sd, sizeof(sd))) {
		printf("ERROR: Access denied reading username response.\nftpRead failure.\n");
		return 1;
	}

	// cleaning buffer
	memset(sd, 0, sizeof(sd));

	// password
	sprintf(sd, "PASS %s\r\n", password);
	if (ftpSend(ftp, sd, strlen(sd))) {
		printf("ERROR: ftpSend failure.\n");
		return 1;
	}

	if (ftpRead(ftp, sd, sizeof(sd))) {
		printf("ERROR: Access denied reading password response.\nftpRead failure.\n");
		return 1;
	}

	return 0;
}

int ftpCWD(ftp* ftp, const char* path) {
	char cwd[1024];

	sprintf(cwd, "CWD %s\r\n", path);
	if (ftpSend(ftp, cwd, strlen(cwd))) {
		printf("ERROR: Cannot send path to CWD.\n");
		return 1;
	}

	if (ftpRead(ftp, cwd, sizeof(cwd))) {
		printf("ERROR: Cannot send path to change directory.\n");
		return 1;
	}

	return 0;
}

int ftpPasv(ftp* ftp) {
	char pasv[1024] = "PASV\r\n";
	if (ftpSend(ftp, pasv, strlen(pasv))) {
		printf("ERROR: Cannot enter in passive mode.\n");
		return 1;
	}

	if (ftpRead(ftp, pasv, sizeof(pasv))) {
		printf("ERROR: None information received to enter in passive mode.\n");
		return 1;
	}

	// starting process information
	int ipPart1, ipPart2, ipPart3, ipPart4;
	int port1, port2;
	if ((sscanf(pasv, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ipPart1,&ipPart2, &ipPart3, &ipPart4, &port1, &port2)) < 0) {
		printf("ERROR: Cannot process information to calculating port.\n");
		return 1;
	}

	// cleaning buffer
	memset(pasv, 0, sizeof(pasv));

	// forming ip
	if ((sprintf(pasv, "%d.%d.%d.%d", ipPart1, ipPart2, ipPart3, ipPart4))< 0) {
		perror("ERROR: Cannot form ip address.\n");
		return 1;
	}

	// calculating new port
	int portResult = port1 * 256 + port2;

	printf("IP: %s\n", pasv);
	printf("PORT: %d\n", portResult);

	if ((ftp->data_socket_fd = connectSocket(pasv, portResult)) < 0) {
		printf("ERROR: Incorrect file descriptor associated to ftp data socket fd.\n");
		return 1;
	}

	return 0;
}

int ftpRetr(ftp* ftp, const char* filename) {
	char retr[1024];

	sprintf(retr, "RETR %s\r\n", filename);
	if (ftpSend(ftp, retr, strlen(retr))) {
		printf("ERROR: Cannot send filename.\n");
		return 1;
	}

	if (ftpRead(ftp, retr, sizeof(retr))) {
		printf("ERROR: None information received.\n");
		return 1;
	}

	return 0;
}

int ftpDownload(ftp* ftp, const char* filename) {
	FILE* file;
	int bytes;

	if (!(file = fopen(filename, "w"))) {
		printf("ERROR: Cannot open file.\n");
		return 1;
	}

	char buf[1024];
	while ((bytes = read(ftp->data_socket_fd, buf, sizeof(buf)))) {
		if (bytes < 0) {
			printf("ERROR: Nothing was received from data socket fd.\n");
			return 1;
		}

		if ((bytes = fwrite(buf, bytes, 1, file)) < 0) {
			printf("ERROR: Cannot write data in file.\n");
			return 1;
		}
	}

	fclose(file);
	close(ftp->data_socket_fd);

	return 0;
}

int ftpDisconnect(ftp* ftp) {
	char disc[1024];

	if (ftpRead(ftp, disc, sizeof(disc))) {
		printf("ERROR: Cannot disconnect account.\n");
		return 1;
	}

	sprintf(disc, "QUIT\r\n");
	if (ftpSend(ftp, disc, strlen(disc))) {
		printf("ERROR: Cannot send QUIT command.\n");
		return 1;
	}

	if (ftp->control_socket_fd)
		close(ftp->control_socket_fd);

	return 0;
}

int ftpSend(ftp* ftp, const char* str, size_t size) {
	int bytes;

	if ((bytes = write(ftp->control_socket_fd, str, size)) <= 0) {
		printf("WARNING: Nothing was sent.\n");
		return 1;
	}

	printf("Bytes send: %d\nInfo: %s\n", bytes, str);

	return 0;
}

int ftpRead(ftp* ftp, char* str, size_t size) {
	FILE* fp = fdopen(ftp->control_socket_fd, "r");

	do {
		memset(str, 0, size);
		str = fgets(str, size, fp);
		int aux = atoi (str);
		if(aux==550){
			printf("ERROR: File not found.\n");
			return 1;
		}
		printf("%s", str);
	} while (!('1' <= str[0] && str[0] <= '5') || str[3] != ' ');

	return 0;
}