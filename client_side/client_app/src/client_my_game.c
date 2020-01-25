#include <stdio.h>      
#include <string.h>     
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <fcntl.h>     
#include <unistd.h>    
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT   27015
#define SERVER_NUMBER 2
#define DEFAULT_PORT_RESULT 27012
#define PLAYER_ORDER_PORT DEFAULT_PORT + 1
#define SEARCHING_TIME 5

char serverMessage[] = "SERVER\n\0";

char loginFlag = 0;
char searchingFlag = 0;
char endGameFlag = 0;
char gameFlag = 0;

int newServer = 0;
char order;

pthread_t findServers;
pthread_t playGame;
pthread_t getOrder;

pthread_mutex_t mutex;

struct svr
{
    struct in_addr sin_addr;
    unsigned short port;
};

typedef struct SearchServer   
{
	struct sockaddr_in addr;
    int size;
	int socket;
}searchServer;

struct svr serverList[SERVER_NUMBER];

char Equal(char* message)
{
    int i;

    for(i = 0; i < strlen(message); i++)
        if(message[i] != serverMessage[i])
            return 0;

    return 1;
}

char Exist(unsigned long addr, struct svr *serverList){
    int i;
    
    for(i = 0; i < SERVER_NUMBER; i++)
        if(addr == serverList[i].sin_addr.s_addr)
            return 1;

    return 0;

}

struct svr ChooseServer(struct svr *serverList){
    
    struct svr min;
    int i;

    min.sin_addr.s_addr = serverList[0].sin_addr.s_addr;
    min.port = serverList[0].port;

    for(i = 1; i < SERVER_NUMBER; i++)
        if((min.sin_addr.s_addr + min.port) > (serverList[i].sin_addr.s_addr + serverList[i].port)){
            min.sin_addr.s_addr = serverList[i].sin_addr.s_addr;
            min.port = serverList[i].port;
        }

    return min;
}

void Merge(char message[], char *username, char *password, int menuChoice){
    
    int i;
    int j;
    char c[2];

    sprintf(c, "%d", menuChoice);

    for(i = 0; i < strlen(username); i++)
        message[i] = username[i];
    
    message[strlen(username)] = '_';
    i++;
 
    for(j = 0; j < strlen(password); j++, i++)
        message[i] = password[j];
    
    message[strlen(username) + strlen(password) + 1] = '_';
    message[strlen(username) + strlen(password) + 2] = c[0];
    message[strlen(username) + strlen(password) + 3] = '\0';

}

void LogInMenu(int sock){

    int menuChoice, readSize;
    char username[11], password[11], message[22], messageServer[21];

    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nMENU\n");
    printf("1.Login\n2.Register\nChoice : ");
    scanf("%d", &menuChoice);
    
    switch(menuChoice){
        case 1:
            
            printf("Warning: Max length for username and password is 10\nUsername and password can contain only alphanumeric characters\n");
            printf("Username : ");
            scanf("%s", username);
            fflush(stdout);
            printf("Password : ");
            scanf("%s" , password);
            fflush(stdout);
            
            Merge(message, username, password, menuChoice);
            
            if(send(sock , message , strlen(message), 0) < 0)
            {
                puts("Send failed");
                return;
            }

            if((readSize = recv(sock , messageServer , 10 , 0)) > 0) 
                messageServer[readSize] = '\0';
            
            if(messageServer[0] == '1')
                loginFlag = 1;
            else 
                printf("Pogresan username ili password\n");
            break;
        
        case 2:
          
            printf("Warning: Max length for username and password is 10\nUsername and password can contain only alphanumeric characters\n");
            printf("Username : ");
            scanf("%s", username);
            fflush(stdout);
            printf("Password : ");
            scanf("%s" , password);
            fflush(stdout);
            
            Merge(message, username, password, menuChoice);

            if(send(sock , message , strlen(message), 0) < 0)
            {
                puts("Send failed");
                return;
            }

            if((readSize = recv(sock , messageServer , 10 , 0)) > 0)
                messageServer[readSize] = '\0';
            
            if(messageServer[0] == '1')
                loginFlag = 1;
            else 
                printf("User vec postoji\n");
            break;
        
        default:
            printf("Invalid input :( try again :)\n");
    }    
}

void* SearchServers(void *param)
{
    searchServer *t = (searchServer*) param;
    struct sockaddr_in udpBroadcast = t->addr;
    int udpSockBroadcast = t->socket;
    int size = t->size;
    int readSize, numberOfServers, i = 0;
    char udpMessage[10];

    while((readSize = recvfrom(udpSockBroadcast , udpMessage , 10 , 0, (struct sockaddr*)&udpBroadcast, &size)) > 0 )
    {     
        udpMessage[readSize] = '\0';
            fflush(stdout);

        if(Equal(udpMessage))
        {
            if(!Exist(udpBroadcast.sin_addr.s_addr, serverList))
            {
                serverList[i].sin_addr.s_addr = udpBroadcast.sin_addr.s_addr;
                serverList[i].port = udpBroadcast.sin_port;
                numberOfServers++;
                i++;            
            }
        }
        if(!searchingFlag)
            break;
    }
}

