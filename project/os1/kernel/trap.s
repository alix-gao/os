	.file	"trap.cpp"
.globl _idt_addr
	.bss
	.align 4
_idt_addr:
	.space 4
.globl _gdt_addr
	.data
	.align 4
_gdt_addr:
	.long	2048
.globl _page_dir_addr
	.align 4
_page_dir_addr:
	.long	4096
.globl _page_table_0_addr
	.align 4
_page_table_0_addr:
	.long	1052672
.globl _page_table_1_addr
	.align 4
_page_table_1_addr:
	.long	1056768
.globl _page_table_2_addr
	.align 4
_page_table_2_addr:
	.long	1060864
.globl _page_table_3_addr
	.align 4
_page_table_3_addr:
	.long	1064960
.globl _cos_version
	.align 4
_cos_version:
	.long	17036
.globl _ip_addr
	.align 4
_ip_addr:
	.long	17032
.globl _curse_x
	.align 4
_curse_x:
	.long	17031
.globl _curse_y
	.align 4
_curse_y:
	.long	17030
.globl _key_make_code
	.align 32
_key_make_code:
	.ascii "qwertyuiop[]ECasdfghjkl;'`S\\zxcvbnm,./\0"
.globl _key_place_control_sel
	.align 4
_key_place_control_sel:
	.long	1
	.text
	.align 2
.globl __Z17modify_descriptorP11desc_structimmt
	.def	__Z17modify_descriptorP11desc_structimmt;	.scl	2;	.type	32;	.endef
__Z17modify_descriptorP11desc_structimmt:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$4, %esp
	movl	24(%ebp), %eax
	movw	%ax, -2(%ebp)
	movl	12(%ebp), %eax
	sall	$3, %eax
	addl	%eax, 8(%ebp)
	movl	8(%ebp), %edx
	movl	20(%ebp), %eax
	movb	%al, (%edx)
	movl	8(%ebp), %edx
	movl	20(%ebp), %eax
	shrl	$8, %eax
	movb	%al, 1(%edx)
	movl	8(%ebp), %edx
	movl	16(%ebp), %eax
	movb	%al, 2(%edx)
	movl	8(%ebp), %edx
	movl	16(%ebp), %eax
	shrl	$8, %eax
	movb	%al, 3(%edx)
	movl	8(%ebp), %edx
	movl	16(%ebp), %eax
	shrl	$16, %eax
	movb	%al, 4(%edx)
	movl	8(%ebp), %edx
	movzwl	-2(%ebp), %eax
	movb	%al, 5(%edx)
	movl	8(%ebp), %ecx
	movl	20(%ebp), %eax
	shrl	$16, %eax
	movb	%al, %dl
	andb	$15, %dl
	movzwl	-2(%ebp), %eax
	shrl	$8, %eax
	andb	$-16, %al
	orb	%dl, %al
	movb	%al, 6(%ecx)
	movl	8(%ebp), %edx
	movl	16(%ebp), %eax
	shrl	$24, %eax
	movb	%al, 7(%edx)
	leave
	ret
	.align 2
.globl __Z8init_tssP11task_structii
	.def	__Z8init_tssP11task_structii;	.scl	2;	.type	32;	.endef
__Z8init_tssP11task_structii:
	pushl	%ebp
	movl	%esp, %ebp
	movl	12(%ebp), %eax
	imull	$1196, %eax, %edx
	movl	8(%ebp), %eax
	movl	$1024, 8(%edx,%eax)
	movl	12(%ebp), %eax
	imull	$1196, %eax, %edx
	movl	8(%ebp), %eax
	movl	$2, 12(%edx,%eax)
	movl	12(%ebp), %eax
	imull	$1196, %eax, %edx
	movl	8(%ebp), %eax
	movl	$4096, 32(%edx,%eax)
	movl	12(%ebp), %eax
	imull	$1196, %eax, %ecx
	movl	8(%ebp), %edx
	movl	16(%ebp), %eax
	movl	%eax, 36(%ecx,%edx)
	movl	12(%ebp), %eax
	imull	$1196, %eax, %edx
	movl	8(%ebp), %eax
	movl	$514, 40(%edx,%eax)
	movl	12(%ebp), %eax
	imull	$1196, %eax, %ecx
	movl	8(%ebp), %edx
	movl	12(%ebp), %eax
	imull	$1196, %eax, %eax
	addl	8(%ebp), %eax
	addl	$1196, %eax
	movl	%eax, 60(%ecx,%edx)
	movl	12(%ebp), %eax
	imull	$1196, %eax, %edx
	movl	8(%ebp), %eax
	movl	$12, 80(%edx,%eax)
	movl	12(%ebp), %eax
	imull	$1196, %eax, %edx
	movl	8(%ebp), %eax
	movl	$20, 88(%edx,%eax)
	movl	12(%ebp), %eax
	imull	$1196, %eax, %edx
	movl	8(%ebp), %eax
	movl	$20, 84(%edx,%eax)
	movl	12(%ebp), %eax
	imull	$1196, %eax, %edx
	movl	8(%ebp), %eax
	movl	$20, 76(%edx,%eax)
	movl	12(%ebp), %eax
	imull	$1196, %eax, %edx
	movl	8(%ebp), %eax
	movl	$20, 92(%edx,%eax)
	movl	12(%ebp), %eax
	imull	$1196, %eax, %edx
	movl	8(%ebp), %eax
	movl	$20, 96(%edx,%eax)
	movl	12(%ebp), %eax
	imull	$1196, %eax, %edx
	movl	8(%ebp), %eax
	movl	$48, 100(%edx,%eax)
	popl	%ebp
	ret
	.align 2
