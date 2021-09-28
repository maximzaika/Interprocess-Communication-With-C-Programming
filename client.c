/*Task Number: client v2
  Description: 1. Execute the file
               2. Check whether the user is redirecting the input from the file
                  or decides to use TTY to input manually
               3. Establish communication with the server
               4. Server needs to send a permission to the client to allow
                  the connection if the number of connected clients < 5
               5. If step 4 = permission allowed:
                    - Send SOCKET_NAME to the server
                  If step 4 = permission disallowed
                    - Close this client and connection in step 3
               6. If step 2 = redirection:
                    - send the text in the file to the server
                    - terminate the client once the end of file has been reached
                  If step 2 = input from TTY:
                    - let user input the text
                    - [CTRL+D] must be used to indicate the end of users input
                    - send to the server the input
                    - loop again until client terminates by typing 'quit' or
                      pressing [CTRL+C]

               SOCKET_NAME:
                 SOCKET_NAME is stored inside tempText variable at the beginning
                 of the main() function. the method is based on a counter called
                 socketText. Whenever socketText = 0, this indicates the name
                 of the socket needs to be sent to the server, otherwise it is
                 a text. Name of the socket is the name of the executable file
                 (argv[0]).

               lient is v2:
                 can be used with  only.

               Error Handling:
                 maximum capacity of the server is 5, therefore, this client
                 may connect to the server when the value is <5, otherwise this
                 client gets rejected, and error gets returned to the client.
  Author: Maxim Zaika ()
  Language: C
  Start date: 24/09/2018 12:00PM
  Last modified: 02/10/2018 6.25PM (added redirection isatty() method)
  How to execute assuming the server is running:
              To let user enter the text to the server manually then run the
              following command in your Terminal:
                 gcc client.c -o client;./client

              To let the user to select the file from where the text is
              going to come, then run the following command in your terminal
              where the filename is the name of the file where text is located:
                 gcc client.c -o client;./client < filename */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/a2-.socket"

/* Pre:
     - CTRL+D is clicked (EOL reached)
     - Text has been provided by the client OR
       Clients name is provided
   Post:
     - sends the text to the server OR a client name
     - adds </n>\n at the end of the string. </n> is used for IoL, while \n
       is used for printing text on a server side.
     - uses counter socketText to detect the clients name. If socketText = 0
       then it is a client name. Everything >0 is a text (or 1)
   Returns: 0*/
int sendTextToServer(int sock, char buffer[1024],int *socketText,char tempText[1024]) {
  char textToServer[1024];
  if ((feof(stdin) && strcmp(buffer,"quit") != 0) || (*socketText == 0)) {
    snprintf(textToServer,1024,"%s</n>\n",tempText); //adds \n</n> of the text
    send(sock,textToServer,sizeof(textToServer),0);
    *socketText+=1;
  }
}

/* Pre:
     - client name / text has been sent to the server
     - client has entered 'quit' and pressed [CTRL+D]
   Post:
     - sends 'quit' to the server to tell that this particular client is quitting
     - server then performs necessary actions based on this
   Returns: 0*/
int checkQuit(int sock, char buffer[1024]) {
  if (strcmp(buffer,"quit\n") == 0) {
    char textToServer[6];
    strcpy(textToServer,"quit");
    send(sock,textToServer,6,0);
  }
}

/* Pre:
     - name of the socket has to be sent to the server before reading this function
       Once it is sent, socketText needs to be equal to 2
     - this function becomes true only if redirection in command detected
       example: ./client < filename (where filename is the file of the redirection)
   Post:
     - reads the text from the file provided by the client & terminates the client
       at the end.
   Returns: 0 or -1*/
int checkRedirection(int *socketText) { //make sure that the name of the socket has been sent
  if ((*socketText == 2) && (isatty(STDIN_FILENO) == 0)) {
    return -1;
  } else {
    return 0;
  }
}

