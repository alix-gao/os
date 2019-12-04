
#include<const.h>
#include<io.h>
#include<process.h>
#include<ram_addr.h>
#include<global_cite.h>
#include<message_cite.h>
#include<message_types.h>
#include <8259a.h>

extern void print_string(char*strp,int color,int row,int col);
extern void print_int(int disp,int color,int row,int col);
extern void print_char(char strp,int color,int row,int col);
extern void time_int();
extern void kb_int();
extern void save_curse(int x,int y);
extern void move_curse(int row,int col);
extern void key_place_control(int dis,int attr);
extern void my_task_1();
extern void my_task_2();
extern void send_message_to_kernal(msg tmp_msg);
extern void disp_color(int begx,int begy,int endx,int endy,int color);
extern void set_vga_mode();
//extern void disp_color(int,int,int,int,int);
extern void openmov();

char key_make_code[0x27]={"qwertyuiop[]ECasdfghjkl;'`S\\zxcvbnm,./"};
int key_place_control_sel=1;

//修改gdt/idt描述符
//atr属性，第no个。base,基址。limit，界限。attribute，属性。
void modify_descriptor(desc_struct*addr,int no,bit_32 base,bit_32 limit,bit_16 attribute)
{
                                   addr=addr+no;
     //---
     addr->limit_l_1=limit&0x0ff; //段界限1（16位）
     addr->limit_l_2=(limit>>8)&0x0ff;
     addr->base_l_1=base&0x0ff; //段基址1（16位）
     addr->base_l_2=(base>>8)&0x0ff;
     addr->base_m=(base>>16)&0xff;
     addr->attr1=attribute&0xff;
     addr->limit_high_attr2=((limit>>16)&0xf)|(attribute>>8)&0xf0;
     addr->base_h=(base>>24)&0xff;
     //g,d,0,avl=0000.tss is 16 bit segment.
}

void init_tss(task_struct*p,int i,int task_addr)
{
     p[i].tss.stack0_len=&(p[i].stack_0[1024*4-1])-&(p[i].stack_0[0]);
     p[i].tss.stack0_sel=2*8+til;//stack 0.
     p[i].tss.cr3=0x120000;//--
     p[i].tss.eip=task_addr;//(bit_32)my_task_0; //段内偏移。
     p[i].tss.eflags=0x0202;
     p[i].tss.esp=(bit_32)&(p[i].stack_0[1024*4-1]);
     p[i].tss.cs_sel=1*8+til;//in ldt.0,null.1,code.
     p[i].tss.ds_sel=2*8+til;
     p[i].tss.ss_sel=2*8+til;//--32bit,not 16bit.--
     p[i].tss.es_sel=2*8+til;
     p[i].tss.fs_sel=2*8+til;
     p[i].tss.gs_sel=2*8+til;
     p[i].tss.ldtr=(6+i*2)*8; //0,null.1,code.2,data.3,tss0.4,ldt0. in gdt.
     //-left i/o.
     p[i].tss.link=0;
     p[i].tss.bitmap=0xff;
     p[i].tss.tss_attr=0;
     p[i].tss.trace_bitmap=&(p[i].tss.bitmap)-(char*)&(p[i].tss.link);

     p[i].tss.eax=0;
     p[i].tss.ebx=0;
     p[i].tss.ecx=0;
     p[i].tss.edx=0;
     p[i].tss.ebp=0;
     p[i].tss.esi=0;
     p[i].tss.edi=0;
}

void init_ldt(task_struct*p,int i)
{
     //only a code.
     modify_descriptor(p[i].ldt,1,0x0,0xfffff,at_code_exe+d_32);//code //函数名表示地址。
     modify_descriptor(p[i].ldt,2,0x0,0xfffff,at_data_write+d_32);//data
}

int find_next_process()
{
    //根据process_ing_no找到下一个进程号。
    //同时将当前进程状态设为ready。
    int pid;
    //while(1);
    //state[2]=p_ed;
    //state[1]=p_ed;
    if(state[last_task_no]!=task_status_zombie)
    state[last_task_no]=task_status_ready;
    //这样如果只有一个内核进程的话，最终下面的循环还会找到内核进程。
    if(last_task_no==my_task_no-1)pid=0;//最后一个。
    else pid=last_task_no+1;
    //int j=0;
    while(pid<my_task_no)
    {
                          if(state[pid]==task_status_ready)
                          return pid;
                          if(pid==my_task_no-1)pid=0;//for(;;i++)
                          else pid++;
    }
}

