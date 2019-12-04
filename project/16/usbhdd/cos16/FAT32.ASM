	ifndef	??version
?debug	macro
	endm
	endif
	?debug	S "fat32.c"
FAT32_TEXT	segment	byte public 'CODE'
DGROUP	group	_DATA,_BSS
	assume	cs:FAT32_TEXT,ds:DGROUP
FAT32_TEXT	ends
_DATA	segment word public 'DATA'
d@	label	byte
d@w	label	word
_DATA	ends
_BSS	segment word public 'BSS'
b@	label	byte
b@w	label	word
	?debug	C E917AA7A4F0766617433322E63
	?debug	C E90FA4AA4009747970656465662E68
	?debug	C E98EB02D4F0766617433322E68
	?debug	C E9AA56AC40056C69622E68
_BSS	ends
_DATA	segment word public 'DATA'
fat_start_sector	label	word
	dw	0
	dw	0
dir_start_sector	label	word
	dw	0
	dw	0
fat_col	label	word
	dw	0
fat_row	label	word
	dw	11
_DATA	ends
FAT32_TEXT	segment	byte public 'CODE'
;	?debug	L 64
fat_print_dec	proc	far
	push	bp
	mov	bp,sp
	sub	sp,6
;	?debug	L 68
	mov	byte ptr [bp-1],1
;	?debug	L 71
	mov	word ptr [bp-6],10000
	jmp	short @5
@4:
;	?debug	L 72
	mov	ax,word ptr [bp+6]
	xor	dx,dx
	div	word ptr [bp-6]
	mov	word ptr [bp-4],ax
;	?debug	L 73
	cmp	word ptr [bp-4],0
	jne	@7
	cmp	byte ptr [bp-1],0
	jne	@6
@7:
;	?debug	L 74
	mov	byte ptr [bp-1],0
;	?debug	L 75
	mov	ax,word ptr [bp-4]
	mul	word ptr [bp-6]
	sub	word ptr [bp+6],ax
;	?debug	L 77
	push	word ptr DGROUP:fat_col
	push	word ptr DGROUP:fat_row
	mov	al,15
	push	ax
	mov	al,byte ptr [bp-4]
	add	al,48
	push	ax
	call	far ptr _print_char
	add	sp,8
;	?debug	L 78
	inc	word ptr DGROUP:fat_col
@6:
@3:
	mov	ax,word ptr [bp-6]
	mov	bx,10
	xor	dx,dx
	div	bx
	mov	word ptr [bp-6],ax
@5:
	cmp	word ptr [bp-6],1
	ja	@4
@2:
;	?debug	L 82
	push	word ptr DGROUP:fat_col
	push	word ptr DGROUP:fat_row
	mov	al,15
	push	ax
	mov	al,byte ptr [bp+6]
	add	al,48
	push	ax
	call	far ptr _print_char
	add	sp,8
;	?debug	L 83
	inc	word ptr DGROUP:fat_col
;	?debug	L 86
	cmp	byte ptr [bp+8],0
	je	@8
	jmp	short @9
@11:
;	?debug	L 88
	sub	word ptr DGROUP:fat_col,80
;	?debug	L 89
	inc	word ptr DGROUP:fat_row
@9:
;	?debug	L 87
	cmp	word ptr DGROUP:fat_col,79
	ja	@11
@10:
;	?debug	L 92
	inc	word ptr DGROUP:fat_row
;	?debug	L 93
	mov	word ptr DGROUP:fat_col,0
@8:
@1:
;	?debug	L 95
	mov	sp,bp
	pop	bp
	ret	
fat_print_dec	endp
;	?debug	L 97
fat_print_hex	proc	far
	push	bp
	mov	bp,sp
;	?debug	L 100
	push	word ptr DGROUP:fat_col
	push	word ptr DGROUP:fat_row
	mov	al,15
	push	ax
	push	word ptr [bp+6]
	call	far ptr _print_hex
	mov	sp,bp
;	?debug	L 102
	mov	ax,word ptr DGROUP:fat_col
	inc	ax
	inc	ax
	mov	word ptr DGROUP:fat_col,ax
;	?debug	L 105
	cmp	byte ptr [bp+8],0
	je	@13
	jmp	short @14
@16:
;	?debug	L 107
	sub	word ptr DGROUP:fat_col,80
;	?debug	L 108
	inc	word ptr DGROUP:fat_row
@14:
;	?debug	L 106
	cmp	word ptr DGROUP:fat_col,79
	ja	@16
@15:
;	?debug	L 111
	inc	word ptr DGROUP:fat_row
;	?debug	L 112
	mov	word ptr DGROUP:fat_col,0
@13:
@12:
;	?debug	L 114
	pop	bp
	ret	
fat_print_hex	endp
;	?debug	L 116
fat_print_dword_hex	proc	far
	push	bp
	mov	bp,sp
;	?debug	L 119
	push	word ptr DGROUP:fat_col
	push	word ptr DGROUP:fat_row
	mov	al,15
	push	ax
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	call	far ptr _print_32bit_hex
	mov	sp,bp
;	?debug	L 121
	mov	ax,word ptr DGROUP:fat_col
	add	ax,8
	mov	word ptr DGROUP:fat_col,ax
;	?debug	L 124
	cmp	byte ptr [bp+10],0
	je	@18
	jmp	short @19
@21:
;	?debug	L 126
	sub	word ptr DGROUP:fat_col,80
;	?debug	L 127
	inc	word ptr DGROUP:fat_row
@19:
;	?debug	L 125
	cmp	word ptr DGROUP:fat_col,79
	ja	@21
@20:
;	?debug	L 130
	inc	word ptr DGROUP:fat_row
;	?debug	L 131
	mov	word ptr DGROUP:fat_col,0
@18:
@17:
;	?debug	L 133
	pop	bp
	ret	
fat_print_dword_hex	endp
;	?debug	L 135
fat_print_char	proc	far
	push	bp
	mov	bp,sp
;	?debug	L 137
	push	word ptr DGROUP:fat_col
	push	word ptr DGROUP:fat_row
	mov	al,15
	push	ax
	push	word ptr [bp+6]
	call	far ptr _print_char
	mov	sp,bp
;	?debug	L 139
	inc	word ptr DGROUP:fat_col
;	?debug	L 142
	cmp	byte ptr [bp+8],0
	je	@23
	jmp	short @24
@26:
;	?debug	L 144
	sub	word ptr DGROUP:fat_col,80
;	?debug	L 145
	inc	word ptr DGROUP:fat_row
@24:
;	?debug	L 143
	cmp	word ptr DGROUP:fat_col,79
	ja	@26
@25:
;	?debug	L 148
	inc	word ptr DGROUP:fat_row
;	?debug	L 149
	mov	word ptr DGROUP:fat_col,0
@23:
@22:
;	?debug	L 151
	pop	bp
	ret	
fat_print_char	endp
;	?debug	L 153
fat_print_str	proc	far
	push	bp
	mov	bp,sp
	jmp	short @28
@30:
;	?debug	L 156
	push	word ptr DGROUP:fat_col
	push	word ptr DGROUP:fat_row
	mov	al,15
	push	ax
	les	bx,dword ptr [bp+6]
	mov	al,byte ptr es:[bx]
	push	ax
	call	far ptr _print_char
	mov	sp,bp
;	?debug	L 157
	inc	word ptr [bp+6]
;	?debug	L 158
	inc	word ptr DGROUP:fat_col
@28:
;	?debug	L 155
	les	bx,dword ptr [bp+6]
	cmp	byte ptr es:[bx],0
	jne	@30
@29:
;	?debug	L 162
	cmp	byte ptr [bp+10],0
	je	@31
	jmp	short @32
@34:
;	?debug	L 164
	sub	word ptr DGROUP:fat_col,80
;	?debug	L 165
	inc	word ptr DGROUP:fat_row
@32:
;	?debug	L 163
	cmp	word ptr DGROUP:fat_col,79
	ja	@34
@33:
;	?debug	L 168
	inc	word ptr DGROUP:fat_row
;	?debug	L 169
	mov	word ptr DGROUP:fat_col,0
@31:
@27:
;	?debug	L 171
	pop	bp
	ret	
fat_print_str	endp
;	?debug	L 198
get_disk_bsp	proc	far
@38:
;	?debug	L 200
	mov	word ptr DGROUP:grf,0
;	?debug	L 200
	mov	word ptr DGROUP:grf+2,-28672
@36:
@37:
@35:
;	?debug	L 201
	ret	
get_disk_bsp	endp
;	?debug	L 204
read_chs_disk	proc	far
	push	bp
	mov	bp,sp
	sub	sp,10
;	?debug	L 206
	mov	word ptr [bp-10],0
;	?debug	L 207
	mov	word ptr [bp-8],0
;	?debug	L 208
	mov	word ptr [bp-6],0
;	?debug	L 209
	mov	word ptr [bp-4],0
;	?debug	L 210
	mov	word ptr [bp-2],0
;	?debug	L 214
	mov	ax,512
	or	ax,word ptr [bp+10]
	mov	word ptr [bp-10],ax
