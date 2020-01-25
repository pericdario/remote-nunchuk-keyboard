#include <stdio.h>      //printf
#include <stdlib.h>
#include <string.h>     //strlen
#include <sys/stat.h>
#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr
#include <fcntl.h>     //for opens
#include <unistd.h>    //for close
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT   27015
#define SERVER_NUMBER 2
#define DEFAULT_PORT_RESULT 27012
#define SEARCHING_TIME 5

//char message
char serverMessage[] = "SERVER\n\0";

//flags
char loginFlag = 0;
char searchingFlag = 0;
char endGameFlag = 0;
char gameFlag = 0;

int numberOfServers = 0;
int newServer = 0;
char order;

//threads
pthread_t findServers;
pthread_t playGame;
pthread_t getOrder;

//mutex
pthread_mutex_t mutex;

typedef struct svr
{
    struct in_addr sin_addr;
    unsigned short port;
}ServerInfo;


ServerInfo serverList[SERVER_NUMBER];

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

ServerInfo ChooseServer(ServerInfo *serverList)
{
    int i;
    ServerInfo min;
    
    min.sin_addr.s_addr = serverList[0].sin_addr.s_addr;
    min.port = serverList[0].port;

    for(i = 1; i < numberOfServers; i++)
        if((min.sin_addr.s_addr + min.port) > (serverList[i].sin_addr.s_addr + serverList[i].port))
        {
            min.sin_addr.s_addr = serverList[i].sin_addr.s_addr;
            min.port = serverList[i].port;
        }


    printf("min : %s:%d\n", inet_ntoa(min.sin_addr), min.port);

    return min;
}

void* SearchServers(void *param)
{
    int readSize, i = 0;
    char udpMessage[10];

    int udpSockBroadcast;
    struct sockaddr_in udp;
    int size = sizeof(udp);
    

    udpSockBroadcast = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(udpSockBroadcast == -1)
    {
        printf("Could not create UDP socket\n");
    }

    udp.sin_family = AF_INET;
    udp.sin_addr.s_addr = INADDR_ANY;
    udp.sin_port = htons(DEFAULT_PORT);


    if(bind(udpSockBroadcast, (struct sockaddr *)&udp, sizeof(udp)) < 0)
    {
        perror("Failed to bind udp socket.\n");
        return 1;
    }
    puts("UDP socket bound.\n");

    while((readSize = recvfrom(udpSockBroadcast , udpMessage , 10 , 0, (struct sockaddr*)&udp, &size)) > 0 )
    {     
        udpMessage[readSize] = '\0';
        if(Equal(udpMessage))
        {
            if(!Exist(udp.sin_addr.s_addr, serverList))
            {
                serverList[i].sin_addr.s_addr = udp.sin_addr.s_addr;
                serverList[i].port = udp.sin_port;
                numberOfServers++;
                i++;            
            }
        }
        if(!searchingFlag)
            break;
    }
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
                printf("Wrong username or password\n");
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
                printf("User exists\n");
            break;
        
        default:
            printf("Invalid input :( try again :)\n");
    }    
}

