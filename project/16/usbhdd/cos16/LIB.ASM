	ifndef	??version
?debug	macro
	endm
	endif
	?debug	S "lib.c"
LIB_TEXT	segment	byte public 'CODE'
DGROUP	group	_DATA,_BSS
	assume	cs:LIB_TEXT,ds:DGROUP
LIB_TEXT	ends
_DATA	segment word public 'DATA'
d@	label	byte
d@w	label	word
_DATA	ends
_BSS	segment word public 'BSS'
b@	label	byte
b@w	label	word
	?debug	C E940A4AA40056C69622E63
	?debug	C E90FA4AA4009747970656465662E68
	?debug	C E9AA56AC40056C69622E68
_BSS	ends
_DATA	segment word public 'DATA'
hex_template	label	byte
	db	48
	db	49
	db	50
	db	51
	db	52
	db	53
	db	54
	db	55
	db	56
	db	57
	db	97
	db	98
	db	99
	db	100
	db	101
	db	102
_DATA	ends
LIB_TEXT	segment	byte public 'CODE'
;	?debug	L 7
_print_char	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
	push	si
	push	di
;	?debug	L 11
	mov	ax,word ptr [bp+10]
	mov	dx,80
	mul	dx
	add	ax,word ptr [bp+12]
	shl	ax,1
	mov	word ptr [bp-2],ax
@4:
;	?debug	L 14
 	push	 ax
;	?debug	L 15
 	push	 es
;	?debug	L 16
 	push	 di
;	?debug	L 18
 	mov	 ax,00b800H
;	?debug	L 19
 	mov	 es,ax
;	?debug	L 20
 	mov	 di,[bp-2]
;	?debug	L 21
 	mov	 ah,[bp+8]
;	?debug	L 22
 	mov	 al,[bp+6]
;	?debug	L 23
 	mov	 es:[di],ax
;	?debug	L 25
 	pop	 di
;	?debug	L 26
 	pop	 es
;	?debug	L 27
 	pop	 ax
@2:
@3:
@1:
;	?debug	L 29
	pop	di
	pop	si
	mov	sp,bp
	pop	bp
	ret	
_print_char	endp
;	?debug	L 31
_print_string	proc	far
	push	bp
	mov	bp,sp
	jmp	short @6
@8:
;	?debug	L 34
	mov	ax,word ptr [bp+14]
	inc	word ptr [bp+14]
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	les	bx,dword ptr [bp+6]
	mov	al,byte ptr es:[bx]
	push	ax
	push	cs
	call	near ptr _print_char
	mov	sp,bp
;	?debug	L 35
	inc	word ptr [bp+6]
@6:
;	?debug	L 33
	les	bx,dword ptr [bp+6]
	cmp	byte ptr es:[bx],0
	jne	@8
@7:
@5:
;	?debug	L 37
	pop	bp
	ret	
_print_string	endp
;	?debug	L 39
_print_hex	proc	far
	push	bp
	mov	bp,sp
;	?debug	L 41
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	word ptr [bp+8]
	mov	al,byte ptr [bp+6]
	mov	cl,4
	shr	al,cl
	mov	ah,0
	mov	bx,ax
	push	word ptr DGROUP:hex_template[bx]
	push	cs
	call	near ptr _print_char
	mov	sp,bp
;	?debug	L 42
	mov	ax,word ptr [bp+12]
	inc	ax
	push	ax
	push	word ptr [bp+10]
	push	word ptr [bp+8]
	mov	al,byte ptr [bp+6]
	mov	ah,0
	mov	bx,ax
	and	bx,15
	push	word ptr DGROUP:hex_template[bx]
	push	cs
	call	near ptr _print_char
	mov	sp,bp
@9:
;	?debug	L 43
	pop	bp
	ret	
_print_hex	endp
;	?debug	L 45
_print_32bit_hex	proc	far
	push	bp
	mov	bp,sp
