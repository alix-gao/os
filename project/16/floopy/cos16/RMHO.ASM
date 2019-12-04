	ifndef	??version
?debug	macro
	endm
	endif
	?debug	S "rmho.c"
RMHO_TEXT	segment	byte public 'CODE'
DGROUP	group	_DATA,_BSS
	assume	cs:RMHO_TEXT,ds:DGROUP
RMHO_TEXT	ends
_DATA	segment word public 'DATA'
d@	label	byte
d@w	label	word
_DATA	ends
_BSS	segment word public 'BSS'
b@	label	byte
b@w	label	word
	?debug	C E9B491653F06726D686F2E63
	?debug	C E93149A64009747970656465662E68
	?debug	C E9FA9EA840056C69622E68
_BSS	ends
_DATA	segment word public 'DATA'
msg_info	label	byte
	db	115
_DATA	ends
RMHO_TEXT	segment	byte public 'CODE'
;	?debug	L 50
_time_int_func	proc	far
;	?debug	L 52
	xor	ax,ax
	push	ax
	mov	ax,6
	push	ax
	mov	al,15
	push	ax
	push	word ptr DGROUP:msg_info
	call	far ptr _print_char
	add	sp,8
;	?debug	L 53
	inc	byte ptr DGROUP:msg_info
;	?debug	L 56
	inc	word ptr DGROUP:no
;	?debug	L 57
	mov	ax,word ptr DGROUP:no
	mov	bx,3
	xor	dx,dx
	div	bx
	mov	word ptr DGROUP:no,dx
;	?debug	L 60
	mov	ax,word ptr DGROUP:no
	mov	dx,2072
	mul	dx
	mov	bx,ax
	add	bx,offset DGROUP:process_table
	push	ds
	pop	es
	inc	bx
	inc	bx
	mov	word ptr DGROUP:_sche_context+2,es
	mov	word ptr DGROUP:_sche_context,bx
@1:
;	?debug	L 61
	ret	
_time_int_func	endp
;	?debug	L 63
modify_time_int	proc	far
	push	si
	push	di
;	?debug	L 65
 	push	 ax
;	?debug	L 66
 	push	 di
;	?debug	L 67
 	mov	 ax,0
;	?debug	L 68
 	mov	 ds,ax
;	?debug	L 69
 	mov	 di,080H
;	?debug	L 70
 	mov	 word ptr ds:[di],offset _time_int
;	?debug	L 71
 	add	 di,2
;	?debug	L 72
 	mov	 word ptr ds:[di],seg _time_int
;	?debug	L 73
 	pop	 di
;	?debug	L 74
 	pop	 ax
@2:
;	?debug	L 75
	pop	di
	pop	si
	ret	
modify_time_int	endp
;	?debug	L 77
open_time_int	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 79
	mov	byte ptr [bp-1],0
;	?debug	L 81
 	push	 ax
;	?debug	L 82
 	in	 al,021H
;	?debug	L 83
 	mov	 [bp-1],al
;	?debug	L 84
 	pop	 ax
;	?debug	L 86
	mov	al,byte ptr [bp-1]
	and	al,254
	mov	byte ptr [bp-1],al
;	?debug	L 88
 	push	 ax
;	?debug	L 89
 	mov	 al,[bp-1]
;	?debug	L 90
 	out	 021H,al
;	?debug	L 91
 	pop	 ax
@3:
;	?debug	L 92
	mov	sp,bp
	pop	bp
	ret	
open_time_int	endp
;	?debug	L 94
time_delay	proc	far
	push	bp
	mov	bp,sp
	sub	sp,6
;	?debug	L 100
	mov	word ptr [bp-4],0
	jmp	@8
@7:
;	?debug	L 101
	mov	word ptr [bp-2],0
	jmp	@12
@11:
;	?debug	L 102
 	push	 ax
;	?debug	L 103
 	push	 dx
;	?debug	L 104
 	mov	 dx,061H
;	?debug	L 105
 	in	 al,dx
