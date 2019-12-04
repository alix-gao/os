	ifndef	??version
?debug	macro
	endm
	endif
	?debug	S "load.c"
LOAD_TEXT	segment	byte public 'CODE'
DGROUP	group	_DATA,_BSS
	assume	cs:LOAD_TEXT,ds:DGROUP
LOAD_TEXT	ends
_DATA	segment word public 'DATA'
d@	label	byte
d@w	label	word
_DATA	ends
_BSS	segment word public 'BSS'
b@	label	byte
b@w	label	word
	?debug	C E9C770774F066C6F61642E63
	?debug	C E90FA4AA4009747970656465662E68
	?debug	C E9AA56AC40056C69622E68
	?debug	C E907A4AA40057666732E68
_BSS	ends
_DATA	segment word public 'DATA'
cos32_version	label	word
	dw	0
_DATA	ends
LOAD_TEXT	segment	byte public 'CODE'
;	?debug	L 32
open_disk_drive	proc	far
;	?debug	L 34
 	push	 ax
;	?debug	L 35
 	xor	 ax,ax
;	?debug	L 36
 	int	 13H
;	?debug	L 37
 	pop	 ax
@1:
;	?debug	L 38
	ret	
open_disk_drive	endp
;	?debug	L 40
read_disk_track	proc	far
	push	bp
	mov	bp,sp
	sub	sp,10
;	?debug	L 42
	mov	word ptr [bp-10],0
;	?debug	L 43
	mov	word ptr [bp-8],0
;	?debug	L 44
	mov	word ptr [bp-6],0
;	?debug	L 45
	mov	word ptr [bp-4],0
;	?debug	L 46
	mov	word ptr [bp-2],0
;	?debug	L 48
	mov	word ptr [bp-10],544
;	?debug	L 49
	mov	ax,word ptr [bp+8]
	and	ax,768
	sar	ax,1
	sar	ax,1
	mov	dx,word ptr [bp+8]
	and	dx,255
	mov	cl,8
	shl	dx,cl
	or	ax,dx
	or	ax,1
	mov	word ptr [bp-6],ax
;	?debug	L 50
	mov	al,byte ptr [bp+6]
	mov	ah,0
	mov	cl,8
	shl	ax,cl
	or	ax,0
	mov	word ptr [bp-4],ax
;	?debug	L 51
	mov	ax,word ptr [bp+10]
	and	ax,15
	mov	word ptr [bp-8],ax
;	?debug	L 52
	mov	ax,word ptr [bp+10]
	mov	cl,4
	shr	ax,cl
	mov	word ptr [bp-2],ax
;	?debug	L 53
	mov	ax,word ptr [bp+12]
	mov	cl,12
	shl	ax,cl
	or	ax,word ptr [bp-2]
	mov	word ptr [bp-2],ax
;	?debug	L 55
 	push	 [bp-10]
;	?debug	L 56
 	push	 [bp-8]
;	?debug	L 57
 	push	 [bp-6]
;	?debug	L 58
 	push	 [bp-4]
;	?debug	L 59
 	push	 [bp-2]
;	?debug	L 60
 	pop	 ax
;	?debug	L 61
 	mov	 es,ax
;	?debug	L 62
 	pop	 dx
;	?debug	L 63
 	pop	 cx
;	?debug	L 64
 	pop	 bx
;	?debug	L 65
 	pop	 ax
;	?debug	L 66
 	int	 13H
@2:
;	?debug	L 67
	mov	sp,bp
	pop	bp
	ret	
read_disk_track	endp
;	?debug	L 69
system_hlt	proc	far
;	?debug	L 71
	jmp	short @4
@6:
;	?debug	L 71
 	hlt	
@4:
;	?debug	L 71
	jmp	short @6
@5:
@3:
;	?debug	L 72
	ret	
system_hlt	endp
;	?debug	L 74
load_asm32	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 78
	mov	dx,19
	mov	ax,8192
	push	dx
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@
	push	ax
	call	far ptr _load_file
	add	sp,8
	mov	word ptr [bp-2],ax
;	?debug	L 79
	cmp	word ptr [bp-2],0
	je	@8
;	?debug	L 80
	push	cs
	call	near ptr system_hlt
@8:
@7:
;	?debug	L 86
	mov	sp,bp
	pop	bp
	ret	
