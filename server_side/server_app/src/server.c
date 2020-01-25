#include<stdio.h>
#include<stdlib.h>
#include<string.h>    
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>    
#include<semaphore.h>
#include<pthread.h>
#include<time.h>
#include <fcntl.h>


#define DEFAULT_PORT   27015
#define PLAYER_ORDER_PORT DEFAULT_PORT + 1
#define DEFAULT_PORT_RESULT 27012
#define SERVER_NUMBER 2

#define NUMBER_OF_CLIENTS 2
#define NUMBER_OF_BOTS 0 
#define NUMBER_OF_PLAYERS NUMBER_OF_CLIENTS + NUMBER_OF_BOTS

#define DEFAULT_BUFLEN 600

char gameFlag = 0;
char stopBroadcastFlag = 0;
char endGameFlag = 0;
char fillWordFlag = 1;
char sendScoreFlag = 0;

int connectedPlayers = 0;
int nextPlayer = -1;
int totalPoints = 0;
int numberInBase;

char toWrite[DEFAULT_BUFLEN];

char flagWrite[NUMBER_OF_CLIENTS]; 

pthread_mutex_t mutex;
pthread_mutex_t broadCastMutex;
sem_t semaphore;

pthread_t broadcast;
pthread_t game[NUMBER_OF_CLIENTS];
pthread_t client[NUMBER_OF_CLIENTS];
pthread_t nodeThread;

typedef struct TcpInfoStruct
{
    int number;
    int sock;
}TcpInfo;

int ReadBase(char *message, int size)
{
    int i = 0, n = 0, flagExist = 0;
    char temp[30];

    FILE *file = fopen("baza.txt" , "r");

    if(file == NULL)
    {
        printf("Unable to locate file.\n");
        fflush(stdout);
    }

    while(fscanf(file, "%s", temp) > 0)
    {
        for(i = 0; i < strlen(temp)-1; i++)
        {
            if(size == strlen(temp))
            {
                if(message[i] == temp[i])
                {
                    flagExist = 1;
                }
                else
                {
                    flagExist = 0;
                    break;
                }
            }
            else
            {
                break;
            }
        }

        if(flagExist)
        {
            fclose(file);
            return 1;
        }
    
    }
    
    fclose(file);
    return 0;
}

int WriteBase(char *message, int size)
{
    if(!ReadBase(message, size))
    {
        FILE *file = fopen("baza.txt", "a");

        if(file == NULL)
        {
            printf("Unable to locate file.\n");
			return 1;
        }
        int i = 0;
        char temp[size];

        for(i = 0; i<size; i++)
        {
        	temp[i] = message[i];
        }

        fprintf(file, "%s\n", temp);
        fclose(file);
        return 0;
    }

    return 1;
}

void* Login(void *param)
{
    TcpInfo *temp = (TcpInfo*)param;
    int i, readSize = 0, threadNumber = temp->number, sock = temp->sock;
    char serverMessage[] = "0\0";
    char clientMessage[DEFAULT_BUFLEN];
    char username[11], repeat = 0;

    sem_post(&semaphore);

    while(!repeat)
    {
        if((readSize = recv(sock, clientMessage, DEFAULT_BUFLEN, 0)) > 0)
        {
            clientMessage[readSize] = "\0";

            if(clientMessage[readSize - 1] == '1')
            {
                pthread_mutex_lock(&mutex);
                
                if(ReadBase(clientMessage, readSize))
                {
                    printf("Login successfull\n");
                    fflush(stdout);
                    serverMessage[0] = '1';
                }
                else
                    printf("Login failed");
                
                pthread_mutex_unlock(&mutex);
            }
            else
            {
                pthread_mutex_lock(&mutex);

                if(!WriteBase(clientMessage, readSize))
                {
                    printf("Register successfull");
                    fflush(stdout);
                    serverMessage[0] = '1';
                }
                else
                    printf("Register failed");
             
                pthread_mutex_unlock(&mutex);
            }

            if(serverMessage[0] == '1')
            {
                repeat = 1;
                
                for(i = 0; i<readSize; i++)
                {
                	if(clientMessage[i] == '_')
                	{
                		break;
                	}
                    username[i] = clientMessage[i];
                }
            }
        
            if(send(sock, serverMessage, strlen(serverMessage), 0) < 0)
            {
                printf("Failed to send login/register message");
                fflush(stdout);
            }
        }
    }
}
void* WriteToNode(void* param)
{
	int fileDesc;
	while(1)
	{
		pthread_mutex_lock(&mutex);
		if(flagWrite[0] && flagWrite[1] /*&& flagWrite[2] && flagWrite[3]*/)
		{
			fileDesc = open("/dev/fake_input", O_RDWR);
			write(fileDesc, toWrite, DEFAULT_BUFLEN);
    	    close(fileDesc);
    	    flagWrite[0] = 0;
    	    flagWrite[1] = 0;
    	    //flagWrite[2] = 0;
    	    //flagWrite[3] = 0;
		}
		if(endGameFlag)
		{
			pthread_mutex_unlock(&mutex);
			break;
		}
		pthread_mutex_unlock(&mutex);
	}
}

