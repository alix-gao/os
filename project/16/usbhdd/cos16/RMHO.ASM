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
	?debug	C E960BD3B4D06726D686F2E63
	?debug	C E90FA4AA4009747970656465662E68
	?debug	C E9AA56AC40056C69622E68
_BSS	ends
_DATA	segment word public 'DATA'
msg_info	label	byte
	db	115
_DATA	ends
RMHO_TEXT	segment	byte public 'CODE'
;	?debug	L 48
_time_int_func	proc	far
;	?debug	L 50
	xor	ax,ax
	push	ax
	mov	ax,6
	push	ax
	mov	al,15
	push	ax
	push	word ptr DGROUP:msg_info
	call	far ptr _print_char
	add	sp,8
;	?debug	L 51
	inc	byte ptr DGROUP:msg_info
;	?debug	L 54
	inc	word ptr DGROUP:no
;	?debug	L 55
	mov	ax,word ptr DGROUP:no
	mov	bx,3
	xor	dx,dx
	div	bx
	mov	word ptr DGROUP:no,dx
;	?debug	L 58
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
;	?debug	L 59
	ret	
_time_int_func	endp
;	?debug	L 61
modify_time_int	proc	far
	push	si
	push	di
;	?debug	L 63
 	push	 ax
;	?debug	L 64
 	push	 di
;	?debug	L 65
 	mov	 ax,0
;	?debug	L 66
 	mov	 ds,ax
;	?debug	L 67
 	mov	 di,080H
;	?debug	L 68
 	mov	 word ptr ds:[di],offset _time_int
;	?debug	L 69
 	add	 di,2
;	?debug	L 70
 	mov	 word ptr ds:[di],seg _time_int
;	?debug	L 71
 	pop	 di
;	?debug	L 72
 	pop	 ax
@2:
;	?debug	L 73
	pop	di
	pop	si
	ret	
modify_time_int	endp
;	?debug	L 75
open_time_int	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 77
	mov	byte ptr [bp-1],0
;	?debug	L 79
 	push	 ax
;	?debug	L 80
 	in	 al,021H
;	?debug	L 81
 	mov	 [bp-1],al
;	?debug	L 82
 	pop	 ax
;	?debug	L 84
	mov	al,byte ptr [bp-1]
	and	al,254
	mov	byte ptr [bp-1],al
;	?debug	L 86
 	push	 ax
;	?debug	L 87
 	mov	 al,[bp-1]
;	?debug	L 88
 	out	 021H,al
;	?debug	L 89
 	pop	 ax
@3:
;	?debug	L 90
	mov	sp,bp
	pop	bp
	ret	
open_time_int	endp
;	?debug	L 92
time_delay	proc	far
	push	bp
	mov	bp,sp
	sub	sp,6
;	?debug	L 98
	mov	word ptr [bp-4],0
	jmp	@8
@7:
;	?debug	L 99
	mov	word ptr [bp-2],0
	jmp	@12
@11:
;	?debug	L 100
 	push	 ax
;	?debug	L 101
 	push	 dx
;	?debug	L 102
 	mov	 dx,061H
;	?debug	L 103
 	in	 al,dx
;	?debug	L 104
 	mov	 [bp-6],al
;	?debug	L 105
 	pop	 dx
;	?debug	L 106
 	pop	 ax
;	?debug	L 107
	mov	al,byte ptr [bp-6]
	and	al,16
	mov	byte ptr [bp-6],al
;	?debug	L 108
	mov	al,byte ptr [bp-6]
	mov	byte ptr [bp-5],al
	jmp	short @13
@15:
;	?debug	L 110
 	push	 ax
;	?debug	L 111
 	push	 dx
;	?debug	L 112
 	mov	 dx,061H
;	?debug	L 113
 	in	 al,dx
;	?debug	L 114
 	mov	 [bp-6],al
;	?debug	L 115
 	pop	 dx
;	?debug	L 116
 	pop	 ax
;	?debug	L 117
	mov	al,byte ptr [bp-6]
	and	al,16
	mov	byte ptr [bp-6],al
;	?debug	L 118
	mov	al,byte ptr [bp-5]
	cmp	al,byte ptr [bp-6]
	je	@16
