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
include ..\dlayout.inc
include ..\grf.inc
include ..\rlayout.inc

include media.inc

;macro
MBR_CACHE_SEG = 2000H
MBR_CACHE_OFFSET = 7c00H

MBR_SS = 1
MBR_SH = 0
MBR_SC = 0

.386P
;in real mode , 16 bit
code segment use16
     assume cs:code
     org 07c00H
     main proc
       start:
         jmp code_area
	 nop
         ;data
         jump_addr dd 00000000H
       code_area:
         cli
         mov ax,system_stack_seg
         mov ss,ax
         mov sp,system_stack_offset
         ;disp info
         call disp_mbr_info
         ;set global ram file
         call set_disk_mbr_bsp
	 ;mov to mbr cache
	 call move_mbr_to_cache
	 ;jump to mbr cache
	 lea ax,mbr_cache_code
         mov word ptr [jump_addr],ax
         mov ax,MBR_CACHE_SEG
         mov word ptr [jump_addr+2],ax
         jmp dword ptr [jump_addr] ;jmp far ptr [jump_addr] is error.
       mbr_cache_code:
         call load_dbr
         ;jump
         mov ax,DBR_OFFSET
         mov word ptr [jump_addr],ax
         mov ax,DBR_SEG
         mov word ptr [jump_addr+2],ax
         jmp [jump_addr]
     main endp

     disp_mbr_info proc
         mov ax,0b800H
         mov es,ax
         mov edi,(80*0+0)*2
         mov ah,0fH
         mov al,'m'
         mov es:[edi],ax
	 ret
     disp_mbr_info endp

     move_mbr_to_cache proc
         mov si,MBR_OFFSET ;mbr offset
	 mov ax,MBR_SEG ;mbr seg
	 mov ds,ax
	 mov di,MBR_CACHE_OFFSET
	 mov ax,MBR_CACHE_SEG
	 mov es,ax
	 mov cx,200H ;move one sector
       mov_mbr:
         mov al,ds:[si]
         mov es:[di],al
	 inc si
	 inc di
         loop mov_mbr
         ret
     move_mbr_to_cache endp

     set_disk_mbr_bsp proc
         ;get disk parameter
         mov dl,DISK_DRIVER_TYPE
         mov ah,08H
	 int 13H
	 mov ax,GRF_SEG
	 mov es,ax
	 mov di,GRF_BDPT_H
	 mov es:[di],dh
	 mov di,GRF_BDPT_C
	 mov es:[di],ch
	 mov al,cl
	 shr al,6
	 mov es:[di+1],al
	 mov di,GRF_BDPT_S
	 and cl,3fH
	 mov es:[di],cl
	 ;set bsp
	 mov di,GRF_BSP_FLAG
	 mov al,1 ;record
	 mov es:[di],al
	 mov di,GRF_BSP_SS
	 mov al,MBR_SS
	 mov es:[di],al
	 mov di,GRF_BSP_SH
	 mov al,MBR_SH
	 mov es:[di],al
	 mov di,GRF_BSP_SC
	 mov al,MBR_SC
	 mov es:[di],al
	 ret
     set_disk_mbr_bsp endp

     load_dbr proc
         xor ax,ax
	 mov ds,ax
         ;file load
         mov ax,DBR_SEG
         mov es,ax
	 mov bx,DBR_OFFSET
         mov ax,0201H ; ah = 02h (read sectors), al = sectors to read count
         mov ch,DBR_CYLINDER ; ch = cylinder,
         mov cl,DBR_SECTOR; cl = sector
         mov dh,DBR_HEAD ; dh = head
	 mov dl,DISK_DRIVER_TYPE ; DL=Çý¶¯Æ÷£¬00H~7FH£ºÈíÅÌ£»80H~0FFH£ºÓ²ÅÌ
         int 13H
         ret
     load_dbr endp

         ;i define data in code segment,
         ;but the next 2 phase cannot be place here,there can be only 0aa55h here.
         org 7dbeH ;here is dpt
         db 80H
         db 01H,01H,00H ;starting head, starting sector, starting cylinder
         db 0cH ;system id
         db 0ffH,0ffH,0ffH ;ending head, ending sector, ending cylinder
         db 3fH,00H,00H,00H ;relative sectors
         db 0c1H,37H,0cH,00H ;total sectors
         ;end signal
     org 7dfeH
         dw 0aa55H
code ends
       end start

