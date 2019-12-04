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
	?debug	C E96782B340066C6F61642E63
	?debug	C E93149A64009747970656465662E68
	?debug	C E9FA9EA840056C69622E68
_BSS	ends
_DATA	segment word public 'DATA'
cos32_version	label	word
	dw	0
_DATA	ends
LOAD_TEXT	segment	byte public 'CODE'
;	?debug	L 62
open_disk_drive	proc	far
;	?debug	L 64
 	push	 ax
;	?debug	L 65
 	xor	 ax,ax
;	?debug	L 66
 	int	 13H
;	?debug	L 67
 	pop	 ax
@1:
;	?debug	L 68
	ret	
open_disk_drive	endp
;	?debug	L 85
get_disk_bsp	proc	far
@5:
;	?debug	L 87
	mov	word ptr DGROUP:grf,0
;	?debug	L 87
	mov	word ptr DGROUP:grf+2,-28672
@3:
@4:
@2:
;	?debug	L 88
	ret	
get_disk_bsp	endp
;	?debug	L 90
read_disk_track	proc	far
	push	bp
	mov	bp,sp
	sub	sp,10
;	?debug	L 92
	mov	word ptr [bp-10],0
;	?debug	L 93
	mov	word ptr [bp-8],0
;	?debug	L 94
	mov	word ptr [bp-6],0
;	?debug	L 95
	mov	word ptr [bp-4],0
;	?debug	L 96
	mov	word ptr [bp-2],0
;	?debug	L 100
	les	bx,dword ptr DGROUP:grf
	mov	al,byte ptr es:[bx]
	mov	ah,0
	or	ax,512
	mov	word ptr [bp-10],ax
;	?debug	L 101
	les	bx,dword ptr DGROUP:grf
	mov	ax,word ptr es:[bx+8]
	add	ax,word ptr [bp+8]
	and	ax,768
	shr	ax,1
	shr	ax,1
	les	bx,dword ptr DGROUP:grf
	mov	dl,byte ptr es:[bx+6]
	mov	dh,0
	and	dx,63
	or	ax,dx
	les	bx,dword ptr DGROUP:grf
	mov	dx,word ptr es:[bx+8]
	add	dx,word ptr [bp+8]
	and	dx,255
	mov	cl,8
	shl	dx,cl
	or	ax,dx
	mov	word ptr [bp-6],ax
;	?debug	L 102
	les	bx,dword ptr DGROUP:grf
	mov	al,byte ptr es:[bx+7]
	mov	ah,0
	mov	dl,byte ptr [bp+6]
	mov	dh,0
	add	ax,dx
	mov	cl,8
	shl	ax,cl
	or	ax,0
	mov	word ptr [bp-4],ax
;	?debug	L 103
	mov	ax,word ptr [bp+10]
	and	ax,15
	mov	word ptr [bp-8],ax
;	?debug	L 104
	mov	ax,word ptr [bp+10]
	mov	cl,4
	shr	ax,cl
	mov	word ptr [bp-2],ax
;	?debug	L 105
	mov	ax,word ptr [bp+12]
	mov	cl,12
	shl	ax,cl
	or	ax,word ptr [bp-2]
	mov	word ptr [bp-2],ax
;	?debug	L 107
 	push	 [bp-10]
;	?debug	L 108
 	push	 [bp-8]
;	?debug	L 109
 	push	 [bp-6]
;	?debug	L 110
 	push	 [bp-4]
;	?debug	L 111
 	push	 [bp-2]
;	?debug	L 112
 	pop	 ax
;	?debug	L 113
 	mov	 es,ax
;	?debug	L 114
 	pop	 dx
;	?debug	L 115
 	pop	 cx
;	?debug	L 116
 	pop	 bx
;	?debug	L 117
 	pop	 ax
;	?debug	L 118
 	int	 13H
@6:
;	?debug	L 119
	mov	sp,bp
	pop	bp
	ret	