;	?debug	L 119
	jmp	short @14
@16:
@13:
;	?debug	L 109
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
;	?debug	L 122
	mov	sp,bp
	pop	bp
	ret	
time_delay	endp
;	?debug	L 124
task1	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 128
	mov	word ptr [bp-2],0
	jmp	short @23
@22:
;	?debug	L 129
	push	word ptr [bp-2]
	mov	ax,7
	push	ax
	mov	al,15
	push	ax
	mov	al,99
	push	ax
	call	far ptr _print_char
	add	sp,8
;	?debug	L 130
	mov	ax,7
	push	ax
	push	cs
	call	near ptr time_delay
	pop	cx
;	?debug	L 131
	cmp	word ptr DGROUP:task1_running,0
	jne	@24
;	?debug	L 132
	jmp	short @20
@24:
@21:
	inc	word ptr [bp-2]
@23:
	cmp	word ptr [bp-2],80
	jb	@22
@20:
;	?debug	L 136
	mov	word ptr DGROUP:task1_running,0
;	?debug	L 138
	jmp	short @25
@27:
@25:
;	?debug	L 138
	jmp	short @27
@26:
@19:
;	?debug	L 139
	mov	sp,bp
	pop	bp
	ret	
task1	endp
;	?debug	L 141
task2	proc	far
	push	bp
	mov	bp,sp
	sub	sp,2
;	?debug	L 145
	mov	word ptr [bp-2],0
	jmp	short @32
@31:
;	?debug	L 146
	push	word ptr [bp-2]
	mov	ax,8
	push	ax
	mov	al,15
	push	ax
	mov	al,115
	push	ax
	call	far ptr _print_char
	add	sp,8
;	?debug	L 147
	mov	ax,7
	push	ax
	push	cs
	call	near ptr time_delay
	pop	cx
;	?debug	L 148
	cmp	word ptr DGROUP:task2_running,0
	jne	@33
;	?debug	L 149
	jmp	short @29
@33:
@30:
	inc	word ptr [bp-2]
@32:
	cmp	word ptr [bp-2],80
	jb	@31
@29:
;	?debug	L 153
	mov	word ptr DGROUP:task2_running,0
;	?debug	L 155
	jmp	short @34
@36:
@34:
;	?debug	L 155
	jmp	short @36
@35:
@28:
;	?debug	L 156
	mov	sp,bp
	pop	bp
	ret	
task2	endp
;	?debug	L 158
init_pt	proc	far
	push	bp
	mov	bp,sp
	sub	sp,4
;	?debug	L 162
	mov	word ptr DGROUP:process_table+2074,0
;	?debug	L 163
	mov	word ptr DGROUP:process_table+2076,0
;	?debug	L 165
	mov	word ptr [bp-2],ds
	mov	word ptr [bp-4],offset DGROUP:process_table+4138
;	?debug	L 166
 	push	 ax
;	?debug	L 167
 	mov	 ax,word ptr [bp-4]+2
;	?debug	L 168
 	mov	 DGROUP:_temp_reg_ss,ax
;	?debug	L 169
 	mov	 ax,word ptr [bp-4]
;	?debug	L 170
 	mov	 DGROUP:_temp_reg_sp,ax
;	?debug	L 171
 	pop	 ax
;	?debug	L 172
	mov	ax,word ptr DGROUP:_temp_reg_sp
	mov	word ptr DGROUP:process_table+2094,ax
;	?debug	L 173
	mov	ax,word ptr DGROUP:_temp_reg_ss
	mov	word ptr DGROUP:process_table+2092,ax
;	?debug	L 175
	mov	word ptr DGROUP:process_table+4142,514
;	?debug	L 176
	mov	word ptr [bp-2],cs
	mov	word ptr [bp-4],offset task1
;	?debug	L 177
 	push	 ax
;	?debug	L 178
 	mov	 ax,word ptr [bp-4]+2
;	?debug	L 179
 	mov	 DGROUP:_temp_reg_ss,ax
;	?debug	L 180
 	mov	 ax,word ptr [bp-4]
;	?debug	L 181
 	mov	 DGROUP:_temp_reg_sp,ax
;	?debug	L 182
 	pop	 ax
