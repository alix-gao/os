	.file	"msg_test.cpp"
	.data
	.align 4
_ZZ9wndproc_1P7tag_msgE8char_pos:
	.long	2
	.text
	.align 2
.globl __Z9wndproc_1P7tag_msg
	.def	__Z9wndproc_1P7tag_msg;	.scl	2;	.type	32;	.endef
__Z9wndproc_1P7tag_msg:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$40, %esp
	movl	8(%ebp), %eax
	movl	8(%eax), %eax
	movl	%eax, -4(%ebp)
	movl	-4(%ebp), %eax
	movl	%eax, -8(%ebp)
	cmpl	$0, -8(%ebp)
	jns	L2
	addl	$15, -8(%ebp)
L2:
	movl	-8(%ebp), %eax
	sarl	$4, %eax
	sall	$4, %eax
	movl	-4(%ebp), %edx
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, 16(%esp)
	movl	$10, 12(%esp)
	movl	$10, 8(%esp)
	movl	$1, 4(%esp)
	movl	$1, (%esp)
	call	__Z10disp_coloriiiii
	movl	$4, 24(%esp)
	movl	_ZZ9wndproc_1P7tag_msgE8char_pos, %eax
	movl	%eax, 20(%esp)
	movl	8(%ebp), %eax
	movl	8(%eax), %eax
	movl	%eax, 16(%esp)
	movl	$380, 12(%esp)
	movl	$420, 8(%esp)
	movl	$220, 4(%esp)
	movl	$260, (%esp)
	call	__Z9note_bookiiiiiii
	movl	%eax, _ZZ9wndproc_1P7tag_msgE8char_pos
	leave
	ret
	.align 2
.globl __Z21init_application_gc_1v
	.def	__Z21init_application_gc_1v;	.scl	2;	.type	32;	.endef
__Z21init_application_gc_1v:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$56, %esp
	movl	$260, -40(%ebp)
	movl	$220, -36(%ebp)
	movl	$420, -32(%ebp)
	movl	$380, -28(%ebp)
	movl	$1, -24(%ebp)
	movl	$4, -20(%ebp)
	movl	$__Z9wndproc_1P7tag_msg, -16(%ebp)
	leal	-40(%ebp), %eax
	movl	%eax, (%esp)
	call	__Z16registerclass_gcP11wndclass_gc
	leave
	ret
	.align 2
.globl __Z18init_instance_gc_1v
	.def	__Z18init_instance_gc_1v;	.scl	2;	.type	32;	.endef
__Z18init_instance_gc_1v:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	$1, (%esp)
	call	__Z16create_window_gci
	leave
	ret
	.align 2
.globl __Z10msg_task_1v
	.def	__Z10msg_task_1v;	.scl	2;	.type	32;	.endef
__Z10msg_task_1v:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$56, %esp
	call	__Z21init_application_gc_1v
	call	__Z18init_instance_gc_1v
L6:
	leal	-40(%ebp), %eax
	movl	%eax, (%esp)
	call	__Z11get_messageP7tag_msg
	testb	%al, %al
	je	L5
	leal	-40(%ebp), %eax
	movl	%eax, (%esp)
	call	__Z15dispatchmessageP7tag_msg
	jmp	L6
L5:
	leave
	ret
	.data
	.align 4
_ZZ9wndproc_2P7tag_msgE8char_pos:
	.long	2
	.text
	.align 2
.globl __Z9wndproc_2P7tag_msg
	.def	__Z9wndproc_2P7tag_msg;	.scl	2;	.type	32;	.endef
__Z9wndproc_2P7tag_msg:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$40, %esp
	movl	8(%ebp), %eax
	movl	8(%eax), %eax
	movl	%eax, -4(%ebp)
	movl	-4(%ebp), %eax
	movl	%eax, -8(%ebp)
	cmpl	$0, -8(%ebp)
	jns	L9
	addl	$15, -8(%ebp)
L9:
	movl	-8(%ebp), %eax
	sarl	$4, %eax
	sall	$4, %eax
	movl	-4(%ebp), %edx
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, 16(%esp)
	movl	$20, 12(%esp)
	movl	$20, 8(%esp)
	movl	$11, 4(%esp)
	movl	$11, (%esp)
	call	__Z10disp_coloriiiii
	movl	$2, 24(%esp)
	movl	_ZZ9wndproc_2P7tag_msgE8char_pos, %eax
	movl	%eax, 20(%esp)
	movl	8(%ebp), %eax
	movl	8(%eax), %eax
	movl	%eax, 16(%esp)
	movl	$200, 12(%esp)
	movl	$620, 8(%esp)
	movl	$40, 4(%esp)
	movl	$460, (%esp)
	call	__Z9note_bookiiiiiii
	movl	%eax, _ZZ9wndproc_2P7tag_msgE8char_pos
	leave
	ret
	.align 2
.globl __Z21init_application_gc_2v
	.def	__Z21init_application_gc_2v;	.scl	2;	.type	32;	.endef
