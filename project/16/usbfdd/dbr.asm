;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; (C)Copyright 2009, gaocheng.
;
; All Rights Reserved.
;
; file name  : dbr.asm
; version    : 1.0
; author     : gaocheng
; date       : 2009-04-22
; file size  : 512K
; description: mbr
;              cs:ip = 0000H:7c00H
;              asm16.bin is 512K*16(2000H-4000H)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;import
include ..\dlayout.inc

include ..\rlayout.inc
include ..\grf.inc

include media.inc

DBR_SS = 01H
DBR_SH = 0ffH ;-1. because bios maybe start from dbr directly.
DBR_SC = 00H

.386P
;in real mode , 16 bit
code segment use16
     assume cs:code
     org 07c00h
     main proc
       start:
         jmp code_area
         ;fat32
         asus db 4dH,53H,57H,49H,4eH
              db 34H,2eH,31H,00H,02H,08H,0fcH,00H
              db 02H,00H,00H,00H,00H,0f8H,00H,00H
              db 3fH,00H,0ffH,00H,3fH,00H,00H,00H
              db 00H,00H,00H,0ffH,0f8H,0fH,00H,00H
	      db 00H,00H,00H,00H,02H,00H,00H,00H
	      db 01H,00H,06H,00H,00H,00H,00H,00H
	      db 00H,00H,00H,00H,00H,00H,00H,00H
	      db 00H,00H,29H,4eH,0c7H,0f8H,9cH,4eH
	      db 4fH,20H,4eH,41H,4dH,45H,20H,20H
	      db 20H,20H,46H,41H,54H,33H,32H,20H
	      db 20H,20H
         ;standard data
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
	 ;call disp_dbr_info
	 ;call set_disk_dbr_bsp
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
	 mov di,GRF_BDPT_S
	 and cl,3fH
	 mov es:[di],cl
	 ;set bsp
	 mov al,es:[GRF_BSP_FLAG]
	 cmp al,1
	 je boot_mbr
	 mov byte ptr es:[GRF_BSP_FLAG],1 ;record
	 mov byte ptr es:[GRF_BSP_SS],DBR_SS ;not use GRF_BSP_SS->di, es:[di] and DBR_SS->al
	 mov byte ptr es:[GRF_BSP_SH],DBR_SH
	 mov byte ptr es:[GRF_BSP_SC],DBR_SC
       boot_mbr:
	 ;call load_asm16
	 ;input global parameter and read disk
	 mov ax,ASM16_LEN
	 xor dx,dx
	 mov cx,BYTE_PER_SECTOR
	 div cx
	 mov cs:[lba_cnt],al ;use al only, less than 255
	 mov dword ptr cs:[lba_no],ASM16_LBA
	 mov word ptr cs:[lba_buffer_seg],ASM16_SEG
	 mov word ptr cs:[lba_buffer_offset],ASM16_OFFSET
         ;call read_disk
     ;description: read disk by lba
     ;             C=LBA DIV (PH*PS) + Cs
     ;             H=(LBA DIV PS) MOD PH + Hs
     ;             S=LBA MOD PS + Ss
     ;input: global variable, lba_no, lba_cnt
     ;read_disk proc
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
;        ret
;     read_disk endp
         ;goto asm16
         mov word ptr cs:[jump_addr],ASM16_OFFSET
         mov word ptr cs:[jump_addr+2],ASM16_SEG
         jmp dword ptr cs:[jump_addr]
     main endp

;     disp_dbr_info proc
;         mov ax,0b800H
;         mov es,ax
;         mov di,(80*1+0)*2
;         mov ah,0fH
;         mov al,'d'
;         mov es:[di],ax
;         ret
;     disp_dbr_info endp

;         ;load
;         mov ax,0000H
;         mov es,ax
;         ;02 is the no of function , 16 is one count sector.
;         mov ax,0210H
;         mov bx,asm16_addr
;         mov cx,0001H
;         mov dx,0100H
;         int 13H

     ;end signal
     org 7dfeH
         dw 0aa55H
code ends
       end start