/* Pre:
     - server must be running
     - used entered ./name_of_executable to the Terminal, where name_of_executable
       is the name of this executable file
   Post:
     - creates a SOCK & establishes connection with the server by waiting for
       a permission from the server to connect to the server.
     - connects only if "oke" has been received, which indicates that there is
       a sufficient space available. If "end" is received, this client shuts
       itself down.
     - sends the name of the client based on the file name. Called by argv[0].
     - asks a client to input any text that needs to be send to the server.
   Termination:
     - client needs to press [CTRL+C] or type 'quit' */
int main(int argc, char *argv[]) {
  int sock, nsock, len;
  int socketText = 2; //boolean to detect socket name or socket text. 0 = name, 1 = text, 2 = redirection
  struct sockaddr_un name;

  char buffer[1024]; //temporary stores received text from stdin
  char tempText[1024]; //stores temporary text that needs to be pushed to the server
  char textToServer[1024]; //stores tempText[1024]+\n</n> to indicate end of string for the server
  char tempUserInput[1024]; //temporary stores user's input
  char flag[5]; //stores server's permission. "oke" = permission granted, "end" = permission denied

  /* Pushes name of the socket to the temporary text variable.
     argv[0] = name of the executable file */
  strcpy(tempText,argv[0]); //pushes name to the socket as temporary text

  /* create SOCKET */
  if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("client: socket");
    exit(1);
  }

  /* create the address of the server */
  memset(&name,0,sizeof(struct sockaddr_un));
  name.sun_family = AF_UNIX;
  strcpy(name.sun_path,SOCKET_PATH);
  len = sizeof(name.sun_family) + strlen(name.sun_path);

  /* connect to the server */
  if (connect(sock,(struct sockaddr *) &name,SUN_LEN(&name)) < 0) {
    perror("client: connect");
    exit(1);
  }

  recv(sock,flag,sizeof(flag),0); //receives server's permission to access the server

  if (strcmp(flag,"end") == 0) {
    printf("client: server's maximum capacity reached. Try again later.\n");
    close(sock);
    exit(0);
  } else {
    if (checkRedirection(&socketText) == 0) { //checks whether command is redirection or not
      printf("client: input text or 'quit', and then press [CTRL+D] to send to the server\n");
    }

    socketText = 0; //changes socketText to 0 to indicate the name of the file

    /* uses socketText to identify whether received text is the name of the socket
       or the actual text. If it is a socket name, then client sends it to the server
       first no matter what, after that constantly requsts user's input until signal
       CTRL+D is detected. CTRL+D indicates the end of user's text. Once it is detected,
       client sends the text to the server. */
    while(strcmp(buffer,"quit") != 0) {
      if (socketText != 0) { //don't let input unless name has been sent

        /* boolean that checks whether first line of text has been received.
           0 indicates that any text is the first line of text, 1 indicates
           all other lines */
        int firstText = 0;

        /* receives data from stdin until the end of stdin, and then adds \n</n>
           at the end of each line whenever necessary */
        while (fgets(buffer,sizeof(buffer),stdin) != NULL) {
          buffer[strcspn(buffer,"\n")] = 0; //removes \n from the stdin string
          if (firstText == 0) {
            strcpy(tempText,buffer); //pushes buffer data to the string that stores all the text
            strcpy(tempUserInput,buffer); //temporary stores user's input
            firstText=1;
          } else {
            snprintf(tempText,sizeof(tempText),"%s</n>\n%s",tempUserInput,buffer); //pushes temporary input to tempText
            strcpy(tempUserInput,tempText); //makes the tempText users temporary text
          }
        }
      }

      sendTextToServer(sock,buffer,&socketText,tempText);
      checkQuit(sock,buffer);

      if (checkRedirection(&socketText) < 0) { //if redirection is detected then do the following
        printf("client: your program will remain running for 5 seconds to display the output on the server.\n");
        sleep(5);
        break;
      }
    }

    close(sock);
    exit(0);
  }
}