;	?debug	L 47
	mov	ax,word ptr [bp+14]
	inc	word ptr [bp+14]
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	mov	al,byte ptr [bp+9]
	mov	cl,4
	shr	al,cl
	mov	ah,0
	mov	bx,ax
	push	word ptr DGROUP:hex_template[bx]
	push	cs
	call	near ptr _print_char
	mov	sp,bp
;	?debug	L 48
	mov	ax,word ptr [bp+14]
	inc	word ptr [bp+14]
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	mov	al,byte ptr [bp+9]
	mov	ah,0
	mov	bx,ax
	and	bx,15
	push	word ptr DGROUP:hex_template[bx]
	push	cs
	call	near ptr _print_char
	mov	sp,bp
;	?debug	L 50
	mov	ax,word ptr [bp+14]
	inc	word ptr [bp+14]
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	mov	al,byte ptr [bp+8]
	mov	cl,4
	shr	al,cl
	mov	ah,0
	mov	bx,ax
	push	word ptr DGROUP:hex_template[bx]
	push	cs
	call	near ptr _print_char
	mov	sp,bp
;	?debug	L 51
	mov	ax,word ptr [bp+14]
	inc	word ptr [bp+14]
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	mov	al,byte ptr [bp+8]
	mov	ah,0
	mov	bx,ax
	and	bx,15
	push	word ptr DGROUP:hex_template[bx]
	push	cs
	call	near ptr _print_char
	mov	sp,bp
;	?debug	L 53
	mov	ax,word ptr [bp+14]
	inc	word ptr [bp+14]
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	mov	al,byte ptr [bp+7]
	mov	cl,4
	shr	al,cl
	mov	ah,0
	mov	bx,ax
	push	word ptr DGROUP:hex_template[bx]
	push	cs
	call	near ptr _print_char
	mov	sp,bp
;	?debug	L 54
	mov	ax,word ptr [bp+14]
	inc	word ptr [bp+14]
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	mov	al,byte ptr [bp+7]
	mov	ah,0
	mov	bx,ax
	and	bx,15
	push	word ptr DGROUP:hex_template[bx]
	push	cs
	call	near ptr _print_char
	mov	sp,bp
;	?debug	L 56
	mov	ax,word ptr [bp+14]
	inc	word ptr [bp+14]
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	mov	al,byte ptr [bp+6]
	mov	cl,4
	shr	al,cl
	mov	ah,0
	mov	bx,ax
	push	word ptr DGROUP:hex_template[bx]
	push	cs
	call	near ptr _print_char
	mov	sp,bp
;	?debug	L 57
	mov	ax,word ptr [bp+14]
	inc	word ptr [bp+14]
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	mov	al,byte ptr [bp+6]
	mov	ah,0
	mov	bx,ax
	and	bx,15
	push	word ptr DGROUP:hex_template[bx]
	push	cs
	call	near ptr _print_char
	mov	sp,bp
@10:
;	?debug	L 58
	pop	bp
	ret	
_print_32bit_hex	endp
;	?debug	L 60
_cpy_mem	proc	far
	push	bp
	mov	bp,sp
	jmp	short @12
@14:
;	?debug	L 63
	dec	word ptr [bp+14]
;	?debug	L 65
	les	bx,dword ptr [bp+10]
	mov	al,byte ptr es:[bx]
	les	bx,dword ptr [bp+6]
	mov	byte ptr es:[bx],al
;	?debug	L 66
	inc	word ptr [bp+6]
;	?debug	L 67
	inc	word ptr [bp+10]
@12:
;	?debug	L 62
	cmp	word ptr [bp+14],0
	jne	@14
@13:
@11:
;	?debug	L 69
	pop	bp
	ret	
_cpy_mem	endp
;	?debug	L 71
_set_32bit_value	proc	far
	push	bp
	mov	bp,sp
;	?debug	L 73
	mov	ax,4
	push	ax
	push	ss
	lea	ax,word ptr [bp+10]
	push	ax
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	cs
	call	near ptr _cpy_mem
	mov	sp,bp
@15:
;	?debug	L 74
	pop	bp
	ret	
_set_32bit_value	endp
;	?debug	L 76
_tc_32bit_cmp	proc	far
	push	bp
	mov	bp,sp