void f_time_int()
{
     /* process id */
     unsigned int pid;
     struct tmp_jmp_addr{long a,b;}tmp;

     /* display */
     if(text_g==0)
     print_char(int_c,12,11,39);
     else
     disp_color(70,70,90,90,int_c%16);
     int_c++;
     //----
     //time_int_switch=((time_int_switch++)&0x1)*2+5;
     //print_int(process_ing_no,15,16,numtt);
         //print_int(last_task_no,15,19,t2++);
         //print_int(process_ing_no,14,22,t5++);
     /* look for next process */
     pid=find_next_process();
     //---------------------------
     //窗口消息的控制。
     if(pid==3)//window 1
     time_window_no=1;
     if(pid==4)//window 2
     time_window_no=2;

     /* display */
     if(text_g)
     {disp_color(11,51,20,60,active_window_id%16);disp_color(11,61,20,70,time_window_no%16);}

     //---------------------------
     if(last_task_no==pid)//仅剩下内核进程了。
     {
         send_master_8259a_eoi();
         //outb(0xa0,0x20);
         //outb(0x20,0x20);//open 8259.
     }
     else
     {
         //while(1);
         last_task_no=pid;
         state[pid]=task_status_running;
         //__asm__("ljmp %0,$0x0"::"L"(npn*8):);
         tmp.b=0;//clear high bit。
         tmp.b=(5+pid*2)*8;
         send_master_8259a_eoi();
         //outb(0xa0,0x20);
         //outb(0x20,0x20);
         __asm__ __volatile__("ljmp %0\n\t"\
                 ::"m"(tmp):);//*&的妙用。
     } //总是有烦人的警告。
//虽然不断尝试，但是ljmp使用2个参数的时候，似乎不能传参数。
//只能使用1个参数的形式来进行ljmp。

//     unsigned int npn;
//     struct{long a,b;}tmp;
//     //---
//     print_char(int_c,12,11,39);
//     int_c++;
//     //----
//     //time_int_switch=((time_int_switch++)&0x1)*2+5;
//     npn=find_next_process();
//     if(last_task_no==npn)//仅剩下内核进程了。
//     outb(0x20,0x20);//open 8259.
//     else
//     {
//         last_task_no=npn;
//         state[npn]=p_ing;
//         outb(0x20,0x20);
//         //__asm__("ljmp %0,$0x0"::"L"(npn*8):);
//         tmp.b=0;//clear high bit。
//         tmp.b=(3+npn*2)*8;
//         __asm__ __volatile__("ljmp %0\n\t"\
//                 ::"m"(*&tmp.a):);//*&的妙用。
//     } //总是有烦人的警告。

//     //----
//     //print_int(npn,15,11,10);
//     //print_int(time_int_switch,15,11,11);
//     //----
//     if(npn==last_task_no)//是其本身，只剩内核进程了。npn=3.
//     outb(0x20,0x20);
//     else
//     {
//         if(npn==5)
//         {
//                            last_task_no=5;
//                            state[1]=p_ing;
//                            outb(0x20,0x20);
//                            __asm__("ljmp $5*8,$0x0");//不能再返回内核了。
//         }
//         if(npn==7)
//         {
//                            last_task_no=7;
//                            state[2]=p_ing;
//                            outb(0x20,0x20);
//                            __asm__("ljmp $7*8,$0x0");
//         }
//         if(npn==3)
//         {
//                   last_task_no=3;
//                   outb(0x20,0x20);
//                   __asm__("ljmp $3*8,$0x0");
//         }
//     }
//     //----
//     //outb(0x20,0x20);
}

void modify_int(void (*p)(),int no)
{
     __asm__ __volatile__("cli;\
              movl %1,%%edi;\
              movl %0,%%edx;\
              movl $0x80000,%%eax;\
              movw %%dx,%%ax;\
              movw $0x8e00,%%dx;\
              movl %%eax,(%%edi);\
              movl %%edx,4(%%edi);"\
              :
              :"c"(p),"b"(no*8+0x100000) //此处已经是地址值了，不用lea
              :"%edi","%eax","%edx");
}

void os_int_install(void (*p)(),int no)
{
     modify_int(p, no);
}

void init_state()
{
     state[0]=state[1]=state[2]=task_status_ready;
     //state[1]=p_ing; //init
     state[0]=task_status_running;
     //process_ing_no=1;
}

void init_switch()
{
     last_task_no=0;//1;
     //time_int_switch=5;
}

void open_time_int()
{
    __asm__ __volatile__("cli;\
                          inb $0x21,%%al;\
                          movb %%al,%%ah;\
                          andb $0xfe,%%ah;\
                          movb %%ah,%%al;\
                          outb %%al,$0x21"\
                          :
                          :
                          :"%eax");
}

