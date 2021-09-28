/*Task Number: Server v.2
  Description: 1. Execute the file
               2. Send 'Shutdown' button to IoL to allow client to shutdown
                  the server directly from the IoL UI.
                  - once pressed, IoL sends 'q' as a standard input to the
                    server. Server then detects 'q', closes all the sockets,
                    and terminates itself. (clients do not need to be notified)
               2. Enable the communication to the server, and make server to
                  listen for incoming connections.
                    - Maximum connections = 5. Reject if >5 and notify client.
               3. If client is connected:
                    - create a unique socket (new_sock)
                    - add client to the array called 'clients_hist_table'
                    - use unique new_sock as a name for the client in IoL UI
                      to be able to store more than 1 client
               4. Stay idle & use select() to differentiate input from new_sock
                  or standard input (0) to receive 'q' to shutdown the server.
               5. If select() = new_sock & new_sock inside 'clients_hist_table':
                    - Push socket's name to the IoL UI once recv() value detected
                      and readSocketName = 0. Make readSocketName = 1
                    - Push all the text to the IoL UI once recv() value detected
                      and readSocketName = 1
                  If select() = 0:
                    Send 'q' to the server to terminate itself & close all the
                    opened sockets

               Termination:
                 1. Admin needs to press [CTRL+C] on server side
                 2. 'Shutdown' button in IoL UI needs to be pressed
                 If 1 or 2 = True:
                   - close all opened sockets & shut down
                 If client presses [CTRL+C] on their side:
                   - socket needs to be closed, while server needs to remain
                     running.

               Server VS clients inputs:
                 - select() is utilised to detect the inputs based on incoming
                   resources.
                 - if client sends a text, select() receives new_sock. If
                   new_sock inside 'clients_hist_table', select() lets server
                   from client to read lines until EOF.
                 - if 'shutdown' is clicked, select() receives standard input
                   such as 0. select() let's server to read 'q' from 'shutdown'
                   button and perform actions based on it

               Reset of IoL UI when clients disconnect:
                 - If total number of clients connected = 0, then reset IoL UI
                 - If != 0 and client reconnected, there is an issue. This is
                   not handled yet

               Assumptions:
                 - client of v2 () is compatible with this server only
  Author: Maxim Zaika ()
  Language: C
  Start date: 13/08/2018 12:00PM
  Last modified: 05/09/2018 7:20PM (improved comments)
  How to execute from the Terminal:
               gcc server.c -o server3;iol -- ./server3 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/a2-.socket"
#define MAX_NUM_CLIENT 5

struct clientData { //used for IOL to identify unique clients
  char clientMain[1024];
  char clientBox[1024];
  char clientText[1024];
};

int sendToIOL(struct clientData clientInfo, char *CLIENT_NAME) {
  printf("<program.push"
         " <%s:box width=100,{%%} bold=false color=x1204AC background-color={white} {\n" //Main#
         "      <%s:box width=100,{%%} size=15,{px} background-color=x1204AC color=x1204AC {<span color={white} bold=true {%s}>}>\n" //Box#,CLIENT_NAME
         "      <%s:box width=100,{%%} size=15,{px} border=0,{px}>\n" //Text#
         " }>\n"
         ">"
  ,clientInfo.clientMain,clientInfo.clientBox,CLIENT_NAME,clientInfo.clientText);
}

int powerButton_onlineStatus(int *clientTotal) {
  if (*clientTotal == 0) { //whenever client count is 0, it will clear all the data, add shutdown button, and online status
    printf("<program.clear>\n");
    printf("<program.push <button background-color=xE0E0E2 {<span color={black} {Shutdown}>} onclick=<putln {Q}>>>\n"); //sets the QUIT button
    printf("<online:box border=0,{px}>\n"); //adds the online box to the IOL
    printf("<online.clear> <online.push {Users online: %d. Slots available: %d}>\n",*clientTotal,MAX_NUM_CLIENT-*clientTotal);
  } else { //whenever clients are connected, it will simply count
    printf("<online.clear> <online.push {Users online: %d. Slots available: %d}>\n",*clientTotal,MAX_NUM_CLIENT-*clientTotal);
  }
}

int checkServerSpace(int clientTotal,int new_sock,struct clientData clientInfo) {
  if (clientTotal == MAX_NUM_CLIENT) { //MAX_NUM_CLIENT = 5
    fprintf(stderr, "server: maximum client capacity reached\n");
    send(new_sock,"end",sizeof("end"),0); //notifies the client that maximum capacity has been reached
    close(new_sock); //terminates newly created socket
    return -1;
  } else {
    send(new_sock,"oke",sizeof("oke"),0); //notifies the client that there is still space on the server
    return 0;
  }
}

int readFromClient(struct clientData clientInfo, int new_sock, int *readSocketName, int randomNumber) {
  char buffer[1024];
  int clientSend = recv(new_sock,buffer,sizeof(buffer),0);

  /* define the unique name of each socket in IOL */
  snprintf(clientInfo.clientMain,20,"Main%d",randomNumber); //Main+randomNumber (for ioL)
  snprintf(clientInfo.clientBox,20,"Box%d",randomNumber); //Main+randomNumber (for ioL)
  snprintf(clientInfo.clientText,20,"Text%d",randomNumber); //Main+randomNumber (for ioL)

  if ((*readSocketName == 0) && (clientSend > 0)) { //client sends its name to the server (received client name)
    fprintf(stderr, "Server: client connected - %s\n", buffer);
    buffer[strcspn(buffer,"\n")] = 0; //removes \n from the string
    sendToIOL(clientInfo,buffer); //displays the Main Box in IOL
    *readSocketName = 1;
    return 0;
  } else {
    if (clientSend < 0) {
      perror("server: read error");
      exit(1);
    } else if (clientSend == 0) {
      return -1;
    } else if (strcmp(buffer,"quit") == 0) {
      return -1;
    } else { //end of file is reached therefore push to the server
      fprintf(stderr, "\n%s", buffer);
      printf("<%s.%s.push"
                "{<span color={black} size=15,{px} {%s}>}"
             ">"
      ,clientInfo.clientMain,clientInfo.clientText,buffer); //push to ioL
      return 0;
    }
  }
}