;	?debug	L 78
	mov	ax,word ptr [bp+8]
	cmp	ax,word ptr [bp+12]
	jbe	@17
;	?debug	L 79
	mov	ax,1
	jmp	short @16
;	?debug	L 80
	jmp	short @18
@17:
;	?debug	L 80
	mov	ax,word ptr [bp+8]
	cmp	ax,word ptr [bp+12]
	jae	@19
;	?debug	L 81
	mov	ax,-1
	jmp	short @16
;	?debug	L 82
	jmp	short @20
@19:
;	?debug	L 83
	mov	ax,word ptr [bp+6]
	cmp	ax,word ptr [bp+10]
	jbe	@21
;	?debug	L 84
	mov	ax,1
	jmp	short @16
;	?debug	L 85
	jmp	short @22
@21:
;	?debug	L 85
	mov	ax,word ptr [bp+6]
	cmp	ax,word ptr [bp+10]
	jae	@23
;	?debug	L 86
	mov	ax,-1
	jmp	short @16
;	?debug	L 87
	jmp	short @24
@23:
;	?debug	L 88
	xor	ax,ax
	jmp	short @16
@24:
@22:
@20:
@18:
@16:
;	?debug	L 91
	pop	bp
	ret	
_tc_32bit_cmp	endp
;	?debug	L 93
_cmp_str	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 95
	mov	word ptr [bp-2],0
	jmp	short @26
@28:
;	?debug	L 98
	les	bx,dword ptr [bp+6]
	add	bx,word ptr [bp-2]
	cmp	byte ptr es:[bx],0
	jne	@29
;	?debug	L 100
	xor	ax,ax
	jmp	short @25
@29:
;	?debug	L 102
	inc	word ptr [bp-2]
@26:
;	?debug	L 97
	les	bx,dword ptr [bp+6]
	add	bx,word ptr [bp-2]
	mov	al,byte ptr es:[bx]
	les	bx,dword ptr [bp+10]
	add	bx,word ptr [bp-2]
	cmp	al,byte ptr es:[bx]
	je	@28
@27:
;	?debug	L 104
	les	bx,dword ptr [bp+6]
	add	bx,word ptr [bp-2]
	mov	al,byte ptr es:[bx]
	mov	ah,0
	les	bx,dword ptr [bp+10]
	add	bx,word ptr [bp-2]
	mov	dl,byte ptr es:[bx]
	mov	dh,0
	sub	ax,dx
	jmp	short @25
@25:
;	?debug	L 105
	mov	sp,bp
	pop	bp
	ret	
_cmp_str	endp
;	?debug	L 109
_tc_32bit_add	proc	far
	push	bp
	mov	bp,sp
;	?debug	L 120
	mov	ax,word ptr [bp+14]
	xor	dx,dx
	add	ax,word ptr [bp+10]
	adc	dx,word ptr [bp+12]
	les	bx,dword ptr [bp+6]
	mov	word ptr es:[bx+2],dx
	mov	word ptr es:[bx],ax
;	?debug	L 122
	mov	ax,word ptr [bp+16]
	les	bx,dword ptr [bp+6]
	add	word ptr es:[bx+2],ax
@30:
;	?debug	L 125
	pop	bp
	ret	
_tc_32bit_add	endp
;	?debug	L 127
_tc_32bit_sub	proc	far
	push	bp
	mov	bp,sp
;	?debug	L 129
	mov	ax,word ptr [bp+10]
	sub	ax,word ptr [bp+14]
	les	bx,dword ptr [bp+6]
	mov	word ptr es:[bx],ax
;	?debug	L 131
	mov	ax,word ptr [bp+10]
	cmp	ax,word ptr [bp+14]
	jae	@32
;	?debug	L 132
	dec	word ptr [bp+12]
@32:
;	?debug	L 135
	mov	ax,word ptr [bp+12]
	sub	ax,word ptr [bp+16]
	les	bx,dword ptr [bp+6]
	mov	word ptr es:[bx+2],ax
@31:
;	?debug	L 136
	pop	bp
	ret	
