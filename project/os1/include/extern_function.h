
#include<message_types.h>
#include<process.h>
//--------------
extern void test_time_int();
extern void move_curse(int row,int col);
extern void print_int(int disp,int color,int row,int col);
//extern void print_string_partly(char*strp,int count,int color,int row,int col);
extern void print_string(char*strp,int color,int row,int col);
extern void print_char(char strp,int color,int row,int col);
extern void play_music(char*music_freq,char*music_time);
//extern void test_keyboard_int();
//extern void save_curse(int x,int y);
//extern void clean_screen(int a,int b);
extern void init_graphics();//for graphics
//extern void disp_color(int begx,int begy,int endx,int endy,int color);
//extern void init_mouse();
extern void openmov();
//extern void start_elsfk();
//extern bool get_message(msg*msg,int no);
extern void init_msg_queue();
extern void set_vga_mode();
extern void init_msg_test();
extern void edit_text();
extern void disp_my_msg();
extern void init_pci();
extern void kmem_init();
extern void kcurse_init();
extern void driver_init();
extern void ipc_init();
extern void clean_screen(int a,int b);
extern void init_page(void);