read_disk_track	endp
;	?debug	L 121
load_asm32	proc	far
;	?debug	L 123
	mov	dx,2
	xor	ax,ax
	push	dx
	push	ax
	mov	ax,2
	push	ax
	mov	al,0
	push	ax
	push	cs
	call	near ptr read_disk_track
	add	sp,8
;	?debug	L 124
	mov	dx,2
	mov	ax,9216
	push	dx
	push	ax
	mov	ax,2
	push	ax
	mov	al,1
	push	ax
	push	cs
	call	near ptr read_disk_track
	add	sp,8
;	?debug	L 127
	xor	dx,dx
	mov	ax,8192
	push	dx
	push	ax
	mov	dx,19
	mov	ax,8192
	push	dx
	push	ax
	mov	dx,2
	xor	ax,ax
	push	dx
	push	ax
	call	far ptr _move_1m
	add	sp,12
@7:
;	?debug	L 128
	ret	
load_asm32	endp
;	?debug	L 130
load_os1	proc	far
;	?debug	L 132
	mov	dx,2
	xor	ax,ax
	push	dx
	push	ax
	mov	ax,3
	push	ax
	mov	al,0
	push	ax
	push	cs
	call	near ptr read_disk_track
	add	sp,8
;	?debug	L 133
	mov	dx,2
	mov	ax,9216
	push	dx
	push	ax
	mov	ax,3
	push	ax
	mov	al,1
	push	ax
	push	cs
	call	near ptr read_disk_track
	add	sp,8
;	?debug	L 134
	mov	dx,2
	mov	ax,18432
	push	dx
	push	ax
	mov	ax,4
	push	ax
	mov	al,0
	push	ax
	push	cs
	call	near ptr read_disk_track
	add	sp,8
;	?debug	L 135
	mov	dx,2
	mov	ax,27648
	push	dx
	push	ax
	mov	ax,4
	push	ax
	mov	al,1
	push	ax
	push	cs
	call	near ptr read_disk_track
	add	sp,8
;	?debug	L 147
	mov	dx,4
	xor	ax,ax
	push	dx
	push	ax
	mov	dx,256
	xor	ax,ax
	push	dx
	push	ax
	mov	dx,2
	xor	ax,ax
	push	dx
	push	ax
	call	far ptr _move_1m
	add	sp,12
@8:
;	?debug	L 148
	ret	
load_os1	endp
;	?debug	L 151
load_os2	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8
;	?debug	L 158
	mov	word ptr [bp-6],2
	mov	word ptr [bp-8],0
;	?debug	L 160
	mov	word ptr [bp-4],5
	jmp	@13
@12:
;	?debug	L 161
	mov	byte ptr [bp-1],0
	jmp	short @17
@16:
;	?debug	L 162
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	push	word ptr [bp-4]
	push	word ptr [bp-1]
	push	cs
	call	near ptr read_disk_track
	add	sp,8
;	?debug	L 163
	xor	dx,dx
	mov	ax,9216
	push	dx
	push	ax
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	push	ss
	lea	ax,word ptr [bp-8]
	push	ax
	call	far ptr _tc_32bit_add
	add	sp,12
	jmp	short @18
@20:
;	?debug	L 165
	xor	ax,ax
	push	ax
	xor	ax,ax
	push	ax
	mov	al,15
	push	ax
	push	word ptr [bp-4]
	call	far ptr _print_hex
	add	sp,8
;	?debug	L 166
	xor	ax,ax
	push	ax
	xor	ax,ax
	push	ax
	mov	al,15
	push	ax
	push	word ptr [bp-1]
	call	far ptr _print_hex
	add	sp,8
;	?debug	L 167
	xor	ax,ax
	push	ax
	xor	ax,ax
	push	ax
	mov	al,15
	push	ax
	push	word ptr [bp-8]
	call	far ptr _print_hex
	add	sp,8