;	?debug	L 215
	les	bx,dword ptr [bp+6]
	mov	ax,word ptr es:[bx+1]
	and	ax,768
	sar	ax,1
	sar	ax,1
	les	bx,dword ptr [bp+6]
	mov	dx,word ptr es:[bx+3]
	and	dx,63
	or	ax,dx
	les	bx,dword ptr [bp+6]
	mov	dx,word ptr es:[bx+1]
	and	dx,255
	mov	cl,8
	shl	dx,cl
	or	ax,dx
	mov	word ptr [bp-6],ax
;	?debug	L 216
	les	bx,dword ptr [bp+6]
	mov	al,byte ptr es:[bx]
	mov	ah,0
	mov	cl,8
	shl	ax,cl
	mov	dl,byte ptr DGROUP:_DISK_DRIVER_TYPE
	mov	dh,0
	or	ax,dx
	mov	word ptr [bp-4],ax
;	?debug	L 217
	mov	ax,word ptr [bp+12]
	and	ax,15
	mov	word ptr [bp-8],ax
;	?debug	L 218
	mov	ax,word ptr [bp+12]
	mov	cl,4
	shr	ax,cl
	mov	word ptr [bp-2],ax
;	?debug	L 219
	mov	ax,word ptr [bp+14]
	mov	cl,12
	shl	ax,cl
	or	ax,word ptr [bp-2]
	mov	word ptr [bp-2],ax
@42:
;	?debug	L 222
 	push	 es
;	?debug	L 223
 	push	 dx
;	?debug	L 224
 	push	 cx
;	?debug	L 225
 	push	 bx
;	?debug	L 226
 	push	 ax
;	?debug	L 228
 	push	 [bp-10]
;	?debug	L 229
 	push	 [bp-8]
;	?debug	L 230
 	push	 [bp-6]
;	?debug	L 231
 	push	 [bp-4]
;	?debug	L 232
 	push	 [bp-2]
;	?debug	L 233
 	pop	 ax
;	?debug	L 234
 	mov	 es,ax
;	?debug	L 235
 	pop	 dx
;	?debug	L 236
 	pop	 cx
;	?debug	L 237
 	pop	 bx
;	?debug	L 238
 	pop	 ax
;	?debug	L 239
 	int	 13H
;	?debug	L 241
 	pop	 ax
;	?debug	L 242
 	pop	 bx
;	?debug	L 243
 	pop	 cx
;	?debug	L 244
 	pop	 dx
;	?debug	L 245
 	pop	 es
@40:
@41:
@39:
;	?debug	L 247
	mov	sp,bp
	pop	bp
	ret	
read_chs_disk	endp
;	?debug	L 250
read_lba_disk	proc	far
	push	bp
	mov	bp,sp
	sub	sp,20
;	?debug	L 263
	mov	al,byte ptr DGROUP:disk_para_table+3
	mov	ah,0
	cwd	
	push	dx
	push	ax
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	ss
	lea	ax,word ptr [bp-10]
	push	ax
	push	ss
	lea	ax,word ptr [bp-14]
	push	ax
	call	far ptr _tc_32bit_div
	add	sp,16
;	?debug	L 264
	les	bx,dword ptr DGROUP:grf
	mov	al,byte ptr es:[bx+6]
	mov	ah,0
	add	ax,word ptr [bp-10]
	mov	word ptr [bp-17],ax
;	?debug	L 265
	mov	al,byte ptr DGROUP:disk_para_table+2
	mov	ah,0
	cwd	
	push	dx
	push	ax
	push	word ptr [bp-12]
	push	word ptr [bp-14]
	push	ss
	lea	ax,word ptr [bp-10]
	push	ax
	push	ss
	lea	ax,word ptr [bp-14]
	push	ax
	call	far ptr _tc_32bit_div
	add	sp,16
;	?debug	L 266
	les	bx,dword ptr DGROUP:grf
	mov	al,byte ptr es:[bx+7]
	add	al,byte ptr [bp-10]
	mov	byte ptr [bp-20],al
;	?debug	L 267
	les	bx,dword ptr DGROUP:grf
	mov	ax,word ptr es:[bx+8]
	add	ax,word ptr [bp-14]
	mov	word ptr [bp-19],ax
;	?debug	L 268
	cmp	byte ptr [bp-20],255
	jne	@44
;	?debug	L 269
	dec	word ptr [bp-19]
@44:
	jmp	@45
@47:
;	?debug	L 274
	mov	al,byte ptr DGROUP:disk_para_table+3
	mov	ah,0
	cmp	ax,word ptr [bp+10]
	jae	@48
;	?debug	L 275
	les	bx,dword ptr DGROUP:grf
	mov	al,byte ptr es:[bx+6]
	mov	ah,0
	mov	dl,byte ptr DGROUP:disk_para_table+3
	mov	dh,0
	sub	dx,word ptr [bp-17]
	add	ax,dx
	mov	word ptr [bp-6],ax
;	?debug	L 276
	jmp	short @49
@48:
;	?debug	L 277
	mov	ax,word ptr [bp+10]
	mov	word ptr [bp-6],ax
@49:
;	?debug	L 280
	mov	ax,word ptr [bp-6]
	sub	word ptr [bp+10],ax
;	?debug	L 282
	push	word ptr [bp+14]
	push	word ptr [bp+12]
	push	word ptr [bp-6]
	push	ss
	lea	ax,word ptr [bp-20]
	push	ax
	push	cs
	call	near ptr read_chs_disk
	add	sp,10
;	?debug	L 285
	les	bx,dword ptr DGROUP:grf
	mov	al,byte ptr es:[bx+6]
	mov	ah,0
	mov	word ptr [bp-17],ax
;	?debug	L 286
	inc	byte ptr [bp-20]
;	?debug	L 287
	mov	al,byte ptr DGROUP:disk_para_table+2
	mov	ah,0
	dec	ax
	mov	dl,byte ptr [bp-20]
	mov	dh,0
	cmp	ax,dx
	jge	@50
;	?debug	L 288
	les	bx,dword ptr DGROUP:grf
	mov	al,byte ptr es:[bx+7]
	mov	byte ptr [bp-20],al
;	?debug	L 289
	inc	word ptr [bp-19]
@50:
;	?debug	L 292
	mov	ax,word ptr DGROUP:bpb_para_table+12
	xor	dx,dx
	push	dx
	push	ax
	mov	ax,word ptr [bp-6]
	xor	dx,dx
	push	dx
	push	ax
	push	ss
	lea	ax,word ptr [bp-4]
	push	ax
	call	far ptr _tc_32bit_mul
	add	sp,12
;	?debug	L 293
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	push	word ptr [bp+14]
	push	word ptr [bp+12]
	push	ss
	lea	ax,word ptr [bp+12]
	push	ax
	call	far ptr _tc_32bit_add
	add	sp,12
@45:
;	?debug	L 272
	cmp	word ptr [bp+10],0
	je	@51
	jmp	@47
@51:
@46:
@43:
;	?debug	L 295
	mov	sp,bp
	pop	bp
	ret	
read_lba_disk	endp
;	?debug	L 298
read_clus_disk	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8
@55:
;	?debug	L 303
	xor	dx,dx
	mov	ax,2
	push	dx
	push	ax
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	call	far ptr _tc_32bit_cmp
	add	sp,8
	cmp	ax,-1
	je	@56
;	?debug	L 303
	jmp	short @57
@56:
;	?debug	L 303
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 303
	mov	al,1
	push	ax
	mov	ax,303
	push	ax
	push	cs
	call	near ptr fat_print_dec
	pop	cx
	pop	cx
@60:
;	?debug	L 303
 	sti	
;	?debug	L 303
	jmp	short @61
@63:
;	?debug	L 303
 	hlt	
@61:
;	?debug	L 303
	jmp	short @63
@62:
@58:
@59:
@57:
@53:
@54:
;	?debug	L 305
	xor	dx,dx
	mov	ax,2
	push	dx
	push	ax
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	ss
	lea	ax,word ptr [bp-8]
	push	ax
	call	far ptr _tc_32bit_sub
	add	sp,12
;	?debug	L 307
	mov	al,byte ptr DGROUP:bpb_para_table
	mov	ah,0
	cwd	
	push	dx
	push	ax
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	push	ss
	lea	ax,word ptr [bp-4]
	push	ax
	call	far ptr _tc_32bit_mul
	add	sp,12
;	?debug	L 308
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	push	word ptr DGROUP:dir_start_sector+2
	push	word ptr DGROUP:dir_start_sector
	push	ss
	lea	ax,word ptr [bp-4]
	push	ax
	call	far ptr _tc_32bit_add
	add	sp,12
;	?debug	L 309
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	mov	al,byte ptr DGROUP:bpb_para_table
	mov	ah,0
	push	ax
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	push	cs
	call	near ptr read_lba_disk
	add	sp,10
@52:
;	?debug	L 310
	mov	sp,bp
	pop	bp
	ret	
read_clus_disk	endp
;	?debug	L 313
extend_read_disk	proc	far
	push	bp
	mov	bp,sp
	sub	sp,10