.globl __Z8init_ldtP11task_structi
	.def	__Z8init_ldtP11task_structi;	.scl	2;	.type	32;	.endef
__Z8init_ldtP11task_structi:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$20, %esp
	movl	$16536, 16(%esp)
	movl	$1048575, 12(%esp)
	movl	$0, 8(%esp)
	movl	$1, 4(%esp)
	movl	12(%ebp), %eax
	imull	$1196, %eax, %eax
	addl	8(%ebp), %eax
	addl	$108, %eax
	movl	%eax, (%esp)
	call	__Z17modify_descriptorP11desc_structimmt
	movl	$16530, 16(%esp)
	movl	$1048575, 12(%esp)
	movl	$0, 8(%esp)
	movl	$2, 4(%esp)
	movl	12(%ebp), %eax
	imull	$1196, %eax, %eax
	addl	8(%ebp), %eax
	addl	$108, %eax
	movl	%eax, (%esp)
	call	__Z17modify_descriptorP11desc_structimmt
	leave
	ret
	.align 2
.globl __Z17find_next_processv
	.def	__Z17find_next_processv;	.scl	2;	.type	32;	.endef
__Z17find_next_processv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	$-1, -8(%ebp)
	movl	$0, -4(%ebp)
L5:
	cmpl	$2, -4(%ebp)
	jg	L6
	movl	-4(%ebp), %eax
	cmpl	$2, _state(,%eax,4)
	jne	L7
	movl	-4(%ebp), %eax
	movl	%eax, -8(%ebp)
L7:
	leal	-4(%ebp), %eax
	incl	(%eax)
	jmp	L5
L6:
	cmpl	$-1, -8(%ebp)
	jne	L9
	cmpl	$1, _state+4
	jne	L10
	movl	$5, -12(%ebp)
	jmp	L4
L10:
	cmpl	$1, _state+8
	jne	L11
	movl	$7, -12(%ebp)
	jmp	L4
L11:
	movl	$3, -12(%ebp)
	jmp	L4
L9:
	cmpl	$1, -8(%ebp)
	jne	L12
	movl	$1, _state+4
	cmpl	$1, _state+8
	jne	L13
	movl	$7, -12(%ebp)
	jmp	L4
L13:
	movl	$3, -12(%ebp)
	jmp	L4
L12:
	cmpl	$2, -8(%ebp)
	jne	L15
	movl	$1, _state+8
	movl	$3, -12(%ebp)
	jmp	L4
L15:
L4:
	movl	-12(%ebp), %eax
	leave
	ret
	.align 2
.globl __Z10f_time_intv
	.def	__Z10f_time_intv;	.scl	2;	.type	32;	.endef
__Z10f_time_intv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	$39, 12(%esp)
	movl	$11, 8(%esp)
	movl	$12, 4(%esp)
	movsbl	_int_c,%eax
	movl	%eax, (%esp)
	call	__Z10print_charciii
	incb	_int_c
	call	__Z17find_next_processv
	movl	%eax, -4(%ebp)
	movl	-4(%ebp), %eax
	cmpl	_time_int_switch, %eax
	jne	L17
	movl	$32, %eax
	movl	$32, %edx
/APP
	outb %al,%dx
/NO_APP
	jmp	L16
L17:
	cmpl	$5, -4(%ebp)
	jne	L19
	movl	$5, _time_int_switch
	movl	$2, _state+4
	movl	$32, %eax
	movl	$32, %edx
/APP
	outb %al,%dx
	ljmp $5*8,$0x0
/NO_APP
L19:
	cmpl	$7, -4(%ebp)
	jne	L20
	movl	$7, _time_int_switch
	movl	$2, _state+8
	movl	$32, %eax
	movl	$32, %edx
