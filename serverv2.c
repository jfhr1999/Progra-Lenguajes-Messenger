#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "user.c"

#define PORT 4444

char* scan_line(char *line)  //puede ser util para crear strings largos
{
    int ch; //as getchar() returns `int`

    if( (line = malloc(sizeof(char))) == NULL) //allocating memory
    {
        //checking if allocation was successful or not
        printf("unsuccessful allocation");
        exit(1);
    }

    line[0]='\0';

    for(int index = 0; ( (ch = getchar())!='\n' ) && (ch != EOF) ; index++)
    {
        if( (line = realloc(line, (index + 2)*sizeof(char))) == NULL )
        {
            //checking if reallocation was successful or not
            printf("unsuccessful reallocation");
            exit(1);
        }

        line[index] = (char) ch; //type casting `int` to `char`
        line[index + 1] = '\0'; //inserting null character at the end
    }

    return line;
}
// get line code

int searchUser(struct UserSocket users[10], int cantUsuarios, char*name){
	struct UserSocket *userTmp = NULL;
	printf("%s%d\n", users->name, users->sockNumber);
	for (int i = 0; i < cantUsuarios; i++)
	{
		userTmp = users+i;
		if(strcmp(userTmp->name, name) == 0){
			printf("%d\n", userTmp->sockNumber);
			return userTmp->sockNumber;
		}
	}
	return -1;
}

int main(){

	int sockfd, ret;
	 struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	char buffer[1024];
	pid_t childpid;

	//array de usuarios
	struct UserSocket users[100];
	int cantUsers = 0;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Server Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in binding.\n");
		exit(1);
	}
	printf("[+]Bind to port %d\n", 4444);

	if(listen(sockfd, 10) == 0){
		printf("[+]Listening....\n");
	}else{
		printf("[-]Error in binding.\n");
	}


	while(1){
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);

		if(newSocket < 0){
			exit(1);
		}

//josh
		char username[15];
		recv(newSocket, buffer, 1024, 0);
		strcpy(username, buffer);
		printf("User: %s\n", username);

		printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
		struct UserSocket newUser;
		strcpy(newUser.name, username);
		newUser.sockNumber = newSocket;
		users[cantUsers] = newUser;
		cantUsers++;

		//pipe
		int usersPipe[2];
		int p[2];
		if(pipe(usersPipe)<0)
			exit(1);
		if(pipe(p)<0)
			exit(1);

		if((childpid = fork()) != 0){
			close(sockfd);

			//read(usersPipe[0],buffer, 1024);
			//read(p[0], users, 100);
			
			//close(usersPipe[0]);
			write(usersPipe[1], buffer, 1024);
			write(p[1], users, 100);

			while(1){
				recv(newSocket, buffer, 1024, 0);
				//write(usersPipe[1], buffer, 1024);
				//read(usersPipe[0],buffer, 1024);
				
				if(strcmp(buffer, ":exit") == 0){
					printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					break;
				}else{
					char sendUser[15];
					strcpy(sendUser, buffer+15);
					printf("%s\n", sendUser);

					int sendSocket = searchUser(users, cantUsers, sendUser);
					printf("newSocket%d\n", newSocket);
					printf("sendsocket%d\n", sendSocket);

					if(sendSocket != -1){
						printf("%dClient: %s\n", newSocket, buffer);
						printf("%dClient: %s\n", newSocket, buffer+15);
						printf("%dClient: %s\n", newSocket, buffer+30);
						send(sendSocket, buffer, strlen(buffer), 0); //dksflsdfjsdjfls
					}else{
						strcpy(buffer, "El usuario no esta conectado.\0");
						send(newSocket, buffer, strlen(buffer), 0);
					}
					bzero(buffer, sizeof(buffer));
				}
			}
		}
		else{
			//close(usersPipe[1]);
			read(usersPipe[0],buffer, 1024);
			read(p[0], users, 100);
			printf("%s\n", buffer);
			printf("%s\n", buffer);
		}

	}

	close(newSocket);


	return 0;
}