_tc_32bit_sub	endp
;	?debug	L 138
_tc_32bit_mul	proc	far
	push	bp
	mov	bp,sp
	sub	sp,4
;	?debug	L 142
	mov	word ptr [bp-2],0
	mov	word ptr [bp-4],0
	jmp	short @37
@36:
;	?debug	L 144
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	ss
	lea	ax,word ptr [bp-4]
	push	ax
	push	cs
	call	near ptr _tc_32bit_add
	add	sp,12
@35:
;	?debug	L 143
	sub	word ptr [bp+14],1
	sbb	word ptr [bp+16],0
@37:
;	?debug	L 143
	mov	ax,word ptr [bp+14]
	or	ax,word ptr [bp+16]
	jne	@36
@34:
;	?debug	L 146
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	cs
	call	near ptr _set_32bit_value
	add	sp,8
@33:
;	?debug	L 147
	mov	sp,bp
	pop	bp
	ret	
_tc_32bit_mul	endp
;	?debug	L 149
_tc_word_div	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 153
	mov	word ptr [bp-2],0
	jmp	short @39
@41:
;	?debug	L 155
	mov	ax,word ptr [bp+16]
	sub	word ptr [bp+14],ax
;	?debug	L 156
	inc	word ptr [bp-2]
@39:
;	?debug	L 154
	mov	ax,word ptr [bp+14]
	cmp	ax,word ptr [bp+16]
	jae	@41
@40:
;	?debug	L 159
	mov	ax,word ptr [bp-2]
	les	bx,dword ptr [bp+6]
	mov	word ptr es:[bx],ax
;	?debug	L 160
	mov	ax,word ptr [bp+14]
	les	bx,dword ptr [bp+10]
	mov	word ptr es:[bx],ax
@38:
;	?debug	L 161
	mov	sp,bp
	pop	bp
	ret	
_tc_word_div	endp
;	?debug	L 163
_tc_32bit_div	proc	far
	push	bp
	mov	bp,sp
	sub	sp,4
;	?debug	L 165
	mov	word ptr [bp-2],0
	mov	word ptr [bp-4],0
	jmp	short @43
@45:
;	?debug	L 168
	push	word ptr [bp+20]
	push	word ptr [bp+18]
	push	word ptr [bp+16]
	push	word ptr [bp+14]
	push	ss
	lea	ax,word ptr [bp+14]
	push	ax
	push	cs
	call	near ptr _tc_32bit_sub
	add	sp,12
;	?debug	L 169
	add	word ptr [bp-4],1
	adc	word ptr [bp-2],0
@43:
;	?debug	L 167
	push	word ptr [bp+20]
	push	word ptr [bp+18]
	push	word ptr [bp+16]
	push	word ptr [bp+14]
	push	cs
	call	near ptr _tc_32bit_cmp
	add	sp,8
	cmp	ax,-1
	jne	@45
@44:
;	?debug	L 172
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	cs
	call	near ptr _set_32bit_value
	add	sp,8
;	?debug	L 173
	push	word ptr [bp+16]
	push	word ptr [bp+14]
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	cs
	call	near ptr _set_32bit_value
	add	sp,8
@42:
;	?debug	L 174
	mov	sp,bp
	pop	bp
	ret	
_tc_32bit_div	endp
;	?debug	L 176
exchange_dword	proc	far
	push	bp
	mov	bp,sp
;	?debug	L 178
	les	bx,dword ptr [bp+6]
	mov	ax,word ptr es:[bx+2]
	les	bx,dword ptr [bp+10]
	xor	ax,word ptr es:[bx+2]
	les	bx,dword ptr [bp+6]
	mov	word ptr es:[bx+2],ax
;	?debug	L 179
	les	bx,dword ptr [bp+6]
	mov	ax,word ptr es:[bx+2]
	les	bx,dword ptr [bp+10]
	xor	ax,word ptr es:[bx+2]
	les	bx,dword ptr [bp+10]
	mov	word ptr es:[bx+2],ax
