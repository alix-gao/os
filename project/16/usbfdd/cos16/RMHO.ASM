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
	?debug	C E9D69A715006726D686F2E63
	?debug	C E98B5EA64009747970656465662E68
	?debug	C E9C4B9B240056C69622E68
_BSS	ends
_DATA	segment word public 'DATA'
msg_info	label	byte
	db	115
tick	label	word
	dw	0
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
;	?debug	L 55
	inc	word ptr DGROUP:tick
;	?debug	L 58
	inc	word ptr DGROUP:no
;	?debug	L 59
	mov	ax,word ptr DGROUP:no
	mov	bx,3
	xor	dx,dx
	div	bx
	mov	word ptr DGROUP:no,dx
;	?debug	L 62
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
;	?debug	L 63
	ret	
_time_int_func	endp
;	?debug	L 65
modify_time_int	proc	far
	push	si
	push	di
;	?debug	L 67
 	push	 ax
;	?debug	L 68
 	push	 di
;	?debug	L 69
 	mov	 ax,0
;	?debug	L 70
 	mov	 ds,ax
;	?debug	L 71
 	mov	 di,080H
;	?debug	L 72
 	mov	 word ptr ds:[di],offset _time_int
;	?debug	L 73
 	add	 di,2
;	?debug	L 74
 	mov	 word ptr ds:[di],seg _time_int
;	?debug	L 75
 	pop	 di
;	?debug	L 76
 	pop	 ax
@2:
;	?debug	L 77
	pop	di
	pop	si
	ret	
modify_time_int	endp
;	?debug	L 79
open_time_int	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 81
	mov	byte ptr [bp-1],0
;	?debug	L 83
 	push	 ax
;	?debug	L 84
 	in	 al,021H
;	?debug	L 85
 	mov	 [bp-1],al
;	?debug	L 86
 	pop	 ax
;	?debug	L 88
	mov	al,byte ptr [bp-1]
	and	al,254
	mov	byte ptr [bp-1],al
;	?debug	L 90
 	push	 ax
;	?debug	L 91
 	mov	 al,[bp-1]
;	?debug	L 92
 	out	 021H,al
;	?debug	L 93
 	pop	 ax
@3:
;	?debug	L 94
	mov	sp,bp
	pop	bp
	ret	
open_time_int	endp
;	?debug	L 96
time_delay	proc	far
	push	bp
	mov	bp,sp
	sub	sp,4
;	?debug	L 101
	mov	word ptr [bp-2],0
	jmp	short @8
@7:
;	?debug	L 102
	mov	ax,word ptr DGROUP:tick
	mov	word ptr [bp-4],ax
	jmp	short @9
@11:
@9:
;	?debug	L 103
	mov	ax,word ptr DGROUP:tick
	sub	ax,word ptr [bp-4]
	cmp	ax,4
	jb	@11
@10:
@6:
	inc	word ptr [bp-2]
@8:
	mov	ax,word ptr [bp-2]
	cmp	ax,word ptr [bp+6]
	jb	@7
@5:
@4:
;	?debug	L 107
	mov	sp,bp
	pop	bp
	ret	
time_delay	endp
;	?debug	L 109
task1	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 113
	mov	word ptr [bp-2],0
	jmp	short @16
@15:
;	?debug	L 114
	push	word ptr [bp-2]
	mov	ax,7
	push	ax
	mov	al,15
	push	ax
	mov	al,99
	push	ax
	call	far ptr _print_char
	add	sp,8
;	?debug	L 115
	mov	ax,4
	push	ax
	push	cs
	call	near ptr time_delay
	pop	cx
;	?debug	L 116
	cmp	word ptr DGROUP:task1_running,0
	jne	@17
;	?debug	L 117
	jmp	short @13
@17:
@14:
	inc	word ptr [bp-2]
@16:
	cmp	word ptr [bp-2],80
	jb	@15
@13:
;	?debug	L 121
	mov	word ptr DGROUP:task1_running,0
;	?debug	L 123
	jmp	short @18
