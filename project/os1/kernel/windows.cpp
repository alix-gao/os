
#include<message_cite.h>
//--------
extern void disp_color(int begx,int begy,int endx,int endy,int color);
//========
void registerclass_gc(wndclass_gc*wc)
{
     //__asm__("cli");//这一对开关不是必须的。
     regist_window_buffer[time_window_no].begx=wc->begx;
     regist_window_buffer[time_window_no].begy=wc->begy;
     regist_window_buffer[time_window_no].endx=wc->endx;
     regist_window_buffer[time_window_no].endy=wc->endy;
     regist_window_buffer[time_window_no].no=wc->no;
     regist_window_buffer[time_window_no].color=wc->color;
     regist_window_buffer[time_window_no].lpfnwndproc=wc->lpfnwndproc;
     //__asm__("sti");
}

void init_application_mfc()//(hinstance hinstance)
{
     //wndclass wc;

     //wc.style=cs_hredraw|cs_vredraw;
     //wc.lpfnwndproc=(wndproc)wndproc;
     //wc.cbclsextra=0;
     //wc.cbwndextra=0;
     //wc.hinstance=hinstance;
     //wc.hicon=loadicon(hinstance,"jjhouricon");
     //wc.cursor=loadcursor(null,idc_arrow);
     //wc.hbrbackground=getstockobject(white_brush);
     //wc.lpszmenuname="genericmenu";
     //wc.lpszclassname=_szappname;

     //return registerclass(&wc);
     //-------------------------------------------------------------------------
}

void init_instance_mfc()//(hinstance hinstance,int ncmdshow)
{
     //_hinst=hinstance;

     /*
     _hwnd=createwindow(
                        _szappname,
                        _sztitle,
                        ws_overlappedwindow,
                        cw_usedefault,
                        cw_usedefault,
                        cw_usedefault,
                        null,
                        null,
                        hinstance,
                        null
                        );
     */

     //if(!_hwnd)
     //return false;

     //showwindow(_hwnd,ncmdshow);
     //updatewindow(_hwnd);
     //return ture;
     //-------------------------------------------------------------------------
}

void create_window_gc(int no)
{
     //__asm__("cli");//这一对开关不是必须的。
     disp_color(regist_window_buffer[no].begx,regist_window_buffer[no].begy,regist_window_buffer[no].endx,regist_window_buffer[no].endy,regist_window_buffer[no].color);

     disp_color(regist_window_buffer[no].endx+1,regist_window_buffer[no].begy+1,regist_window_buffer[no].endx+1,regist_window_buffer[no].endy+1,0);//阴影,col
     disp_color(regist_window_buffer[no].begx+1,regist_window_buffer[no].endy+1,regist_window_buffer[no].endx,regist_window_buffer[no].endy+1,0);//阴影,row
     //纸张
     disp_color(regist_window_buffer[no].endx+1+1,regist_window_buffer[no].begy+1,regist_window_buffer[no].endx+1+2,regist_window_buffer[no].endy+1+2,regist_window_buffer[no].color);//阴影,col
     disp_color(regist_window_buffer[no].begx+1,regist_window_buffer[no].endy+1+1,regist_window_buffer[no].endx+1,regist_window_buffer[no].endy+1+2,regist_window_buffer[no].color);//阴影,row
     //----------------
     disp_color(regist_window_buffer[no].endx+1+2+1,regist_window_buffer[no].begy+1+1,regist_window_buffer[no].endx+1+2+1,regist_window_buffer[no].endy+1+2+1,0);//阴影,col
     disp_color(regist_window_buffer[no].begx+1+1,regist_window_buffer[no].endy+1+2+1,regist_window_buffer[no].endx+1+2,regist_window_buffer[no].endy+1+2+1,0);//阴影,row

     disp_color(regist_window_buffer[no].endx+1+2+1+1,regist_window_buffer[no].begy+3,regist_window_buffer[no].endx+1+2+1+2,regist_window_buffer[no].endy+6,regist_window_buffer[no].color);//阴影,col
     disp_color(regist_window_buffer[no].begx+2,regist_window_buffer[no].endy+5,regist_window_buffer[no].endx+4,regist_window_buffer[no].endy+6,regist_window_buffer[no].color);//阴影,row
     //__asm__("sti");
}