;	?debug	L 180
	les	bx,dword ptr [bp+6]
	mov	ax,word ptr es:[bx+2]
	les	bx,dword ptr [bp+10]
	xor	ax,word ptr es:[bx+2]
	les	bx,dword ptr [bp+6]
	mov	word ptr es:[bx+2],ax
;	?debug	L 182
	les	bx,dword ptr [bp+6]
	mov	ax,word ptr es:[bx]
	les	bx,dword ptr [bp+10]
	xor	ax,word ptr es:[bx]
	les	bx,dword ptr [bp+6]
	mov	word ptr es:[bx],ax
;	?debug	L 183
	les	bx,dword ptr [bp+6]
	mov	ax,word ptr es:[bx]
	les	bx,dword ptr [bp+10]
	xor	ax,word ptr es:[bx]
	les	bx,dword ptr [bp+10]
	mov	word ptr es:[bx],ax
;	?debug	L 184
	les	bx,dword ptr [bp+6]
	mov	ax,word ptr es:[bx]
	les	bx,dword ptr [bp+10]
	xor	ax,word ptr es:[bx]
	les	bx,dword ptr [bp+6]
	mov	word ptr es:[bx],ax
@46:
;	?debug	L 185
	pop	bp
	ret	
exchange_dword	endp
;	?debug	L 187
_add_is_overflow	proc	far
	push	bp
	mov	bp,sp
	sub	sp,4
	push	si
;	?debug	L 192
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	mov	dx,-1
	mov	ax,-1
	push	dx
	push	ax
	push	ss
	lea	ax,word ptr [bp-4]
	push	ax
	push	cs
	call	near ptr _tc_32bit_sub
	add	sp,12
;	?debug	L 193
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	push	cs
	call	near ptr _tc_32bit_cmp
	add	sp,8
	mov	si,ax
;	?debug	L 194
	cmp	si,-1
	jne	@48
;	?debug	L 195
	mov	al,1
	jmp	short @47
;	?debug	L 196
	jmp	short @49
@48:
;	?debug	L 197
	mov	al,0
	jmp	short @47
@49:
@47:
;	?debug	L 199
	pop	si
	mov	sp,bp
	pop	bp
	ret	
_add_is_overflow	endp
;	?debug	L 201
_mul_is_overflow	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8
	push	si
;	?debug	L 207
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	cs
	call	near ptr _tc_32bit_cmp
	add	sp,8
	mov	si,ax
;	?debug	L 208
	cmp	si,-1
	jne	@51
;	?debug	L 210
	push	ss
	lea	ax,word ptr [bp+10]
	push	ax
	push	ss
	lea	ax,word ptr [bp+6]
	push	ax
	push	cs
	call	near ptr exchange_dword
	add	sp,8
@51:
;	?debug	L 213
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	mov	dx,-1
	mov	ax,-1
	push	dx
	push	ax
	push	ss
	lea	ax,word ptr [bp-4]
	push	ax
	push	ss
	lea	ax,word ptr [bp-8]
	push	ax
	push	cs
	call	near ptr _tc_32bit_div
	add	sp,16
;	?debug	L 214
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	word ptr [bp-6]
	push	word ptr [bp-8]
	push	cs
	call	near ptr _tc_32bit_cmp
	add	sp,8
	mov	si,ax
;	?debug	L 215
	cmp	si,-1
	jne	@52
;	?debug	L 216
	mov	al,1
	jmp	short @50
;	?debug	L 217
	jmp	short @53
@52:
;	?debug	L 217
	cmp	si,1
	jne	@54
;	?debug	L 218
	mov	al,0
	jmp	short @50
@54:
@53:
;	?debug	L 219
	or	si,si
	jne	@55
;	?debug	L 220
	mov	ax,word ptr [bp-4]
	or	ax,word ptr [bp-2]
	je	@56
;	?debug	L 221
	mov	al,1
	jmp	short @50
;	?debug	L 222
	jmp	short @57
@56:
;	?debug	L 223
	mov	al,0
	jmp	short @50
@57:
;	?debug	L 225
	jmp	short @58
@55:
;	?debug	L 227
	mov	al,0
	jmp	short @50
