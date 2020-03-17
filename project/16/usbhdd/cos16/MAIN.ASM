	ifndef	??version
?debug	macro
	endm
	endif
	?debug	S "main.c"
MAIN_TEXT	segment	byte public 'CODE'
DGROUP	group	_DATA,_BSS
	assume	cs:MAIN_TEXT,ds:DGROUP
MAIN_TEXT	ends
_DATA	segment word public 'DATA'
d@	label	byte
d@w	label	word
_DATA	ends
_BSS	segment word public 'BSS'
b@	label	byte
b@w	label	word
	?debug	C E9F9907150066D61696E2E63
	?debug	C E90FA4AA4009747970656465662E68
	?debug	C E9AA56AC40056C69622E68
	?debug	C E99560EE4C06726D686F2E68
	?debug	C E9F35EEE4C066C6F61642E68
_BSS	ends
MAIN_TEXT	segment	byte public 'CODE'
;	?debug	L 19
os_8259a	proc	far
;	?debug	L 21
 	push	 ax
;	?debug	L 23
 	mov	 al,11H
;	?debug	L 24
 	out	 20H,al
@4:
;	?debug	L 25
 	nop	
;	?debug	L 25
 	nop	
;	?debug	L 25
 	nop	
;	?debug	L 25
 	nop	
@2:
@3:
;	?debug	L 26
 	out	 0a0H,al
@7:
;	?debug	L 27
 	nop	
;	?debug	L 27
 	nop	
;	?debug	L 27
 	nop	
;	?debug	L 27
 	nop	
@5:
@6:
;	?debug	L 29
 	mov	 al,020H
;	?debug	L 30
 	out	 21H,al
@10:
;	?debug	L 31
 	nop	
;	?debug	L 31
 	nop	
;	?debug	L 31
 	nop	
;	?debug	L 31
 	nop	
@8:
@9:
;	?debug	L 33
 	mov	 al,28H
;	?debug	L 34
 	out	 0a1H,al
@13:
;	?debug	L 35
 	nop	
;	?debug	L 35
 	nop	
;	?debug	L 35
 	nop	
;	?debug	L 35
 	nop	
@11:
@12:
;	?debug	L 37
 	mov	 al,04H
;	?debug	L 38
 	out	 21H,al
@16:
;	?debug	L 39
 	nop	
;	?debug	L 39
 	nop	
;	?debug	L 39
 	nop	
;	?debug	L 39
 	nop	
@14:
@15:
;	?debug	L 41
 	mov	 al,02H
;	?debug	L 42
 	out	 0a1H,al
@19:
;	?debug	L 43
 	nop	
;	?debug	L 43
 	nop	
;	?debug	L 43
 	nop	
;	?debug	L 43
 	nop	
@17:
@18:
;	?debug	L 45
 	mov	 al,01H
;	?debug	L 46
 	out	 21H,al
@22:
;	?debug	L 47
 	nop	
;	?debug	L 47
 	nop	
;	?debug	L 47
 	nop	
;	?debug	L 47
 	nop	
@20:
@21:
;	?debug	L 48
 	out	 0a1H,al
@25:
;	?debug	L 49
 	nop	
;	?debug	L 49
 	nop	
;	?debug	L 49
 	nop	
;	?debug	L 49
 	nop	
@23:
@24:
;	?debug	L 50
 	mov	 al,11111111B
;	?debug	L 51
 	out	 21H,al
@28:
;	?debug	L 52
 	nop	
;	?debug	L 52
 	nop	
;	?debug	L 52
 	nop	
;	?debug	L 52
 	nop	
@26:
@27:
;	?debug	L 53
 	mov	 al,11111111B
;	?debug	L 54
 	out	 0a1H,al
@31:
;	?debug	L 55
 	nop	
;	?debug	L 55
 	nop	
;	?debug	L 55
 	nop	
;	?debug	L 55
 	nop	
@29:
@30:
;	?debug	L 56
 	pop	 ax
@1:
;	?debug	L 57
	ret	
os_8259a	endp
;	?debug	L 59
bios_8259a	proc	far
;	?debug	L 61
 	push	 ax
;	?debug	L 62
 	mov	 al,011H
;	?debug	L 63
 	out	 020H,al
@35:
;	?debug	L 64
 	nop	
;	?debug	L 64
 	nop	
;	?debug	L 64
 	nop	
;	?debug	L 64
 	nop	
@33:
@34:
;	?debug	L 65
 	out	 0a0H,al
@38:
;	?debug	L 66
 	nop	
;	?debug	L 66
 	nop	
;	?debug	L 66
 	nop	
;	?debug	L 66
 	nop	
@36:
@37:
;	?debug	L 67
 	mov	 al,008H
;	?debug	L 68
 	out	 021H,al
@41:
;	?debug	L 69
 	nop	
;	?debug	L 69
 	nop	
;	?debug	L 69
 	nop	
;	?debug	L 69
 	nop	
@39:
@40:
;	?debug	L 70
 	mov	 al,070H
;	?debug	L 71
 	out	 0a1H,al
@44:
;	?debug	L 72
 	nop	
;	?debug	L 72
 	nop	
;	?debug	L 72
 	nop	
;	?debug	L 72
 	nop	
@42:
@43:
;	?debug	L 73
 	mov	 al,004H
;	?debug	L 74
 	out	 021H,al
@47:
;	?debug	L 75
 	nop	
;	?debug	L 75
 	nop	
;	?debug	L 75
 	nop	
;	?debug	L 75
 	nop	
@45:
@46:
;	?debug	L 76
 	mov	 al,002H
;	?debug	L 77
 	out	 0a1H,al
@50:
;	?debug	L 78
 	nop	
;	?debug	L 78
 	nop	
