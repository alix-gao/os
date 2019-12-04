
#ifndef _message_types_h
#define _message_types_h

#define msg_keyboard 1
#define msg_mouse_left 2
#define msg_mouse_right 3
#define window_max_count 3//���ڸ��������ֵ��

typedef struct tag_msg
{
        int hwnd;//hwnd hwnd;//���͸��Ķ�Ӧ���ھ����
        int message;//uint message;//��Ϣ�����͡�
        int wparam;//wparam wparam;//��Ϣ���͵�һ��32λ������
        int lparam;//lparam lparam;//��Ϣ���͵ڶ���32λ������
        int time;//dword time;//������Ϣ��ʱ�䡣
        int pt;//point pt;//������Ϣʱ������ڵ�λ�á�
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