@18:
@19:
@15:
	inc	byte ptr [bp-1]
@17:
	cmp	byte ptr [bp-1],1
	jbe	@16
@14:
@11:
	inc	word ptr [bp-4]
@13:
	cmp	word ptr [bp-4],16
	ja	@21
	jmp	@12
@21:
@10:
;	?debug	L 195
	mov	dx,4
	xor	ax,ax
	push	dx
	push	ax
	mov	dx,256
	xor	ax,ax
	push	dx
	push	ax
	mov	dx,2
	xor	ax,ax
	push	dx
	push	ax
	call	far ptr _move_1m
	add	sp,12
@9:
;	?debug	L 196
	mov	sp,bp
	pop	bp
	ret	
load_os2	endp
;	?debug	L 199
load_os3	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8
;	?debug	L 205
	mov	word ptr [bp-6],256
	mov	word ptr [bp-8],0
;	?debug	L 207
	mov	word ptr [bp-4],17
	jmp	@26
@25:
;	?debug	L 208
	mov	byte ptr [bp-1],0
	jmp	@30
@29:
;	?debug	L 209
	mov	dx,2
	xor	ax,ax
	push	dx
	push	ax
	push	word ptr [bp-4]
	push	word ptr [bp-1]
	push	cs
	call	near ptr read_disk_track
	add	sp,8
;	?debug	L 210
	xor	dx,dx
	mov	ax,9216
	push	dx
	push	ax
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	mov	dx,2
	xor	ax,ax
	push	dx
	push	ax
	call	far ptr _move_1m
	add	sp,12
;	?debug	L 211
	xor	dx,dx
	mov	ax,9216
	push	dx
	push	ax
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	push	ss
	lea	ax,word ptr [bp-8]
	push	ax
	call	far ptr _tc_32bit_add
	add	sp,12
	jmp	short @31
@33:
;	?debug	L 214
	xor	ax,ax
	push	ax
	xor	ax,ax
	push	ax
	mov	al,15
	push	ax
	push	word ptr [bp-4]
	call	far ptr _print_hex
	add	sp,8
;	?debug	L 215
	xor	ax,ax
	push	ax
	xor	ax,ax
	push	ax
	mov	al,15
	push	ax
	push	word ptr [bp-1]
	call	far ptr _print_hex
	add	sp,8
;	?debug	L 216
	xor	ax,ax
	push	ax
	xor	ax,ax
	push	ax
	mov	al,15
	push	ax
	push	word ptr [bp-8]
	call	far ptr _print_hex
	add	sp,8
@31:
@32:
@28:
	inc	byte ptr [bp-1]
@30:
	cmp	byte ptr [bp-1],1
	ja	@34
	jmp	@29
@34:
@27:
@24:
	inc	word ptr [bp-4]
@26:
	cmp	word ptr [bp-4],32
	ja	@35
	jmp	@25
@35:
@23:
@22:
;	?debug	L 219
	mov	sp,bp
	pop	bp
	ret	
load_os3	endp
;	?debug	L 221
load_os4	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8
;	?debug	L 227
	mov	word ptr [bp-6],256
	mov	word ptr [bp-8],0
;	?debug	L 229
	mov	word ptr [bp-4],33
	jmp	@40
@39:
;	?debug	L 230
	mov	byte ptr [bp-1],0
	jmp	@44
@43:
;	?debug	L 231
	mov	dx,2
	xor	ax,ax
	push	dx
	push	ax
	push	word ptr [bp-4]
	push	word ptr [bp-1]
	push	cs
	call	near ptr read_disk_track
	add	sp,8
;	?debug	L 232
	xor	dx,dx
	mov	ax,9216
	push	dx
	push	ax
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	mov	dx,2
	xor	ax,ax
	push	dx
	push	ax
	call	far ptr _move_1m
	add	sp,12