/APP
	outb %al,%dx
	ljmp $7*8,$0x0
/NO_APP
L20:
	cmpl	$3, -4(%ebp)
	jne	L16
	movl	$3, _time_int_switch
	movl	$32, %eax
	movl	$32, %edx
/APP
	outb %al,%dx
	ljmp $3*8,$0x0
/NO_APP
L16:
	leave
	ret
	.align 2
.globl __Z10modify_intPFvvEi
	.def	__Z10modify_intPFvvEi;	.scl	2;	.type	32;	.endef
__Z10modify_intPFvvEi:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%ebx
	movl	8(%ebp), %ecx
	movl	12(%ebp), %eax
	leal	0(,%eax,8), %ebx
/APP
	cli;              movl %ebx,%edi;              movl %ecx,%edx;              movl $0x80000,%eax;              movw %dx,%ax;              movw $0x8e00,%dx;              movl %eax,(%edi);              movl %edx,4(%edi);              sti
/NO_APP
	popl	%ebx
	popl	%edi
	popl	%ebp
	ret
	.align 2
.globl __Z10init_statev
	.def	__Z10init_statev;	.scl	2;	.type	32;	.endef
__Z10init_statev:
	pushl	%ebp
	movl	%esp, %ebp
	movl	$1, _state+8
	movl	$1, _state+4
	movl	$1, _state
	movl	$2, _state+4
	popl	%ebp
	ret
	.align 2
.globl __Z11init_switchv
	.def	__Z11init_switchv;	.scl	2;	.type	32;	.endef
__Z11init_switchv:
	pushl	%ebp
	movl	%esp, %ebp
	movl	$5, _time_int_switch
	popl	%ebp
	ret
	.align 2
.globl __Z13test_time_intv
	.def	__Z13test_time_intv;	.scl	2;	.type	32;	.endef