@58:
@50:
;	?debug	L 229
	pop	si
	mov	sp,bp
	pop	bp
	ret	
_mul_is_overflow	endp
;	?debug	L 231
_test_libc	proc	far
	push	bp
	mov	bp,sp
	sub	sp,24
;	?debug	L 234
	mov	word ptr [bp-4],15
;	?debug	L 235
	mov	word ptr [bp-2],0
;	?debug	L 238
	mov	word ptr [bp-18],0
	mov	word ptr [bp-20],1
;	?debug	L 239
	mov	word ptr [bp-14],0
	mov	word ptr [bp-16],2
;	?debug	L 240
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	word ptr [bp-18]
	push	word ptr [bp-20]
	push	ss
	lea	ax,word ptr [bp-12]
	push	ax
	push	cs
	call	near ptr _tc_32bit_add
	add	sp,12
;	?debug	L 241
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	push	cs
	call	near ptr _print_32bit_hex
	add	sp,10
;	?debug	L 241
	add	word ptr [bp-2],9
;	?debug	L 244
	mov	word ptr [bp-18],-1
	mov	word ptr [bp-20],-4
;	?debug	L 245
	mov	word ptr [bp-14],0
	mov	word ptr [bp-16],8
;	?debug	L 246
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	word ptr [bp-18]
	push	word ptr [bp-20]
	push	ss
	lea	ax,word ptr [bp-12]
	push	ax
	push	cs
	call	near ptr _tc_32bit_add
	add	sp,12
;	?debug	L 247
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	push	cs
	call	near ptr _print_32bit_hex
	add	sp,10
;	?debug	L 247
	add	word ptr [bp-2],9
;	?debug	L 251
	mov	word ptr [bp-18],0
	mov	word ptr [bp-20],1
;	?debug	L 252
	mov	word ptr [bp-14],0
	mov	word ptr [bp-16],2
;	?debug	L 253
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	word ptr [bp-18]
	push	word ptr [bp-20]
	push	ss
	lea	ax,word ptr [bp-12]
	push	ax
	push	cs
	call	near ptr _tc_32bit_sub
	add	sp,12
;	?debug	L 254
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	push	cs
	call	near ptr _print_32bit_hex
	add	sp,10
;	?debug	L 254
	add	word ptr [bp-2],9
;	?debug	L 257
	mov	word ptr [bp-18],0
	mov	word ptr [bp-20],4
;	?debug	L 258
	mov	word ptr [bp-14],0
	mov	word ptr [bp-16],2
;	?debug	L 259
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	word ptr [bp-18]
	push	word ptr [bp-20]
	push	ss
	lea	ax,word ptr [bp-12]
	push	ax
	push	cs
	call	near ptr _tc_32bit_sub
	add	sp,12
;	?debug	L 260
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	push	cs
	call	near ptr _print_32bit_hex
	add	sp,10
;	?debug	L 260
	add	word ptr [bp-2],9
;	?debug	L 263
	mov	word ptr [bp-18],1
	mov	word ptr [bp-20],1
;	?debug	L 264
	mov	word ptr [bp-14],2
	mov	word ptr [bp-16],2
;	?debug	L 265
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	word ptr [bp-18]
	push	word ptr [bp-20]
	push	ss
	lea	ax,word ptr [bp-12]
	push	ax
	push	cs
	call	near ptr _tc_32bit_sub
	add	sp,12
;	?debug	L 266
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	push	cs
	call	near ptr _print_32bit_hex
	add	sp,10
;	?debug	L 266
	add	word ptr [bp-2],9
;	?debug	L 269
	mov	word ptr [bp-18],2
	mov	word ptr [bp-20],2
;	?debug	L 270
	mov	word ptr [bp-14],1
	mov	word ptr [bp-16],2
;	?debug	L 271
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	word ptr [bp-18]
	push	word ptr [bp-20]
	push	ss
	lea	ax,word ptr [bp-12]
	push	ax
	push	cs
	call	near ptr _tc_32bit_sub
	add	sp,12