__Z21init_application_gc_2v:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$56, %esp
	movl	$460, -40(%ebp)
	movl	$40, -36(%ebp)
	movl	$620, -32(%ebp)
	movl	$200, -28(%ebp)
	movl	$2, -24(%ebp)
	movl	$2, -20(%ebp)
	movl	$__Z9wndproc_2P7tag_msg, -16(%ebp)
	leal	-40(%ebp), %eax
	movl	%eax, (%esp)
	call	__Z16registerclass_gcP11wndclass_gc
	leave
	ret
	.align 2
.globl __Z18init_instance_gc_2v
	.def	__Z18init_instance_gc_2v;	.scl	2;	.type	32;	.endef
__Z18init_instance_gc_2v:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	$2, (%esp)
	call	__Z16create_window_gci
	leave
	ret
	.align 2
.globl __Z10msg_task_2v
	.def	__Z10msg_task_2v;	.scl	2;	.type	32;	.endef
__Z10msg_task_2v:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$56, %esp
	call	__Z21init_application_gc_2v
	call	__Z18init_instance_gc_2v
L13:
	leal	-40(%ebp), %eax
	movl	%eax, (%esp)
	call	__Z11get_messageP7tag_msg
	testb	%al, %al
	je	L12
	leal	-40(%ebp), %eax
	movl	%eax, (%esp)
	call	__Z15dispatchmessageP7tag_msg
	jmp	L13
L12:
	leave
	ret
	.align 2
.globl __Z13init_msg_testv
	.def	__Z13init_msg_testv;	.scl	2;	.type	32;	.endef
__Z13init_msg_testv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$40, %esp
/APP
	cli
/NO_APP
	movl	$3, -4(%ebp)
	movl	$4, -8(%ebp)
	movl	-4(%ebp), %eax
	imull	$4272, %eax, %eax
	movl	$1, _process_table(%eax)
	movl	$__Z10msg_task_1v, 8(%esp)
	movl	-4(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$_process_table, (%esp)
	call	__Z8init_tssP11task_structii
	movl	-4(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$_process_table, (%esp)
	call	__Z8init_ldtP11task_structi
	movl	$49289, 16(%esp)
	movl	$1048575, 12(%esp)
	movl	-4(%ebp), %eax
	imull	$4272, %eax, %eax
	addl	$_process_table+4, %eax
	movl	%eax, 8(%esp)
	movl	$11, 4(%esp)
	movl	_gdt_addr, %eax
	movl	%eax, (%esp)
	call	__Z17modify_descriptorP11desc_structimmt
	movl	$49282, 16(%esp)
	movl	$1048575, 12(%esp)
	movl	-4(%ebp), %eax
	imull	$4272, %eax, %eax
	addl	$_process_table+112, %eax
	movl	%eax, 8(%esp)
	movl	$12, 4(%esp)
	movl	_gdt_addr, %eax
	movl	%eax, (%esp)
	call	__Z17modify_descriptorP11desc_structimmt
	movl	-4(%ebp), %eax
	imull	$4272, %eax, %eax
	movl	$2, _process_table(%eax)
	movl	$__Z10msg_task_2v, 8(%esp)
	movl	-8(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$_process_table, (%esp)
	call	__Z8init_tssP11task_structii
	movl	-8(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$_process_table, (%esp)
	call	__Z8init_ldtP11task_structi
	movl	$49289, 16(%esp)
	movl	$1048575, 12(%esp)
	movl	-8(%ebp), %eax
	imull	$4272, %eax, %eax
	addl	$_process_table+4, %eax
	movl	%eax, 8(%esp)
	movl	$13, 4(%esp)
	movl	_gdt_addr, %eax
	movl	%eax, (%esp)
	call	__Z17modify_descriptorP11desc_structimmt
	movl	$49282, 16(%esp)
	movl	$1048575, 12(%esp)
	movl	-8(%ebp), %eax
	imull	$4272, %eax, %eax
	addl	$_process_table+112, %eax
	movl	%eax, 8(%esp)
	movl	$14, 4(%esp)
	movl	_gdt_addr, %eax
	movl	%eax, (%esp)
	call	__Z17modify_descriptorP11desc_structimmt
	movl	$1, _state+16
	movl	_state+16, %eax
	movl	%eax, _state+12
	movl	$5, _my_task_no
/APP
	sti
/NO_APP
	leave
	ret
	.def	__Z17modify_descriptorP11desc_structimmt;	.scl	3;	.type	32;	.endef
	.def	__Z8init_ldtP11task_structi;	.scl	3;	.type	32;	.endef
	.def	__Z8init_tssP11task_structii;	.scl	3;	.type	32;	.endef
	.def	__Z15dispatchmessageP7tag_msg;	.scl	3;	.type	32;	.endef
	.def	__Z11get_messageP7tag_msg;	.scl	3;	.type	32;	.endef
	.def	__Z16create_window_gci;	.scl	3;	.type	32;	.endef
	.def	__Z16registerclass_gcP11wndclass_gc;	.scl	3;	.type	32;	.endef
	.def	__Z9note_bookiiiiiii;	.scl	3;	.type	32;	.endef
	.def	__Z10disp_coloriiiii;	.scl	3;	.type	32;	.endef