;	?debug	L 233
	xor	dx,dx
	mov	ax,9216
	push	dx
	push	ax
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	push	ss
	lea	ax,word ptr [bp-8]
	push	ax
	call	far ptr _tc_32bit_add
	add	sp,12
	jmp	short @45
@47:
;	?debug	L 236
	xor	ax,ax
	push	ax
	xor	ax,ax
	push	ax
	mov	al,15
	push	ax
	push	word ptr [bp-4]
	call	far ptr _print_hex
	add	sp,8
;	?debug	L 237
	xor	ax,ax
	push	ax
	xor	ax,ax
	push	ax
	mov	al,15
	push	ax
	push	word ptr [bp-1]
	call	far ptr _print_hex
	add	sp,8
;	?debug	L 238
	xor	ax,ax
	push	ax
	xor	ax,ax
	push	ax
	mov	al,15
	push	ax
	push	word ptr [bp-8]
	call	far ptr _print_hex
	add	sp,8
@45:
@46:
@42:
	inc	byte ptr [bp-1]
@44:
	cmp	byte ptr [bp-1],1
	ja	@48
	jmp	@43
@48:
@41:
@38:
	inc	word ptr [bp-4]
@40:
	cmp	word ptr [bp-4],79
	ja	@49
	jmp	@39
@49:
@37:
@36:
;	?debug	L 241
	mov	sp,bp
	pop	bp
	ret	
load_os4	endp
;	?debug	L 243
load_cos32	proc	far
;	?debug	L 245
	mov	ax,word ptr DGROUP:cos32_version
	dec	ax
	cmp	ax,3
	ja	@56
	mov	bx,ax
	shl	bx,1
	jmp	word ptr cs:@57[bx]
@57	label	word
	dw	@52
	dw	@53
	dw	@54
	dw	@55
@52:
;	?debug	L 247
	push	cs
	call	near ptr load_os1
;	?debug	L 248
	jmp	short @51
@53:
;	?debug	L 251
	push	cs
	call	near ptr load_os2
;	?debug	L 252
	jmp	short @51
@54:
;	?debug	L 255
	push	cs
	call	near ptr load_os3
;	?debug	L 256
	jmp	short @51
@55:
;	?debug	L 259
	push	cs
	call	near ptr load_os4
;	?debug	L 260
	jmp	short @51
@56:
;	?debug	L 263
	jmp	short @51
@51:
@50:
;	?debug	L 265
	ret	
load_cos32	endp
;	?debug	L 267
close_disk_drive	proc	far
;	?debug	L 269
 	push	 ax
;	?debug	L 270
 	push	 dx
;	?debug	L 271
 	mov	 dx,03f2H
;	?debug	L 272
 	mov	 al,00H
;	?debug	L 273
 	out	 dx,al
;	?debug	L 274
 	pop	 dx
;	?debug	L 275
 	pop	 ax
@58:
;	?debug	L 276
	ret	
close_disk_drive	endp
;	?debug	L 278
_keyboard_int_func	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 282
 	push	 ax
;	?debug	L 283
 	in	 al,060H
;	?debug	L 284
 	mov	 [bp-1],al
;	?debug	L 285
 	pop	 ax
;	?debug	L 290
	cmp	byte ptr [bp-1],59
	jb	@60
	cmp	byte ptr [bp-1],68
	ja	@60
;	?debug	L 291
	mov	al,byte ptr [bp-1]
	mov	ah,0
	add	ax,-58
	mov	word ptr DGROUP:cos32_version,ax
;	?debug	L 292
	mov	ax,17
	push	ax
	mov	ax,13
	push	ax
	mov	al,15
	push	ax
	mov	al,byte ptr DGROUP:cos32_version
	add	al,48
	push	ax
	call	far ptr _print_char
	add	sp,8
@60:
@59:
;	?debug	L 294
	mov	sp,bp
	pop	bp
	ret	
_keyboard_int_func	endp
;	?debug	L 296
modify_keyboard_int	proc	far
	push	si
	push	di