;	?debug	L 324
	mov	word ptr [bp-10],0
;	?debug	L 325
	mov	word ptr [bp-8],0
;	?debug	L 326
	mov	word ptr [bp-6],0
;	?debug	L 327
	mov	word ptr [bp-4],0
;	?debug	L 328
	mov	word ptr [bp-2],0
;	?debug	L 332
	mov	ax,512
	or	ax,word ptr [bp+12]
	mov	word ptr [bp-10],ax
;	?debug	L 333
	mov	ax,word ptr [bp+8]
	and	ax,768
	sar	ax,1
	sar	ax,1
	mov	dx,word ptr [bp+10]
	and	dx,63
	or	ax,dx
	mov	dx,word ptr [bp+8]
	and	dx,255
	mov	cl,8
	shl	dx,cl
	or	ax,dx
	mov	word ptr [bp-6],ax
;	?debug	L 334
	mov	al,byte ptr [bp+6]
	mov	ah,0
	mov	cl,8
	shl	ax,cl
	mov	dl,byte ptr DGROUP:_DISK_DRIVER_TYPE
	mov	dh,0
	or	ax,dx
	mov	word ptr [bp-4],ax
;	?debug	L 335
	mov	ax,word ptr [bp+14]
	and	ax,15
	mov	word ptr [bp-8],ax
;	?debug	L 336
	mov	ax,word ptr [bp+14]
	mov	cl,4
	shr	ax,cl
	mov	word ptr [bp-2],ax
;	?debug	L 337
	mov	ax,word ptr [bp+16]
	mov	cl,12
	shl	ax,cl
	or	ax,word ptr [bp-2]
	mov	word ptr [bp-2],ax
@67:
;	?debug	L 340
 	push	 es
;	?debug	L 341
 	push	 dx
;	?debug	L 342
 	push	 cx
;	?debug	L 343
 	push	 bx
;	?debug	L 344
 	push	 ax
;	?debug	L 346
 	push	 [bp-10]
;	?debug	L 347
 	push	 [bp-8]
;	?debug	L 348
 	push	 [bp-6]
;	?debug	L 349
 	push	 [bp-4]
;	?debug	L 350
 	push	 [bp-2]
;	?debug	L 351
 	pop	 ax
;	?debug	L 352
 	mov	 es,ax
;	?debug	L 353
 	pop	 dx
;	?debug	L 354
 	pop	 cx
;	?debug	L 355
 	pop	 bx
;	?debug	L 356
 	pop	 ax
;	?debug	L 357
 	int	 13H
;	?debug	L 359
 	pop	 ax
;	?debug	L 360
 	pop	 bx
;	?debug	L 361
 	pop	 cx
;	?debug	L 362
 	pop	 dx
;	?debug	L 363
 	pop	 es
@65:
@66:
@64:
;	?debug	L 365
	mov	sp,bp
	pop	bp
	ret	
extend_read_disk	endp
;	?debug	L 367
test_extend13H	proc	far
;	?debug	L 369
 	push	 ax
;	?debug	L 370
 	push	 bx
;	?debug	L 371
 	push	 cx
;	?debug	L 373
 	mov	 ah,41H
;	?debug	L 374
 	mov	 al,00H
;	?debug	L 375
 	mov	 bx,55aaH
;	?debug	L 376
 	mov	 cx,0000H
;	?debug	L 378
 	pop	 cx
;	?debug	L 379
 	pop	 bx
;	?debug	L 380
 	pop	 ax
@68:
;	?debug	L 381
	ret	
test_extend13H	endp
;	?debug	L 383
get_disk_para	proc	far
	push	bp
	mov	bp,sp
	sub	sp,4
@72:
;	?debug	L 391
 	push	 ax
;	?debug	L 392
 	push	 cx
;	?debug	L 393
 	push	 dx
;	?debug	L 395
 	mov	 ah,08
;	?debug	L 396
 	mov	 dl,DGROUP:_DISK_DRIVER_TYPE
;	?debug	L 397
 	int	 13H
;	?debug	L 398
 	mov	 [bp-4],ch
;	?debug	L 399
 	mov	 [bp-3],cl
;	?debug	L 400
 	mov	 [bp-1],dh
;	?debug	L 401
 	mov	 [bp-2],dl
;	?debug	L 403
 	pop	 dx
;	?debug	L 404
 	pop	 cx
;	?debug	L 405
 	pop	 ax
@70:
@71:
;	?debug	L 414
	mov	al,byte ptr [bp-3]
	mov	ah,0
	shl	ax,1
	shl	ax,1
	and	ax,768
	mov	dl,byte ptr [bp-4]
	mov	dh,0
	or	ax,dx
	mov	word ptr DGROUP:disk_para_table,ax
;	?debug	L 415
	inc	word ptr DGROUP:disk_para_table
;	?debug	L 416
	mov	al,byte ptr [bp-1]
	mov	byte ptr DGROUP:disk_para_table+2,al
;	?debug	L 417
	inc	byte ptr DGROUP:disk_para_table+2
;	?debug	L 418
	mov	al,byte ptr [bp-3]
	and	al,63
	mov	byte ptr DGROUP:disk_para_table+3,al
;	?debug	L 419
	mov	al,byte ptr [bp-2]
	mov	byte ptr DGROUP:disk_para_table+4,al
;	?debug	L 421
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+11
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 422
	mov	al,0
	push	ax
	mov	ax,word ptr DGROUP:disk_para_table
	mov	cl,8
	shr	ax,cl
	push	ax
	push	cs
	call	near ptr fat_print_hex
	pop	cx
	pop	cx
;	?debug	L 423
	mov	al,0
	push	ax
	push	word ptr DGROUP:disk_para_table
	push	cs
	call	near ptr fat_print_hex
	pop	cx
	pop	cx
;	?debug	L 425
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+29
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 426
	mov	al,0
	push	ax
	push	word ptr DGROUP:disk_para_table+2
	push	cs
	call	near ptr fat_print_hex
	pop	cx
	pop	cx
;	?debug	L 428
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+43
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 429
	mov	al,0
	push	ax
	push	word ptr DGROUP:disk_para_table+3
	push	cs
	call	near ptr fat_print_hex
	pop	cx
	pop	cx
;	?debug	L 431
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+59
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 432
	mov	al,0
	push	ax
	push	word ptr DGROUP:disk_para_table+4
	push	cs
	call	near ptr fat_print_hex
	pop	cx
	pop	cx
;	?debug	L 434
	mov	al,1
	push	ax
	mov	al,0
	push	ax
	push	cs
	call	near ptr fat_print_char
	pop	cx
	pop	cx
@69:
;	?debug	L 435
	mov	sp,bp
	pop	bp
	ret	
get_disk_para	endp
FAT32_TEXT	ends
_DATA	segment word public 'DATA'
disk_max_clus_no	label	word
	dw	0
	dw	0
_DATA	ends
FAT32_TEXT	segment	byte public 'CODE'
;	?debug	L 440
get_fat32_para	proc	far
	push	bp
	mov	bp,sp
	sub	sp,16
;	?debug	L 451
	mov	word ptr [bp-6],0
	mov	word ptr [bp-8],0
;	?debug	L 454
	mov	al,1
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+75
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 456
	push	cs
	call	near ptr get_disk_para
;	?debug	L 458
	push	cs
	call	near ptr get_disk_bsp
;	?debug	L 461
	mov	dx,4
	xor	ax,ax
	push	dx
	push	ax
	mov	ax,1
	push	ax
	xor	dx,dx
	mov	ax,63
	push	dx
	push	ax
	push	cs
	call	near ptr read_lba_disk
	add	sp,10
@76:
;	?debug	L 465
	mov	word ptr [bp-4],0
;	?debug	L 465
	mov	word ptr [bp-2],16384
@74:
@75:
;	?debug	L 468
	les	bx,dword ptr [bp-4]
	add	bx,13
	mov	al,byte ptr es:[bx]
	mov	byte ptr DGROUP:bpb_para_table,al
;	?debug	L 469
	les	bx,dword ptr [bp-4]
	mov	ax,word ptr es:[bx+14]
	mov	word ptr DGROUP:bpb_para_table+2,ax
;	?debug	L 470
	les	bx,dword ptr [bp-4]
	mov	al,byte ptr es:[bx+16]
	mov	byte ptr DGROUP:bpb_para_table+1,al
;	?debug	L 471
	les	bx,dword ptr [bp-4]
	mov	ax,word ptr es:[bx+11]
	mov	word ptr DGROUP:bpb_para_table+12,ax
;	?debug	L 473
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+86
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 474
	mov	al,0
	push	ax
	push	word ptr DGROUP:bpb_para_table
	push	cs
	call	near ptr fat_print_hex
	pop	cx
	pop	cx
;	?debug	L 475
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+107
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 476
	mov	al,0
	push	ax
	push	word ptr DGROUP:bpb_para_table+2
	push	cs
	call	near ptr fat_print_hex
	pop	cx
	pop	cx
