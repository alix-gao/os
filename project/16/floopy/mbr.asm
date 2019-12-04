;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; (C)Copyright 2009, gaocheng.
;
; All Rights Reserved.
;
; file name  : mbr.asm
; version    : 1.0
; author     : gaocheng
; date       : 2009-04-22
; file size  : 512K
; description: mbr
;              cs:ip = 0000H:7c00H
;              asm16.bin is 512*16(4000H-6000H)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;import
include ..\rlayout.inc
include ..\grf.inc

include dlayout.inc

;macro
MBR_MSG_LEN = 3

MBR_SS = 1
MBR_SH = 0
MBR_SC = 0

.386P

;in real mode, 16 bit.
code segment use16
     assume cs:code
     org 7c00H
     main proc
       start:
         jmp code_area
	 ;nop
         ;code data section
         mbr_msg db 'mbr'
	 ;long jump data
         jump_addr dd 00000000H
	 ;lba disk read input para
	 lba_buffer_seg dw 0000H
	 lba_buffer_offset dw 0000H
	 lba_no dd 00000000H
	 lba_cnt db 00H
	 ;read_disk var
	 h_cache db 00H ;start header
	 s_cache db 00H ;start sector
	 n_cache db 00H ;sector num
	 c_cache dw 0000H
       code_area:
         cli
         mov ax,SYSTEM_STACK_SEG
         mov ss,ax
         mov sp,SYSTEM_STACK_OFFSET
         call clean_screen
         call disp_msg
	 call set_disk_mbr_bsp
	 call load_asm16
         ;jump to load.
         mov ax,ASM16_OFFSET
         mov word ptr [jump_addr],ax
         mov ax,ASM16_SEG
         mov word ptr [jump_addr+2],ax
         jmp [jump_addr]
     main endp

     clean_screen proc
         mov ax,0b800H
         mov es,ax
         mov edi,(80*0+0)*2
         mov ecx,(80*24+79)*2
       clean:
         ;color is black, disp is null.
         xor ax,ax
         mov es:[edi],ax
         add edi,2
         loop clean
         ret
     clean_screen endp

     disp_msg proc
         ;cursor position
         mov ah,02H
         mov bh,00H
         mov dx,0100H
         int 10H
         ;cs is 0.
         mov ax,cs
         mov es,ax
         lea bp,mbr_msg
         mov cx,MBR_MSG_LEN
         ;use bios
         mov ax,1301H
         mov bx,000fH
         mov dl,00H
         int 10H
         ret
     disp_msg  endp

     set_disk_mbr_bsp proc
         ;get disk parameter
         mov dl,DISK_DRIVER_TYPE ;floppy disk
         mov ah,08H
	 int 13H
	 mov ax,GRF_SEG
	 mov es,ax
	 mov es:[GRF_BDPT_H],dh
	 mov es:[GRF_BDPT_C],ch
	 mov al,cl
	 shr al,6
	 mov es:[GRF_BDPT_C+1],al
	 and cl,3fH
	 mov es:[GRF_BDPT_S],cl
	 ;set bsp
	 mov al,1 ;record
	 mov es:[GRF_BSP_FLAG],al
	 mov al,MBR_SS
	 mov es:[GRF_BSP_SS],al
	 mov al,MBR_SH
	 mov es:[GRF_BSP_SH],al
	 mov al,MBR_SC
	 mov es:[GRF_BSP_SC],al
	 ret
     set_disk_mbr_bsp endp

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

     load_asm16 proc
         ;input global parameter and read disk
	 mov ax,ASM16_LEN
	 xor dx,dx
	 mov cx,BYTE_PER_SECTOR
	 div cx
	 mov cs:[lba_cnt],al ;use al only, less than 255
	 mov eax,ASM16_LBA
	 mov cs:[lba_no],eax
	 mov ax,ASM16_SEG
	 mov cs:[lba_buffer_seg],ax
	 mov ax,ASM16_OFFSET
	 mov cs:[lba_buffer_offset],ax
         call read_disk
	 ret
     load_asm16 endp

;     read_disk_chs proc
;         mov ax,ASM16_SEG
;         mov es,ax
;         ;02 is the function no, 0x10 is sector no.
;         mov ax,0210H
;         ;dest addr is es:bx
;         mov bx,ASM16_OFFSET
;         ;the no 2 of sector
;         mov cx,0001H
;         ;driver 0 is drive a, head 0.
;         mov dx,0100H
;         int 13H
;         ret
;     read_disk_chs endp

        ;7c00H+510
        org 7dfeH
        ;end signal
        dw 0aa55H
code ends
       end start

