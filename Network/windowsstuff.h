#include <windows.h>
#include <WinAble.h>
#include <string>
#include <time.h>
#include <iostream>
using namespace std;

#define DOWN 0
#define UP 1

void Wait(int ticks)
{                                                                                                    //25
  clock_t endwait;
  endwait = clock () + ticks ;
  while (clock() < endwait) {
        if (GetKeyState(VK_ESCAPE) != 0 && GetKeyState(VK_ESCAPE) != 1)
            break;
  }
}

void WaitPause(int ticks)
{                                                                                                    //25
  clock_t endwait;
  endwait = clock () + ticks ;
  while (clock() < endwait) {
        if (GetKeyState(VK_ESCAPE) != 0 && GetKeyState(VK_ESCAPE) != 1)
            break;
        if (GetKeyState(0x50) != 0 && GetKeyState(0x50) != 1) {
               while (GetKeyState(0x50) != 0 && GetKeyState(0x50) != 1) {}
               cout << "Paused\n";
               while (GetKeyState(0x50) == 0 || GetKeyState(0x50) == 1) {}
               while (GetKeyState(0x50) != 0 && GetKeyState(0x50) != 1) {}
               cout << "Unpaused\n";
        }
  }
}

void ReadKeyboard( char* keys )
{ 
  for (int x = 0; x < 256; x++)
    keys[x] = (char) (GetAsyncKeyState(x) >> 8);
}

/*void HideConsole()
{
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);
}

void ShowConsole()
{
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_SHOW);
}*/


void LeftClick (bool dir)
{  
  INPUT    Input={0};
  if (dir == DOWN)    {
  // left down 
      Input.type      = INPUT_MOUSE;
      Input.mi.dwFlags  = MOUSEEVENTF_LEFTDOWN;
      ::SendInput(1,&Input,sizeof(INPUT));
  }
  else    {
  // left up
      ::ZeroMemory(&Input,sizeof(INPUT));
      Input.type      = INPUT_MOUSE;
      Input.mi.dwFlags  = MOUSEEVENTF_LEFTUP;
      ::SendInput(1,&Input,sizeof(INPUT));
  }
}

void RightClick (bool dir )
{  
  INPUT    Input={0};
  if (dir == DOWN)    {
  // right down 
      Input.type      = INPUT_MOUSE;
      Input.mi.dwFlags  = MOUSEEVENTF_RIGHTDOWN;
      ::SendInput(1,&Input,sizeof(INPUT));
  }
  else    {
  // right up
      ::ZeroMemory(&Input,sizeof(INPUT));
      Input.type      = INPUT_MOUSE;
      Input.mi.dwFlags  = MOUSEEVENTF_RIGHTUP;
      ::SendInput(1,&Input,sizeof(INPUT));
  }
}

void MouseMove (int x, int y )
{  
  double fScreenWidth    = ::GetSystemMetrics( SM_CXSCREEN )-1; 
  double fScreenHeight  = ::GetSystemMetrics( SM_CYSCREEN )-1; 
  double fx = x*(65535.0f/fScreenWidth);
  double fy = y*(65535.0f/fScreenHeight);
  INPUT  Input={0};
  Input.type      = INPUT_MOUSE;
  Input.mi.dwFlags  = MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE;
  Input.mi.dx = (long)fx;
  Input.mi.dy = (long)fy;
  ::SendInput(1,&Input,sizeof(INPUT));
}

void SimKey(int Keycode)
{
      INPUT input[2];
      ::ZeroMemory(input, sizeof(input));
      input[0].type = input[1].type = INPUT_KEYBOARD;
      input[0].ki.wVk  = input[1].ki.wVk = Keycode;        
      input[1].ki.dwFlags = KEYEVENTF_KEYUP;  // THIS IS IMPORTANT
      ::SendInput(2, input, sizeof(INPUT));  
}

void SimKeyUD(int Keycode, int dir) //Up and Down
{
      INPUT input[1];
      ::ZeroMemory(input, sizeof(input));
      if (dir == DOWN)    {
          input[0].type = INPUT_KEYBOARD;
          input[0].ki.wVk = Keycode;
      }
      else if (dir == UP)    {
          input[0].type = INPUT_KEYBOARD;
          input[0].ki.wVk = Keycode;
          input[0].ki.dwFlags = KEYEVENTF_KEYUP; 
      }
      ::SendInput(1, input, sizeof(INPUT));  
}

void TypeStr(string str)
{
     int i;
      for (i = 0; i < str.length(); i++)    {
         if (str[i] >= 'A' && str[i] <= 'Z')    {
             if (GetKeyState(VK_CAPITAL) == 0)    {
                 SimKey(VK_CAPITAL);
                 SimKey((int)str[i]);
                 SimKey(VK_CAPITAL);
             }
             else
                  SimKey((int)str[i]);
         }
         if (str[i] >= 'A' + 32 && str[i] <= 'Z' + 32)    {
             if (GetKeyState(VK_CAPITAL) != 0)    {
                 SimKey(VK_CAPITAL);
                 SimKey((int)str[i] - 32);
                 SimKey(VK_CAPITAL);
             }
             else
                  SimKey((int)str[i] - 32);
         }
         if (str[i] == ' ')
             SimKey(VK_SPACE);
     }
}