void test_time_int()
{
     //*****************
     int task_0,task_1,task_2;
     task_0=0;
     task_1=1;
     task_2=2;
     //---------------------------------------------------------------
     process_table[task_1].pid=0x12345678;//find it
     init_tss(process_table,task_1,(bit_32)my_task_1); //task 0
     init_ldt(process_table,task_1);
     //--//o,null.1,code.2,data.--
     modify_descriptor(gdt_addr,7,(bit_32)(&(process_table[task_1].tss)),0xfffff,at_386_tss+d_32);//tss0 0x12345678
     modify_descriptor(gdt_addr,8,(bit_32)(process_table[task_1].ldt),0xfffff,at_ldt+d_32);//ldt
     //---------------------------------------------------------------
     process_table[task_1].pid=0x11111111;
     init_tss(process_table,task_2,(bit_32)my_task_2); //task 1
     init_ldt(process_table,task_2);
     //--//o,null.1,code.2,data.--
     modify_descriptor(gdt_addr,9,(bit_32)(&(process_table[task_2].tss)),0xfffff,at_386_tss+d_32);
     modify_descriptor(gdt_addr,10,(bit_32)(process_table[task_2].ldt),0xfffff,at_ldt+d_32);
     //--------------------temp task-----------------------------------
     process_table[task_0].pid=0x10101010;
     init_tss(process_table,task_0,0); //不需要初始化,所以只写了个0。
     init_ldt(process_table,task_0);
     //--
     modify_descriptor(gdt_addr,5,(bit_32)(&(process_table[task_0].tss)),0xfffff,at_386_tss+d_32);
     modify_descriptor(gdt_addr,6,(bit_32)(process_table[task_0].ldt),0xfffff,at_ldt+d_32);//****
     //**********************
     //=========================================================================
     //增加一个16位段的选择子。
     //modify_descriptor(gdt_addr,9,0,0xfffff,at_386_tss+d_32);
     //=========================================================================
     init_state();
     init_switch();
     //-----

     #if 0
     /* init 8253/8254, channel 0. timer is 10ms. */
     outb_p(0x43, 0x36);
     outb_p(0x40, (1193180/100) & 0xff);
     outb(0x40, (1193180/100) >> 8);
     #endif

     modify_int(time_int,0x20);

     open_time_int();

     int_c='#';

     active_window_id=0;

//     __asm__("sti");
//     __asm__ __volatile__("ltr %%ax;\
//              ljmp $7*8,$0x2000"\
//             :
//             :"a"(5*8)); //保存内核任务状态.
//     //__asm__("int $0x20");//先显示一个。

     __asm__ __volatile__("ltr %%ax;\
                           sti"\
                           :
                           :"a"(5*8));
}

void debug_break()
{
     ;
}

void f_kb_int()
{
     __asm__("cli");
     unsigned char scan_code;
     msg keyboardmsg;

     keyboardmsg.hwnd=active_window_id;
     keyboardmsg.message=msg_keyboard;

     inb(0x60,scan_code);
     if(text_g)
     disp_color(1,41,10,50,scan_code%16);

     #if 0
     /* 换心脏 */
     if ((0x3c == scan_code) && (task_status_running == state[0]))
     {
         debug_break();
         /* 具体的地址参照map文件 */
         #if 0
         __asm__("ljmp 0x00820290\n");
         译成了ljmp ds:0x820290
         #endif
         if (1)
         {
             set_vga_mode();//new
             //dead();
             outb(0x3ce,0x5);
             outb(0x3cf,0x6);
             text_g=1;
             openmov();//开机画面。
             while(1);
         }
         __asm__ __volatile__("movl $0x00820290,%%eax;\
                               jmp %%eax;"\
                               :
                               :
                               :"%eax");
     }
     #endif

     if(scan_code>1&&scan_code<0xc)//num
     {//disp_color(500,400,539,479,2);
                                      if(text_g==1)
                                      {
                                        keyboardmsg.wparam=scan_code-1;
                                        send_message_to_kernal(keyboardmsg);
                                        }
                                        else
                                        {
                                        //图形界面和消息机制，下面的没用。
         if(key_place_control_sel)//需要改变位置
         key_place_control(scan_code-1,0);
         else
         {
             print_int(scan_code-1,15,*curse_x,*curse_y);
             save_curse(*curse_x,*curse_y+1);
             move_curse(*curse_x,*curse_y);
         }
         }
     }
     if(scan_code>0xf&&scan_code<0x36)
     {//disp_color(500,400,539,479,4);
                                      if(text_g==1)
                                      {
                                      keyboardmsg.wparam=scan_code-0x10+0xb;
                                      send_message_to_kernal(keyboardmsg);
                                      //图形界面和消息机制，下面的没用。
                                      }
                                      else
                                      {
                                      if(key_place_control_sel)
                                      key_place_control(key_make_code[scan_code-0x10],1);
                                      else
                                      {
                                          print_char(key_make_code[scan_code-0x10],15,*curse_x,*curse_y);
                                          save_curse(*curse_x,*curse_y+1);
                                          move_curse(*curse_x,*curse_y);
                                      }
                                      }
     }
     if(text_g==1)
     {
     if(scan_code==0xf)//tab键，就暂时用这个做活动窗口的切换。
     active_window_id=(active_window_id+1)%window_count;//加1式的切换。
     }
     __asm__("sti");
     if(text_g)
     {disp_color(1,51,10,60,active_window_id%16);disp_color(1,61,10,70,time_window_no%16);}

     send_master_8259a_eoi();
     //outb(0xa0,0x20);
     //outb(0x20,0x20);
}

void open_kb_int()
{
    __asm__ __volatile__("cli;\
                          inb $0x21,%%al;\
                          movb %%al,%%ah;\
                          andb $0xfd,%%ah;\
                          movb %%ah,%%al;\
                          outb %%al,$0x21"\
                          :
                          :
                          :"%eax");
}

void test_keyboard_int()
{
     //print_char('d',15,20,1);
     modify_int(kb_int,0x21);

     open_kb_int();

     __asm__("sti");
     //__asm__("int $0x21");
}