void* Game(void* param)
{
    TcpInfo *temp = (TcpInfo*)param;
    int i, readSize, threadNumber = temp->number, sock = temp->sock;

    sem_post(&semaphore);

    char client_message[11];

    int j = 0;
    int k = 0;

    int command[DEFAULT_BUFLEN];

    int mushroom_x = 0;
    int mushroom_y = 0;
    int button_Z = 0;
    int button_C = 0;

    char c_mushroom_x[4];     
    char c_mushroom_y[4];
    char c_button_Z;
    char c_button_C;

    while(1)
    {
        client_message[0] = 'A';
        if((readSize = recv(sock , client_message , 10, 0)) < 0)
        {
            printf("Failed to get data from client number %d\n", threadNumber);
        }
        else
   	    {    
   	    	j = 0;
            k = 0;

            client_message[readSize] = '\0';

//            printf("%s\n", client_message);
//            fflush(stdout);

            while(client_message[j] != '|')
            {
                c_mushroom_x[j] = client_message[j];
                j++;
            }
            
            c_mushroom_x[j] = '\0';
            j++;

            while(client_message[j] != '|')
            {
                c_mushroom_y[k] = client_message[j];
                k++; j++;
            }

            c_mushroom_y[k] = '\0';
            k = 0;
            j++;

            c_button_C = client_message[j];
            j++;
            c_button_Z = client_message[j];

            mushroom_x = atoi(c_mushroom_x);
            mushroom_y = atoi(c_mushroom_y);
            button_C = (int)c_button_C;
            button_Z = (int)c_button_Z;

            printf("MX %d MY %d C %d Z %d\n", mushroom_x, mushroom_y, button_C, button_Z);
            fflush(stdout);
	if(threadNumber == 0)
	{
      		if(mushroom_x < 90) //40
       		{
       			toWrite[4*threadNumber] = 'A';
       		}
            else
            {
                toWrite[4*threadNumber] = '0';
            }
        	if(mushroom_x > 190) //220
        	{
       			toWrite[4*threadNumber+1] = 'D';
       		}
            else
            {
                toWrite[4*threadNumber+1] = '0';
            }
        	if(mushroom_y > 190)
        	{
        		toWrite[4*threadNumber+2] = 'W';
        	}
            else
            {
                toWrite[4*threadNumber+2] = '0';
            }
        	if(mushroom_y < 90)
        	{
        		toWrite[4*threadNumber+3] = 'S';
        	}
            else
            {
                toWrite[4*threadNumber+3] = '0';
            }
            if(button_C == 49)
            {
                toWrite[8] = 'Q';
            }
            else
            {
                toWrite[8] = '0';
            }
            if(button_Z == 49)
            {
                toWrite[9] = 'E';
            }
            else
            {
                toWrite[9] = '0';
            }
	}
	if(threadNumber == 1)
	{
      		if(mushroom_x < 90)
       		{
       			toWrite[4*threadNumber] = 'J';
       		}
            	else
            	{
                toWrite[4*threadNumber] = '0';
            	}
        	if(mushroom_x > 190)
        	{
       			toWrite[4*threadNumber+1] = 'L';
       		}
            	else
            	{
                	toWrite[4*threadNumber+1] = '0';
            	}
        	if(mushroom_y > 190)
        	{
        		toWrite[4*threadNumber+2] = 'I';
        	}
            	else
            	{
               		toWrite[4*threadNumber+2] = '0';
            	}
        	if(mushroom_y < 90)
        	{
        		toWrite[4*threadNumber+3] = 'K';
        	}
            	else
            	{
                	toWrite[4*threadNumber+3] = '0';
            	}
            	if(button_C == 49)
            	{
                	toWrite[10] = 'O';
            	}
            	else
            	{
                	toWrite[10] = '0';
            	}
            	if(button_Z == 49)
            	{
                	toWrite[11] = 'P';
            	}
            	else
            	{
                	toWrite[11] = '0';
            	}
	}
            pthread_mutex_lock(&mutex);
            flagWrite[threadNumber] = 1;
            pthread_mutex_unlock(&mutex);
        }
    }
}