@20:
@18:
;	?debug	L 123
	jmp	short @20
@19:
@12:
;	?debug	L 124
	mov	sp,bp
	pop	bp
	ret	
task1	endp
;	?debug	L 126
task2	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 130
	mov	word ptr [bp-2],0
	jmp	short @25
@24:
;	?debug	L 131
	push	word ptr [bp-2]
	mov	ax,8
	push	ax
	mov	al,15
	push	ax
	mov	al,115
	push	ax
	call	far ptr _print_char
	add	sp,8
;	?debug	L 132
	mov	ax,4
	push	ax
	push	cs
	call	near ptr time_delay
	pop	cx
;	?debug	L 133
	cmp	word ptr DGROUP:task2_running,0
	jne	@26
;	?debug	L 134
	jmp	short @22
@26:
@23:
	inc	word ptr [bp-2]
@25:
	cmp	word ptr [bp-2],80
	jb	@24
@22:
;	?debug	L 138
	mov	word ptr DGROUP:task2_running,0
;	?debug	L 140
	jmp	short @27
@29:
@27:
;	?debug	L 140
	jmp	short @29
@28:
@21:
;	?debug	L 141
	mov	sp,bp
	pop	bp
	ret	
task2	endp
;	?debug	L 143
init_pt	proc	far
	push	bp
	mov	bp,sp
	sub	sp,4
;	?debug	L 147
	mov	word ptr DGROUP:process_table+2074,0
;	?debug	L 148
	mov	word ptr DGROUP:process_table+2076,0
;	?debug	L 150
	mov	word ptr [bp-2],ds
	mov	word ptr [bp-4],offset DGROUP:process_table+4138
;	?debug	L 151
 	push	 ax
;	?debug	L 152
 	mov	 ax,word ptr [bp-4]+2
;	?debug	L 153
 	mov	 DGROUP:_temp_reg_ss,ax
;	?debug	L 154
 	mov	 ax,word ptr [bp-4]
;	?debug	L 155
 	mov	 DGROUP:_temp_reg_sp,ax
;	?debug	L 156
 	pop	 ax
;	?debug	L 157
	mov	ax,word ptr DGROUP:_temp_reg_sp
	mov	word ptr DGROUP:process_table+2094,ax
;	?debug	L 158
	mov	ax,word ptr DGROUP:_temp_reg_ss
	mov	word ptr DGROUP:process_table+2092,ax
;	?debug	L 160
	mov	word ptr DGROUP:process_table+4142,514
;	?debug	L 161
	mov	word ptr [bp-2],cs
	mov	word ptr [bp-4],offset task1
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
	mov	ax,word ptr DGROUP:_temp_reg_ss
	mov	word ptr DGROUP:process_table+4140,ax
;	?debug	L 169
	mov	ax,word ptr DGROUP:_temp_reg_sp
	mov	word ptr DGROUP:process_table+4138,ax
;	?debug	L 171
	mov	word ptr DGROUP:process_table+4146,0
;	?debug	L 172
	mov	word ptr DGROUP:process_table+4148,0
;	?debug	L 174
	mov	word ptr [bp-2],ds
	mov	word ptr [bp-4],offset DGROUP:process_table+6210
;	?debug	L 175
 	push	 ax
;	?debug	L 176
 	mov	 ax,word ptr [bp-4]+2
;	?debug	L 177
 	mov	 DGROUP:_temp_reg_ss,ax
;	?debug	L 178
 	mov	 ax,word ptr [bp-4]
;	?debug	L 179
 	mov	 DGROUP:_temp_reg_sp,ax
;	?debug	L 180
 	pop	 ax
;	?debug	L 181
	mov	ax,word ptr DGROUP:_temp_reg_sp
	mov	word ptr DGROUP:process_table+4166,ax
;	?debug	L 182
	mov	ax,word ptr DGROUP:_temp_reg_ss
	mov	word ptr DGROUP:process_table+4164,ax
;	?debug	L 184
	mov	word ptr DGROUP:process_table+6214,514
