# Interprocess Communication With C Programming and IoL Graphical Shell

## Video Demonstration

[YouTube Video Demonstration](https://youtu.be/vE2Y7IOUXIE)

## Requirement

ioL graphical shell is required to run this program. To learn more about ioL, refer to
[ioL's documentation](http://doc.iol.science/)

## Description

Simple display server displaying information from multiple clients on one screen, based on sockets.

Server:

- Receives connections of up to 5 clients.
- Receives client names and messages to separate windows.
- Allows clients to shut down the server by click of a 'shutdown' button.

Client:

- Sends data required to be displayed on the server (plain text or a file that
  contains plain text).

## How to Run

1. Open your terminal (by default in Ubuntu Mate: right click within the
   folder -> Open in Terminal) where the file server.c is
   located.
2. Enter the following command:
   `gcc server.c -o server;sudo ./runstandalone.sh`
3. Enter the password of your account requested by the system.
4. Server window will launch. Press `[RightCtrl+F7]` to temporary hide it.
5. Open new terminal. Refer to step 1.
6. Refer to step `i` to manually send message, or to step `ii` to assign a 
   file that contains a message
   1. Enter the following command: `gcc client.c -o client;./client`
       - Type any message into the client Terminal and press [CTRL+D] to send
       the message to the server. Press RightCtrl+F8 to launch server window.
   2. Enter the following command where 'filename' is the name of the file 
      that contains a message: `gcc client.c -o client;./client < filename`
       - 5 seconds is given to preview the message on server side. Client is 
         then terminated automatically.

## Server Termination

- Press `[CTRL+C]` in the Server Terminal
- Press `Shutdown` button on the server side window.

## Client Termination

- Press `[CTRL+C]` in the Client Terminal
- Type `quit` in the Client Terminal and press `[CTRL+D]`

## Keyboard Shortcuts

| Key | Usage | Description |
| --- | --- | --- |
| CTRL+C | Client & Server Terminal |  Terminates a Client or the Server |
| CTRL+D | Client Terminal | Sends a message to the Server |
| ENTER | Client Terminal | Allows users to type on a new line |
| RightCtrl+F7 | Server Window | Switches to Linux View |
| RightCtrl+F8 | Linux View | Switches to Server Window |

## Assumptions

| Assumption | Description |
| --- | --- |
| Client Execution | Server must be running before connecting the client |
| Client Reconnection | Reconnecting clients are treated as new clients |
| Redirection From File | The following command: `gcc client.c -o client;./client < filename` connects to the server and displays file's content in plain text for 5 seconds and disconnects the client. Input is not allowed. |

## Extra Features 

| Feature | Description |
| --- | --- |
| Server Capacity | Server rejects more than 5 clients and clients are notified. |
| Online Status | Server displays number of connected clients. |
| Client Window Reset | Server resets its content if number of concurrent users becomes 0. |
| Client Name | Server interprets client names as unique but clients are not aware of it. Client sees its name as the name of the executable file. |
| Socket File Location | File is stored in `tmp` folder. File is removed on restart / shut down of the server. File's directory: `/tmp/a2-.socket`. |

## Limitations

| Limitation | Description |
| --- | --- |
| Server Termination | Clients are disconnected once server is disconnected. |
| Client Communication | No direct communication between clients is allowed. All the communication is displayed in the server's window. |
| Client Interaction | User has no interaction with the server's window. Shutdown button is available for use only. |
| Client Disconnection | Window of disconnected client remain open. Once the same client is reconnected, the previously used window is not utilised and a new window is created instead. Windows are cleared once client count reaches 0. |

## Known Bugs 

| Bug | Description |
| --- | --- |
| Client Disconnection & Reconnection | Server might get stuck in infinite loop when client's keep disconnecting or reconnecting. |




