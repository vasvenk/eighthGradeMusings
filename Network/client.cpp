// Microsoft Development Environment 2003 - Version 7.1.3088

// Copyright (r) 1987-2002 Microsoft Corporation. All Right Reserved

// Microsoft .NET Framework 1.1 - Version 1.1.4322

// Copyright (r) 1998-2002 Microsoft Corporation. All Right Reserved

//

// Run on Windows XP Pro machine, version 2002, SP 2

//

// <windows.h> already included

// WINVER = 0x0501 for Xp already defined in windows.h

// A sample of client program using TCP

 

#include <stdio.h>

#include <winsock2.h>

 

int main()

{

    // Initialize Winsock.

    WSADATA wsaData;

    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (iResult != NO_ERROR)

              printf("Client: Error at WSAStartup().\n");

    else

              printf("Client: WSAStartup() is OK.\n");

 

    // Create a socket.

    SOCKET m_socket;

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

 

    if (m_socket == INVALID_SOCKET)

    {

        printf("Client: socket() - Error at socket(): %ld\n", WSAGetLastError());

        WSACleanup();

        return 0;

    }

   else

       printf("Client: socket() is OK.\n");

 

    // Connect to a server.

    sockaddr_in clientService;

 

    clientService.sin_family = AF_INET;

    // Just test using the localhost, you can try other IP address

    clientService.sin_addr.s_addr = inet_addr("192.164.1.94");

    clientService.sin_port = htons(55555);

 

    if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR)

    {

        printf("Client: connect() - Failed to connect.\n");

        WSACleanup();

        return 0;

    }

    else

    {

       printf("Client: connect() is OK.\n");

       printf("Client: Can start sending and receiving data...\n");

    }

 

    // Send and receive data.

    int bytesSent;

    int bytesRecv = SOCKET_ERROR;

    // Be careful with the array bound, provide some checking mechanism...

    char sendbuf[200] = "This is a test string from client";

    char recvbuf[200] = "";

 

       // Receives some test string to server...

       while(bytesRecv == SOCKET_ERROR)

       {

           bytesRecv = recv(m_socket, recvbuf, 200, 0);

        if (bytesRecv == 0 || bytesRecv == WSAECONNRESET)

        {

             printf("Client: Connection Closed.\n");

            break;

        }

 

        if (bytesRecv < 0)

            return 0;

       else

       {

              printf("Client: recv() is OK.\n");

              printf("Client: Received data is: \"%s\"\n", recvbuf);

              printf("Client: Bytes received is: %ld.\n", bytesRecv);

       }

    }

 

       // Sends some test data to server...

       bytesSent = send(m_socket, sendbuf, strlen(sendbuf), 0);

       if(bytesSent == SOCKET_ERROR)

              printf("Client: send() error %ld.\n", WSAGetLastError());

       else

       {

              printf("Client: send() is OK - Bytes sent: %ld\n", bytesSent);

              printf("Client: The test string sent: \"%s\"\n", sendbuf);

       }

 

    WSACleanup();
    system("pause");
    return 0;

}