;	?debug	L 106
 	mov	 [bp-6],al
;	?debug	L 107
 	pop	 dx
;	?debug	L 108
 	pop	 ax
;	?debug	L 109
	mov	al,byte ptr [bp-6]
	and	al,16
	mov	byte ptr [bp-6],al
;	?debug	L 110
	mov	al,byte ptr [bp-6]
	mov	byte ptr [bp-5],al
	jmp	short @13
@15:
;	?debug	L 112
 	push	 ax
;	?debug	L 113
 	push	 dx
;	?debug	L 114
 	mov	 dx,061H
;	?debug	L 115
 	in	 al,dx
;	?debug	L 116
 	mov	 [bp-6],al
;	?debug	L 117
 	pop	 dx
;	?debug	L 118
 	pop	 ax
;	?debug	L 119
	mov	al,byte ptr [bp-6]
	and	al,16
	mov	byte ptr [bp-6],al
;	?debug	L 120
	mov	al,byte ptr [bp-5]
	cmp	al,byte ptr [bp-6]
	je	@16
;	?debug	L 121
	jmp	short @14
@16:
@13:
;	?debug	L 111
	jmp	short @15
@14:
@10:
	inc	word ptr [bp-2]
@12:
	cmp	word ptr [bp-2],10
	jae	@17
	jmp	@11
@17:
@9:
@6:
	inc	word ptr [bp-4]
@8:
	mov	ax,word ptr [bp+6]
	mov	dx,100
	mul	dx
	cmp	ax,word ptr [bp-4]
	jbe	@18
	jmp	@7
@18:
@5:
@4:
;	?debug	L 124
	mov	sp,bp
	pop	bp
	ret	
time_delay	endp
;	?debug	L 126
p1	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 130
	mov	word ptr [bp-2],0
	jmp	short @23
@22:
;	?debug	L 131
	push	word ptr [bp-2]
	mov	ax,7
	push	ax
	mov	al,15
	push	ax
	mov	al,112
	push	ax
	call	far ptr _print_char
	add	sp,8
;	?debug	L 132
	mov	ax,7
	push	ax
	push	cs
	call	near ptr time_delay
	pop	cx
@21:
	inc	word ptr [bp-2]
@23:
	cmp	word ptr [bp-2],80
	jb	@22
@20:
;	?debug	L 135
	mov	word ptr DGROUP:process_1,1
;	?debug	L 137
	jmp	short @24
@26:
@24:
;	?debug	L 137
	jmp	short @26
@25:
@19:
;	?debug	L 138
	mov	sp,bp
	pop	bp
	ret	
p1	endp
;	?debug	L 140
p2	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 144
	mov	word ptr [bp-2],0
	jmp	short @31
@30:
;	?debug	L 145
	push	word ptr [bp-2]
	mov	ax,8
	push	ax
	mov	al,15
	push	ax
	mov	al,115
	push	ax
	call	far ptr _print_char
	add	sp,8
;	?debug	L 146
	mov	ax,7
	push	ax
	push	cs
	call	near ptr time_delay
	pop	cx
@29:
	inc	word ptr [bp-2]
@31:
	cmp	word ptr [bp-2],80
	jb	@30
@28:
;	?debug	L 149
	mov	word ptr DGROUP:process_2,1
;	?debug	L 151
	jmp	short @32
@34:
@32:
;	?debug	L 151
	jmp	short @34
@33:
@27:
;	?debug	L 152
	mov	sp,bp
	pop	bp
	ret	
p2	endp
;	?debug	L 154
init_pt	proc	far
	push	bp
	mov	bp,sp
	sub	sp,4
;	?debug	L 158
	mov	word ptr DGROUP:process_table+2074,0
;	?debug	L 159
	mov	word ptr DGROUP:process_table+2076,0
;	?debug	L 161
	mov	word ptr [bp-2],ds
	mov	word ptr [bp-4],offset DGROUP:process_table+4138
;	?debug	L 162
 	push	 ax