;	?debug	L 477
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+124
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 478
	mov	al,0
	push	ax
	push	word ptr DGROUP:bpb_para_table+1
	push	cs
	call	near ptr fat_print_hex
	pop	cx
	pop	cx
@79:
;	?debug	L 481
	mov	al,byte ptr DGROUP:bpb_para_table
	mov	ah,0
	mul	word ptr DGROUP:bpb_para_table+12
	xor	dx,dx
	push	dx
	push	ax
	mov	dx,1
	xor	ax,ax
	push	dx
	push	ax
	call	far ptr _tc_32bit_cmp
	add	sp,8
	cmp	ax,-1
	je	@80
;	?debug	L 481
	jmp	short @81
@80:
;	?debug	L 481
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+136
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 481
	mov	al,1
	push	ax
	mov	ax,481
	push	ax
	push	cs
	call	near ptr fat_print_dec
	pop	cx
	pop	cx
@84:
;	?debug	L 481
 	sti	
;	?debug	L 481
	jmp	short @85
@87:
;	?debug	L 481
 	hlt	
@85:
;	?debug	L 481
	jmp	short @87
@86:
@82:
@83:
@81:
@77:
@78:
@90:
;	?debug	L 482
	mov	al,byte ptr DGROUP:bpb_para_table
	mov	ah,0
	mul	word ptr DGROUP:bpb_para_table+12
	xor	dx,dx
	push	dx
	push	ax
	mov	dx,1
	xor	ax,ax
	push	dx
	push	ax
	call	far ptr _tc_32bit_cmp
	add	sp,8
	cmp	ax,-1
	je	@91
;	?debug	L 482
	jmp	short @92
@91:
;	?debug	L 482
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+147
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 482
	mov	al,1
	push	ax
	mov	ax,482
	push	ax
	push	cs
	call	near ptr fat_print_dec
	pop	cx
	pop	cx
@95:
;	?debug	L 482
 	sti	
;	?debug	L 482
	jmp	short @96
@98:
;	?debug	L 482
 	hlt	
@96:
;	?debug	L 482
	jmp	short @98
@97:
@93:
@94:
@92:
@88:
@89:
;	?debug	L 485
	les	bx,dword ptr [bp-4]
	push	word ptr es:[bx+38]
	push	word ptr es:[bx+36]
	push	ds
	mov	ax,offset DGROUP:bpb_para_table+8
	push	ax
	call	far ptr _set_32bit_value
	add	sp,8
;	?debug	L 487
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+158
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 488
	mov	al,0
	push	ax
	push	word ptr DGROUP:bpb_para_table+10
	push	word ptr DGROUP:bpb_para_table+8
	push	cs
	call	near ptr fat_print_dword_hex
	add	sp,6
;	?debug	L 503
	mov	al,1
	push	ax
	mov	al,0
	push	ax
	push	cs
	call	near ptr fat_print_char
	pop	cx
	pop	cx
;	?debug	L 511
	mov	al,byte ptr DGROUP:disk_para_table+3
	xor	ah,ah
	xor	dx,dx
	mov	word ptr [bp-6],dx
	mov	word ptr [bp-8],ax
;	?debug	L 512
	mov	ax,word ptr DGROUP:bpb_para_table+2
	xor	dx,dx
	add	ax,word ptr [bp-8]
	adc	dx,word ptr [bp-6]
	mov	word ptr DGROUP:fat_start_sector+2,dx
	mov	word ptr DGROUP:fat_start_sector,ax
;	?debug	L 514
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+171
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 515
	mov	al,0
	push	ax
	push	word ptr DGROUP:fat_start_sector+2
	push	word ptr DGROUP:fat_start_sector
	push	cs
	call	near ptr fat_print_dword_hex
	add	sp,6
;	?debug	L 520
	push	word ptr DGROUP:bpb_para_table+10
	push	word ptr DGROUP:bpb_para_table+8
	mov	al,byte ptr DGROUP:bpb_para_table+1
	mov	ah,0
	cwd	
	push	dx
	push	ax
	push	ds
	mov	ax,offset DGROUP:dir_start_sector
	push	ax
	call	far ptr _tc_32bit_mul
	add	sp,12
;	?debug	L 521
	push	word ptr DGROUP:fat_start_sector+2
	push	word ptr DGROUP:fat_start_sector
	push	word ptr DGROUP:dir_start_sector+2
	push	word ptr DGROUP:dir_start_sector
	push	ds
	mov	ax,offset DGROUP:dir_start_sector
	push	ax
	call	far ptr _tc_32bit_add
	add	sp,12
;	?debug	L 523
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+192
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 524
	mov	al,0
	push	ax
	push	word ptr DGROUP:dir_start_sector+2
	push	word ptr DGROUP:dir_start_sector
	push	cs
	call	near ptr fat_print_dword_hex
	add	sp,6
;	?debug	L 526
	mov	al,1
	push	ax
	mov	al,0
	push	ax
	push	cs
	call	near ptr fat_print_char
	pop	cx
	pop	cx
@101:
;	?debug	L 533
	xor	dx,dx
	mov	ax,4
	push	dx
	push	ax
	mov	ax,word ptr DGROUP:bpb_para_table+12
	xor	dx,dx
	push	dx
	push	ax
	push	ss
	lea	ax,word ptr [bp-12]
	push	ax
	push	ss
	lea	ax,word ptr [bp-16]
	push	ax
	call	far ptr _tc_32bit_div
	add	sp,16
;	?debug	L 534
	push	word ptr DGROUP:bpb_para_table+10
	push	word ptr DGROUP:bpb_para_table+8
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	ds
	mov	ax,offset DGROUP:disk_max_clus_no
	push	ax
	call	far ptr _tc_32bit_mul
	add	sp,12
@99:
@100:
@73:
;	?debug	L 536
	mov	sp,bp
	pop	bp
	ret	
get_fat32_para	endp
FAT32_TEXT	ends
_DATA	segment word public 'DATA'
curr_min_clus_no	label	word
	dw	0
	dw	0
curr_max_clus_no	label	word
	dw	0
	dw	0
_DATA	ends
FAT32_TEXT	segment	byte public 'CODE'
;	?debug	L 545
read_fat32_table	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8
;	?debug	L 550
	mov	word ptr DGROUP:curr_min_clus_no+2,0
	mov	word ptr DGROUP:curr_min_clus_no,0
;	?debug	L 551
	mov	ax,word ptr DGROUP:bpb_para_table+12
	xor	dx,dx
	push	dx
	push	ax
	mov	dx,1
	xor	ax,ax
	push	dx
	push	ax
	push	ss
	lea	ax,word ptr [bp-4]
	push	ax
	push	ss
	lea	ax,word ptr [bp-8]
	push	ax
	call	far ptr _tc_32bit_div
	add	sp,16
;	?debug	L 552
	mov	ax,word ptr DGROUP:bpb_para_table+12
	shr	ax,1
	shr	ax,1
	xor	dx,dx
	push	dx
	push	ax
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	call	far ptr _mul_is_overflow
	add	sp,8
	cmp	al,1
	je	@103
;	?debug	L 553
	mov	ax,word ptr DGROUP:bpb_para_table+12
	shr	ax,1
	shr	ax,1
	xor	dx,dx
	push	dx
	push	ax
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	push	ds
	mov	ax,offset DGROUP:curr_max_clus_no
	push	ax
	call	far ptr _tc_32bit_mul
	add	sp,12
;	?debug	L 554
	jmp	short @104
@103:
;	?debug	L 555
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+213
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
@107:
;	?debug	L 556
 	sti	
;	?debug	L 556
	jmp	short @108
@110:
;	?debug	L 556
 	hlt	
@108:
;	?debug	L 556
	jmp	short @110
@109:
@105:
@106:
@104:
;	?debug	L 560
	mov	dx,2
	xor	ax,ax
	push	dx
	push	ax
	push	word ptr [bp-8]
	push	word ptr DGROUP:fat_start_sector+2
	push	word ptr DGROUP:fat_start_sector
	push	cs
	call	near ptr read_lba_disk
	add	sp,10
;	?debug	L 562
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+234
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 563
	mov	al,0
	push	ax
	push	word ptr DGROUP:curr_max_clus_no+2
	push	word ptr DGROUP:curr_max_clus_no
	push	cs
	call	near ptr fat_print_dword_hex
	add	sp,6
;	?debug	L 564
	mov	al,1
	push	ax
	mov	al,0
	push	ax
	push	cs
	call	near ptr fat_print_char
	pop	cx
	pop	cx
@102:
;	?debug	L 565
	mov	sp,bp
	pop	bp
	ret	
read_fat32_table	endp
;	?debug	L 567
reread_fat_table	proc	far
	push	bp
	mov	bp,sp
	sub	sp,16
;	?debug	L 574
	mov	ax,word ptr DGROUP:bpb_para_table+12
	xor	dx,dx
	push	dx
	push	ax
	mov	dx,1
	xor	ax,ax
	push	dx
	push	ax
	push	ss
	lea	ax,word ptr [bp-16]
	push	ax
	push	ss
	lea	ax,word ptr [bp-12]
	push	ax
	call	far ptr _tc_32bit_div
	add	sp,16
