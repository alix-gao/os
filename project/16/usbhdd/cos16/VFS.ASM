	ifndef	??version
?debug	macro
	endm
	endif
	?debug	S "vfs.c"
VFS_TEXT	segment	byte public 'CODE'
DGROUP	group	_DATA,_BSS
	assume	cs:VFS_TEXT,ds:DGROUP
VFS_TEXT	ends
_DATA	segment word public 'DATA'
d@	label	byte
d@w	label	word
_DATA	ends
_BSS	segment word public 'BSS'
b@	label	byte
b@w	label	word
	?debug	C E9BA733944057666732E63
	?debug	C E90FA4AA4009747970656465662E68
	?debug	C E98EB02D4F0766617433322E68
_BSS	ends
VFS_TEXT	segment	byte public 'CODE'
;	?debug	L 5
_init_vfs	proc	far
;	?debug	L 7
	call	far ptr _init_fat32
@1:
;	?debug	L 8
	ret	
_init_vfs	endp
;	?debug	L 10
_load_file	proc	far
	push	bp
	mov	bp,sp
;	?debug	L 12
	mov	ax,word ptr [bp+6]
	or	ax,word ptr [bp+8]
	je	@3
;	?debug	L 13
	push	word ptr [bp+12]
	push	word ptr [bp+10]
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	call	far ptr _fat32_load_file
	mov	sp,bp
	jmp	short @2
@3:
;	?debug	L 15
	mov	ax,1
	jmp	short @2
@2:
;	?debug	L 16
	pop	bp
	ret	
_load_file	endp
;	?debug	L 18
_change_directory	proc	far
	push	bp
	mov	bp,sp
;	?debug	L 20
	mov	ax,word ptr [bp+6]
	or	ax,word ptr [bp+8]
	je	@5
;	?debug	L 21
	push	word ptr [bp+8]
	push	word ptr [bp+6]
	call	far ptr _fat32_change_directory
	mov	sp,bp
	jmp	short @4
@5:
;	?debug	L 23
	mov	ax,1
	jmp	short @4
@4:
;	?debug	L 24
	pop	bp
	ret	
_change_directory	endp
;	?debug	L 26
_list_vfs	proc	far
;	?debug	L 28
	call	far ptr _list_fat32
@6:
;	?debug	L 29
	ret	
_list_vfs	endp
VFS_TEXT	ends
	?debug	C E9
_DATA	segment word public 'DATA'
s@	label	byte
_DATA	ends
	extrn	_fat32_load_file:far
	extrn	_list_fat32:far
	extrn	_init_fat32:far
	extrn	_fat32_change_directory:far
VFS_TEXT	segment	byte public 'CODE'
VFS_TEXT	ends
	public	_list_vfs
	public	_init_vfs
	public	_load_file
	public	_change_directory
	end