;	?debug	L 163
 	mov	 ax,word ptr [bp-4]+2
;	?debug	L 164
 	mov	 DGROUP:_temp_reg_ss,ax
;	?debug	L 165
 	mov	 ax,word ptr [bp-4]
;	?debug	L 166
 	mov	 DGROUP:_temp_reg_sp,ax
;	?debug	L 167
 	pop	 ax
;	?debug	L 168
	mov	ax,word ptr DGROUP:_temp_reg_sp
	mov	word ptr DGROUP:process_table+2094,ax
;	?debug	L 169
	mov	ax,word ptr DGROUP:_temp_reg_ss
	mov	word ptr DGROUP:process_table+2092,ax
;	?debug	L 171
	mov	word ptr DGROUP:process_table+4142,514
;	?debug	L 172
	mov	word ptr [bp-2],cs
	mov	word ptr [bp-4],offset p1
;	?debug	L 173
 	push	 ax
;	?debug	L 174
 	mov	 ax,word ptr [bp-4]+2
;	?debug	L 175
 	mov	 DGROUP:_temp_reg_ss,ax
;	?debug	L 176
 	mov	 ax,word ptr [bp-4]
;	?debug	L 177
 	mov	 DGROUP:_temp_reg_sp,ax
;	?debug	L 178
 	pop	 ax
;	?debug	L 179
	mov	ax,word ptr DGROUP:_temp_reg_ss
	mov	word ptr DGROUP:process_table+4140,ax
;	?debug	L 180
	mov	ax,word ptr DGROUP:_temp_reg_sp
	mov	word ptr DGROUP:process_table+4138,ax
;	?debug	L 182
	mov	word ptr DGROUP:process_table+4146,0
;	?debug	L 183
	mov	word ptr DGROUP:process_table+4148,0
;	?debug	L 185
	mov	word ptr [bp-2],ds
	mov	word ptr [bp-4],offset DGROUP:process_table+6210
;	?debug	L 186
 	push	 ax
;	?debug	L 187
 	mov	 ax,word ptr [bp-4]+2
;	?debug	L 188
 	mov	 DGROUP:_temp_reg_ss,ax
;	?debug	L 189
 	mov	 ax,word ptr [bp-4]
;	?debug	L 190
 	mov	 DGROUP:_temp_reg_sp,ax
;	?debug	L 191
 	pop	 ax
;	?debug	L 192
	mov	ax,word ptr DGROUP:_temp_reg_sp
	mov	word ptr DGROUP:process_table+4166,ax
;	?debug	L 193
	mov	ax,word ptr DGROUP:_temp_reg_ss
	mov	word ptr DGROUP:process_table+4164,ax
;	?debug	L 195
	mov	word ptr DGROUP:process_table+6214,514
;	?debug	L 196
	mov	word ptr [bp-2],cs
	mov	word ptr [bp-4],offset p2
;	?debug	L 197
 	push	 ax
;	?debug	L 198
 	mov	 ax,word ptr [bp-4]+2
;	?debug	L 199
 	mov	 DGROUP:_temp_reg_ss,ax
;	?debug	L 200
 	mov	 ax,word ptr [bp-4]
;	?debug	L 201
 	mov	 DGROUP:_temp_reg_sp,ax
;	?debug	L 202
 	pop	 ax
;	?debug	L 203
	mov	ax,word ptr DGROUP:_temp_reg_ss
	mov	word ptr DGROUP:process_table+6212,ax
;	?debug	L 204
	mov	ax,word ptr DGROUP:_temp_reg_sp
	mov	word ptr DGROUP:process_table+6210,ax
@35:
;	?debug	L 205
	mov	sp,bp
	pop	bp
	ret	
init_pt	endp
;	?debug	L 207
init_flag	proc	far
;	?debug	L 209
	mov	word ptr DGROUP:process_1,0
;	?debug	L 210
	mov	word ptr DGROUP:process_2,0
@36:
;	?debug	L 211
	ret	