;	?debug	L 577
	mov	ax,word ptr DGROUP:bpb_para_table+12
	shr	ax,1
	shr	ax,1
	xor	dx,dx
	push	dx
	push	ax
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	push	ss
	lea	ax,word ptr [bp-8]
	push	ax
	call	far ptr _tc_32bit_mul
	add	sp,12
;	?debug	L 580
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	ss
	lea	ax,word ptr [bp-16]
	push	ax
	push	ss
	lea	ax,word ptr [bp-4]
	push	ax
	call	far ptr _tc_32bit_div
	add	sp,16
;	?debug	L 583
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	push	ss
	lea	ax,word ptr [bp-16]
	push	ax
	call	far ptr _tc_32bit_mul
	add	sp,12
;	?debug	L 584
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	ds
	mov	ax,offset DGROUP:curr_min_clus_no
	push	ax
	call	far ptr _set_32bit_value
	add	sp,8
;	?debug	L 585
	push	word ptr DGROUP:curr_min_clus_no+2
	push	word ptr DGROUP:curr_min_clus_no
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	push	ds
	mov	ax,offset DGROUP:curr_max_clus_no
	push	ax
	call	far ptr _tc_32bit_add
	add	sp,12
;	?debug	L 588
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	push	ss
	lea	ax,word ptr [bp-16]
	push	ax
	call	far ptr _tc_32bit_mul
	add	sp,12
;	?debug	L 589
	push	word ptr DGROUP:fat_start_sector+2
	push	word ptr DGROUP:fat_start_sector
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	ss
	lea	ax,word ptr [bp-16]
	push	ax
	call	far ptr _tc_32bit_add
	add	sp,12
;	?debug	L 590
	mov	dx,2
	xor	ax,ax
	push	dx
	push	ax
	push	word ptr [bp-12]
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	cs
	call	near ptr read_lba_disk
	add	sp,10
@111:
;	?debug	L 591
	mov	sp,bp
	pop	bp
	ret	
reread_fat_table	endp
;	?debug	L 593
get_next_clus	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8
@115:
;	?debug	L 598
	les	bx,dword ptr [bp+6]
	push	word ptr es:[bx+2]
	push	word ptr es:[bx]
	push	word ptr DGROUP:curr_min_clus_no+2
	push	word ptr DGROUP:curr_min_clus_no
	call	far ptr _tc_32bit_cmp
	add	sp,8
	cmp	ax,1
	je	@116
	les	bx,dword ptr [bp+6]
	push	word ptr es:[bx+2]
	push	word ptr es:[bx]
	push	word ptr DGROUP:curr_max_clus_no+2
	push	word ptr DGROUP:curr_max_clus_no
	call	far ptr _tc_32bit_cmp
	add	sp,8
	cmp	ax,1
	jne	@116
;	?debug	L 598
	jmp	short @117
@116:
;	?debug	L 598
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+250
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 598
	mov	al,1
	push	ax
	mov	ax,598
	push	ax
	push	cs
	call	near ptr fat_print_dec
	pop	cx
	pop	cx
@120:
;	?debug	L 598
 	sti	
;	?debug	L 598
	jmp	short @121
@123:
;	?debug	L 598
 	hlt	
@121:
;	?debug	L 598
	jmp	short @123
@122:
@118:
@119:
@117:
@113:
@114:
@126:
;	?debug	L 600
	mov	word ptr [bp-8],0
;	?debug	L 600
	mov	word ptr [bp-6],8192
@124:
@125:
;	?debug	L 608
	push	word ptr DGROUP:curr_min_clus_no+2
	push	word ptr DGROUP:curr_min_clus_no
	les	bx,dword ptr [bp+6]
	push	word ptr es:[bx+2]
	push	word ptr es:[bx]
	push	ss
	lea	ax,word ptr [bp-4]
	push	ax
	call	far ptr _tc_32bit_sub
	add	sp,12
;	?debug	L 609
	mov	ax,word ptr [bp-4]
	shl	ax,1
	shl	ax,1
	add	word ptr [bp-8],ax
;	?debug	L 610
	les	bx,dword ptr [bp-8]
	push	word ptr es:[bx+2]
	push	word ptr es:[bx]
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	call	far ptr _set_32bit_value
	add	sp,8
@112:
;	?debug	L 616
	mov	sp,bp
	pop	bp
	ret	
get_next_clus	endp
;	?debug	L 621
_file_name_is_same	proc	far
	push	bp
	mov	bp,sp
	sub	sp,4
;	?debug	L 626
	mov	word ptr [bp-4],0
	jmp	@131
@130:
;	?debug	L 627
	les	bx,dword ptr [bp+10]
	add	bx,word ptr [bp-4]
	cmp	byte ptr es:[bx],97
	jb	@132
	les	bx,dword ptr [bp+10]
	add	bx,word ptr [bp-4]
	cmp	byte ptr es:[bx],122
	ja	@132
;	?debug	L 628
	les	bx,dword ptr [bp+10]
	add	bx,word ptr [bp-4]
	mov	al,byte ptr es:[bx]
	add	al,224
	mov	byte ptr [bp-1],al
;	?debug	L 629
	jmp	short @133
@132:
;	?debug	L 630
	les	bx,dword ptr [bp+10]
	add	bx,word ptr [bp-4]
	mov	al,byte ptr es:[bx]
	mov	byte ptr [bp-1],al
@133:
;	?debug	L 633
	les	bx,dword ptr [bp+6]
	add	bx,word ptr [bp-4]
	mov	al,byte ptr es:[bx]
	cmp	al,byte ptr [bp-1]
	je	@134
;	?debug	L 634
;	?debug	L 635
	les	bx,dword ptr [bp+6]
	add	bx,word ptr [bp-4]
	cmp	byte ptr es:[bx],32
	jne	@135
	les	bx,dword ptr [bp+10]
	add	bx,word ptr [bp-4]
	cmp	byte ptr es:[bx],46
	je	@136
	les	bx,dword ptr [bp+10]
	add	bx,word ptr [bp-4]
	cmp	byte ptr es:[bx],0
	jne	@135
@136:
;	?debug	L 636
	mov	al,1
	jmp	short @127
;	?debug	L 637
	jmp	short @137
@135:
;	?debug	L 638
	mov	al,0
	jmp	short @127
@137:
@134:
@129:
	inc	word ptr [bp-4]
@131:
	cmp	word ptr [bp-4],8
	jae	@138
	jmp	@130
@138:
@128:
;	?debug	L 642
	mov	al,1
	jmp	short @127
@127:
;	?debug	L 643
	mov	sp,bp
	pop	bp
	ret	
_file_name_is_same	endp
FAT32_TEXT	ends
_DATA	segment word public 'DATA'
dir_clus_no	label	word
	dw	0
	dw	0
_DATA	ends
FAT32_TEXT	segment	byte public 'CODE'
;	?debug	L 657
get_file_clus_no	proc	far
	push	bp
	mov	bp,sp
	sub	sp,20
;	?debug	L 666
	push	word ptr DGROUP:dir_clus_no+2
	push	word ptr DGROUP:dir_clus_no
	push	ss
	lea	ax,word ptr [bp-20]
	push	ax
	call	far ptr _set_32bit_value
	add	sp,8
@142:
;	?debug	L 669
	mov	dx,3
	xor	ax,ax
	push	dx
	push	ax
	push	word ptr [bp-18]
	push	word ptr [bp-20]
	push	cs
	call	near ptr read_clus_disk
	add	sp,8
@145:
;	?debug	L 672
	mov	word ptr [bp-4],0
;	?debug	L 672
	mov	word ptr [bp-2],12288
@143:
@144:
;	?debug	L 673
	mov	ax,word ptr DGROUP:bpb_para_table+12
	mov	cl,5
	shr	ax,cl
	mov	dl,byte ptr DGROUP:bpb_para_table
	mov	dh,0
	mul	dx
	xor	dx,dx
	mov	word ptr [bp-14],dx
	mov	word ptr [bp-16],ax
;	?debug	L 674
	mov	word ptr [bp-10],0
	mov	word ptr [bp-12],0
	jmp	@149
@148:
;	?debug	L 675
	mov	ax,word ptr [bp-12]
	mov	cl,5
	shl	ax,cl
	les	bx,dword ptr [bp-4]
	add	bx,ax
	mov	word ptr [bp-6],es
	mov	word ptr [bp-8],bx
;	?debug	L 678
	les	bx,dword ptr [bp-8]
	cmp	byte ptr es:[bx],0
	jne	@150
;	?debug	L 679
	xor	ax,ax
	push	ax
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	call	far ptr _set_32bit_value
	add	sp,8
;	?debug	L 680
	mov	ax,1
	jmp	@139
@150:
;	?debug	L 684
	les	bx,dword ptr [bp-8]
	cmp	byte ptr es:[bx],229
	jne	@151
;	?debug	L 684
	jmp	@147