int main(int argc, char *argv[]) {
  int sock, new_sock, len, num_ready, readSocketName;
  int clients_hist_table[5]; //stores all the connected clients
  int clientIoLHistory[5]; //stores random name for the client. To make it unique.
  int clientTotal = 0; //just a counter that stores total number of clients

  /*  used for accessing the clients_hist_table. Client connects then counter++,
      client disconnects then becomes # of disconnected  */
  int historyCounter = 0;
  struct sockaddr_un name;
  struct clientData clientInfo; //used for IOL to identify unique clients

  /* set_of_resources = all resources; read_resource = currently reading */
  fd_set set_of_resources, read_resource;

  /* create a main socket */
  if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("server: socket");
    exit(1);
  }

  /* create the address of the server */
  memset(&name,0,sizeof(struct sockaddr_un));
  name.sun_family = AF_UNIX;
  strcpy(name.sun_path,SOCKET_PATH);
  len = sizeof(name.sun_family) + strlen(name.sun_path);

  /* remove any previous socket */
  unlink(name.sun_path);

  /* bind the socket to the address */
  if (bind(sock,(struct sockaddr*) &name,SUN_LEN(&name)) < 0) {
    perror("server: bind");
    exit(1);
  }

  /* listen for connections */
  if (listen(sock,1) < 0) {
    perror("server: listen");
    exit(1);
  }

  FD_ZERO(&set_of_resources); //resets the set of resources before use
  FD_SET(sock,&set_of_resources); //adds sock descriptor of the open socket to the set of resources
  FD_SET(0,&set_of_resources); //adds descriptor of stdin

  while(1) {
    fprintf(stderr, "server: clients connected: %d\n", clientTotal);
    powerButton_onlineStatus(&clientTotal); //activates the power button & online status

    read_resource = set_of_resources; //current resource becomes readable
    if (num_ready = select(FD_SETSIZE,&read_resource,NULL,NULL,NULL) < 0) {
      perror("server: select");
      exit(1);
    };

    while (num_ready < FD_SETSIZE) { //scan through the max number of resources
      if (FD_ISSET(num_ready,&read_resource)) { //detects action on any type of sockets (main or new)
        if (num_ready == sock) { //when action on main sock is detected -> create a new sock of a new client.
          new_sock = accept(sock,(struct sockaddr*) &name, &len); //creats a new socket

          /* keeps track of max number of clients & creates when necessary */
          if (checkServerSpace(clientTotal,new_sock,clientInfo) == 0) {
            int r = rand(); //gets random value for IoL
            clientIoLHistory[historyCounter] = r;
            clients_hist_table[historyCounter] = new_sock;
            historyCounter++;
            clientTotal++;
            FD_SET(new_sock,&set_of_resources); //sets the new socket to the FD_SET for future executions
            readSocketName = 0; //flag: 0 = socket_name, 1 = text
          }
        } else {
          /* compares the value of accesing client with the value of clients_hist_table.
             If the value is the same, this means specific client is sending the data.
             When there is no clients_hist_table, then never check the table */
          for(int i=0;i<MAX_NUM_CLIENT;i++) { //MAX_NUM_CLIENT = 5
            if (num_ready == clients_hist_table[i] & clients_hist_table[i] != 0) {
              if (readFromClient(clientInfo,clients_hist_table[i],&readSocketName,clientIoLHistory[i]) < 0) { //pushes data to socket or closes new socket
                fprintf(stderr, "server: socket closed\n");
                close(clients_hist_table[i]); //terminates socket if value <0 received
                FD_CLR (clients_hist_table[i], &set_of_resources);
                clients_hist_table[i] = 0;
                clientTotal--;
                historyCounter=i;
              }
            }
          }
        }
      } else if (FD_ISSET(0,&read_resource)) { //whenever the click of the exit button is detected
        char buffer[3];
        fgets(buffer,sizeof(buffer),stdin); //receives Q and pushes to the buffer
        if (strcmp(buffer,"Q\n") == 0) {
          for(int i=0;i<clientTotal;i++) {
            close(clients_hist_table[i]);
          }
          close(sock);
          fprintf(stderr, "server: shutting down\n");
          exit(0);
        }
      }
      num_ready++;
    }
  }
}