load_asm32	endp
;	?debug	L 88
load_os1	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 92
	mov	dx,256
	xor	ax,ax
	push	dx
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+10
	push	ax
	call	far ptr _load_file
	add	sp,8
	mov	word ptr [bp-2],ax
;	?debug	L 93
	cmp	word ptr [bp-2],0
	je	@10
;	?debug	L 94
	push	cs
	call	near ptr system_hlt
@10:
@9:
;	?debug	L 102
	mov	sp,bp
	pop	bp
	ret	
load_os1	endp
;	?debug	L 104
load_os2	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 108
	mov	dx,256
	xor	ax,ax
	push	dx
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+18
	push	ax
	call	far ptr _load_file
	add	sp,8
	mov	word ptr [bp-2],ax
;	?debug	L 109
	cmp	word ptr [bp-2],0
	je	@12
;	?debug	L 110
	push	cs
	call	near ptr system_hlt
@12:
@11:
;	?debug	L 134
	mov	sp,bp
	pop	bp
	ret	
load_os2	endp
;	?debug	L 136
load_os3	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 140
	mov	dx,256
	xor	ax,ax
	push	dx
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+26
	push	ax
	call	far ptr _load_file
	add	sp,8
	mov	word ptr [bp-2],ax
;	?debug	L 141
	cmp	word ptr [bp-2],0
	je	@14
;	?debug	L 142
	push	cs
	call	near ptr system_hlt
@14:
@13:
;	?debug	L 144
	mov	sp,bp
	pop	bp
	ret	
load_os3	endp
LOAD_TEXT	ends
_DATA	segment word public 'DATA'
osx_name	label	byte
	db	111
	db	115
	db	120
	db	46
	db	98
	db	105
	db	110
	db	0
_DATA	ends
LOAD_TEXT	segment	byte public 'CODE'
;	?debug	L 150
load_cos32	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 154
	cmp	word ptr DGROUP:cos32_version,0
	jne	@16
;	?debug	L 155
	mov	word ptr DGROUP:cos32_version,4
;	?debug	L 156
	mov	ax,17
	push	ax
	mov	ax,9
	push	ax
	mov	al,15
	push	ax
	mov	al,byte ptr DGROUP:cos32_version
	add	al,48
	push	ax
	call	far ptr _print_char
	add	sp,8
@16:
;	?debug	L 158
	mov	al,byte ptr DGROUP:cos32_version
	add	al,48
	mov	byte ptr DGROUP:osx_name+2,al
;	?debug	L 159
	mov	dx,256
	xor	ax,ax
	push	dx
	push	ax
	push	ds
	mov	ax,offset DGROUP:osx_name
	push	ax
	call	far ptr _load_file
	add	sp,8
	mov	word ptr [bp-2],ax
;	?debug	L 160
	cmp	word ptr [bp-2],0
	je	@17
;	?debug	L 161
	mov	ax,17
	push	ax
	mov	ax,9
	push	ax
	mov	al,15
	push	ax
	mov	al,120
	push	ax
	call	far ptr _print_char
	add	sp,8
;	?debug	L 162
	push	cs
	call	near ptr system_hlt
@17:
@15:
;	?debug	L 164
	mov	sp,bp
	pop	bp
	ret	
load_cos32	endp
;	?debug	L 166
close_disk_drive	proc	far
;	?debug	L 168
 	push	 ax
;	?debug	L 169
 	push	 dx
;	?debug	L 170
 	mov	 dx,03f2H
;	?debug	L 171
 	mov	 al,00H
;	?debug	L 172
 	out	 dx,al
;	?debug	L 173
 	pop	 dx
;	?debug	L 174
 	pop	 ax
@18:
;	?debug	L 175
	ret	
close_disk_drive	endp
;	?debug	L 191
_keyboard_int_func	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 195
 	push	 ax
;	?debug	L 196
 	in	 al,060H
;	?debug	L 197
 	mov	 [bp-1],al
;	?debug	L 198
 	pop	 ax
;	?debug	L 203
	cmp	byte ptr [bp-1],59
	jb	@20
	cmp	byte ptr [bp-1],68
	ja	@20
;	?debug	L 204
	mov	al,byte ptr [bp-1]
	mov	ah,0
	add	ax,-58
	mov	word ptr DGROUP:cos32_version,ax