@151:
;	?debug	L 687
	les	bx,dword ptr [bp-8]
	cmp	byte ptr es:[bx+11],15
	jne	@152
;	?debug	L 687
	jmp	@147
@152:
;	?debug	L 689
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	push	cs
	call	near ptr _file_name_is_same
	add	sp,8
	or	al,al
	jne	@165
	jmp	@153
@165:
;	?debug	L 690
	les	bx,dword ptr [bp-8]
	cmp	byte ptr es:[bx+11],16
	je	@166
	jmp	@154
@166:
;	?debug	L 691
	les	bx,dword ptr [bp-8]
	mov	ax,word ptr es:[bx+20]
	xor	dx,dx
	mov	dx,ax
	xor	ax,ax
	les	bx,dword ptr [bp-8]
	mov	bx,word ptr es:[bx+26]
	xor	cx,cx
	add	ax,bx
	adc	dx,cx
	or	dx,ax
	jne	@155
;	?debug	L 694
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	ds
	mov	ax,offset DGROUP:s@+261
	push	ax
	call	far ptr _cmp_str
	add	sp,8
	or	ax,ax
	jne	@156
;	?debug	L 695
	xor	dx,dx
	mov	ax,2
	push	dx
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	call	far ptr _set_32bit_value
	add	sp,8
;	?debug	L 696
	xor	ax,ax
	jmp	@139
;	?debug	L 697
	jmp	short @157
@156:
;	?debug	L 698
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+264
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
@160:
;	?debug	L 699
 	sti	
;	?debug	L 699
	jmp	short @161
@163:
;	?debug	L 699
 	hlt	
@161:
;	?debug	L 699
	jmp	short @163
@162:
@158:
@159:
;	?debug	L 700
	mov	ax,1
	jmp	@139
@157:
@155:
@154:
;	?debug	L 704
	les	bx,dword ptr [bp-8]
	mov	ax,word ptr es:[bx+20]
	xor	dx,dx
	mov	dx,ax
	xor	ax,ax
	les	bx,dword ptr [bp-8]
	mov	bx,word ptr es:[bx+26]
	xor	cx,cx
	add	ax,bx
	adc	dx,cx
	push	dx
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	call	far ptr _set_32bit_value
	add	sp,8
;	?debug	L 705
	xor	ax,ax
	jmp	short @139
@153:
@147:
	add	word ptr [bp-12],1
	adc	word ptr [bp-10],0
@149:
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	call	far ptr _tc_32bit_cmp
	add	sp,8
	cmp	ax,-1
	jne	@167
	jmp	@148
@167:
@146:
;	?debug	L 710
	push	ss
	lea	ax,word ptr [bp-20]
	push	ax
	push	cs
	call	near ptr get_next_clus
	pop	cx
	pop	cx
@140:
;	?debug	L 711
	cmp	word ptr [bp-18],4095
	je	@168
	jmp	@142
@168:
	cmp	word ptr [bp-20],-1
	je	@169
	jmp	@142
@169:
@164:
@141:
;	?debug	L 713
	xor	ax,ax
	push	ax
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	call	far ptr _set_32bit_value
	add	sp,8
;	?debug	L 714
	mov	ax,1
	jmp	short @139
@139:
;	?debug	L 715
	mov	sp,bp
	pop	bp
	ret	
get_file_clus_no	endp
;	?debug	L 717
_fat32_change_directory	proc	far
	push	bp
	mov	bp,sp
	sub	sp,6
;	?debug	L 725
	mov	ax,word ptr [bp+6]
	or	ax,word ptr [bp+8]
	jne	@171
;	?debug	L 726
	mov	ax,1
	jmp	short @170
@171:
;	?debug	L 729
	push	ss
	lea	ax,word ptr [bp-4]
	push	ax
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	cs
	call	near ptr get_file_clus_no
	add	sp,8
	mov	word ptr [bp-6],ax
;	?debug	L 730
	cmp	word ptr [bp-6],0
	jne	@172
;	?debug	L 731
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	push	ds
	mov	ax,offset DGROUP:dir_clus_no
	push	ax
	call	far ptr _set_32bit_value
	add	sp,8
;	?debug	L 732
	xor	ax,ax
	jmp	short @170
;	?debug	L 733
	jmp	short @173
@172:
;	?debug	L 734
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+283
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 735
	mov	ax,1
	jmp	short @170
@173:
@170:
;	?debug	L 737
	mov	sp,bp
	pop	bp
	ret	
_fat32_change_directory	endp
FAT32_TEXT	ends
_DATA	segment word public 'DATA'
	db	0
_DATA	ends
FAT32_TEXT	segment	byte public 'CODE'
;	?debug	L 739
read_file_by_clus_no	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8
;	?debug	L 743
	mov	word ptr [bp-2],0
@177:
;	?debug	L 746
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	word ptr DGROUP:curr_min_clus_no+2
	push	word ptr DGROUP:curr_min_clus_no
	call	far ptr _tc_32bit_cmp
	add	sp,8
	cmp	ax,1
	je	@179
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	word ptr DGROUP:curr_max_clus_no+2
	push	word ptr DGROUP:curr_max_clus_no
	call	far ptr _tc_32bit_cmp
	add	sp,8
	cmp	ax,1
	je	@178
@179:
;	?debug	L 747
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	cs
	call	near ptr reread_fat_table
	pop	cx
	pop	cx
@178:
;	?debug	L 750
	mov	dx,4
	xor	ax,ax
	push	dx
	push	ax
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	cs
	call	near ptr read_clus_disk
	add	sp,8
;	?debug	L 751
	inc	word ptr [bp-2]
;	?debug	L 754
	mov	al,byte ptr DGROUP:bpb_para_table
	mov	ah,0
	mul	word ptr DGROUP:bpb_para_table+12
	xor	dx,dx
	push	dx
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	mov	dx,4
	xor	ax,ax
	push	dx
	push	ax
	call	far ptr _move_1m
	add	sp,12
;	?debug	L 756
	mov	al,byte ptr DGROUP:bpb_para_table
	mov	ah,0
	mul	word ptr DGROUP:bpb_para_table+12
	xor	dx,dx
	push	dx
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	ss
	lea	ax,word ptr [bp+10]
	push	ax
	call	far ptr _tc_32bit_add
	add	sp,12
;	?debug	L 759
	push	ss
	lea	ax,word ptr [bp+6]
	push	ax
	push	cs
	call	near ptr get_next_clus
	pop	cx
	pop	cx
	jmp	short @180
@182:
;	?debug	L 763
	mov	al,0
	push	ax
	push	word ptr [bp+6]
	push	cs
	call	near ptr fat_print_hex
	pop	cx
	pop	cx
;	?debug	L 764
	mov	al,0
	push	ax
	push	word ptr [bp+10]
	push	cs
	call	near ptr fat_print_hex
	pop	cx
	pop	cx
@180:
@181:
@175:
;	?debug	L 766
	cmp	word ptr [bp+8],4095
	je	@192
	jmp	@177
@192:
	cmp	word ptr [bp+6],-1
	je	@193
	jmp	@177
@193:
@191:
@176:
;	?debug	L 768
	jmp	@183
;	?debug	L 774
	mov	word ptr DGROUP:fat_col,0
;	?debug	L 775
	mov	word ptr DGROUP:fat_row,1
;	?debug	L 776
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+297
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 777
	mov	al,0
	push	ax
	push	word ptr [bp-2]
	push	cs
	call	near ptr fat_print_dec
	pop	cx
	pop	cx
;	?debug	L 778
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+312
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 779
	mov	al,0
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	cs
	call	near ptr fat_print_dword_hex
	add	sp,6
;	?debug	L 781
	mov	al,byte ptr DGROUP:bpb_para_table
	mov	ah,0
	mul	word ptr DGROUP:bpb_para_table+12
	xor	dx,dx
	push	dx
	push	ax
	mov	ax,word ptr [bp-2]
	xor	dx,dx
	push	dx
	push	ax
	push	ss
	lea	ax,word ptr [bp-8]
	push	ax
	call	far ptr _tc_32bit_mul
	add	sp,12
;	?debug	L 782
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+324
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 784
	mov	dx,93
	mov	ax,4216
	push	dx
	push	ax
	push	ss
	lea	ax,word ptr [bp-8]
	push	ax
	call	far ptr _set_32bit_value
	add	sp,8
;	?debug	L 785
	mov	al,0
	push	ax
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	push	cs
	call	near ptr fat_print_dword_hex
	add	sp,6
;	?debug	L 786
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+329
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 787
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	mov	dx,256
	xor	ax,ax
	push	dx
	push	ax
	call	far ptr _crc16
	add	sp,8
	mov	word ptr [bp-4],ax
;	?debug	L 788
	mov	al,0
	push	ax
	mov	ax,word ptr [bp-4]
	and	ax,-256
	mov	cl,8
	shr	ax,cl
	push	ax
	push	cs
	call	near ptr fat_print_hex
	pop	cx
	pop	cx
;	?debug	L 789
	mov	al,0
	push	ax
	mov	al,byte ptr [bp-4]
	and	al,255
	push	ax
	push	cs
	call	near ptr fat_print_hex
	pop	cx
	pop	cx
