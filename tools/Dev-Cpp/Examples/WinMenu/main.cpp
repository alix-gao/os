
#include <windows.h>
#include "main.h"

/* Declare WindowsProcedure */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
/* Make the classname into a global variable */
char szClassName[] = "Windows Example";
HINSTANCE hThisInstance;

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nFunsterStil)

{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application is saved */
    WNDCLASSEX wincl;        /* Datastructure for the windowclass */
    HMENU menu;              /* Handle of the menu */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Ctach double-clicks */
    wincl.cbSize = sizeof(WNDCLASSEX);

    /* Use default icon and mousepointer */
    wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL; /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use lightgray as the background of the window */
    wincl.hbrBackground = (HBRUSH) GetStockObject(LTGRAY_BRUSH);

    /* Register the window class, if fail quit the program */
    if(!RegisterClassEx(&wincl)) return 0;

    /* The class is registered, lets create the program*/
    hwnd = CreateWindowEx(
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           "Windows Example",         /* Title Text */
           WS_OVERLAPPEDWINDOW, /* defaultwindow */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window end up on the screen */
           544,                 /* The programs width */
           375,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a childwindow to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow(hwnd, nFunsterStil);

    menu = LoadMenu(hThisInstance, MAKEINTRESOURCE(ID_MENU));
    SetMenu(hwnd, menu);

    /* Run the nessageloop. It will run until GetMessage( ) returns 0 */
    while(GetMessage(&messages, NULL, 0, 0))
    {
           /* Send message to WindowProcedure */
           DispatchMessage(&messages);
    }

    /* The program returvalue is 0 - The value that PostQuitMessage( ) gave */
    return messages.wParam;
}

/* This function is called by the Windowsfunction DispatchMessage( ) */
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
       case WM_COMMAND:
           switch( wParam )
           {
             case IDM_FILENEW:
             case IDM_FILEOPEN:
             case IDM_FILESAVE:
             case IDM_FILESAVEAS:
             case IDM_FILEPRINT:
             case IDM_FILEPAGESETUP:
             case IDM_FILEPRINTSETUP:

             case IDM_EDITUNDO:
             case IDM_EDITCUT:
             case IDM_EDITCOPY:
             case IDM_EDITPASTE:
             case IDM_EDITDELETE:
                  MessageBox( hwnd, (LPSTR) "Function Not Yet Implemented.",
                              (LPSTR) szClassName,
                              MB_ICONINFORMATION | MB_OK );
                  return 0;

             case IDM_HELPCONTENTS:
                  WinHelp( hwnd, (LPSTR) "HELPFILE.HLP",
                           HELP_CONTENTS, 0L );
                  return 0;

             case IDM_HELPSEARCH:
                  WinHelp( hwnd, (LPSTR) "HELPFILE.HLP",
                           HELP_PARTIALKEY, 0L );
                  return 0;

             case IDM_HELPHELP:
                  WinHelp( hwnd, (LPSTR) "HELPFILE.HLP",
                           HELP_HELPONHELP, 0L );
                  return 0;

             case IDM_FILEEXIT:
                  SendMessage( hwnd, WM_CLOSE, 0, 0L );
                  return 0;

             case IDM_HELPABOUT:
                  MessageBox (NULL, "About..." , "Windows example version 0.01", 1);
                  return 0;

           }
           break;

      case WM_CLOSE:
           DestroyWindow( hwnd );
           return 0;

      case WM_DESTROY:
           PostQuitMessage (0);
           return 0;

      break;
      default:                   /* for messages that we don't deal with */
      return DefWindowProc(hwnd, message, wParam, lParam);
   }
  return 0;
}