__Z13test_time_intv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$32, %esp
	movl	$0, -4(%ebp)
	movl	$1, -8(%ebp)
	movl	$2, -12(%ebp)
	movl	-8(%ebp), %eax
	imull	$1196, %eax, %eax
	movl	$305419896, _process_table(%eax)
	movl	$__Z9my_task_1v, 8(%esp)
	movl	-8(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$_process_table, (%esp)
	call	__Z8init_tssP11task_structii
	movl	-8(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$_process_table, (%esp)
	call	__Z8init_ldtP11task_structi
	movl	$16521, 16(%esp)
	movl	$1048575, 12(%esp)
	movl	-8(%ebp), %eax
	imull	$1196, %eax, %eax
	addl	$_process_table+4, %eax
	movl	%eax, 8(%esp)
	movl	$5, 4(%esp)
	movl	_gdt_addr, %eax
	movl	%eax, (%esp)
	call	__Z17modify_descriptorP11desc_structimmt
	movl	$16514, 16(%esp)
	movl	$1048575, 12(%esp)
	movl	-8(%ebp), %eax
	imull	$1196, %eax, %eax
	addl	$_process_table+108, %eax
	movl	%eax, 8(%esp)
	movl	$6, 4(%esp)
	movl	_gdt_addr, %eax
	movl	%eax, (%esp)
	call	__Z17modify_descriptorP11desc_structimmt
	movl	-8(%ebp), %eax
	imull	$1196, %eax, %eax
	movl	$286331153, _process_table(%eax)
	movl	$__Z9my_task_2v, 8(%esp)
	movl	-12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$_process_table, (%esp)
	call	__Z8init_tssP11task_structii
	movl	-12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$_process_table, (%esp)
	call	__Z8init_ldtP11task_structi
	movl	$16521, 16(%esp)
	movl	$1048575, 12(%esp)
	movl	-12(%ebp), %eax
	imull	$1196, %eax, %eax
	addl	$_process_table+4, %eax
	movl	%eax, 8(%esp)
	movl	$7, 4(%esp)
	movl	_gdt_addr, %eax
	movl	%eax, (%esp)
	call	__Z17modify_descriptorP11desc_structimmt
	movl	$16514, 16(%esp)
	movl	$1048575, 12(%esp)
	movl	-12(%ebp), %eax
	imull	$1196, %eax, %eax
	addl	$_process_table+108, %eax
	movl	%eax, 8(%esp)
	movl	$8, 4(%esp)
	movl	_gdt_addr, %eax
	movl	%eax, (%esp)
	call	__Z17modify_descriptorP11desc_structimmt
	movl	$16521, 16(%esp)
	movl	$1048575, 12(%esp)
	movl	-4(%ebp), %eax
	imull	$1196, %eax, %eax
	addl	$_process_table+4, %eax
	movl	%eax, 8(%esp)
	movl	$3, 4(%esp)
	movl	_gdt_addr, %eax
	movl	%eax, (%esp)
	call	__Z17modify_descriptorP11desc_structimmt
	call	__Z10init_statev
	call	__Z11init_switchv
	movl	$32, 4(%esp)
	movl	$__Z8time_intv, (%esp)
	call	__Z10modify_intPFvvEi
	movb	$35, _int_c
	movl	$24, %eax
/APP
	ltr %ax;              ljmp $5*8,$0x2000
	sti
/NO_APP
	leave
	ret
	.align 2
.globl __Z8f_kb_intv
	.def	__Z8f_kb_intv;	.scl	2;	.type	32;	.endef
__Z8f_kb_intv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	$96, %edx
/APP
	inb %dx,%al
/NO_APP
	movb	%al, -1(%ebp)
	cmpb	$1, -1(%ebp)
	jbe	L27
	cmpb	$10, -1(%ebp)
	ja	L27
	cmpl	$0, _key_place_control_sel
	je	L28
	movl	$0, 4(%esp)
	movzbl	-1(%ebp), %eax
	decl	%eax
	movl	%eax, (%esp)
	call	__Z17key_place_controlii
	jmp	L27
L28:
	movl	_curse_y, %eax
	movsbl	(%eax),%eax
	movl	%eax, 12(%esp)
	movl	_curse_x, %eax
	movsbl	(%eax),%eax
	movl	%eax, 8(%esp)
	movl	$15, 4(%esp)
	movzbl	-1(%ebp), %eax
	decl	%eax
	movl	%eax, (%esp)
	call	__Z9print_intiiii
	movl	_curse_y, %eax
	movsbl	(%eax),%eax
	incl	%eax
	movl	%eax, 4(%esp)
	movl	_curse_x, %eax
	movsbl	(%eax),%eax
	movl	%eax, (%esp)
	call	__Z10save_curseii
	movl	_curse_y, %eax
	movsbl	(%eax),%eax
	movl	%eax, 4(%esp)
	movl	_curse_x, %eax
	movsbl	(%eax),%eax
	movl	%eax, (%esp)
	call	__Z10move_curseii
L27:
	cmpb	$15, -1(%ebp)
	jbe	L30
	cmpb	$53, -1(%ebp)
	ja	L30
	cmpl	$0, _key_place_control_sel
	je	L31
	movl	$1, 4(%esp)
	movzbl	-1(%ebp), %eax
	movsbl	_key_make_code-16(%eax),%eax
	movl	%eax, (%esp)
	call	__Z17key_place_controlii
	jmp	L30
L31:
	movl	_curse_y, %eax
	movsbl	(%eax),%eax
	movl	%eax, 12(%esp)
	movl	_curse_x, %eax
	movsbl	(%eax),%eax
	movl	%eax, 8(%esp)
	movl	$15, 4(%esp)
	movzbl	-1(%ebp), %eax
	movsbl	_key_make_code-16(%eax),%eax
	movl	%eax, (%esp)
	call	__Z10print_charciii
	movl	_curse_y, %eax
	movsbl	(%eax),%eax
	incl	%eax
	movl	%eax, 4(%esp)
	movl	_curse_x, %eax
	movsbl	(%eax),%eax
	movl	%eax, (%esp)
	call	__Z10save_curseii
	movl	_curse_y, %eax
	movsbl	(%eax),%eax
	movl	%eax, 4(%esp)
	movl	_curse_x, %eax
	movsbl	(%eax),%eax
	movl	%eax, (%esp)
	call	__Z10move_curseii
L30:
	movl	$32, %eax
	movl	$32, %edx
/APP
	outb %al,%dx
/NO_APP
	leave
	ret
	.align 2
.globl __Z17test_keyboard_intv
	.def	__Z17test_keyboard_intv;	.scl	2;	.type	32;	.endef
__Z17test_keyboard_intv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	$33, 4(%esp)
	movl	$__Z6kb_intv, (%esp)
	call	__Z10modify_intPFvvEi
	leave
	ret
	.def	__Z6kb_intv;	.scl	3;	.type	32;	.endef
	.def	__Z10move_curseii;	.scl	3;	.type	32;	.endef
	.def	__Z10save_curseii;	.scl	3;	.type	32;	.endef
	.def	__Z9print_intiiii;	.scl	3;	.type	32;	.endef
	.def	__Z17key_place_controlii;	.scl	3;	.type	32;	.endef
	.def	__Z8time_intv;	.scl	3;	.type	32;	.endef
	.def	__Z9my_task_2v;	.scl	3;	.type	32;	.endef
	.def	__Z9my_task_1v;	.scl	3;	.type	32;	.endef
	.def	__Z10print_charciii;	.scl	3;	.type	32;	.endef