void* Game(void *param)
{
    int readSize; 
    int *tcpSockGame = (int*) param;
    int sock = *tcpSockGame;
    int i, j;
    char message[24];
    char c_mushroom_x[3];
    char c_mushroom_y[3];
    char c_c_button[2];
    char c_z_button[2];

    int mushroom_x;
    int mushroom_y;
    int accelerometer_x;
    int accelerometer_y;
    int accelerometer_z;
    int c_button;
    int z_button;

    int file_desc;      
    int ret_val;
    int data_size = 0;




    while(1) 
    {

        file_desc = open("/dev/nunchuk", O_RDWR);

        if(file_desc < 0)
        {
            printf("Error, 'nunchuk' not opened\n");
            return -1;
        }

        readSize = read(file_desc, message, strlen(message));        

        mushroom_x = (int)message[0];
        mushroom_y = (int)message[1];
        accelerometer_x = (int)message[2];
        accelerometer_y = (int)message[3];
        accelerometer_z = (int)message[4];
        c_button = (int)message[5];
        z_button = (int)message[6];

        sprintf(c_mushroom_x,"%d",mushroom_x);
        sprintf(c_mushroom_y,"%d",mushroom_y);
        sprintf(c_c_button,"%d",c_button);
        sprintf(c_z_button,"%d",z_button);

        for (i = 0; i < strlen(c_mushroom_x); i++)
            message[i] = c_mushroom_x[i];

        message[i] = '|';
        i++;

        for(j = 0; j < strlen(c_mushroom_y); j++, i++)
            message[i] = c_mushroom_y[j];

        message[i] = '|';
        i++;
        message[i] = c_c_button[0];
        i++;
        message[i] = c_z_button[0];
        i++;
        message[i] = '\0';

        close(file_desc);

        printf("%s\n", message);

        if( send(sock , message , 10, 0) < 0)
        {
            puts("Send failed\n");
            return 1;
        }

        usleep(11);
    }

}

int main(int argc , char *argv[])
{
    int tcpSockGame, serverGameIndex, i;
    struct sockaddr_in tcpGame, serverByFormula;
    ServerInfo server;

    pthread_mutex_init(&mutex, NULL);

    printf("Searching for online server....\nSearching time : %d\n", SEARCHING_TIME);
    fflush(stdout);

    pthread_create(&findServers, NULL, SearchServers, NULL);
    sleep(SEARCHING_TIME);
    
    searchingFlag = 1;

    printf("Online servers\n");
    fflush(stdout);

    for(i = 0; i < numberOfServers; i++)
        printf("%d : %s:%d\n", i+1, inet_ntoa(serverList[i].sin_addr), serverList[i].port);

    printf("Choosing server by formula : min(sum(value(IPv4_address(server) + src_port(server))\n");
    fflush(stdout);

    server = ChooseServer(serverList);

    tcpSockGame = socket(AF_INET, SOCK_STREAM, 0);
    if(tcpSockGame == -1)
    {
        printf("Could not create TCP game socket\n");
        fflush(stdout);
        return -1;
    }

    printf("TCP game socket created\n");
    fflush(stdout);

    serverByFormula.sin_addr.s_addr = inet_addr(inet_ntoa(server.sin_addr));        
    serverByFormula.sin_family = AF_INET;
    serverByFormula.sin_port = htons(DEFAULT_PORT);

    printf("Chosen server by formula : %s:%d\n", inet_ntoa(serverByFormula.sin_addr), serverByFormula.sin_port);
    fflush(stdout);

    printf("Index of server with game Emulator : ");
    fflush(stdout);

    scanf("%d", &serverGameIndex);
    serverGameIndex--;

    server.sin_addr.s_addr = serverList[serverGameIndex].sin_addr.s_addr;
    server.port = serverList[serverGameIndex].port;

    tcpGame.sin_addr.s_addr = inet_addr(inet_ntoa(server.sin_addr));        
    tcpGame.sin_family = AF_INET;
    tcpGame.sin_port = htons(DEFAULT_PORT);


    printf("Connected to server : %s:%d\n", inet_ntoa(server.sin_addr), server.port);
    fflush(stdout);

    if (connect(tcpSockGame , (struct sockaddr*)&tcpGame , sizeof(tcpGame)) < 0)
    {
        perror("Connect failed via TCP game socket\n Error!");
        fflush(stdout);
        return -1;
    }

    while(!loginFlag)   
        LogInMenu(tcpSockGame);

    pthread_create(&playGame, NULL, Game, (void*)&tcpSockGame);
    pthread_join(findServers, NULL);
    pthread_join(playGame, NULL);
    pthread_mutex_destroy(&mutex);

    close(tcpSockGame);
    return 0;
}