;	?debug	L 205
	mov	ax,17
	push	ax
	mov	ax,9
	push	ax
	mov	al,15
	push	ax
	mov	al,byte ptr DGROUP:cos32_version
	add	al,48
	push	ax
	call	far ptr _print_char
	add	sp,8
;	?debug	L 206
	call	far ptr _stop_rmho
@20:
@19:
;	?debug	L 208
	mov	sp,bp
	pop	bp
	ret	
_keyboard_int_func	endp
;	?debug	L 210
modify_keyboard_int	proc	far
	push	si
	push	di
;	?debug	L 212
 	push	 ax
;	?debug	L 213
 	push	 di
;	?debug	L 214
 	mov	 ax,0
;	?debug	L 215
 	mov	 ds,ax
;	?debug	L 216
 	mov	 di,080H+4
;	?debug	L 217
 	mov	 word ptr ds:[di],offset _keyboard_int
;	?debug	L 218
 	add	 di,2
;	?debug	L 219
 	mov	 word ptr ds:[di],seg _keyboard_int
;	?debug	L 220
 	pop	 di
;	?debug	L 221
 	pop	 ax
@21:
;	?debug	L 222
	pop	di
	pop	si
	ret	
modify_keyboard_int	endp
;	?debug	L 224
open_keyboard_int	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 226
	mov	byte ptr [bp-1],0
;	?debug	L 228
 	push	 ax
;	?debug	L 229
 	in	 al,021H
;	?debug	L 230
 	mov	 [bp-1],al
;	?debug	L 231
 	pop	 ax
;	?debug	L 233
	mov	al,byte ptr [bp-1]
	and	al,253
	mov	byte ptr [bp-1],al
;	?debug	L 235
 	push	 ax
;	?debug	L 236
 	mov	 al,[bp-1]
;	?debug	L 237
 	out	 021H,al
;	?debug	L 239
 	in	 al,060H
;	?debug	L 240
 	pop	 ax
@22:
;	?debug	L 241
	mov	sp,bp
	pop	bp
	ret	
open_keyboard_int	endp
;	?debug	L 243
set_curse_pos	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 245
	mov	al,byte ptr [bp+6]
	mov	ah,0
	mov	dx,80
	mul	dx
	mov	dl,byte ptr [bp+8]
	mov	dh,0
	add	ax,dx
	mov	word ptr [bp-2],ax
;	?debug	L 247
 	push	 ax
;	?debug	L 248
 	push	 dx
;	?debug	L 249
 	push	 bx
;	?debug	L 250
 	db	 66H
;	?debug	L 251
 	mov	 bx,[bp-2]
;	?debug	L 252
 	mov	 al,0fH
;	?debug	L 253
 	mov	 dx,03d4H
;	?debug	L 254
 	out	 dx,al
;	?debug	L 255
 	nop	
;	?debug	L 256
 	db	 66H
;	?debug	L 257
 	mov	 ax,bx
;	?debug	L 258
 	db	 66H
;	?debug	L 259
 	and	 ax,00ffH
;	?debug	L 260
 	db	 00H,00H
;	?debug	L 261
 	mov	 dx,03d5H
;	?debug	L 262
 	db	 66H
;	?debug	L 263
 	out	 dx,ax
;	?debug	L 264
 	nop	
;	?debug	L 265
 	db	 66H
;	?debug	L 266
 	mov	 ax,000eH
;	?debug	L 267
 	db	 00H,00H
;	?debug	L 268
 	mov	 dx,03d4H
;	?debug	L 269
 	db	 66H
;	?debug	L 270
 	out	 dx,ax
;	?debug	L 271
 	nop	
;	?debug	L 272
 	db	 66H
;	?debug	L 273
 	shr	 bx,1
;	?debug	L 274
 	db	 66H
;	?debug	L 275
 	shr	 bx,1
;	?debug	L 276
 	db	 66H
;	?debug	L 277
 	shr	 bx,1
;	?debug	L 278
 	db	 66H
;	?debug	L 279
 	shr	 bx,1
;	?debug	L 280
 	db	 66H
;	?debug	L 281
 	shr	 bx,1
;	?debug	L 282
 	db	 66H
;	?debug	L 283
 	shr	 bx,1
