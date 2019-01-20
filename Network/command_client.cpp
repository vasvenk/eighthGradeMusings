#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "windowsstuff.h"


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "1997"
#define COMMANDS "\
Exit\n\
BlockInputs\n\
UnblockInputs\n\
Type\n\
Sim\n\
Press\n\
Rel\n\
MsgBox\n\
System\n\
Web\n\
Close\n\
AltTab\n\
AltF4\n\
CtrlA\n\
LeftClick\n\
PressLeft\n\
RelLeft\n\
RightClick\n\
PressRight\n\
RelRight\n\
"

int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    char sendbuf[DEFAULT_BUFLEN];
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    
    // Validate the parameters
  /*  if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        system("pause");
        return 1;
    }*/
    
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    char ipaddr[20];
    printf("Please enter the ip address\n");
    scanf("%s", ipaddr);
    iResult = getaddrinfo(ipaddr, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    
    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }
    printf("Successfully connected to server!\n");
    
    std::string command;
    
    int i;
    bool mirror = false;
    char kbstate[256];
    char prekbstate[256];
    ReadKeyboard(prekbstate);
    
    while (1)    {
        printf("<cmd> ");
        if (mirror == true)    {
            ReadKeyboard(kbstate);
            for (i = 0; i < 256; i++)
                 if (prekbstate[i] != kbstate[i])    {
                     command[0] = kbstate[i];
                     command[1] = i;
                   //  sendbuf = command.c_str();
                     iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
                 }
            for (i = 0; i < 256; i++)
                prekbstate[i] = kbstate[i];
        }
        else    {
            fgets(sendbuf, DEFAULT_BUFLEN, stdin);
            command = sendbuf;
            if (command == "help\n")
                printf(COMMANDS);
            if (command == "Mirror\n")
                mirror = true;
            if (command == "EndMirror\n")
                mirror = false;
            else
                iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
            if (iResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
            }
            printf("Command successfully sent\n");
        }
        command.clear();
      //  system("pause");
    }
    printf("Bytes Sent: %ld\n", iResult);

    // shutdown the connection since no more data will be sent
 /*   iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }   

    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
            printf("Bytes received: %d\n", iResult);
        else if ( iResult == 0 )
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while( iResult > 0 ); */

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
