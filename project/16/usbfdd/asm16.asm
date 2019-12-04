;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; (C)Copyright 2009, gaocheng.
;
; All Rights Reserved.
;
; file name  : asm16.asm
; version    : 1.0
; author     : gaocheng
; date       : 2009-04-22
; file size  : 512*16
; description: cs:ip = 0000H:2000H
;              asm16.bin is 512*16(2000H-4000H)
;              cos16.bin is 512*63*2(4000H-10000H)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include ..\dlayout.inc

include ..\grf.inc
include ..\rlayout.inc

include media.inc

;macro
ASM16_MSG_LEN = 10

.386P

;in real mode, 16 bit
code segment use16
     assume cs:code
     org 2000H
     main proc
       start:
         jmp code_area
         nop
         ;jump mem
         jump_addr dd 00000000H
         ;display info
         asm16_msg db '16 bit asm'
	 ;lba disk read input para
	 lba_no dd 00000000H
	 lba_cnt db 00H
	 rsvd1 db 00H
	 lba_buffer_seg dw 0000H
	 lba_buffer_offset dw 0000H
	 ;read_disk var
	 c_cache dw 0000H
	 h_cache db 00H ;start header
	 s_cache db 00H ;start sector
	 n_cache db 00H ;sector num
	 rsvd2 db 00H
       code_area:
         cli
         mov ax,SYSTEM_STACK_SEG
         mov ss,ax
         mov esp,SYSTEM_STACK_OFFSET
         call far ptr disp_msg
         ;call far ptr get_mem_info
         ;call far ptr get_video_info
         ;call far ptr get_hd_info
         call far ptr set_system_timer
         call far ptr set_rtc
         call far ptr ring
         call far ptr open_a20
         ;load 16 bit cos
         call far ptr open_drivea
         call far ptr load_cos16
         call far ptr close_drivea
         ;go to cos16
         mov ax,COS16_OFFSET
         mov word ptr cs:[jump_addr],ax
         mov ax,COS16_SEG
         mov word ptr cs:[jump_addr+2],ax
         jmp cs:[jump_addr]
         ret
     main endp

     disp_msg proc far
         ;set cursor position
         mov ah,02H
         mov bh,00H
         mov dx,0300H
         int 10H
         ;display
         mov ax,cs
         mov ds,ax
         mov es,ax
         lea ax,asm16_msg
         mov bp,ax
         mov cx,ASM16_MSG_LEN
         mov ax,1301H
         mov bx,000fH
         mov dl,00H
         int 10H
         ret
     disp_msg  endp

     get_mem_info proc far
         mov ah,88H
         int 15H
         ;mov ds:[addr],ax
         ret
     get_mem_info endp

     get_video_info proc far
         mov ah,0fH
         int 10H
         ;mov ds:[addr],bx ;bh=display page
         ;mov ds:[addr],ax ;al=video mode, ah=window width
         ;check for ega/vga and some config parameters
         mov ah,12H
         mov bl,10H
         int 10H
         ;mov ds:[addr],ax
         ;mov ds:[addr],bx ;bl=video ram, bh=display state
         ;mov ds:[addr],cx ;video card parameters.
         ret
     get_video_info endp

     get_hd_info proc far
         ;get hd0 info
         mov ax,0000H
         mov ds,ax
         mov si,[4*41H]
         ;mov ax,0000H ;segment value
         ;mov es,ax
         ;mov di,0cH ;offset
         ;mov cx,10H
       ;ghd0_info:
         ;mov ax,ds:[si]
         ;mov es:[di],ax
         ;loop ghd0_info
         ;get hd1 info
         mov ax,0000H
         mov ds,ax
         mov si,[4*46H]
         ;mov ax,0000H
         ;mov es,ax
         ;mov di,1cH
         ;mov cx,10H
       ;ghd1_info:
         ;mov ax,ds:[si]
         ;mov es:[di],ax
         ;loop ghd1_info
         ;check hd1 exist
         mov ax,1500H
         mov dl,81H
         int 13H
         jc disk1_no_exist
         cmp ah,3
         je disk1_exist
       disk1_no_exist:
         ;mov ax,0000H
         ;mov es,ax
         ;mov di,1cH
         ;mov cx,10H
         ;mov ax,0
       ;disk1_0:
         ;mov es:[di],ax
         ;loop disk1_0
       disk1_exist:
         ret
     get_hd_info endp

     set_system_timer proc far
         ;1 tick = 10ms
         ;init 8253/8254, channel 0. mode 3.
         mov al,36H
         out 43H,al
         nop
         nop
         nop
         nop
         ;latch (1193180/100)&0xff = 9cH
         mov al,00H
         out 40H,al
         nop
         nop
         nop
         nop
         ;latch (1193180/100)>>8 = 2eH
         mov al,00H
         out 40H,al
         nop
         nop
         nop
         nop
         ret
     set_system_timer endp

     set_rtc proc far
         ret
     set_rtc endp

     ring proc far
         push ax
         push dx
         push bx
         push cx
         mov bx,0100H
         mov cx,0100H
         mov dx,cx
         in al,61H
         and al,11111100B
       trig:
         xor al,02H
         out 61H,al
         mov cx,bx
       delay:
         loop delay
         dec dx
         jne trig
         pop cx
         pop bx
         pop dx
         pop ax
         ret
     ring endp

     open_a20 proc far
       test_8042_1:
         in al,64H
         test al,2
         jnz test_8042_1
         mov al,0d1H
         out 64H,al
       test_8042_2:
         in al,64H
         test al,2
         jnz test_8042_2
         mov al,0dfH
         out 60H,al
       test_8042_3:
         in al,64H
         test al,2
         jnz test_8042_3
         mov al,0ffH
         out 64H,al
         ret
     open_a20 endp

     open_drivea proc far
         xor ax,ax
         int 13H
         ret
     open_drivea endp

     ;description: read disk by lba
     ;             C=LBA DIV (PH*PS) + Cs
     ;             H=(LBA DIV PS) MOD PH + Hs
     ;             S=LBA MOD PS + Ss
     ;input: global variable, lba_no, lba_cnt
     read_disk proc
       read_disk_loop:
         ;calculate start sector
         mov ax,GRF_SEG
	 mov es,ax
         mov edx,cs:[lba_no]
	 mov ax,dx
	 shr edx,16
	 xor cx,cx
	 mov cl,es:[GRF_BDPT_S]
	 div cx
	 add dl,es:[GRF_BSP_SS] ;remainder is dl
	 mov cs:[s_cache],dl
	 ;calculate sector num
         mov al,cs:[lba_cnt]
	 cmp al,0
	 je read_disk_end
         cmp al,es:[GRF_BDPT_S]
	 jb read_disk_last ;use al directly
	 mov al,es:[GRF_BDPT_S]
       read_disk_last:
         mov cs:[n_cache],al
	 mov ah,es:[GRF_BDPT_S]
	 mov al,cs:[s_cache]
	 sub ah,al
	 inc ah
         cmp ah,cs:[n_cache]
	 jge read_disk_so ;no bigger impossible
         mov cs:[n_cache],ah
       read_disk_so:
	 ;calculate start header
	 mov edx,cs:[lba_no]
	 mov ax,dx
	 shr edx,16
	 xor cx,cx
	 mov cl,es:[GRF_BDPT_S]
	 div cx
	 mov cl,es:[GRF_BDPT_H]
	 inc cx ;bugfix by gaocheng, max is 256
	 xor dx,dx
	 div cx
	 add dl,es:[GRF_BSP_SH] ;remainder is dx
	 mov cs:[h_cache],dl
	 ;calculate start cylinder
	 mov al,es:[GRF_BDPT_S]
	 xor ch,ch
	 mov cl,es:[GRF_BDPT_H]
	 inc cx ;bugfix by gaocheng, max is 256
	 mul cx
	 mov cx,ax ;result of mul is 16 bit
	 mov edx,cs:[lba_no]
	 mov ax,dx
	 shr edx,16
	 div cx
	 add ax,es:[GRF_BSP_SC]
	 mov cs:[c_cache],ax
	 ;update lba_no and lba_cnt
	 xor eax,eax
	 mov al,cs:[n_cache]
	 add cs:[lba_no],eax
	 mov al,cs:[n_cache]
         sub cs:[lba_cnt],al
	 ;use bios chs function
	 mov ax,cs:[lba_buffer_seg]
         mov es,ax
         mov bx,cs:[lba_buffer_offset] ;dest addr is es:bx
	 ;update buffer
	 mov al,cs:[n_cache]
	 mov cx,BYTE_PER_SECTOR
	 mul cx
	 add cs:[lba_buffer_offset],ax
	 adc dx,0 ;dx is set by mul instruction
	 shl dx,12
	 add cs:[lba_buffer_seg],dx
	 ;fill cx
	 mov ax,cs:[c_cache]
	 mov ch,al
	 shl ah,6
	 add ah,cs:[s_cache]
	 mov cl,ah
         ;driver 0 is drive a.
	 mov dl,DISK_DRIVER_TYPE
         mov dh,cs:[h_cache]
	 ;sector num
	 mov al,cs:[n_cache]
         ;02 is the function no
         mov ah,02H
         int 13H
	 jmp read_disk_loop
       read_disk_end:
	 ret
     read_disk endp

     load_cos16 proc far
         ;input global parameter and read disk
	 mov ax,cs
	 mov es,ax
	 mov ax,COS16_LEN
	 xor dx,dx
	 mov cx,BYTE_PER_SECTOR
	 div cx
	 mov cs:[lba_cnt],al ;use al only, less than 255
	 mov eax,COS16_LBA
	 mov cs:[lba_no],eax
	 mov ax,COS16_SEG
	 mov cs:[lba_buffer_seg],ax
	 mov ax,COS16_OFFSET
	 mov cs:[lba_buffer_offset],ax
         call read_disk
	 ret
     load_cos16 endp

     load_cos16_chs proc far
         ;head 2, track 0
         mov ax,COS16_SEG
         mov es,ax
         ;02 is the no of function, 0x3f is 63 sectors.
         mov ax,023fH
         mov bx,COS16_OFFSET
         ;track 0, the no 1 of sector
         mov cx,0001H
         ;driver DISK_DRIVER_TYPE is drive a, head 2
         mov dh,02H
         mov dl,DISK_DRIVER_TYPE
         int 13H
	 ;head 3, track 0
         mov ax,COS16_SEG
         mov es,ax
         ;02 is the no of function, 0x3f is 63 sectors.
         mov ax,023fH
         mov bx,COS16_OFFSET+3fH*200H
         ;track 0, the no 1 of sector
         mov cx,0001H
         ;driver DISK_DRIVER_TYPE is drive a, head 3
         mov dh,03H
         mov dl,DISK_DRIVER_TYPE
         int 13H
         ret
     load_cos16_chs endp

     close_drivea proc far
         push dx
         ;floppy disk control port, read only
         mov dx,03f2H
         ;close fdc, prohibit dma and int, close drive a:
         mov al,00H
         ;must use short format. 03f2H is over 0ffH.
         out dx,al
         pop dx
         ret
     close_drivea endp
code ends
       end start