void* Broadcast(void* param)
{
    struct sockaddr_in udp; 
    int broadcastSock, size = sizeof(udp); 
    char message[] = "SERVER\n\0";
    int broadcast_enable = 1;

    broadcastSock = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
    
    if(broadcastSock == -1)
    {
        printf("Could not create UDP broadcast socket\n");
        fflush(stdout);
        return -1;
    }
    
    udp.sin_addr.s_addr = inet_addr("10.81.30.255"); 
    udp.sin_family = AF_INET;
    udp.sin_port = htons(DEFAULT_PORT);

    if(bind(broadcastSock, (struct sockaddr *)&udp, sizeof(udp)) < 0)
    {
        perror("Failed to bind udp socket.\n");
        fflush(stdout);
        return 1;
    }

    setsockopt(broadcastSock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)); 
   
    
    time_t start = clock();
    time_t end;
    while(1)
    {
        end = clock();
        if((end-start)/CLOCKS_PER_SEC >= 1)
        {
            start = clock();
            sendto(broadcastSock , message, strlen(message), 0, (struct sockaddr *)&udp, size);
            //printf("udpBeacon ticked...broadcast sent.\n");
        }

        pthread_mutex_lock(&broadCastMutex);

        if(stopBroadcastFlag)
        {
           break;
        }

        pthread_mutex_unlock(&broadCastMutex);

    }
    printf("Broadcast stoped...\n");
}

int main(int argc , char *argv[]) 
{
    int sock, clientSock[5] , c, i;
    struct sockaddr_in server; 
    
    srand(time(0));

    numberInBase = rand() % 10;
   
    TcpInfo temp; 
    
    sem_init(&semaphore, 0, 1); 
                               
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&broadCastMutex, NULL);
 
    sock = socket(AF_INET , SOCK_STREAM , 0); 
    if (sock == -1)
    {
        printf("Could not create socket\n");
        fflush(stdout);
        return -1;
    }
    printf("Socket created\n");
    fflush(stdout);


    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);

    pthread_create(&broadcast, NULL, Broadcast, NULL);

    pthread_create(&nodeThread, NULL, WriteToNode, NULL);

    if( bind(sock,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("bind TCP1 failed. Error\n");
        fflush(stdout);
        return 1;
    }
    printf("bind TCP done\n");
    fflush(stdout);

    listen(sock , NUMBER_OF_CLIENTS+2); 

    printf("Waiting for incoming connections...\n");
    c = sizeof(struct sockaddr_in);
    i = 0;
    while(clientSock[i] = accept(sock, (struct sockaddr *)(client+i), (socklen_t*)&c))
	{
    	if (clientSock[i] < 0)
    	{
        	printf("accept failed");
            fflush(stdout);
        	return 1;
    	}
    	printf("Connection with client:%d established.\n", i);
        fflush(stdout);
    	sem_wait(&semaphore);
    	temp.number = i;
    	temp.sock = clientSock[i]; 
		pthread_create(&client[i], NULL, Login, (void*)&temp);
        sem_wait(&semaphore);
        pthread_create(&game[i], NULL, Game, (void*)&temp);
		i++;
        connectedPlayers++;

		if(i == NUMBER_OF_CLIENTS) 
		{
            pthread_mutex_lock(&broadCastMutex);
            stopBroadcastFlag = 1;
            pthread_mutex_unlock(&broadCastMutex);
            break;
        }
	}

    pthread_join(broadcast, NULL);

    size_t start = clock();
    size_t end;

    pthread_mutex_lock(&mutex);
    endGameFlag = 1;
    pthread_mutex_unlock(&mutex);
	
	for(i = 0;i<i;i++)
	{
		pthread_join(client[i], NULL);
        pthread_join(game[i], NULL);
   //     pthread_join(hClient_result[l], NULL);
    }

	pthread_join(nodeThread, NULL);
 
	int closed = 0;
	for(i = 0;i<NUMBER_OF_CLIENTS;i++)
	{
		if(close(clientSock[i]) == 0) 
		{
			closed++; 
		}
	}
	if(closed == i) 
	{
		printf("Connections with all clients have been successfully terminated.\n");
        fflush(stdout);
	}
	else if(closed > 0)
	{
		perror("Failed to close some sockets!\n");
        fflush(stdout);
	}
	else
	{
		perror("Failed to close all sockets!\n");
        fflush(stdout);
    }
	sem_destroy(&semaphore);
	pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&broadCastMutex);
    return 0;
}