;	?debug	L 298
 	push	 ax
;	?debug	L 299
 	push	 di
;	?debug	L 300
 	mov	 ax,0
;	?debug	L 301
 	mov	 ds,ax
;	?debug	L 302
 	mov	 di,080H+4
;	?debug	L 303
 	mov	 word ptr ds:[di],offset _keyboard_int
;	?debug	L 304
 	add	 di,2
;	?debug	L 305
 	mov	 word ptr ds:[di],seg _keyboard_int
;	?debug	L 306
 	pop	 di
;	?debug	L 307
 	pop	 ax
@61:
;	?debug	L 308
	pop	di
	pop	si
	ret	
modify_keyboard_int	endp
;	?debug	L 310
open_keyboard_int	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 312
	mov	byte ptr [bp-1],0
;	?debug	L 314
 	push	 ax
;	?debug	L 315
 	in	 al,021H
;	?debug	L 316
 	mov	 [bp-1],al
;	?debug	L 317
 	pop	 ax
;	?debug	L 319
	mov	al,byte ptr [bp-1]
	and	al,253
	mov	byte ptr [bp-1],al
;	?debug	L 321
 	push	 ax
;	?debug	L 322
 	mov	 al,[bp-1]
;	?debug	L 323
 	out	 021H,al
;	?debug	L 325
 	in	 al,060H
;	?debug	L 326
 	pop	 ax
@62:
;	?debug	L 327
	mov	sp,bp
	pop	bp
	ret	
open_keyboard_int	endp
;	?debug	L 329
set_curse_pos	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 331
	mov	al,byte ptr [bp+6]
	mov	ah,0
	mov	dx,80
	mul	dx
	mov	dl,byte ptr [bp+8]
	mov	dh,0
	add	ax,dx
	mov	word ptr [bp-2],ax
;	?debug	L 333
 	push	 ax
;	?debug	L 334
 	push	 dx
;	?debug	L 335
 	push	 bx
;	?debug	L 336
 	db	 66H
;	?debug	L 337
 	mov	 bx,[bp-2]
;	?debug	L 338
 	mov	 al,0fH
;	?debug	L 339
 	mov	 dx,03d4H
;	?debug	L 340
 	out	 dx,al
;	?debug	L 341
 	nop	
;	?debug	L 342
 	db	 66H
;	?debug	L 343
 	mov	 ax,bx
;	?debug	L 344
 	db	 66H
;	?debug	L 345
 	and	 ax,00ffH
;	?debug	L 346
 	db	 00H,00H
;	?debug	L 347
 	mov	 dx,03d5H
;	?debug	L 348
 	db	 66H
;	?debug	L 349
 	out	 dx,ax
;	?debug	L 350
 	nop	
;	?debug	L 351
 	db	 66H
;	?debug	L 352
 	mov	 ax,000eH
;	?debug	L 353
 	db	 00H,00H
;	?debug	L 354
 	mov	 dx,03d4H
;	?debug	L 355
 	db	 66H
;	?debug	L 356
 	out	 dx,ax
;	?debug	L 357
 	nop	
;	?debug	L 358
 	db	 66H
;	?debug	L 359
 	shr	 bx,1
;	?debug	L 360
 	db	 66H
;	?debug	L 361
 	shr	 bx,1
;	?debug	L 362
 	db	 66H
;	?debug	L 363
 	shr	 bx,1
;	?debug	L 364
 	db	 66H
;	?debug	L 365
 	shr	 bx,1
;	?debug	L 366
 	db	 66H
;	?debug	L 367
 	shr	 bx,1
;	?debug	L 368
 	db	 66H
;	?debug	L 369
 	shr	 bx,1
;	?debug	L 370
 	db	 66H
;	?debug	L 371
 	shr	 bx,1
;	?debug	L 372
 	db	 66H
;	?debug	L 373
 	shr	 bx,1