;	?debug	L 790
	mov	al,byte ptr DGROUP:d@+28
	inc	byte ptr DGROUP:d@+28
	or	al,al
	je	@184
@187:
;	?debug	L 790
 	sti	
;	?debug	L 790
	jmp	short @188
@190:
;	?debug	L 790
 	hlt	
@188:
;	?debug	L 790
	jmp	short @190
@189:
@185:
@186:
@184:
@183:
;	?debug	L 793
	xor	ax,ax
	jmp	short @174
@174:
;	?debug	L 794
	mov	sp,bp
	pop	bp
	ret	
read_file_by_clus_no	endp
;	?debug	L 797
_fat32_load_file	proc	far
	push	bp
	mov	bp,sp
	sub	sp,6
;	?debug	L 799
	mov	word ptr [bp-4],18
	mov	word ptr [bp-6],13398
;	?debug	L 802
	mov	ax,word ptr [bp+6]
	or	ax,word ptr [bp+8]
	je	@195
;	?debug	L 804
	push	ss
	lea	ax,word ptr [bp-6]
	push	ax
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	cs
	call	near ptr get_file_clus_no
	add	sp,8
	mov	word ptr [bp-2],ax
;	?debug	L 805
	cmp	word ptr [bp-2],0
	jne	@196
	mov	ax,word ptr [bp-6]
	or	ax,word ptr [bp-4]
	je	@196
;	?debug	L 807
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	word ptr [bp-4]
	push	word ptr [bp-6]
	push	cs
	call	near ptr read_file_by_clus_no
	add	sp,8
	jmp	short @194
;	?debug	L 808
	jmp	short @197
@196:
;	?debug	L 809
	mov	al,1
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+334
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
@197:
@195:
;	?debug	L 812
	mov	ax,1
	jmp	short @194
@194:
;	?debug	L 813
	mov	sp,bp
	pop	bp
	ret	
_fat32_load_file	endp
;	?debug	L 816
_list_fat32	proc	far
	push	bp
	mov	bp,sp
	sub	sp,16
;	?debug	L 823
	mov	dx,3
	xor	ax,ax
	push	dx
	push	ax
	push	word ptr DGROUP:dir_clus_no+2
	push	word ptr DGROUP:dir_clus_no
	push	cs
	call	near ptr read_clus_disk
	add	sp,8
@201:
;	?debug	L 825
	mov	word ptr [bp-12],0
;	?debug	L 825
	mov	word ptr [bp-10],12288
@199:
@200:
;	?debug	L 827
	mov	ax,word ptr DGROUP:bpb_para_table+12
	mov	cl,5
	shr	ax,cl
	mov	dl,byte ptr DGROUP:bpb_para_table
	mov	dh,0
	mul	dx
	xor	dx,dx
	mov	word ptr [bp-6],dx
	mov	word ptr [bp-8],ax
;	?debug	L 828
	mov	word ptr [bp-4],0
	jmp	@205
@204:
;	?debug	L 832
	mov	ax,word ptr [bp-4]
	mov	cl,5
	shl	ax,cl
	les	bx,dword ptr [bp-12]
	add	bx,ax
	mov	word ptr [bp-14],es
	mov	word ptr [bp-16],bx
;	?debug	L 834
	les	bx,dword ptr [bp-16]
	cmp	byte ptr es:[bx],0
	jne	@206
	jmp	@198
@206:
;	?debug	L 836
	les	bx,dword ptr [bp-16]
	cmp	byte ptr es:[bx],229
	jne	@207
;	?debug	L 836
	jmp	@203
@207:
;	?debug	L 838
	les	bx,dword ptr [bp-16]
	cmp	byte ptr es:[bx+11],15
	jne	@208
;	?debug	L 838
	jmp	@203
@208:
;	?debug	L 841
	mov	al,0
	push	ax
	les	bx,dword ptr [bp-16]
	mov	ax,word ptr es:[bx+24]
	and	ax,-512
	mov	cl,9
	shr	ax,cl
	add	ax,1980
	push	ax
	push	cs
	call	near ptr fat_print_dec
	pop	cx
	pop	cx
;	?debug	L 842
	mov	al,0
	push	ax
	mov	al,45
	push	ax
	push	cs
	call	near ptr fat_print_char
	pop	cx
	pop	cx
;	?debug	L 843
	mov	al,0
	push	ax
	les	bx,dword ptr [bp-16]
	mov	ax,word ptr es:[bx+24]
	and	ax,480
	mov	cl,5
	sar	ax,cl
	push	ax
	push	cs
	call	near ptr fat_print_dec
	pop	cx
	pop	cx
;	?debug	L 844
	mov	al,0
	push	ax
	mov	al,45
	push	ax
	push	cs
	call	near ptr fat_print_char
	pop	cx
	pop	cx
;	?debug	L 845
	mov	al,0
	push	ax
	les	bx,dword ptr [bp-16]
	mov	ax,word ptr es:[bx+24]
	and	ax,31
	push	ax
	push	cs
	call	near ptr fat_print_dec
	pop	cx
	pop	cx
;	?debug	L 847
	mov	al,0
	push	ax
	mov	al,32
	push	ax
	push	cs
	call	near ptr fat_print_char
	pop	cx
	pop	cx
;	?debug	L 850
	mov	al,0
	push	ax
	les	bx,dword ptr [bp-16]
	mov	ax,word ptr es:[bx+22]
	and	ax,-2048
	mov	cl,11
	shr	ax,cl
	push	ax
	push	cs
	call	near ptr fat_print_dec
	pop	cx
	pop	cx
;	?debug	L 851
	mov	al,0
	push	ax
	mov	al,58
	push	ax
	push	cs
	call	near ptr fat_print_char
	pop	cx
	pop	cx
;	?debug	L 852
	mov	al,0
	push	ax
	les	bx,dword ptr [bp-16]
	mov	ax,word ptr es:[bx+22]
	and	ax,2016
	mov	cl,5
	sar	ax,cl
	push	ax
	push	cs
	call	near ptr fat_print_dec
	pop	cx
	pop	cx
;	?debug	L 853
	mov	al,0
	push	ax
	mov	al,58
	push	ax
	push	cs
	call	near ptr fat_print_char
	pop	cx
	pop	cx
;	?debug	L 854
	mov	al,0
	push	ax
	les	bx,dword ptr [bp-16]
	mov	ax,word ptr es:[bx+22]
	and	ax,31
	shl	ax,1
	push	ax
	push	cs
	call	near ptr fat_print_dec
	pop	cx
	pop	cx
;	?debug	L 856
	mov	al,0
	push	ax
	mov	al,32
	push	ax
	push	cs
	call	near ptr fat_print_char
	pop	cx
	pop	cx
;	?debug	L 859
	les	bx,dword ptr [bp-16]
	cmp	byte ptr es:[bx+11],16
	jne	@209
;	?debug	L 860
	mov	al,0
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+352
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 861
	jmp	short @210
@209:
;	?debug	L 862
	mov	al,0
	push	ax
	les	bx,dword ptr [bp-16]
	push	word ptr es:[bx+30]
	push	word ptr es:[bx+28]
	push	cs
	call	near ptr fat_print_dword_hex
	add	sp,6
@210:
;	?debug	L 865
	mov	al,0
	push	ax
	mov	al,32
	push	ax
	push	cs
	call	near ptr fat_print_char
	pop	cx
	pop	cx
;	?debug	L 868
	mov	word ptr [bp-2],0
	jmp	short @214
@213:
;	?debug	L 869
	cmp	word ptr [bp-2],8
	jne	@215
;	?debug	L 869
	mov	al,0
	push	ax
	mov	al,46
	push	ax
	push	cs
	call	near ptr fat_print_char
	pop	cx
	pop	cx
@215:
;	?debug	L 870
	les	bx,dword ptr [bp-16]
	add	bx,word ptr [bp-2]
	cmp	byte ptr es:[bx],32
	je	@216
;	?debug	L 870
	mov	al,0
	push	ax
	les	bx,dword ptr [bp-16]
	add	bx,word ptr [bp-2]
	mov	al,byte ptr es:[bx]
	push	ax
	push	cs
	call	near ptr fat_print_char
	pop	cx
	pop	cx
@216:
@212:
	inc	word ptr [bp-2]
@214:
	cmp	word ptr [bp-2],11
	jb	@213
@211:
;	?debug	L 873
	mov	al,1
	push	ax
	mov	al,0
	push	ax
	push	cs
	call	near ptr fat_print_char
	pop	cx
	pop	cx
@203:
	inc	word ptr [bp-4]
@205:
	mov	ax,word ptr [bp-4]
	xor	dx,dx
	cmp	dx,word ptr [bp-6]
	jae	@218
	jmp	@204
@218:
	jne	@217
	cmp	ax,word ptr [bp-8]
	jae	@219
	jmp	@204
@219:
@217:
@202:
@198:
;	?debug	L 875
	mov	sp,bp
	pop	bp
	ret	