;	?debug	L 272
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	push	cs
	call	near ptr _print_32bit_hex
	add	sp,10
;	?debug	L 272
	add	word ptr [bp-2],9
;	?debug	L 275
	mov	word ptr [bp-18],1
	mov	word ptr [bp-20],2
;	?debug	L 276
	mov	word ptr [bp-14],1
	mov	word ptr [bp-16],2
;	?debug	L 277
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	word ptr [bp-18]
	push	word ptr [bp-20]
	push	ss
	lea	ax,word ptr [bp-12]
	push	ax
	push	cs
	call	near ptr _tc_32bit_sub
	add	sp,12
;	?debug	L 278
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	push	cs
	call	near ptr _print_32bit_hex
	add	sp,10
;	?debug	L 278
	add	word ptr [bp-2],9
;	?debug	L 282
	mov	word ptr [bp-18],0
	mov	word ptr [bp-20],258
;	?debug	L 283
	mov	word ptr [bp-14],0
	mov	word ptr [bp-16],257
;	?debug	L 284
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	word ptr [bp-18]
	push	word ptr [bp-20]
	push	ss
	lea	ax,word ptr [bp-12]
	push	ax
	push	cs
	call	near ptr _tc_32bit_mul
	add	sp,12
;	?debug	L 285
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	push	cs
	call	near ptr _print_32bit_hex
	add	sp,10
;	?debug	L 285
	add	word ptr [bp-2],9
;	?debug	L 288
	mov	word ptr [bp-18],1
	mov	word ptr [bp-20],2
;	?debug	L 289
	mov	word ptr [bp-14],1
	mov	word ptr [bp-16],1
;	?debug	L 290
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	word ptr [bp-18]
	push	word ptr [bp-20]
	push	ss
	lea	ax,word ptr [bp-12]
	push	ax
	push	cs
	call	near ptr _tc_32bit_mul
	add	sp,12
;	?debug	L 291
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	push	cs
	call	near ptr _print_32bit_hex
	add	sp,10
;	?debug	L 291
	add	word ptr [bp-2],9
;	?debug	L 294
	mov	word ptr [bp-18],-32768
	mov	word ptr [bp-20],2
;	?debug	L 295
	mov	word ptr [bp-14],8
	mov	word ptr [bp-16],1
;	?debug	L 296
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	word ptr [bp-18]
	push	word ptr [bp-20]
	push	ss
	lea	ax,word ptr [bp-12]
	push	ax
	push	cs
	call	near ptr _tc_32bit_mul
	add	sp,12
;	?debug	L 297
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	push	cs
	call	near ptr _print_32bit_hex
	add	sp,10
;	?debug	L 297
	add	word ptr [bp-2],9
;	?debug	L 301
	mov	word ptr [bp-18],-32768
	mov	word ptr [bp-20],2
;	?debug	L 302
	mov	word ptr [bp-14],8
	mov	word ptr [bp-16],1
;	?debug	L 303
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	word ptr [bp-18]
	push	word ptr [bp-20]
	push	ss
	lea	ax,word ptr [bp-8]
	push	ax
	push	ss
	lea	ax,word ptr [bp-12]
	push	ax
	push	cs
	call	near ptr _tc_32bit_div
	add	sp,16
;	?debug	L 304
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	push	cs
	call	near ptr _print_32bit_hex
	add	sp,10
;	?debug	L 304
	add	word ptr [bp-2],9
;	?debug	L 307
	mov	word ptr [bp-18],0
	mov	word ptr [bp-20],291
;	?debug	L 308
	mov	word ptr [bp-14],8
	mov	word ptr [bp-16],1
;	?debug	L 309
	push	word ptr [bp-14]
	push	word ptr [bp-16]
	push	word ptr [bp-18]
	push	word ptr [bp-20]
	push	ss
	lea	ax,word ptr [bp-8]
	push	ax
	push	ss
	lea	ax,word ptr [bp-12]
	push	ax
	push	cs
	call	near ptr _tc_32bit_div
	add	sp,16
;	?debug	L 310
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-10]
	push	word ptr [bp-12]
	push	cs
	call	near ptr _print_32bit_hex
	add	sp,10