;	?debug	L 374
 	db	 66H
;	?debug	L 375
 	and	 bx,00ffH
;	?debug	L 376
 	db	 00H,00H
;	?debug	L 377
 	db	 66H
;	?debug	L 378
 	mov	 ax,bx
;	?debug	L 379
 	mov	 dx,03d5H
;	?debug	L 380
 	db	 66H
;	?debug	L 381
 	out	 dx,ax
;	?debug	L 382
 	nop	
;	?debug	L 383
 	pop	 bx
;	?debug	L 384
 	pop	 dx
;	?debug	L 385
 	pop	 ax
@63:
;	?debug	L 386
	mov	sp,bp
	pop	bp
	ret	
set_curse_pos	endp
;	?debug	L 388
select_os	proc	far
;	?debug	L 395
	call	far ptr _os_8259a
;	?debug	L 397
	push	cs
	call	near ptr modify_keyboard_int
;	?debug	L 399
	push	cs
	call	near ptr open_keyboard_int
;	?debug	L 401
	xor	ax,ax
	push	ax
	mov	ax,13
	push	ax
	mov	al,15
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@
	push	ax
	call	far ptr _print_string
	add	sp,10
;	?debug	L 403
	mov	al,17
	push	ax
	mov	al,13
	push	ax
	push	cs
	call	near ptr set_curse_pos
	pop	cx
	pop	cx
;	?debug	L 405
 	sti	
	jmp	short @65
@67:
;	?debug	L 408
 	hlt	
@65:
;	?debug	L 407
	cmp	word ptr DGROUP:cos32_version,0
	je	@67
@66:
;	?debug	L 411
 	cli	
;	?debug	L 413
	call	far ptr _bios_8259a
@64:
;	?debug	L 414
	ret	
select_os	endp
;	?debug	L 416
get_disk_para	proc	far
	push	bp
	mov	bp,sp
	sub	sp,4
;	?debug	L 423
 	mov	 ah,08
;	?debug	L 424
 	mov	 dl,000H 
;	?debug	L 425
 	int	 13H
;	?debug	L 426
 	mov	 [bp-4],ch
;	?debug	L 427
 	mov	 [bp-3],cl
;	?debug	L 428
 	mov	 [bp-1],dh
;	?debug	L 429
 	mov	 [bp-2],dl
;	?debug	L 431
	mov	al,byte ptr [bp-3]
	mov	ah,0
	shl	ax,1
	shl	ax,1
	and	ax,255
	mov	dl,byte ptr [bp-4]
	mov	dh,0
	or	ax,dx
	mov	word ptr DGROUP:disk_para_table,ax
;	?debug	L 432
	mov	al,byte ptr [bp-1]
	mov	byte ptr DGROUP:disk_para_table+2,al
;	?debug	L 433
	mov	al,byte ptr [bp-3]
	and	al,63
	mov	byte ptr DGROUP:disk_para_table+3,al
;	?debug	L 434
	mov	al,byte ptr [bp-2]
	mov	byte ptr DGROUP:disk_para_table+4,al
;	?debug	L 436
	xor	ax,ax
	push	ax
	mov	ax,9
	push	ax
	mov	al,15
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+16
	push	ax
	call	far ptr _print_string
	add	sp,10
;	?debug	L 437
	mov	ax,16
	push	ax
	mov	ax,9
	push	ax
	mov	al,15
	push	ax
	push	word ptr DGROUP:disk_para_table
	call	far ptr _print_hex
	add	sp,8
;	?debug	L 439
	xor	ax,ax
	push	ax
	mov	ax,10
	push	ax
	mov	al,15
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+31
	push	ax
	call	far ptr _print_string
	add	sp,10
;	?debug	L 440
	mov	ax,11
	push	ax
	mov	ax,10
	push	ax
	mov	al,15
	push	ax
	push	word ptr DGROUP:disk_para_table+2
	call	far ptr _print_hex
	add	sp,8