;	?debug	L 185
	mov	word ptr [bp-2],cs
	mov	word ptr [bp-4],offset task2
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
	mov	ax,word ptr DGROUP:_temp_reg_ss
	mov	word ptr DGROUP:process_table+6212,ax
;	?debug	L 193
	mov	ax,word ptr DGROUP:_temp_reg_sp
	mov	word ptr DGROUP:process_table+6210,ax
@30:
;	?debug	L 194
	mov	sp,bp
	pop	bp
	ret	
init_pt	endp
;	?debug	L 196
init_flag	proc	far
;	?debug	L 198
	mov	word ptr DGROUP:task1_running,1
;	?debug	L 199
	mov	word ptr DGROUP:task2_running,1
@31:
;	?debug	L 200
	ret	
init_flag	endp
;	?debug	L 202
ori_task	proc	far
;	?debug	L 204
	push	cs
	call	near ptr init_flag
;	?debug	L 206
	mov	ax,1
	push	ax
	mov	ax,6
	push	ax
	mov	al,4
	push	ax
	mov	al,104
	push	ax
	call	far ptr _print_char
	add	sp,8
;	?debug	L 207
	mov	word ptr DGROUP:p+2,seg _print_char
	mov	word ptr DGROUP:p,offset _print_char
;	?debug	L 209
	mov	word ptr DGROUP:process_table,-1
;	?debug	L 210
	mov	word ptr DGROUP:process_table+2072,21845
;	?debug	L 211
	mov	word ptr DGROUP:process_table+4144,-21846
;	?debug	L 213
	mov	word ptr DGROUP:no,0
;	?debug	L 214
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
;	?debug	L 216
	push	cs
	call	near ptr init_pt
;	?debug	L 218
	push	cs
	call	near ptr modify_time_int
;	?debug	L 219
	push	cs
	call	near ptr open_time_int
;	?debug	L 221
 	sti	
	jmp	short @33
@35:
;	?debug	L 224
 	hlt	
@33:
;	?debug	L 223
	cmp	word ptr DGROUP:task1_running,0
	je	@36
	cmp	word ptr DGROUP:task2_running,0
	jne	@35
@36:
@34:
;	?debug	L 226
 	cli	
;	?debug	L 228
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
@32:
;	?debug	L 229
	ret	
ori_task	endp
;	?debug	L 231
_rmho	proc	far
;	?debug	L 233
	push	cs
	call	near ptr ori_task
@37:
;	?debug	L 234
	ret	
_rmho	endp
;	?debug	L 236
_stop_rmho	proc	far
;	?debug	L 238
	xor	ax,ax
	mov	word ptr DGROUP:task2_running,ax
	mov	word ptr DGROUP:task1_running,ax
@38:
;	?debug	L 239
	ret	
_stop_rmho	endp
RMHO_TEXT	ends
_BSS	segment word public 'BSS'
p	label	dword
	db	4 dup (?)
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
task1_running	label	word
	db	2 dup (?)
task2_running	label	word
	db	2 dup (?)
_sche_context	label	dword
	db	4 dup (?)
_BSS	ends
	?debug	C E9
_DATA	segment word public 'DATA'
s@	label	byte
_DATA	ends
	extrn	_print_char:far
	extrn	_time_int:far
RMHO_TEXT	segment	byte public 'CODE'
RMHO_TEXT	ends
_open_time_int	equ	open_time_int
	public	_sche_context
_task2_running	equ	task2_running
_task1_running	equ	task1_running
_process_table	equ	process_table
	public	_time_int_func
	public	_temp_reg_ss
_time_delay	equ	time_delay
	public	_temp_reg_sp
	public	_stop_rmho
_msg_info	equ	msg_info
_init_flag	equ	init_flag
_ori_task	equ	ori_task
_init_pt	equ	init_pt
	public	_temp_reg
	public	_rmho
_tick	equ	tick
_no	equ	no
_p	equ	p
_task2	equ	task2
_task1	equ	task1
_modify_time_int	equ	modify_time_int
	end
