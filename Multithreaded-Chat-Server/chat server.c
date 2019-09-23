#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/select.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100


typedef struct{
    int sock;
    char * name;
    int alive;
}client_t;

client_t * active[100];
int current = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


static int compare(const void *a, const void *b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

void * newclient(void * arg){
    client_t* client = (client_t*) arg;
    int newsock = client->sock;
    char * name = client->name;
    unsigned short tid = (unsigned short) pthread_self();
    int n = 0;
    int k = 0;
    k++;
    do{
      char buffer[BUFFER_SIZE];
      n = recv(newsock, buffer, BUFFER_SIZE, 0);
     
      if (n<0){
        continue;     
      }

      else{
        buffer[n] = '\0';
        // convert char array to char *
        char * full_command = (char*) calloc(n+1, sizeof(char));
        memcpy(full_command,buffer,n+1);

        // initialize needed variables
        char * userid = (char*) calloc(20, sizeof(char));
        int msglen= 0;
        char * tmp = (char*) calloc(100, sizeof(char));
        
        // convert the found pointer to the indexx # of the first character after \n.. or +1 to store that \0
        char * ptr;
        ptr = strchr(buffer, '\n');
        int location = 0;
        while (ptr != NULL){
          location = ptr-buffer+1;
          break;
        }

        // initialize command string
        char command[location];
        int count = 0;
        for(count = 0; count < location - 1; count++){
          command[count] = buffer[count];
        }
        command[location] = '\0';

        // at this point the command string will include all the way up to the \n character, but NOT including the \n
        k = sscanf(full_command, "%s %s", command, tmp); 

        if (strcmp(command, "LOGIN") == 0){
          // initialize active
          k = sscanf(full_command, "LOGIN %s\n", userid);
          printf("CHILD <%u>: Rcvd LOGIN request for userid %s\n", tid, userid);
          tmp = (char*) calloc(70, sizeof(char));
          // check if valid string
          if (strlen(userid) <4 || strlen(userid) >16){
              tmp = "ERROR Invalid userid\n";
              printf("CHILD <%u>: Sent ERROR (Invalid userid)\n", tid);
              k = send( newsock, tmp, strlen(tmp), 0 );
              continue;
          }

          int exist = 0;
          for (int i = 0; i<100; i++){
            if (active[i] != NULL && (active[i]->alive == 1)){
                if (strcmp(((active[i])->name), userid)==0){
                   exist = 1;
              }
            }
          }
         if (exist == 1){
          tmp = "ERROR Already connected\n";
          k = send( newsock, tmp, strlen(tmp), 0 );
          printf("CHILD <%u>: Sent ERROR (Already connected)\n", tid);
         }
         else{
          fflush( NULL );
          tmp = "OK!\n";
          k = send( newsock, tmp, strlen(tmp), 0 );
          
          pthread_mutex_lock(&mutex);

          int relogin = 0;
          for (int i = 0; i<100; i++){
            if (active[i] != NULL && (active[i])->alive == 0){
                if (strcmp(((active[i])->name), userid)==0){
                   relogin = 1;
                   (active[i])->alive = 1;
                   client->alive = 1;
                   name = userid;
              }
            }
          }
          pthread_mutex_unlock(&mutex);   
          
          if (relogin == 0){
            pthread_mutex_lock(&mutex);
            name = userid;
            client_t* new_client = (client_t*)malloc(sizeof(client_t));
            new_client->sock = newsock;
            new_client->name = userid;
            new_client->alive = 1;
            active[current] = new_client;
            current++;
            /*
            for (int i = 0; i<100; i++){
               if (active[i] != NULL && active[i]->alive == 1){
                printf("NOW WE HAVE: %s\n", active[i]->name);
           }
          }*/
           pthread_mutex_unlock(&mutex);
          }
          

         }

        }
        else if (strcmp(command, "WHO") == 0){
          tmp = (char*) calloc(1024, sizeof(char));
          tmp = "OK!\n";
          printf("CHILD <%u>: Rcvd WHO request\n", tid);
          

          pthread_mutex_lock(&mutex);
        
          int number=0;
           for (int i = 0; i<100; i++){
               if (active[i] != NULL && active[i]->alive == 1 ){
                number +=1;
           }
          }

          char** arr = (char**) calloc(number+1, sizeof(char*));
          int i = 0;

          int offset = 0;
          for (i = 0; i<100; i++){
            if ( active[i] != NULL && (active[i])->alive == 1){
                   arr[offset] = (active[i])->name;
                   offset++;
            }
          }

          qsort(arr, number, sizeof(const char*), compare);
          char * final = (char*) calloc(1000, sizeof(char));
          strcat(final, tmp);

          for (i=0; i<number; i++){
            strcat(final, arr[i]);
            strcat(final, "\n");
          }

          pthread_mutex_unlock(&mutex);
          k = send( newsock, final, strlen(final), 0 );
          continue;

        }
        else if (strcmp(command, "LOGOUT") == 0){
          tmp = (char*) calloc(1024, sizeof(char));
          tmp = "OK!\n";
          k = send( newsock, tmp, strlen(tmp), 0 );
          printf("CHILD <%u>: Rcvd LOGOUT request\n", tid);
          pthread_mutex_lock(&mutex);
          for (int i = 0; i<100; i++){
            if (active[i] != NULL && (active[i])->alive == 1){
                if (strcmp(((active[i])->name), name)==0){
                   (active[i])->alive = 0;
              }
           }
          }
          pthread_mutex_unlock(&mutex);
          continue;
        }
        else if (strcmp(command, "SEND") == 0){
         tmp = (char*) calloc(70, sizeof(char));
        //  int args = 0;
          k = sscanf(full_command, "SEND %s %d\n", userid, &msglen);
         // if (k == 2) { args+=2;}
          if (k == 1){
              tmp = "ERROR Invalid SEND format\n";
              printf("CHILD <%u>: Sent ERROR (Invalid SEND format)\n", tid);
              k = send( newsock, tmp, strlen(tmp), 0 );
              continue;
          }
          char * message = (char*) calloc(1500, sizeof(char));
          int index=0;
          for (int i = 0; i<strlen(full_command); i++){
            if (buffer[i] == '\n'){
              index = i;
              break;
            }
          }
          for (int i = index+1; i< strlen(full_command); i++){
            //args++;
            char str[2];
            str[0] = buffer[i];
            str[1] = '\0';
            strcat(message, str);
          }
          printf("CHILD <%u>: Rcvd SEND request to userid %s\n", tid, userid);
          if (msglen <1 || msglen > 990){
              tmp = "ERROR Invalid msglen\n";
              printf("CHILD <%u>: Sent ERROR (Invalid msglen)\n", tid);
              k = send( newsock, tmp, strlen(tmp), 0 );
              continue;
          }
          /*if (k != 3){
              tmp = "ERROR Invalid SEND format\n";
              printf("CHILD <%u>: Sent ERROR (Invalid SEND format)\n", tid);
              k = send( newsock, tmp, strlen(tmp), 0 );
              continue;
          }
          */
          /*
            if (args<3){
              tmp = "ERROR Invalid SEND format\n";
              printf("CHILD <%u>: Sent ERROR (Invalid SEND format)\n", tid);
              k = send( newsock, tmp, strlen(tmp), 0 );
              continue;
          }
          */

           int exist = 0;
           int rep = 0;
            for (int i = 0; i<100; i++){
            if (active[i] != NULL){
                if ((strcmp(((active[i])->name), userid)==0) && ((active[i])->alive == 1)){
                   exist = 1;
                   rep = (active[i])->sock;
              }
           } 
          }
          if (exist == 0){
              tmp = "ERROR Unknown userid\n";
              printf("CHILD <%u>: Sent ERROR (Unknown userid)\n", tid);
              k = send( newsock, tmp, strlen(tmp), 0 );
              continue;
          }   
          else{
            fflush( NULL );
            tmp = "OK!\n";
            k = send( newsock, tmp, strlen(tmp), 0 );
            char * final = (char*)calloc(1500, sizeof(char));
            strcpy(final, "FROM ");
            strcat(final, name);
            strcat(final, " ");
            char hey[10];
            sprintf(hey, "%d", msglen);
            strcat(final, hey);
            strcat(final, " ");
            strcat(final, message);
            k = send( rep, final, strlen(final), 0 );
          } 
          continue;    
        }
        else if (strcmp(command, "BROADCAST") == 0){
      //  int args = 0;
          printf("CHILD <%u>: Rcvd BROADCAST request\n", tid);
          k = sscanf(full_command, "BROADCAST %d\n", &msglen);
         // if (k ==1){args++;}

          char * message = (char*) calloc(1500, sizeof(char));
          int index=0;
          for (int i = 0; i<strlen(full_command); i++){
            if (buffer[i] == '\n'){
              index = i;
              break;
            }
          }
          for (int i = index+1; i< strlen(full_command); i++){
            char str[2];
            str[0] = buffer[i];
            str[1] = '\0';
        //    args ++;
            strcat(message, str);
          }

         tmp = (char*) calloc(70, sizeof(char));
        /*
        if (k != 2){
              tmp = "ERROR Invalid BROADCAST format\n";
              printf("CHILD <%u>: Sent ERROR (Invalid BROADCAST format)\n", tid);
              k = send( newsock, tmp, strlen(tmp), 0 );
              continue;
          }   
          */      
         if (msglen <1 || msglen > 990){
              tmp = "ERROR Invalid msglen\n";
              printf("CHILD <%u>: Sent ERROR (Invalid msglen)\n", tid);
              k = send( newsock, tmp, strlen(tmp), 0 );
              continue;
          }
  /*
          if (args < 2){
              tmp = "ERROR Invalid SEND format\n";
              printf("CHILD <%u>: Sent ERROR (Invalid SEND format)\n", tid);
              k = send( newsock, tmp, strlen(tmp), 0 );
              continue;
          }
          */
          fflush( NULL );
          tmp = "OK!\n";
          k = send( newsock, tmp, strlen(tmp), 0 );
          char *final = (char*) calloc(1500, sizeof(char)) ;
          strcpy(final, "FROM ");
          strcat(final, name);
          strcat(final, " ");
          char hey[10];
          sprintf(hey, "%d", msglen);
          strcat(final, hey);
          strcat(final, " ");
          strcat(final, message);

          pthread_mutex_lock(&mutex);
          int arr[100];
          int i = 0;
          for (i = 0;i<100;i++){
            arr[i] = -1;
          }
          int offset = 0;
          for (i = 0; i<100; i++){
            if ( active[i] != NULL && (active[i])->alive == 1){
                   arr[offset] = (active[i])->sock;
                   offset++;
            }
          }
          
          for (i = 0; i<100; i++){
            if (arr[i] == -1) {break;}
            k = send(arr[i], final, strlen(final), 0 );
          }
          pthread_mutex_unlock(&mutex);  
          continue;
        }
      }
    }while (n>0);
    printf("CHILD <%u>: Client disconnected\n", tid);
    close(newsock);
    return NULL;
}


int main(int argc, char** argv)
{
  setvbuf( stdout, NULL, _IONBF, 0 );
  if (argc != 2){
    fprintf(stderr,"ERROR: Invalid argument(s)\n");
    return EXIT_FAILURE;
  }

  unsigned short port = atoi(argv[1]);
  /* Create the listener socket as TCP socket (SOCK_STREAM) */
  int tcp_socket = socket( PF_INET, SOCK_STREAM, 0 );
  int udp_socket = socket( AF_INET, SOCK_DGRAM, 0 );
    /* here, the sd is a socket descriptor (part of the fd table) */
  
  pthread_t tid;
  fd_set rset;
  int ready =0;
  ready ++;
  int o=0;
  for (o = 0; o<100; o++){
      active[o] = NULL;
  }
  /* socket structures */
  struct sockaddr_in server;
  server.sin_family = PF_INET;  /* AF_INET */
  server.sin_addr.s_addr = htonl( INADDR_ANY );
    /* allow any IP address to connect */
  /* htons() is host-to-network short for data marshalling */
  /* Internet is big endian; Intel is little endian */
  server.sin_port = htons( port );
  int len = sizeof( server );

  /* attempt to bind (or associate) port 8123 with the socket */
  if ( bind( tcp_socket, (struct sockaddr *)&server, len ) == -1 )
  {
    perror( "bind() failed" );
    return EXIT_FAILURE;
  }
  if ( bind( udp_socket, (struct sockaddr *)&server, len ) == -1 )
  {
    perror( "bind() failed" );
    return EXIT_FAILURE;
  }

  printf("MAIN: Started server\n");
  printf("MAIN: Listening for TCP connections on port: \n");
  printf("MAIN: Listening for UDP datagrams on port: \n");

  /* identify this port as a TCP listener port */
  if ( listen( tcp_socket, 5 ) == -1 )
  {
    perror( "listen() failed" );
    return EXIT_FAILURE;
  }

  struct sockaddr_in client;
  int fromlen = sizeof( client );
  int n=0;
  n++;
  

  while ( 1 )
  {
    FD_ZERO(&rset);
    FD_SET(tcp_socket, &rset);
    FD_SET(udp_socket, &rset);

    ready = select(FD_SETSIZE, &rset, NULL, NULL, NULL);

    if (FD_ISSET(tcp_socket, &rset)){
      printf( "MAIN: Rcvd incoming TCP connection from\n");
      int newsd = accept( tcp_socket, (struct sockaddr *)&client, (socklen_t *)&fromlen );
      client_t * client_arg = (client_t *)malloc(sizeof(client_t));
      client_arg->sock = newsd;
      client_arg->name = (char *)calloc(17, sizeof(char));
      client_arg->alive = 0;

      n = pthread_create(&tid, NULL, newclient, (void *)client_arg);
    }

    else if (FD_ISSET(udp_socket, &rset)){
      printf( "MAIN: Rcvd incoming UDP datagram from\n");
      char buffer[BUFFER_SIZE];
      n = recvfrom( udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &client,(socklen_t *) &len );
      int k = 0;
      k++;
      if (n<0){
        printf( "recv() failed\n" );      
      }

      else{
          buffer[n] = '\0';
        // convert char array to char *
          char * full_command = (char*) calloc(n+1, sizeof(char));
          memcpy(full_command,buffer,n+1);

        // initialize needed variables
          int msglen= 0;
          char * tmp = (char*) calloc(100, sizeof(char));
        // convert the found pointer to the indexx # of the first character after \n.. or +1 to store that \0
          char * ptr;
          ptr = strchr(buffer, '\n');
          int location = 0;
          while (ptr != NULL){
            location = ptr-buffer+1;
            break;
          }

        // initialize command string
          char command[location];
          int count = 0;
          for(count = 0; count < location - 1; count++){
            command[count] = buffer[count];
          }
          command[location] = '\0';

        // at this point the command string will include all the way up to the \n character, but NOT including the \n
          k = sscanf(full_command, "%s %s", command, tmp); 

          if (strcmp(command, "WHO")==0){
             tmp = (char*) calloc(1024, sizeof(char));
             tmp = "OK!\n";
             printf("MAIN: Rcvd WHO request\n");
          

            pthread_mutex_lock(&mutex);
        
            int number=0;
            for (int i = 0; i<100; i++){
                if (active[i] != NULL && active[i]->alive == 1 ){
                  number +=1;
            }
            }

            char** arr = (char**) calloc(number+1, sizeof(char*));
            int i = 0;

            int offset = 0;
            for (i = 0; i<100; i++){
              if ( active[i] != NULL && (active[i])->alive == 1){
                    arr[offset] = (active[i])->name;
                    offset++;
              }
            }

            qsort(arr, number, sizeof(const char*), compare);
            char * final = (char*) calloc(1000, sizeof(char));
            strcat(final, tmp);

            for (i=0; i<number; i++){
              strcat(final, arr[i]);
              strcat(final, "\n");
            }

            pthread_mutex_unlock(&mutex);
            sendto( udp_socket, final, strlen(final), 0, (struct sockaddr *) &client, len);
            continue;
          }

          else if(strcmp(command, "BROADCAST")){
        //  int args =0;
          k = sscanf(full_command, "BROADCAST %d\n", &msglen);
       //   if (k ==1){args++;}
          char * message = (char*) calloc(1500, sizeof(char));
          int index=0;
          for (int i = 0; i<strlen(full_command); i++){
            if (buffer[i] == '\n'){
              index = i;
              break;
            }
          }
          for (int i = index+1; i< strlen(full_command); i++){
            char str[2];
            str[0] = buffer[i];
            str[1] = '\0';
          //  args ++;
            strcat(message, str);
          }           

            printf("MAIN: Rcvd BROADCAST request\n");
            tmp = (char*) calloc(1500, sizeof(char));
        
          /*
            if (k != 2){
              tmp = "ERROR Invalid BROADCAST format\n";
              printf("CHILD <%u>: Sent ERROR (Invalid BROADCAST format)\n", tid);
              k = send( newsock, tmp, strlen(tmp), 0 );
              continue;
            }  
            */       
            if (msglen <1 || msglen > 990){
              tmp = "ERROR Invalid msglen\n";
              printf("MAIN: Sent ERROR (Invalid msglen)\n");
              sendto( udp_socket, tmp, strlen(tmp), 0, (struct sockaddr *) &client, len);
              continue;
            }
            /*
            if (args < 2){
              tmp = "ERROR Invalid SEND format\n";
              printf("MAIN: Sent ERROR (Invalid SEND format)\n");
              sendto( udp_socket, tmp, strlen(tmp), 0, (struct sockaddr *) &client, len);
              continue;
            }*/
        
            tmp = "OK!\n";
            k = send( udp_socket, tmp, strlen(tmp), 0 );
            char *final = (char*) calloc(1500, sizeof(char)) ;
            strcpy(final, "FROM ");
            strcat(final, "UDP-client ");
            char hey[10];
            sprintf(hey, "%d", msglen);
            strcat(final, hey);
            strcat(final, " ");
            strcat(final, message);
            pthread_mutex_lock(&mutex);
            int arr[100];
            int i = 0;
            for (i = 0;i<100;i++){
              arr[i] = -1;
            }
            int offset = 0;
            for (i = 0; i<100; i++){
              if ( active[i] != NULL && (active[i])->alive == 1){
                  arr[offset] = (active[i])->sock;
                  offset++;
              }
            }
          
            for (i = 0; i<100; i++){
              if (arr[i] == -1) {break;
              }
              k = sendto(arr[i], final, strlen(final), 0, (struct sockaddr *)&client, len);
           }
            pthread_mutex_unlock(&mutex);  
            continue;

          }
               else if(strcmp(command, "SEND")){
             tmp = (char*) calloc(1500, sizeof(char));
             tmp = "SEND not supported over UDP\n";
              printf("MAIN: Sent ERROR (SEND not supported over UDP)\n");
              sendto( udp_socket, tmp, strlen(tmp), 0, (struct sockaddr *) &client, len);
              continue;

          }
      }
    }
}
  close(tcp_socket);
  close(udp_socket);
  return EXIT_SUCCESS;
}

