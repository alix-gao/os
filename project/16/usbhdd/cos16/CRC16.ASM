	ifndef	??version
?debug	macro
	endm
	endif
	?debug	S "crc16.c"
CRC16_TEXT	segment	byte public 'CODE'
DGROUP	group	_DATA,_BSS
	assume	cs:CRC16_TEXT,ds:DGROUP
CRC16_TEXT	ends
_DATA	segment word public 'DATA'
d@	label	byte
d@w	label	word
_DATA	ends
_BSS	segment word public 'BSS'
b@	label	byte
b@w	label	word
	?debug	C E95C7B2B4F0763726331362E63
	?debug	C E90FA4AA4009747970656465662E68
	?debug	C E9AA56AC40056C69622E68
_BSS	ends
CRC16_TEXT	segment	byte public 'CODE'
;	?debug	L 7
_crc16	proc	far
	push	bp
	mov	bp,sp
	sub	sp,4
;	?debug	L 10
	mov	word ptr [bp-2],-1
	jmp	@2
@4:
;	?debug	L 13
	xor	dx,dx
	mov	ax,1
	push	dx
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	ss
	lea	ax,word ptr [bp+10]
	push	ax
	call	far ptr _tc_32bit_sub
	add	sp,12
;	?debug	L 14
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	call	far ptr _read_hmb
	pop	cx
	pop	cx
	mov	dx,word ptr [bp-2]
	mov	cl,8
	shr	dx,cl
	xor	al,dl
	mov	byte ptr [bp-3],al
;	?debug	L 15
	xor	dx,dx
	mov	ax,1
	push	dx
	push	ax
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	push	ss
	lea	ax,word ptr [bp+6]
	push	ax
	call	far ptr _tc_32bit_add
	add	sp,12
;	?debug	L 16
	mov	al,byte ptr [bp-3]
	mov	cl,4
	shr	al,cl
	xor	byte ptr [bp-3],al
;	?debug	L 17
	mov	ax,word ptr [bp-2]
	mov	cl,8
	shl	ax,cl
	mov	dl,byte ptr [bp-3]
	mov	dh,0
	mov	cl,12
	shl	dx,cl
	xor	ax,dx
	mov	dl,byte ptr [bp-3]
	mov	dh,0
	mov	cl,5
	shl	dx,cl
	xor	ax,dx
	mov	dl,byte ptr [bp-3]
	mov	dh,0
	xor	ax,dx
	mov	word ptr [bp-2],ax
@2:
;	?debug	L 12
	xor	dx,dx
	mov	ax,1
	push	dx
	push	ax
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	call	far ptr _tc_32bit_cmp
	add	sp,8
	cmp	ax,-1
	je	@5
	jmp	@4
@5:
@3:
;	?debug	L 19
	mov	ax,word ptr [bp-2]
	jmp	short @1
@1:
;	?debug	L 20
	mov	sp,bp
	pop	bp
	ret	
_crc16	endp
CRC16_TEXT	ends
	?debug	C E9
_DATA	segment word public 'DATA'
s@	label	byte
_DATA	ends
	extrn	_tc_32bit_cmp:far
	extrn	_tc_32bit_add:far
	extrn	_tc_32bit_sub:far
	extrn	_read_hmb:far
CRC16_TEXT	segment	byte public 'CODE'
CRC16_TEXT	ends
	public	_crc16
	end