_list_fat32	endp
;	?debug	L 877
read_fat32_root_dir	proc	far
;	?debug	L 879
	mov	al,1
	push	ax
	push	ds
	mov	ax,offset DGROUP:s@+358
	push	ax
	push	cs
	call	near ptr fat_print_str
	add	sp,6
;	?debug	L 882
	xor	dx,dx
	mov	ax,2
	push	dx
	push	ax
	push	ds
	mov	ax,offset DGROUP:dir_clus_no
	push	ax
	call	far ptr _set_32bit_value
	add	sp,8
@220:
;	?debug	L 885
	ret	
read_fat32_root_dir	endp
;	?debug	L 887
read_fat32_data	proc	far
	push	bp
	mov	bp,sp
	sub	sp,6
;	?debug	L 893
	mov	dx,4
	xor	ax,ax
	push	dx
	push	ax
	xor	dx,dx
	mov	ax,3
	push	dx
	push	ax
	push	cs
	call	near ptr read_clus_disk
	add	sp,8
@224:
;	?debug	L 895
	mov	word ptr [bp-4],0
;	?debug	L 895
	mov	word ptr [bp-2],16384
@222:
@223:
;	?debug	L 896
	mov	word ptr [bp-6],0
	jmp	short @228
@227:
;	?debug	L 897
	mov	al,0
	push	ax
	les	bx,dword ptr [bp-4]
	add	bx,word ptr [bp-6]
	mov	al,byte ptr es:[bx]
	push	ax
	push	cs
	call	near ptr fat_print_hex
	pop	cx
	pop	cx
@226:
	inc	word ptr [bp-6]
@228:
	cmp	word ptr [bp-6],32
	jb	@227
@225:
@221:
;	?debug	L 899
	mov	sp,bp
	pop	bp
	ret	
read_fat32_data	endp
	?debug	C E932BD3B4D0C2E2E2F6D656469612E696E63
;	?debug	L 901
_init_fat32	proc	far
;	?debug	L 15
	mov	byte ptr DGROUP:_DISK_DRIVER_TYPE,128
;	?debug	L 905
	push	cs
	call	near ptr get_fat32_para
;	?debug	L 907
	push	cs
	call	near ptr read_fat32_table
;	?debug	L 909
	push	cs
	call	near ptr read_fat32_root_dir
@229:
;	?debug	L 912
	ret	
_init_fat32	endp
FAT32_TEXT	ends
_BSS	segment word public 'BSS'
grf	label	dword
	db	4 dup (?)
_DISK_DRIVER_TYPE	label	byte
	db	1 dup (?)
bpb_para_table	label	word
	db	14 dup (?)
disk_para_table	label	word
	db	8 dup (?)
_BSS	ends
	?debug	C E9
_DATA	segment word public 'DATA'
s@	label	byte
	db	97
	db	115
	db	115
	db	101
	db	114
	db	116
	db	33
	db	33
	db	33
	db	32
	db	0
	db	32
	db	46
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
	db	32
	db	0
	db	32
	db	46
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
	db	32
	db	0
	db	32
	db	46
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
	db	32
	db	0
	db	32
	db	46
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
	db	32
	db	0
	db	105
	db	110
	db	105
	db	116
	db	32
	db	102
	db	97
	db	116
	db	51
	db	50
	db	0
	db	32
	db	46
	db	115
	db	101
	db	99
	db	116
	db	111
	db	114
	db	115
	db	32
	db	112
	db	101
	db	114
	db	32
	db	99
	db	108
	db	117
	db	115
	db	58
	db	32
	db	0
	db	32
	db	46
	db	114
	db	115
	db	118
	db	100
	db	32
	db	115
	db	101
	db	99
	db	116
	db	111
	db	114
	db	115
	db	58
	db	32
	db	0
	db	32
	db	46
	db	102
	db	97
	db	116
	db	32
	db	110
	db	117
	db	109
	db	58
	db	32
	db	0
	db	97
	db	115
	db	115
	db	101
	db	114
	db	116
	db	33
	db	33
	db	33
	db	32
	db	0
	db	97
	db	115
	db	115
	db	101
	db	114
	db	116
	db	33
	db	33
	db	33
	db	32
	db	0
	db	32
	db	46
	db	102
	db	97
	db	116
	db	32
	db	115
	db	105
	db	122
	db	101
	db	58
	db	32
	db	0
	db	32
	db	46
	db	102
	db	97
	db	116
	db	32
	db	115
	db	116
	db	97
	db	114
	db	116
	db	32
	db	115
	db	101
	db	99
	db	116
	db	111
	db	114
	db	58
	db	32
	db	0
	db	32
	db	46
	db	100
	db	105
	db	114
	db	32
	db	115
	db	116
	db	97
	db	114
	db	116
	db	32
	db	115
	db	101
	db	99
	db	116
	db	111
	db	114
	db	58
	db	32
	db	0
	db	109
	db	97
	db	120
	db	32
	db	99
	db	108
	db	117
	db	115
	db	32
	db	110
	db	111
	db	32
	db	111
	db	118
	db	101
	db	114
	db	102
	db	108
	db	111
	db	119
	db	0
	db	32
	db	46
	db	109
	db	97
	db	120
	db	32
	db	99
	db	108
	db	117
	db	115
	db	32
	db	110
	db	111
	db	58
	db	32
	db	0
	db	97
	db	115
	db	115
	db	101
	db	114
	db	116
	db	33
	db	33
	db	33
	db	32
	db	0
	db	46
	db	46
	db	0
	db	99
	db	100
	db	32
	db	99
	db	108
	db	117
	db	115
	db	32
	db	110
	db	111
	db	32
	db	105
	db	115
	db	32
	db	122
	db	101
	db	114
	db	111
	db	0
	db	100
	db	105
	db	114
	db	32
	db	110
	db	111
	db	116
	db	32
	db	101
	db	120
	db	105
	db	115
	db	116
	db	0
	db	117
	db	115
	db	101
	db	100
	db	45
	db	99
	db	108
	db	117
	db	115
	db	116
	db	101
	db	114
	db	58
	db	32
	db	0
	db	108
	db	97
	db	115
	db	116
	db	32
	db	97
	db	100
	db	100
	db	114
	db	58
	db	32
	db	0
	db	108
	db	101
	db	110
	db	58
	db	0
	db	99
	db	114
	db	99
	db	58
	db	0
	db	99
	db	97
	db	110
	db	32
	db	110
	db	111
	db	116
	db	32
	db	102
	db	105
	db	110
	db	100
	db	32
	db	102
	db	105
	db	108
	db	101
	db	0
	db	60
	db	100
	db	105
	db	114
	db	62
	db	0
	db	114
	db	111
	db	111
	db	116
	db	32
	db	100
	db	105
	db	114
	db	0
_DATA	ends
	extrn	_set_32bit_value:far
	extrn	_tc_32bit_div:far
	extrn	_tc_32bit_cmp:far
	extrn	_tc_32bit_mul:far
	extrn	_tc_32bit_add:far
	extrn	_tc_32bit_sub:far
	extrn	_print_char:far
	extrn	_print_hex:far
	extrn	_cmp_str:far
	extrn	_move_1m:far
	extrn	_crc16:far
	extrn	_print_32bit_hex:far
	extrn	_mul_is_overflow:far
FAT32_TEXT	segment	byte public 'CODE'
FAT32_TEXT	ends
_fat_print_char	equ	fat_print_char
_fat_print_hex	equ	fat_print_hex
_read_clus_disk	equ	read_clus_disk
	public	_fat32_load_file
_disk_para_table	equ	disk_para_table
_read_fat32_data	equ	read_fat32_data
_get_next_clus	equ	get_next_clus
_fat_print_str	equ	fat_print_str
_read_lba_disk	equ	read_lba_disk
_read_chs_disk	equ	read_chs_disk
_bpb_para_table	equ	bpb_para_table
_get_fat32_para	equ	get_fat32_para
_get_disk_bsp	equ	get_disk_bsp
_fat_print_dec	equ	fat_print_dec
_get_disk_para	equ	get_disk_para
_dir_clus_no	equ	dir_clus_no
	public	_DISK_DRIVER_TYPE
_test_extend13H	equ	test_extend13H
_fat_row	equ	fat_row
_fat_col	equ	fat_col
	public	_list_fat32
	public	_init_fat32
_grf	equ	grf
	public	_fat32_change_directory
_read_file_by_clus_no	equ	read_file_by_clus_no
_fat_print_dword_hex	equ	fat_print_dword_hex
_read_fat32_root_dir	equ	read_fat32_root_dir
_fat_start_sector	equ	fat_start_sector
_dir_start_sector	equ	dir_start_sector
_get_file_clus_no	equ	get_file_clus_no
_disk_max_clus_no	equ	disk_max_clus_no
_curr_max_clus_no	equ	curr_max_clus_no
_curr_min_clus_no	equ	curr_min_clus_no
	public	_file_name_is_same
_extend_read_disk	equ	extend_read_disk
_reread_fat_table	equ	reread_fat_table
_read_fat32_table	equ	read_fat32_table
	end
