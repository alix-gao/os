
//实现保护模式下设置显卡的模式。
void save_pm_info()
{
     char*psi=(char*)(0x000);;
     char*pdi=(char*)(0x200000);
     int i;
     for(i=0;i<0x100000;i++)
     *(pdi+i)=*(psi+i);
}

void save_bios()
{
     ;//no need
}

//这个函数完成了之后，分页就没了。当使用函数返回，出栈入栈，mov指令等需要寻址时，必将出错。
void load_bios()
{
     char*pdi=(char*)(0x000);;
     char*psi=(char*)(0x104000);
     int i;
     for(i=0;i<0x0a0000;i++)
     *(pdi+i)=*(psi+i);
     for(i=0x0c0000;i<0x100000;i++)
     *(pdi+i)=*(psi+i);
     //这个函数必须加大代码。
     //gotorealmode
     //close
}

//先关中断，关分页，进实模式，转移代码。
//返回保护模式的过程中，各寄存器需要保护，堆栈需要保护。
void load_bios_e()
{//打算全部用汇编了。
     struct gdtv_tmp
     {
            short limit;
            int offset;
     }gdtv;
     gdtv.limit=256*8-1;
     gdtv.offset=0x2806;//0x200000+0x800;
     //传的地址是32位，而传入后用的时候是16位。
     __asm__("cli;\
     movl %%cr0,%%eax;\
     andl $0x7fffffff,%%eax;\
     movl %%eax,%%cr0;\
     ljmp $8,$1f;\
     1:\
     movl $0x104000,%%esi;\
     movl $0x0,%%edi;\
     movl $0x500,%%ecx;\
     rep;\
     movsb;\
     movl $0x104000+0x9fc00,%%esi;\
     movl $0x9fc00,%%edi;\
     movl $0x0a0000-0x9fc00,%%ecx;\
     rep;\
     movsb;\
     movl $0x104000+0x0c0000,%%esi;\
     movl $0x0c0000,%%edi;\
     movl $0x100000-0x0c0000,%%ecx;\
     rep;\
     movsb;\
     ljmp $24,$2f;\
     2:\
     movl %%cr0,%%eax;\
     andl $0xfffffffe,%%eax;\
     movl %%eax,%%cr0;\
     ljmp $0,$3f;\
     3:\
     movb $0,%%ah;\
     movb $0x12,%%al;\
     int $0x10;\
     .byte 0x66,0x67;\
     lgdt %%gs:(0x2005);\
     .byte 0x66;\
     movl %%cr0,%%eax;\
     .byte 0x66;\
     orl $1,%%eax;\
     .byte 0x66;\
     movl %%eax,%%cr0;\
     .byte 0x66;\
     ljmp $8,$4f;\
     4:\
     movw $16,%%ax;\
     movw %%ax,%%ds;\
     movw %%ax,%%es;\
     movw %%ax,%%fs;\
     movw %%ax,%%gs;\
     movw %%ax,%%ss;\
     movl $0x200000,%%esi;\
     movl $0x0,%%edi;\
     movl $0x500,%%ecx;\
     rep;\
     movsb;\
     movl $0x200000+0x9fc00,%%esi;\
     movl $0x9fc00,%%edi;\
     movl $0x0a0000-0x9fc00,%%ecx;\
     rep;\
     movsb;\
     movl $0x200000+0x0c0000,%%esi;\
     movl $0x0c0000,%%edi;\
     movl $0x100000-0x0c0000,%%ecx;\
     rep;\
     movsb;\
     lidt (0x200b);\
     movl $0x1000,%%eax;\
     movl %%eax,%%cr3;\
     movl %%cr0,%%eax;\
     orl $0x80000000,%%eax;\
     movl %%eax,%%cr0;\
     ljmp $8,$5f;\
     5:\
     sti"\
     :
     :
     :"%eax","%edi","%esi","%ecx","%ebx");
//          ljmp $0,$2f;
//     2:
}

void load_pm_info()
{
     char*pdi=(char*)(0x000);;
     char*psi=(char*)(0x204000);
     int i;
     for(i=0;i<0x100000;i++)
     *(pdi+i)=*(psi+i);
}