;	?debug	L 78
 	nop	
;	?debug	L 78
 	nop	
@48:
@49:
;	?debug	L 79
 	mov	 al,001H
;	?debug	L 80
 	out	 021H,al
@53:
;	?debug	L 81
 	nop	
;	?debug	L 81
 	nop	
;	?debug	L 81
 	nop	
;	?debug	L 81
 	nop	
@51:
@52:
;	?debug	L 82
 	out	 0a1H,al
@56:
;	?debug	L 83
 	nop	
;	?debug	L 83
 	nop	
;	?debug	L 83
 	nop	
;	?debug	L 83
 	nop	
@54:
@55:
;	?debug	L 84
 	mov	 al,0b8H
;	?debug	L 85
 	out	 021H,al
@59:
;	?debug	L 86
 	nop	
;	?debug	L 86
 	nop	
;	?debug	L 86
 	nop	
;	?debug	L 86
 	nop	
@57:
@58:
;	?debug	L 87
 	mov	 al,09fH
;	?debug	L 88
 	out	 0a1H,al
@62:
;	?debug	L 89
 	nop	
;	?debug	L 89
 	nop	
;	?debug	L 89
 	nop	
;	?debug	L 89
 	nop	
@60:
@61:
;	?debug	L 90
 	pop	 ax
@32:
;	?debug	L 91
	ret	
bios_8259a	endp
;	?debug	L 93
cos16_info	proc	far
;	?debug	L 95
	xor	ax,ax
	push	ax
	mov	ax,5
	push	ax
	mov	al,15
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@
	push	ax
	call	far ptr _print_string
	add	sp,10
@63:
;	?debug	L 96
	ret	
cos16_info	endp
;	?debug	L 98
debug_mem	proc	far
	push	bp
	mov	bp,sp
	sub	sp,6
;	?debug	L 100
	mov	word ptr [bp-6],0
;	?debug	L 101
	mov	ax,word ptr [bp+6]
	xor	dx,dx
	mov	word ptr [bp-2],dx
	mov	word ptr [bp-4],ax
;	?debug	L 103
	mov	word ptr [bp-6],0
	jmp	short @68
@67:
;	?debug	L 104
	push	word ptr [bp-6]
	mov	ax,20
	push	ax
	mov	al,15
	push	ax
	les	bx,dword ptr [bp-4]
	mov	al,byte ptr es:[bx]
	push	ax
	call	far ptr _print_hex
	add	sp,8
;	?debug	L 105
	inc	word ptr [bp-4]
@66:
	mov	ax,word ptr [bp-6]
	inc	ax
	inc	ax
	mov	word ptr [bp-6],ax
@68:
	cmp	word ptr [bp-6],32
	jb	@67
@65:
@64:
;	?debug	L 107
	mov	sp,bp
	pop	bp
	ret	
debug_mem	endp
;	?debug	L 109
get_type	proc	far
;	?debug	L 111
	xor	ax,ax
	push	ax
	mov	ax,20
	push	ax
	mov	al,15
	push	ax
	mov	al,1
	push	ax
	call	far ptr _print_hex
	add	sp,8
;	?debug	L 112
	mov	ax,4
	push	ax
	mov	ax,20
	push	ax
	mov	al,15
	push	ax
	mov	al,2
	push	ax
	call	far ptr _print_hex
	add	sp,8
;	?debug	L 113
	mov	ax,8
	push	ax
	mov	ax,20
	push	ax
	mov	al,15
	push	ax
	mov	al,4
	push	ax
	call	far ptr _print_hex
	add	sp,8
;	?debug	L 114
	mov	ax,12
	push	ax
	mov	ax,20
	push	ax
	mov	al,15
	push	ax
	mov	al,1
	push	ax
	call	far ptr _print_hex
	add	sp,8
;	?debug	L 115
	mov	ax,16
	push	ax
	mov	ax,20
	push	ax
	mov	al,15
	push	ax
	mov	al,2
	push	ax
	call	far ptr _print_hex
	add	sp,8
;	?debug	L 116
	mov	ax,20
	push	ax
	mov	ax,20
	push	ax
	mov	al,15
	push	ax
	mov	al,4
	push	ax
	call	far ptr _print_hex
	add	sp,8
@69:
;	?debug	L 117
	ret	
get_type	endp
;	?debug	L 121
_main	proc	far
;	?debug	L 123
	push	cs
	call	near ptr cos16_info
;	?debug	L 125
	call	far ptr _show_vfs
;	?debug	L 127
	push	cs
	call	near ptr os_8259a
;	?debug	L 129
	call	far ptr _select_os
;	?debug	L 131
	call	far ptr _rmho
;	?debug	L 133
	push	cs
	call	near ptr bios_8259a
;	?debug	L 135
	call	far ptr _load_os
;	?debug	L 139
	call	far ptr _goto_asm32
@70:
;	?debug	L 140
	ret	
_main	endp
MAIN_TEXT	ends
	?debug	C E9
_DATA	segment word public 'DATA'
s@	label	byte
	db	49
	db	54
	db	32
	db	98
	db	105
	db	116
	db	32
	db	99
	db	111
	db	115
	db	0
_DATA	ends
	extrn	_print_string:far
	extrn	_print_hex:far
	extrn	_select_os:far
	extrn	_show_vfs:far
	extrn	_load_os:far
	extrn	_rmho:far
	extrn	_goto_asm32:far
MAIN_TEXT	segment	byte public 'CODE'
MAIN_TEXT	ends
_cos16_info	equ	cos16_info
_debug_mem	equ	debug_mem
_bios_8259a	equ	bios_8259a
_get_type	equ	get_type
_os_8259a	equ	os_8259a
	public	_main
	end