init_flag	endp
;	?debug	L 213
task_switch	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 215
	mov	byte ptr [bp-1],104
;	?debug	L 217
	push	cs
	call	near ptr init_flag
;	?debug	L 219
	mov	ax,1
	push	ax
	mov	ax,6
	push	ax
	mov	al,4
	push	ax
	push	word ptr [bp-1]
	call	far ptr _print_char
	add	sp,8
;	?debug	L 220
	mov	word ptr DGROUP:p+2,seg _print_char
	mov	word ptr DGROUP:p,offset _print_char
;	?debug	L 222
	push	cs
	call	near ptr modify_time_int
;	?debug	L 224
	mov	word ptr DGROUP:process_table,-1
;	?debug	L 225
	mov	word ptr DGROUP:process_table+2072,21845
;	?debug	L 226
	mov	word ptr DGROUP:process_table+4144,-21846
;	?debug	L 228
	mov	word ptr DGROUP:no,0
;	?debug	L 229
	mov	ax,word ptr DGROUP:no
	mov	dx,2072
	mul	dx
	mov	bx,ax
	add	bx,offset DGROUP:process_table
	push	ds
	pop	es
	inc	bx
	inc	bx
	mov	word ptr DGROUP:_sche_context+2,es
	mov	word ptr DGROUP:_sche_context,bx
;	?debug	L 231
	push	cs
	call	near ptr open_time_int
;	?debug	L 233
	push	cs
	call	near ptr init_pt
;	?debug	L 235
 	sti	
	jmp	short @38
@40:
;	?debug	L 238
 	hlt	
@38:
;	?debug	L 237
	cmp	word ptr DGROUP:process_1,0
	jne	@41
	cmp	word ptr DGROUP:process_2,0
	je	@40
@41:
@39:
;	?debug	L 240
 	cli	
;	?debug	L 242
	mov	ax,2
	push	ax
	mov	ax,6
	push	ax
	mov	al,4
	push	ax
	mov	al,111
	push	ax
	call	far ptr _print_char
	add	sp,8
@37:
;	?debug	L 243
	mov	sp,bp
	pop	bp
	ret	
task_switch	endp
;	?debug	L 245
_rmho	proc	far
;	?debug	L 247
	call	far ptr _os_8259a
;	?debug	L 249
	push	cs
	call	near ptr task_switch
;	?debug	L 251
	call	far ptr _bios_8259a
@42:
;	?debug	L 252
	ret	
_rmho	endp
RMHO_TEXT	ends
_BSS	segment word public 'BSS'
p	label	dword
	db	4 dup (?)
process_1	label	word
	db	2 dup (?)
process_2	label	word
	db	2 dup (?)
no	label	word
	db	2 dup (?)
_temp_reg	label	word
	db	2 dup (?)
_temp_reg_sp	label	word
	db	2 dup (?)
_temp_reg_ss	label	word
	db	2 dup (?)
process_table	label	word
	db	6216 dup (?)
_sche_context	label	dword
	db	4 dup (?)
_BSS	ends
	?debug	C E9
_DATA	segment word public 'DATA'
s@	label	byte
_DATA	ends
	extrn	_print_char:far
	extrn	_time_int:far
	extrn	_bios_8259a:far
	extrn	_os_8259a:far
RMHO_TEXT	segment	byte public 'CODE'
RMHO_TEXT	ends
_open_time_int	equ	open_time_int
	public	_sche_context
_process_table	equ	process_table
	public	_time_int_func
	public	_temp_reg_ss
_time_delay	equ	time_delay
	public	_temp_reg_sp
_task_switch	equ	task_switch
_msg_info	equ	msg_info
_init_flag	equ	init_flag
_init_pt	equ	init_pt
	public	_temp_reg
	public	_rmho
_no	equ	no
_process_2	equ	process_2
_process_1	equ	process_1
_p	equ	p
_p2	equ	p2
_p1	equ	p1
_modify_time_int	equ	modify_time_int
	end