;	?debug	L 284
 	db	 66H
;	?debug	L 285
 	shr	 bx,1
;	?debug	L 286
 	db	 66H
;	?debug	L 287
 	shr	 bx,1
;	?debug	L 288
 	db	 66H
;	?debug	L 289
 	and	 bx,00ffH
;	?debug	L 290
 	db	 00H,00H
;	?debug	L 291
 	db	 66H
;	?debug	L 292
 	mov	 ax,bx
;	?debug	L 293
 	mov	 dx,03d5H
;	?debug	L 294
 	db	 66H
;	?debug	L 295
 	out	 dx,ax
;	?debug	L 296
 	nop	
;	?debug	L 297
 	pop	 bx
;	?debug	L 298
 	pop	 dx
;	?debug	L 299
 	pop	 ax
@23:
;	?debug	L 300
	mov	sp,bp
	pop	bp
	ret	
set_curse_pos	endp
;	?debug	L 302
_select_os	proc	far
;	?debug	L 309
	push	cs
	call	near ptr modify_keyboard_int
;	?debug	L 311
	push	cs
	call	near ptr open_keyboard_int
;	?debug	L 313
	xor	ax,ax
	push	ax
	mov	ax,9
	push	ax
	mov	al,15
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+34
	push	ax
	call	far ptr _print_string
	add	sp,10
;	?debug	L 315
	mov	al,16
	push	ax
	mov	al,9
	push	ax
	push	cs
	call	near ptr set_curse_pos
	pop	cx
	pop	cx
@24:
;	?debug	L 316
	ret	
_select_os	endp
;	?debug	L 318
_show_vfs	proc	far
;	?debug	L 320
	push	cs
	call	near ptr open_disk_drive
;	?debug	L 323
	call	far ptr _init_vfs
;	?debug	L 324
	push	ds
	mov	ax,offset DGROUP:s@+50
	push	ax
	call	far ptr _change_directory
	pop	cx
	pop	cx
;	?debug	L 325
	call	far ptr _list_vfs
@25:
;	?debug	L 326
	ret	
_show_vfs	endp
;	?debug	L 328
_load_os	proc	far
;	?debug	L 330
	push	cs
	call	near ptr load_asm32
;	?debug	L 335
	push	cs
	call	near ptr load_cos32
@26:
;	?debug	L 341
	ret	
_load_os	endp
LOAD_TEXT	ends
	?debug	C E9
_DATA	segment word public 'DATA'
s@	label	byte
	db	97
	db	115
	db	109
	db	51
	db	50
	db	46
	db	98
	db	105
	db	110
	db	0
	db	111
	db	115
	db	49
	db	46
	db	98
	db	105
	db	110
	db	0
	db	111
	db	115
	db	50
	db	46
	db	98
	db	105
	db	110
	db	0
	db	111
	db	115
	db	51
	db	46
	db	98
	db	105
	db	110
	db	0
	db	115
	db	101
	db	108
	db	101
	db	99
	db	116
	db	32
	db	111
	db	115
	db	40
	db	49
	db	47
	db	110
	db	41
	db	58
	db	0
	db	115
	db	121
	db	115
	db	116
	db	101
	db	109
	db	0
_DATA	ends
	extrn	_keyboard_int:far
	extrn	_print_string:far
	extrn	_print_char:far
	extrn	_stop_rmho:far
	extrn	_list_vfs:far
	extrn	_init_vfs:far
	extrn	_load_file:far
	extrn	_change_directory:far
LOAD_TEXT	segment	byte public 'CODE'
LOAD_TEXT	ends
_read_disk_track	equ	read_disk_track
_close_disk_drive	equ	close_disk_drive
_open_disk_drive	equ	open_disk_drive
_set_curse_pos	equ	set_curse_pos
_cos32_version	equ	cos32_version
_system_hlt	equ	system_hlt
	public	_select_os
	public	_show_vfs
	public	_load_os
_osx_name	equ	osx_name
_load_cos32	equ	load_cos32
_load_asm32	equ	load_asm32
_load_os3	equ	load_os3
_load_os2	equ	load_os2
_load_os1	equ	load_os1
_modify_keyboard_int	equ	modify_keyboard_int
_open_keyboard_int	equ	open_keyboard_int
	public	_keyboard_int_func
	end