;	?debug	L 310
	add	word ptr [bp-2],9
@62:
;	?debug	L 317
	mov	dx,-32768
	mov	ax,1
	push	dx
	push	ax
	mov	dx,-28673
	mov	ax,16
	push	dx
	push	ax
	push	cs
	call	near ptr _add_is_overflow
	add	sp,8
	mov	byte ptr [bp-21],al
;	?debug	L 318
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-21]
	push	cs
	call	near ptr _print_hex
	add	sp,8
;	?debug	L 318
	add	word ptr [bp-2],3
;	?debug	L 321
	mov	dx,-32768
	mov	ax,1
	push	dx
	push	ax
	xor	dx,dx
	mov	ax,16
	push	dx
	push	ax
	push	cs
	call	near ptr _add_is_overflow
	add	sp,8
	mov	byte ptr [bp-21],al
;	?debug	L 322
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-21]
	push	cs
	call	near ptr _print_hex
	add	sp,8
;	?debug	L 322
	add	word ptr [bp-2],3
;	?debug	L 325
	mov	dx,8
	mov	ax,1
	push	dx
	push	ax
	mov	dx,-32768
	mov	ax,2
	push	dx
	push	ax
	push	cs
	call	near ptr _mul_is_overflow
	add	sp,8
	mov	byte ptr [bp-21],al
;	?debug	L 326
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-21]
	push	cs
	call	near ptr _print_hex
	add	sp,8
;	?debug	L 326
	add	word ptr [bp-2],3
;	?debug	L 329
	mov	dx,8
	mov	ax,1
	push	dx
	push	ax
	xor	dx,dx
	mov	ax,18
	push	dx
	push	ax
	push	cs
	call	near ptr _mul_is_overflow
	add	sp,8
	mov	byte ptr [bp-21],al
;	?debug	L 330
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-21]
	push	cs
	call	near ptr _print_hex
	add	sp,8
;	?debug	L 330
	add	word ptr [bp-2],3
;	?debug	L 333
	mov	dx,257
	mov	ax,257
	push	dx
	push	ax
	xor	dx,dx
	mov	ax,255
	push	dx
	push	ax
	push	cs
	call	near ptr _mul_is_overflow
	add	sp,8
	mov	byte ptr [bp-21],al
;	?debug	L 334
	push	word ptr [bp-2]
	push	word ptr [bp-4]
	mov	al,15
	push	ax
	push	word ptr [bp-21]
	push	cs
	call	near ptr _print_hex
	add	sp,8
;	?debug	L 334
	add	word ptr [bp-2],3
@60:
@61:
@65:
@68:
;	?debug	L 340
	mov	word ptr [bp-24],13398
;	?debug	L 340
	mov	word ptr [bp-22],8192
@66:
@67:
;	?debug	L 341
	les	bx,dword ptr [bp-24]
	mov	byte ptr es:[bx],205
@63:
@64:
@71:
;	?debug	L 345
 	sti	
;	?debug	L 345
	jmp	short @72
@74:
;	?debug	L 345
 	hlt	
@72:
;	?debug	L 345
	jmp	short @74
@73:
@69:
@70:
@59:
;	?debug	L 346
	mov	sp,bp
	pop	bp
	ret	
_test_libc	endp
LIB_TEXT	ends
	?debug	C E9
_DATA	segment word public 'DATA'
s@	label	byte
_DATA	ends
LIB_TEXT	segment	byte public 'CODE'
LIB_TEXT	ends
	public	_set_32bit_value
	public	_tc_32bit_div
_exchange_dword	equ	exchange_dword
	public	_tc_32bit_cmp
	public	_tc_word_div
	public	_tc_32bit_mul
	public	_print_string
	public	_tc_32bit_add
_hex_template	equ	hex_template
	public	_tc_32bit_sub
	public	_print_char
	public	_print_hex
	public	_test_libc
	public	_cmp_str
	public	_cpy_mem
	public	_print_32bit_hex
	public	_mul_is_overflow
	public	_add_is_overflow
	end
