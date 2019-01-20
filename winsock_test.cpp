    // Microsoft Development Environment 2003 - Version 7.1.3088

    // Copyright (r) 1987-2002 Microsoft Corporation. All Right Reserved

    // Microsoft .NET Framework 1.1 - Version 1.1.4322

    // Copyright (r) 1998-2002 Microsoft Corporation. All Right Reserved

    //

    // Run on Windows XP Pro machine, version 2002, SP 2

    //

    // <windows.h> already included

    // WINVER = 0x0501 for Xp already defined in windows.h

     

    #include <stdio.h>

    #include <winsock2.h>

     

    int main()

    {

    WORD wVersionRequested;

    WSADATA wsaData;

    int wsaerr;

     

    // Using MAKEWORD macro, Winsock version request 2.2

    wVersionRequested = MAKEWORD(2, 2);

     

    wsaerr = WSAStartup(wVersionRequested, &wsaData);

    if (wsaerr != 0)

    {

        /* Tell the user that we could not find a usable */

        /* WinSock DLL.*/

        printf("The Winsock dll not found!\n");

        return 0;

    }

    else

    {

           printf("The Winsock dll found!\n");

           printf("The status: %s.\n", wsaData.szSystemStatus);

    }

     

    /* Confirm that the WinSock DLL supports 2.2.*/

    /* Note that if the DLL supports versions greater    */

    /* than 2.2 in addition to 2.2, it will still return */

    /* 2.2 in wVersion since that is the version we      */

    /* requested.                                        */

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)

    {

        /* Tell the user that we could not find a usable */

        /* WinSock DLL.*/

        printf("The dll do not support the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));

        WSACleanup();

        return 0;

    }

    else

    {

        printf("The dll supports the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));

        printf("The highest version this dll can support: %u.%u\n", LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));

    }

     

    //////////Create a socket////////////////////////

    //Create a SOCKET object called m_socket.

    SOCKET m_socket;

     

    // Call the socket function and return its value to the m_socket variable.

    // For this application, use the Internet address family, streaming sockets, and

    // the TCP/IP protocol.

    // using AF_INET family, TCP socket type and protocol of the AF_INET - IPv4

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

     

    // Check for errors to ensure that the socket is a valid socket.

    if (m_socket == INVALID_SOCKET)

    {

        printf("Error at socket(): %ld.\n", WSAGetLastError());

        WSACleanup();

        return 0;

    }

    else

    {

        printf("socket() is OK!\n");

    }

     

    ////////////////bind()//////////////////////////////

    // Create a sockaddr_in object and set its values.

    sockaddr_in service;

     

    // AF_INET is the Internet address family.

    service.sin_family = AF_INET;

    // "127.0.0.1" is the local IP address to which the socket will be bound.

    service.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 55555 is the port number to which the socket will be bound.

    service.sin_port = htons(55555);

     

    // Call the bind function, passing the created socket and the sockaddr_in structure as parameters.

    // Check for general errors.

     

    if (bind(m_socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)

    {

        printf("bind() failed: %ld.\n", WSAGetLastError());

        closesocket(m_socket);

        return 0;

    }

    else

    {

       printf("bind() is OK!\n");

    }

     
    system("pause");
    return 0;

    }