//为了在实模式下能使用4G内存，
//此处必须从关闭分页机制，进入实模式
//设置vga显示模式，
//进入保护模式，设置分页机制
//一次性写完。
void goto_real_mode()
{
     __asm__("cli;\
              movl %%cr0,%%eax;\
              andl $0xfffffffe,%%eax;\
              movl %%eax,%%cr0;\
              ljmp $0,$1f;\
              1:\
              ljmp $0,$1b;\
              sti"\
              :
              :
              :"%eax");
}

void goto_pm_mode()
{
     __asm__("cli;\
              movl %%cr0,%%eax;\
              orl $1,%%eax;\
              movl %%eax,%%cr0;\
              ljmp $8,$1f;\
              1:\
              sti"\
              :
              :
              :"%eax");
}

void use_bios_1m()
{
     int *ldt16code = (int*)(0x110000);

     *(ldt16code + 7) = 0x00009801;

     __asm__("movl %%esp,(0x1000290-16);\
	 		 movw %%ss,(0x1000290-20);\
		     lidt (0x132008);\
		     ljmp $8,$1f;\
		     1:\
		     ljmp $24,$2f-0x1000000;\
		     2:\
		     movl %%cr0,%%eax;\
		     andl $0xfffffffe,%%eax;\
		     movl %%eax,%%cr0;\
		     .byte 0x66,0x67;\
		     ljmp $0x1000,$3f-0x1000000;\
		     3:\
		     .byte 0x66,0x67;\
		     movw $0x4000,%%ax;\
		     .byte 0x66,0x67;\
		     movw %%ax,%%ss;\
		     .byte 0x66,0x67;\
		     movl $0xfffc,%%esp;\
		     movb $0,%%dh;\
		     movb $0,%%dl;\
		     movb $0,%%ch;\
		     movb $0,%%cl;\
		     movb $0,%%bh;\
		     movb $0,%%bl;\
		     movb $0,%%ah;\
		     movb $0x12,%%al;\
		     int $0x10;\
		     .byte 0x66,0x67;\
		     lgdt %%gs:(0x132010);\
		     .byte 0x66;\
		     movl %%cr0,%%eax;\
		     .byte 0x66;\
		     orl $1,%%eax;\
		     .byte 0x66;\
		     movl %%eax,%%cr0;\
		     .byte 0x66;\
		     ljmp $8,$4f;\
		     4:\
		     movw $16,%%ax;\
		     movw %%ax,%%ds;\
		     movw %%ax,%%es;\
		     movw %%ax,%%fs;\
		     movw %%ax,%%gs;\
		     movw %%ax,%%ss;\
		     movw (0x1000290-20),%%ax;\
		     movw %%ax,%%ss;\
		     movl (0x1000290-16),%%esp;\
		     lidt (0x132018);\
		     ljmp $8,$5f;\
		     5:\
		     nop"\
		     :
		     :
		     :"%eax","%edi","%esi","%ecx","%ebx");
}

void move_os_1k()
{
    char *src = (char *)(0x1000000);
    char *dest = (char *)(0x10000);
	int i;

	/* mov 1k */
	for (i=0; i<0x10000; i++)
    {
        *dest = *src;
        dest++;
        src++;
    }
}

void set_vga_mode()
{
//     __asm__("cli;\
//     1:\
//     ljmp $8,$1b;"\
//     :
//     :
//     :);//can be used.
//__asm__("int $0");
move_os_1k();

use_bios_1m();

//              save_pm_info();//先后顺序的考虑。
//              //load_bios();//注意，这部分1M内存中有一些不能写。如显卡内存。
//              load_bios_e();


              //while(1){;}
//     __asm__("int $0;\
//     1:\
//     ljmp $8,$1b;"\
//     :
//     :
//     :);//can be used.

//一开始关中断了，所以时钟，键盘都不响应了。

              //goto_real_mode();
//              __asm__("cli;\
//              movb $0,%%ah;\
//              movb $0x12,%%al;\
//              int $0x10;\
//              sti"\
//              :
//              :
//              :"%eax");
////              mov ah,0
////              mov al,12h
////              int 10h
////重返保护模式，应该不需要重新加载ldt和gdt。
//              goto_pm_mode();//与下面的函数顺序不能颠倒。
              //load_pm_info();//保护模式下，才能使用这个函数。
}

