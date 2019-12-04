;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; (C)Copyright 2009, gaocheng.
;
; All Rights Reserved.
;
; file name  : asm32.asm
; version    : 1.0
; author     : gaocheng
; date       : 2009-04-22
; file size  : 512K*16
; description: ip = 102000H
;              asm32.bin is 512*16(102000H-104000H)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;macro
%define system_stack 132000H

%define idt_addr 100000H

%define gdt_addr 110000H

%define page_dir_addr 120000H

%define page_addr 400000H

%define msg_color 0fH

%define asm32_msg_len 10

%define asm32_msg_pos 15

%define pm_msg_len 12

%define pm_msg_pos 16

%define page_msg_len 11

%define page_msg_pos 17

[section .text]
[bits 32]
     ;use predigest define to compatible c program.
     org 132000H
     main:
       start:
         ;instruction
         jmp code_area
         nop
         ;bios idt pseudo
                     dw 0000H
         bios_pseudo dw 256*8-1
                     dd 00000000H ;offset bios
         ;gdt pseudo
                    dw 0000H
         gdt_pseudo dw 0ffffH ;limit
                    dd gdt_addr ;gdt base
         ;idt pseudo
                    dw 0000H
         idt_pseudo dw 0ffffH
                    dd idt_addr ;offset idt
         ;display info
         asm32_msg DB '32 bit asm'
         pm_msg db 'protect mode'
         page_msg db 'page opened'
       code_area:
	 cli
         ;init segment register
         mov ax,32
         mov ds,ax
         mov es,ax
         mov fs,ax
         mov gs,ax
         mov ss,ax
         mov esp,system_stack
         ;display info
         call disp_asm32_msg
         call init_gdt
         ;set idt&gdt
         lidt [ds:idt_pseudo]
         lgdt [ds:gdt_pseudo]
         ;init register
         mov ax,32
         mov ds,ax
         mov es,ax
         mov fs,ax
         mov gs,ax
         mov ss,ax
         mov esp,system_stack
         ;execute
         call disp_pm_msg
         ;call set_page
         call disp_page_msg
         call move_curse
         call set_pic
         ;jump to os
         mov eax,1000290H
         jmp eax
     ;main function end

     disp_asm32_msg:
         mov esi,0
         mov ecx,asm32_msg_len
         mov edi,0b8000H + (80*asm32_msg_pos+0)*2
         mov ah,msg_color
       setup_msg_loop:
         mov al,[ds:asm32_msg+esi]
         mov [gs:edi],ax
         inc esi
         add edi,2
         loop setup_msg_loop
         ret

     init_gdt:
         ;null
         xor eax,eax
         mov [ds:gdt_addr+00H],eax
         mov [ds:gdt_addr+04H],eax
	 ;code 16
         mov eax,0000ffffH
         mov [ds:gdt_addr+08H],eax
         mov eax,00009800H
         mov [ds:gdt_addr+0cH],eax
         ;data 16
         mov eax,0000ffffH
         mov [ds:gdt_addr+10H],eax
         mov eax,00009200H
         mov [ds:gdt_addr+14H],eax
         ;code 32
         mov eax,0000ffffH
         mov [ds:gdt_addr+18H],eax
         mov eax,00cf9a00H
         mov [ds:gdt_addr+1cH],eax
         ;data 32
         mov eax,0000ffffH
         mov [ds:gdt_addr+20H],eax
         mov eax,00cf9200H
         mov [ds:gdt_addr+24H],eax
         ret

     disp_pm_msg:
         mov esi,0
         mov ecx,pm_msg_len
         mov edi,0b8000H+(80*pm_msg_pos+0)*2
         mov ah,msg_color
       pm_msg_loop:
         mov al,[ds:pm_msg+esi]
         mov [ds:edi],ax
         inc esi
         add edi,2
         loop pm_msg_loop
         ret

     set_page:
         ;init page dir
         ;addr is ?&fffff,
         ;attribute is ?&fff,
         ;0x07 is the page existed and can be readed, writed.
         mov esi,page_dir_addr
         mov dword [esi+00],page_addr+0000H+7 ;page0+7
         mov dword [esi+04],page_addr+1000H+7 ;page1+7
         mov dword [esi+08],page_addr+2000H+7 ;page2+7
         mov dword [esi+12],page_addr+3000H+7 ;page3+7
         ;add new for 32M.
         mov dword [esi+16],page_addr+4000H+7
         mov dword [esi+20],page_addr+5000H+7
         mov dword [esi+24],page_addr+6000H+7
         mov dword [esi+28],page_addr+7000H+7
         ;init pagetable, but only page0.
         mov eax,0+7 ;4M
         mov edi,0 ;last offset addr of page0
         mov cx,1024*4*2 ;16 bit = 32767. so we can use it.
       set_page_loop:
         mov [page_addr+edi],eax
         add eax,1000H ;1000H is 4k...
         add edi,4 ;every item is 4 byte.
         loop set_page_loop
         ;open page mechanism
         mov eax,page_dir_addr
         ;pagedir must be 4k*n, so low 12 is 0.
         mov cr3,eax
         mov eax,cr0
         or eax,80000000H
         mov cr0,eax
         ret

     disp_page_msg:
         mov esi,0
         mov ecx,page_msg_len
         mov edi,0b8000H+(80*page_msg_pos+0)*2
         mov ah,msg_color
       page_msg_loop:
         mov al,[ds:page_msg+esi]
         mov [ds:edi],ax
         inc esi
         add edi,2
         loop page_msg_loop
         ret

     iodelay:
         nop
         nop
         nop
         nop
         ret

     move_curse:
         mov ebx,80*page_msg_pos+0 ;row*80+col
         mov al,0fH
         mov dx,3d4H
         out dx,al
         call iodelay
         mov eax,ebx
         and eax,0ffH
         mov dx,3d5H
         out dx,eax
         call iodelay
         mov eax,0eH
         mov dx,3d4H
         out dx,eax
         call iodelay
         shr ebx,8
         and ebx,0ffH
         mov eax,ebx
         mov dx,3d5H
         out dx,eax
         call iodelay
         ret

     set_pic:
         mov al,11H
         ;master, icw1
         out 20H,al
         call iodelay
         ;slave, icw1
         out 0a0H,al
         call iodelay
         ;irq0 int 0x20
         mov al,020H
         ;master, icw2
         out 21H,al
         call iodelay
         ;irq8 int 0x28
         mov al,28H
         ;slave, icw2
         out 0a1H,al
         call iodelay
         ;ir2 int
         mov al,04H
         ;master, icw3
         out 21H,al
         call iodelay
         ;master, ir2
         mov al,02H
         ;slave, icw3
         out 0a1H,al
         call iodelay
         mov al,01H
         ;master, icw4
         out 21H,al
         call iodelay
         ;slave, icw4
         out 0a1H,al
         call iodelay
         ;mask master int
         mov al,11111111B
         ;master ocw1
         out 21H,al
         call iodelay
         ;mask slave int
         mov al,11111111B
         ;slave, ocw1
         out 0a1H,al
         call iodelay
         ret

