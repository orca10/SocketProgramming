#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#define BUF_SIZE 1024
#define BACKLOG 10
#define FILE_NAME 100

int main(int argc, char *argv[])
{
	int sockfd, newsockfd;
	char buffer[BUF_SIZE];

	struct sockaddr_in s_address, c_address; // s_address: server's address, c_address: client's address
	int portNum;														 // port number
	int n;
	socklen_t client_len;
	char animal[20]; // filename

	if (argc < 2)
	{
		fprintf(stderr, "ERROR, no port provided\n");
		exit(0);
	}

	// make socket
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket error");
		exit(0);
	}

	// bind socket
	bzero((char *)&s_address, sizeof(s_address));
	portNum = atoi(argv[1]);
	s_address.sin_family = AF_INET;
	s_address.sin_addr.s_addr = INADDR_ANY;
	s_address.sin_port = htons(portNum);

	if (bind(sockfd, (struct sockaddr *)&s_address, sizeof(s_address)) < 0)
	{
		perror("bind error");
		exit(0);
	}

	// listen
	if (listen(sockfd, BACKLOG) == -1)
	{
		perror("listen error");
		exit(0);
	}

	client_len = sizeof(c_address);

	while (1)
	{
		// accept
		if ((newsockfd = accept(sockfd, (struct sockaddr *)&c_address, &client_len)) == -1)
		{
			perror("accept error");
			continue;
		}

		printf("connection success!\n");
		printf("server : %s\n", inet_ntoa(c_address.sin_addr));

		bzero(buffer, BUF_SIZE);
		n = read(newsockfd, buffer, BUF_SIZE);
		if (n < 0)
			perror("reading error");
		printf("%s", buffer);

		// file name
		char *ptr = strtok(buffer, " ");
		ptr = strtok(NULL, " ");
		strcpy(animal, ptr);
		char pwd[BUF_SIZE];
		char fbuf[BUF_SIZE];

		getcwd(pwd, BUF_SIZE);
		strcat(pwd, "/");
		strcat(pwd, animal);

		// header
		char imageheader[] = "HTTP/1.1 200 OK\r\n"
												 "Content-Type: image/jpeg\r\n\r\n";

		// 404 not found
		char defaulthtml[] = "HTTP/1.1 404 Not Found\r\n"
												 "Content-Type: text/html; charset=UTF-8\r\n\r\n"
												 "<!DOCTYPE html>\r\n"
												 "<html><head><title>Hello</title>\r\n"
												 "<body>404 - NOT FOUND</body>"
												 "</html>";

		// main
		char homehtml[] = "HTTP/1.1 200 OK\r\n"
											"Content-Type: text/html; charset=UTF-8\r\n\r\n"
											"<!DOCTYPE html>\r\n"
											"<html><head><title>Hello</title>\r\n"
											"<body>Hello world!<br>Choose one of the items below.<br>cat.jpg   turtle.jpg   lion.jpg   dog.jpg   elephant.jpg</body>"
											"</html>";

		int fd, i;

		if (strlen(animal) == 1)
		{
			write(newsockfd, homehtml, sizeof(homehtml) - 1);
			printf("This is home\n");
		}

		else if ((fd = open(pwd, O_RDONLY)) == -1)
		{
			write(newsockfd, defaulthtml, sizeof(defaulthtml) - 1);
			printf("404 not found\n");
		}

		// send file
		else
		{
			write(newsockfd, imageheader, sizeof(imageheader) - 1); // send header

			while ((n = read(fd, fbuf, BUF_SIZE)) > 0)
			{ // send image
				write(newsockfd, fbuf, n);
			}
		}

		close(newsockfd);
		close(fd);
	}
}