;	?debug	L 183
	mov	ax,word ptr DGROUP:_temp_reg_ss
	mov	word ptr DGROUP:process_table+4140,ax
;	?debug	L 184
	mov	ax,word ptr DGROUP:_temp_reg_sp
	mov	word ptr DGROUP:process_table+4138,ax
;	?debug	L 186
	mov	word ptr DGROUP:process_table+4146,0
;	?debug	L 187
	mov	word ptr DGROUP:process_table+4148,0
;	?debug	L 189
	mov	word ptr [bp-2],ds
	mov	word ptr [bp-4],offset DGROUP:process_table+6210
;	?debug	L 190
 	push	 ax
;	?debug	L 191
 	mov	 ax,word ptr [bp-4]+2
;	?debug	L 192
 	mov	 DGROUP:_temp_reg_ss,ax
;	?debug	L 193
 	mov	 ax,word ptr [bp-4]
;	?debug	L 194
 	mov	 DGROUP:_temp_reg_sp,ax
;	?debug	L 195
 	pop	 ax
;	?debug	L 196
	mov	ax,word ptr DGROUP:_temp_reg_sp
	mov	word ptr DGROUP:process_table+4166,ax
;	?debug	L 197
	mov	ax,word ptr DGROUP:_temp_reg_ss
	mov	word ptr DGROUP:process_table+4164,ax
;	?debug	L 199
	mov	word ptr DGROUP:process_table+6214,514
;	?debug	L 200
	mov	word ptr [bp-2],cs
	mov	word ptr [bp-4],offset task2
;	?debug	L 201
 	push	 ax
;	?debug	L 202
 	mov	 ax,word ptr [bp-4]+2
;	?debug	L 203
 	mov	 DGROUP:_temp_reg_ss,ax
;	?debug	L 204
 	mov	 ax,word ptr [bp-4]
;	?debug	L 205
 	mov	 DGROUP:_temp_reg_sp,ax
;	?debug	L 206
 	pop	 ax
;	?debug	L 207
	mov	ax,word ptr DGROUP:_temp_reg_ss
	mov	word ptr DGROUP:process_table+6212,ax
;	?debug	L 208
	mov	ax,word ptr DGROUP:_temp_reg_sp
	mov	word ptr DGROUP:process_table+6210,ax
@37:
;	?debug	L 209
	mov	sp,bp
	pop	bp
	ret	
init_pt	endp
;	?debug	L 211
init_flag	proc	far
;	?debug	L 213
	mov	word ptr DGROUP:task1_running,1
;	?debug	L 214
	mov	word ptr DGROUP:task2_running,1
@38:
;	?debug	L 215
	ret	
init_flag	endp
;	?debug	L 217
ori_task	proc	far
;	?debug	L 219
	push	cs
	call	near ptr init_flag
;	?debug	L 221
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
;	?debug	L 222
	mov	word ptr DGROUP:p+2,seg _print_char
	mov	word ptr DGROUP:p,offset _print_char
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
	call	near ptr init_pt
;	?debug	L 233
	push	cs
	call	near ptr modify_time_int
;	?debug	L 234
	push	cs
	call	near ptr open_time_int
;	?debug	L 236
 	sti	
	jmp	short @40
@42:
;	?debug	L 239
 	hlt	
@40:
;	?debug	L 238
	cmp	word ptr DGROUP:task1_running,0
	je	@43
	cmp	word ptr DGROUP:task2_running,0
	jne	@42
@43:
@41:
;	?debug	L 241
 	cli	
;	?debug	L 243
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
@39:
;	?debug	L 244
	ret	
ori_task	endp
;	?debug	L 246
_rmho	proc	far
;	?debug	L 248
	push	cs
	call	near ptr ori_task
@44:
;	?debug	L 249
	ret	
_rmho	endp
;	?debug	L 251
_stop_rmho	proc	far
;	?debug	L 253
	xor	ax,ax
	mov	word ptr DGROUP:task2_running,ax
	mov	word ptr DGROUP:task1_running,ax
@45:
;	?debug	L 254
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
_no	equ	no
_p	equ	p
_task2	equ	task2
_task1	equ	task1
_modify_time_int	equ	modify_time_int
	end