void* Game(void *param)
{
    int readSize, *tcpSockGame = (int*) param;
    char character, serverMessage[2], clientMessage[2], usedCharacters[100], resultMessage[100], winners[45];
    char clientId;

    if((readSize = recv(*tcpSockGame , serverMessage, 10 , 0)) > 0)
        clientId = serverMessage[0];

    gameFlag = 1;

    system("@cls||clear");
    printf("Game started");

    while(gameFlag)
    {
        while(clientId != order);

        printf("Unesite karakter : ");
        scanf(" %c", &character);
        clientMessage[0] = character;
        clientMessage[1] = '\0';

        if(send(*tcpSockGame , clientMessage , 1, 0) < 0)
        {
                puts("Send failed");
                return -1;
        }

        if((readSize = recv(*tcpSockGame , serverMessage, 1 , 0)) > 0)
                clientMessage[readSize] = '\0';

        if((readSize = recv(*tcpSockGame , usedCharacters, 1 , 0)) > 0)
                usedCharacters[readSize] = '\0';
        
        if((readSize = recv(*tcpSockGame , resultMessage, 1 , 0)) > 0)
                resultMessage[readSize] = '\0';
        
        system("@cls||clear");

        printf("%s" , usedCharacters);
        printf("%s" , resultMessage);

        if(serverMessage[0] == '1')
            gameFlag = 0;
    }

    if((readSize = recv(*tcpSockGame , winners, 1 , 0)) > 0)
                winners[readSize] = '\0';

    printf("%s", winners);
    printf("End game");
}

void* PlayerOrder(void *param)
{
    int sock, readSize;
    struct sockaddr_in udp;
    char message[2];
    
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock == -1)
    {
        printf("Could not create UDP socket\n");
        fflush(stdout);
        return -1;
    }

    udp.sin_family = AF_INET;
    udp.sin_addr.s_addr = INADDR_ANY;
    udp.sin_port = htons(PLAYER_ORDER_PORT);

    if(bind(sock, (struct sockaddr *)&udp, sizeof(udp)) < 0)
    {
        perror("Failed to bind udp socket.\n");
        fflush(stdout);
        return 1;
    }

    while((readSize = recvfrom(sock , message , 1 , 0, (struct sockaddr*)&udp, sizeof(udp))) > 0 )
    {     
        pthread_mutex_lock(&mutex);
        order = message[0];
        pthread_mutex_unlock(&mutex);

        if(!gameFlag)
            break;
    }

    close(sock);
}

int main(int argc, char *argv[])
{
    int tcpSockGame,tcpSockResult, udpSockBroadcast;
    struct sockaddr_in tcpGame, udpBroadcast;
    struct svr server;
    int i, size = sizeof(udpBroadcast), numberOfServers = 0;
    searchServer searchServers;

    pthread_mutex_init(&mutex, NULL);

    tcpSockGame = socket(AF_INET, SOCK_STREAM, 0);
    if(tcpSockGame == -1)
    {
        printf("Could not create TCP socket\n");
        fflush(stdout);
        return -1;
    }

    printf("TCP game socket created\n");
    fflush(stdout);

    udpSockBroadcast = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(udpSockBroadcast == -1)
    {
        printf("Could not create UDP broadcast socket\n");
        fflush(stdout);
        return -1;
    }
    
    printf("UDP broadcast socket created\n");
    fflush(stdout);

    udpBroadcast.sin_family = AF_INET;
    udpBroadcast.sin_addr.s_addr = INADDR_ANY;
    udpBroadcast.sin_port = htons(DEFAULT_PORT);

    if(bind(udpSockBroadcast, (struct sockaddr *)&udpBroadcast, sizeof(udpBroadcast)) < 0)
    {
        perror("Failed to bind udp socket.\n");
        fflush(stdout);
        return 1;
    }
    
    printf("Searching for online server....");
    fflush(stdout);
    
    searchServers.addr = udpBroadcast;
    searchServers.size = size;
    searchServers.socket = udpSockBroadcast;
    
    pthread_create(&findServers, NULL, SearchServers, (void*)&searchServers);
    pthread_join(findServers, NULL);
    sleep(SEARCHING_TIME);
    
    searchingFlag = 1;

    close(udpSockBroadcast);

    printf("Online servers\n");
    fflush(stdout);

    for(i = 0; i < numberOfServers; i++)
        printf("%d : %s:%d\n", i, inet_ntoa(serverList[i].sin_addr), serverList[i].port);

    printf("Choosing server");
    fflush(stdout);

    server = ChooseServer(serverList);

    tcpGame.sin_addr.s_addr = inet_addr(inet_ntoa(server.sin_addr)); 
    tcpGame.sin_family = AF_INET;
    tcpGame.sin_port = htons(DEFAULT_PORT);

    if (connect(tcpSockGame , (struct sockaddr *)&tcpGame , sizeof(tcpGame)) < 0)
    {
        perror("connect failed via TCP game socket\n Error!");
        fflush(stdout);
        return -1;
    }

    printf("Connected to server : %s:%d\n", inet_ntoa(server.sin_addr), server.port);
    fflush(stdout);

    while(!loginFlag)
    {
        LogInMenu(tcpSockGame);
    }

    pthread_create(&playGame, NULL, Game, (void*)&udpSockBroadcast);
    pthread_create(&getOrder, NULL, PlayerOrder, NULL);
    pthread_join(playGame, NULL);
    pthread_join(getOrder, NULL);

    pthread_mutex_destroy(&mutex);

    close(tcpSockGame);
    close(tcpSockResult);
}
