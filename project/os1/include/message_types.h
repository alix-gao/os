
#ifndef _message_types_h
#define _message_types_h

#define msg_keyboard 1
#define msg_mouse_left 2
#define msg_mouse_right 3
#define window_max_count 3//窗口个数的最大值。

typedef struct tag_msg
{
        int hwnd;//hwnd hwnd;//发送给的对应窗口句柄。
        int message;//uint message;//消息的类型。
        int wparam;//wparam wparam;//消息传送第一个32位参数。
        int lparam;//lparam lparam;//消息传送第二个32位参数。
        int time;//dword time;//发送消息的时间。
        int pt;//point pt;//发送消息时鼠标所在的位置。
}msg;

typedef struct wndclass_gc
{
        int begx;
        int begy;
        int endx;
        int endy;
        int no;
        int color;
        void(*lpfnwndproc)(msg*tmp_msg);
}wndclass_gc;

#endif