;	?debug	L 442
	xor	ax,ax
	push	ax
	mov	ax,11
	push	ax
	mov	al,15
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+42
	push	ax
	call	far ptr _print_string
	add	sp,10
;	?debug	L 443
	mov	ax,13
	push	ax
	mov	ax,11
	push	ax
	mov	al,15
	push	ax
	push	word ptr DGROUP:disk_para_table+3
	call	far ptr _print_hex
	add	sp,8
;	?debug	L 445
	xor	ax,ax
	push	ax
	mov	ax,12
	push	ax
	mov	al,15
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+55
	push	ax
	call	far ptr _print_string
	add	sp,10
;	?debug	L 446
	mov	ax,13
	push	ax
	mov	ax,12
	push	ax
	mov	al,15
	push	ax
	push	word ptr DGROUP:disk_para_table+4
	call	far ptr _print_hex
	add	sp,8
@68:
;	?debug	L 447
	mov	sp,bp
	pop	bp
	ret	
get_disk_para	endp
;	?debug	L 449
_load_os	proc	far
;	?debug	L 451
	push	cs
	call	near ptr open_disk_drive
;	?debug	L 453
	push	cs
	call	near ptr get_disk_para
;	?debug	L 455
	push	cs
	call	near ptr get_disk_bsp
;	?debug	L 457
	push	cs
	call	near ptr select_os
;	?debug	L 459
	push	cs
	call	near ptr load_asm32
;	?debug	L 461
	push	cs
	call	near ptr load_cos32
;	?debug	L 463
	push	cs
	call	near ptr close_disk_drive
@69:
;	?debug	L 464
	ret	
_load_os	endp
LOAD_TEXT	ends
_BSS	segment word public 'BSS'
grf	label	dword
	db	4 dup (?)
disk_para_table	label	word
	db	5 dup (?)
_BSS	ends
	?debug	C E9
_DATA	segment word public 'DATA'
s@	label	byte
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
	db	50
	db	41
	db	58
	db	0
	db	100
	db	105
	db	115
	db	107
	db	32
	db	99
	db	121
	db	108
	db	105
	db	110
	db	100
	db	101
	db	114
	db	58
	db	0
	db	100
	db	105
	db	115
	db	107
	db	32
	db	104
	db	101
	db	97
	db	100
	db	58
	db	0
	db	100
	db	105
	db	115
	db	107
	db	32
	db	115
	db	101
	db	99
	db	116
	db	111
	db	114
	db	58
	db	0
	db	100
	db	105
	db	115
	db	107
	db	32
	db	100
	db	114
	db	105
	db	118
	db	101
	db	114
	db	58
	db	0
_DATA	ends
	extrn	_keyboard_int:far
	extrn	_print_string:far
	extrn	_tc_32bit_add:far
	extrn	_print_char:far
	extrn	_print_hex:far
	extrn	_bios_8259a:far
	extrn	_move_1m:far
	extrn	_os_8259a:far
LOAD_TEXT	segment	byte public 'CODE'
LOAD_TEXT	ends
_read_disk_track	equ	read_disk_track
_close_disk_drive	equ	close_disk_drive
_open_disk_drive	equ	open_disk_drive
_set_curse_pos	equ	set_curse_pos
_disk_para_table	equ	disk_para_table
_cos32_version	equ	cos32_version
_get_disk_bsp	equ	get_disk_bsp
_get_disk_para	equ	get_disk_para
_select_os	equ	select_os
	public	_load_os
_load_cos32	equ	load_cos32
_load_asm32	equ	load_asm32
_grf	equ	grf
_load_os4	equ	load_os4
_load_os3	equ	load_os3
_load_os2	equ	load_os2
_load_os1	equ	load_os1
_modify_keyboard_int	equ	modify_keyboard_int
_open_keyboard_int	equ	open_keyboard_int
	public	_keyboard_int_func
	end
