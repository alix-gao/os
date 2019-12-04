	.file	"console.cpp"
.globl _mus_bar_add_1
	.data
	.align 4
_mus_bar_add_1:
	.long	4
.globl _mus_bar_add_2
	.align 4
_mus_bar_add_2:
	.long	73
	.text
	.align 2
.globl __Z10move_curseii
	.def	__Z10move_curseii;	.scl	2;	.type	32;	.endef
__Z10move_curseii:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$4, %esp
	movl	8(%ebp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	sall	$4, %eax
	addl	12(%ebp), %eax
	movl	%eax, -4(%ebp)
	movl	$15, %eax
	movl	$980, %edx
/APP
	outb %al,%dx
/NO_APP
	movzbl	-4(%ebp),%eax
	movl	$981, %edx
/APP
	outb %al,%dx
/NO_APP
	movl	$14, %eax
	movl	$980, %edx
/APP
	outb %al,%dx
/NO_APP
	movl	-4(%ebp), %eax
	sarl	$8, %eax
	andl	$255, %eax
	movl	$981, %edx
/APP
	outb %al,%dx
/NO_APP
	leave
	ret
	.align 2
.globl __Z9print_intiiii
	.def	__Z9print_intiiii;	.scl	2;	.type	32;	.endef
__Z9print_intiiii:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$20, %esp
	movl	$1, -8(%ebp)
	movl	16(%ebp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	sall	$4, %eax
	addl	20(%ebp), %eax
	addl	%eax, %eax
	addl	$753664, %eax
	movl	%eax, -16(%ebp)
	movl	$10000, -4(%ebp)
L3:
	cmpl	$0, -4(%ebp)
	jle	L4
	movl	8(%ebp), %edx
	leal	-4(%ebp), %eax
	movl	%eax, -20(%ebp)
	movl	%edx, %eax
	movl	-20(%ebp), %ecx
	cltd
	idivl	(%ecx)
	movl	%eax, -20(%ebp)
	movzbl	-20(%ebp), %eax
	addb	$48, %al
	movb	%al, -9(%ebp)
	cmpb	$48, -9(%ebp)
	jne	L7
	cmpl	$0, -8(%ebp)
	jne	L5
L7:
	movl	$0, -8(%ebp)
	movsbl	-9(%ebp),%eax
	subl	$48, %eax
	imull	-4(%ebp), %eax
	subl	%eax, 8(%ebp)
	movl	-16(%ebp), %edx
	movzbl	-9(%ebp), %eax
	movb	%al, (%edx)
	leal	-16(%ebp), %eax
	incl	(%eax)
	movl	-16(%ebp), %edx
	movl	12(%ebp), %eax
	movb	%al, (%edx)
	leal	-16(%ebp), %eax
	incl	(%eax)
L5:
	movl	-4(%ebp), %ecx
	movl	$1717986919, %eax
	imull	%ecx
	sarl	$2, %edx
	movl	%ecx, %eax
	sarl	$31, %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -4(%ebp)
	jmp	L3
L4:
	cmpl	$0, -8(%ebp)
	je	L2
	movl	-16(%ebp), %eax
	movb	$48, (%eax)
	leal	-16(%ebp), %eax
	incl	(%eax)
	movl	-16(%ebp), %edx
	movl	12(%ebp), %eax
	movb	%al, (%edx)
	leal	-16(%ebp), %eax
	incl	(%eax)
L2:
	leave
	ret
	.align 2
.globl __Z19print_string_partlyPciiii
	.def	__Z19print_string_partlyPciiii;	.scl	2;	.type	32;	.endef
__Z19print_string_partlyPciiii:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	20(%ebp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	sall	$4, %eax
	addl	24(%ebp), %eax
	addl	%eax, %eax
	addl	$753664, %eax
	movl	%eax, -4(%ebp)
	movl	$0, -8(%ebp)
L10:
	movl	-8(%ebp), %eax
	cmpl	12(%ebp), %eax
	jge	L9
	movl	-4(%ebp), %edx
	movl	8(%ebp), %eax
	movzbl	(%eax), %eax
	movb	%al, (%edx)
	leal	-4(%ebp), %eax
	incl	(%eax)
	incl	8(%ebp)
	movl	-4(%ebp), %edx
	movl	16(%ebp), %eax
	movb	%al, (%edx)
	leal	-4(%ebp), %eax
	incl	(%eax)
	leal	-8(%ebp), %eax
	incl	(%eax)
	jmp	L10
L9:
	leave
	ret
	.align 2
.globl __Z12print_stringPciii
	.def	__Z12print_stringPciii;	.scl	2;	.type	32;	.endef
__Z12print_stringPciii:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$4, %esp
	movl	16(%ebp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	sall	$4, %eax
	addl	20(%ebp), %eax
	addl	%eax, %eax
	addl	$753664, %eax
	movl	%eax, -4(%ebp)
L14:
	movl	8(%ebp), %eax
	cmpb	$0, (%eax)
	je	L13
	movl	-4(%ebp), %edx
	movl	8(%ebp), %eax
	movzbl	(%eax), %eax
	movb	%al, (%edx)
	leal	-4(%ebp), %eax
	incl	(%eax)
	incl	8(%ebp)
	movl	-4(%ebp), %edx
	movl	12(%ebp), %eax
	movb	%al, (%edx)
	leal	-4(%ebp), %eax
	incl	(%eax)
	jmp	L14
L13:
	leave
	ret
	.align 2
.globl __Z10print_charciii
	.def	__Z10print_charciii;	.scl	2;	.type	32;	.endef
__Z10print_charciii:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	movb	%al, -1(%ebp)
	movl	16(%ebp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	sall	$4, %eax
	addl	20(%ebp), %eax
	addl	%eax, %eax
	addl	$753664, %eax
	movl	%eax, -8(%ebp)
	movl	-8(%ebp), %edx
	movzbl	-1(%ebp), %eax
	movb	%al, (%edx)
	leal	-8(%ebp), %eax
	incl	(%eax)
	movl	-8(%ebp), %edx
	movl	12(%ebp), %eax
	movb	%al, (%edx)
	leal	-8(%ebp), %eax
	incl	(%eax)
	leave
	ret
	.align 2
.globl __Z11read_screenii
	.def	__Z11read_screenii;	.scl	2;	.type	32;	.endef
__Z11read_screenii:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$4, %esp
	movl	8(%ebp), %edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	sall	$4, %eax
	addl	12(%ebp), %eax
	addl	%eax, %eax
	addl	$753664, %eax
	movl	%eax, -4(%ebp)
	movl	-4(%ebp), %eax
	movsbl	(%eax),%eax
	leave
	ret
	.align 2
.globl __Z10save_curseii
	.def	__Z10save_curseii;	.scl	2;	.type	32;	.endef
__Z10save_curseii:
	pushl	%ebp
	movl	%esp, %ebp
	movl	_curse_x, %edx
	movl	8(%ebp), %eax
	movb	%al, (%edx)
	movl	_curse_y, %edx
	movl	12(%ebp), %eax
	movb	%al, (%edx)
	popl	%ebp
	ret
	.section .rdata,"dr"
LC0:
	.ascii "  \0"
	.text
	.align 2
.globl __Z11cls_mus_barv
	.def	__Z11cls_mus_barv;	.scl	2;	.type	32;	.endef
__Z11cls_mus_barv:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$20, %esp
	movl	$0, -4(%ebp)
L20:
	cmpl	$9, -4(%ebp)
	jg	L19
	movl	_mus_bar_add_1, %eax
	movl	%eax, 12(%esp)
	movl	-4(%ebp), %eax
	incl	%eax
	movl	%eax, 8(%esp)
	movl	$15, 4(%esp)
	movl	$LC0, (%esp)
	call	__Z12print_stringPciii
	movl	_mus_bar_add_2, %eax
	movl	%eax, 12(%esp)
	movl	-4(%ebp), %eax
	incl	%eax
	movl	%eax, 8(%esp)
	movl	$15, 4(%esp)
	movl	$LC0, (%esp)
	call	__Z12print_stringPciii
	leal	-4(%ebp), %eax
	incl	(%eax)
	jmp	L20
L19:
	leave
	ret
	.section .rdata,"dr"
LC1:
	.ascii "==\0"
	.text
	.align 2
.globl __Z12disp_mus_barj
	.def	__Z12disp_mus_barj;	.scl	2;	.type	32;	.endef
__Z12disp_mus_barj:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$20, %esp
	movl	$0, -4(%ebp)
L24:
	movl	-4(%ebp), %eax
	cmpl	8(%ebp), %eax
	jae	L23
	movl	_mus_bar_add_1, %eax
	movl	%eax, 12(%esp)
	movl	$10, %eax
	subl	-4(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	$2, 4(%esp)
	movl	$LC1, (%esp)
	call	__Z12print_stringPciii
	movl	_mus_bar_add_2, %eax
	movl	%eax, 12(%esp)
	movl	$10, %eax
	subl	-4(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	$3, 4(%esp)
	movl	$LC1, (%esp)
	call	__Z12print_stringPciii
	leal	-4(%ebp), %eax
	incl	(%eax)
	jmp	L24
L23:
	leave
	ret
	.align 2
.globl __Z10time_delayi
	.def	__Z10time_delayi;	.scl	2;	.type	32;	.endef
__Z10time_delayi:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	movl	$0, -8(%ebp)
L28:
	movl	8(%ebp), %eax
	movl	%eax, %edx
	sall	$2, %edx
	addl	%eax, %edx
	movl	%edx, %eax
	sall	$5, %eax
	addl	%eax, %edx
	cmpl	-8(%ebp), %edx
	jbe	L27
	movl	-8(%ebp), %edx
	movl	$-30194921, %eax
	mull	%edx
	movl	%edx, %eax
	shrl	$12, %eax
	addl	%eax, %eax
	addl	$2, %eax
	movl	%eax, (%esp)
	call	__Z12disp_mus_barj
	movl	$97, %edx
/APP
	inb %dx,%al
/NO_APP
	movb	%al, -1(%ebp)
	leal	-1(%ebp), %eax
	andb	$16, (%eax)
	movzbl	-1(%ebp), %eax
	movb	%al, -2(%ebp)
L31:
	movl	$97, %edx
/APP
	inb %dx,%al
/NO_APP
	movb	%al, -1(%ebp)
	leal	-1(%ebp), %eax
	andb	$16, (%eax)
	movzbl	-2(%ebp), %eax
	cmpb	-1(%ebp), %al
	je	L31
	leal	-8(%ebp), %eax
	incl	(%eax)
	jmp	L28
L27:
	leave
	ret
	.align 2
.globl __Z4ringic
	.def	__Z4ringic;	.scl	2;	.type	32;	.endef
__Z4ringic:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	12(%ebp), %eax
	movb	%al, -1(%ebp)
	movl	$182, %eax
	movl	$67, %edx
/APP
	outb %al,%dx
/NO_APP
	movl	$1193100, %eax
	cltd
	idivl	8(%ebp)
	movl	%eax, -8(%ebp)
	movzbl	-8(%ebp),%eax
	movl	$66, %edx
/APP
	outb %al,%dx
/NO_APP
	call	__Z8io_delayv
	movl	-8(%ebp), %eax
	sarl	$8, %eax
	andl	$255, %eax
	movl	$66, %edx
/APP
	outb %al,%dx
/NO_APP
	movl	$97, %edx
/APP
	inb %dx,%al
/NO_APP
	movb	%al, -3(%ebp)
	movzbl	-3(%ebp), %eax
	movb	%al, -2(%ebp)
	movzbl	-3(%ebp), %eax
	orb	$3, %al
	movzbl	%al, %eax
	movl	$97, %edx
/APP
	outb %al,%dx
/NO_APP
	movsbl	-1(%ebp),%eax
	movl	%eax, (%esp)
	call	__Z10time_delayi
	movzbl	-2(%ebp), %eax
	movl	$97, %edx
/APP
	outb %al,%dx
/NO_APP
	leave
	ret
	.align 2
.globl __Z13music_combinecc
	.def	__Z13music_combinecc;	.scl	2;	.type	32;	.endef
__Z13music_combinecc:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	movl	12(%ebp), %edx
	movb	%al, -1(%ebp)
	movb	%dl, -2(%ebp)
	movsbl	-1(%ebp),%edx
	movl	%edx, %eax
	sall	$2, %eax
	addl	%edx, %eax
	leal	0(,%eax,4), %edx
	addl	%edx, %eax
	leal	0(,%eax,4), %edx
	movsbl	-2(%ebp),%eax
	leal	(%edx,%eax), %eax
	movl	%eax, -8(%ebp)
	movl	-8(%ebp), %eax
	leave
	ret
	.align 2
.globl __Z10play_musicPcS_
	.def	__Z10play_musicPcS_;	.scl	2;	.type	32;	.endef
__Z10play_musicPcS_:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	$0, -8(%ebp)
	movl	$0, -4(%ebp)
	movl	-4(%ebp), %eax
	addl	8(%ebp), %eax
	incl	%eax
	movsbl	(%eax),%eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	addl	-4(%ebp), %eax
	movsbl	(%eax),%eax
	movl	%eax, (%esp)
	call	__Z13music_combinecc
	movl	%eax, -12(%ebp)
L37:
	cmpl	$0, -12(%ebp)
	je	L36
	movl	-8(%ebp), %eax
	addl	12(%ebp), %eax
	movsbl	(%eax),%eax
	movl	%eax, 4(%esp)
	leal	-8(%ebp), %eax
	incl	(%eax)
	movl	-12(%ebp), %eax
	movl	%eax, (%esp)
	call	__Z4ringic
	leal	-4(%ebp), %eax
	addl	$2, (%eax)
	movl	-4(%ebp), %eax
	addl	8(%ebp), %eax
	incl	%eax
	movsbl	(%eax),%eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	addl	-4(%ebp), %eax
	movsbl	(%eax),%eax
	movl	%eax, (%esp)
	call	__Z13music_combinecc
	movl	%eax, -12(%ebp)
	call	__Z11cls_mus_barv
	jmp	L37
L36:
	leave
	ret
	.align 2
.globl __Z17key_place_controlii
	.def	__Z17key_place_controlii;	.scl	2;	.type	32;	.endef
__Z17key_place_controlii:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$28, %esp
	movl	_curse_y, %eax
	movsbl	(%eax),%edx
	movl	_text_frame_l, %eax
	addl	_text_frame_y, %eax
	cmpl	%eax, %edx
	jle	L40
	movl	_curse_x, %eax
	movsbl	(%eax),%eax
	subl	_text_frame_x, %eax
	addl	$2, %eax
	cmpl	_text_frame_h, %eax
	jle	L41
	movl	_text_frame_x, %eax
	addl	$2, %eax
	movl	%eax, -4(%ebp)
L42:
	movl	_text_frame_h, %eax
	addl	_text_frame_x, %eax
	cmpl	-4(%ebp), %eax
	jle	L43
	movl	_text_frame_y, %eax
	incl	%eax
	movl	%eax, -8(%ebp)
L45:
	movl	_text_frame_l, %eax
	addl	_text_frame_y, %eax
	incl	%eax
	cmpl	-8(%ebp), %eax
	jle	L44
	movl	-8(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	-4(%ebp), %eax
	movl	%eax, (%esp)
	call	__Z11read_screenii
	movb	%al, -9(%ebp)
	movl	-8(%ebp), %eax
	movl	%eax, 12(%esp)
	movl	-4(%ebp), %eax
	decl	%eax
	movl	%eax, 8(%esp)
	movl	$15, 4(%esp)
	movsbl	-9(%ebp),%eax
	movl	%eax, (%esp)
	call	__Z10print_charciii
	leal	-8(%ebp), %eax
	incl	(%eax)
	jmp	L45
L44:
	leal	-4(%ebp), %eax
	incl	(%eax)
	jmp	L42
L43:
	movl	_text_frame_y, %eax
	incl	%eax
	movl	%eax, -8(%ebp)
L48:
	movl	_text_frame_l, %eax
	addl	_text_frame_y, %eax
	incl	%eax
	cmpl	-8(%ebp), %eax
	jle	L49
	movl	-8(%ebp), %eax
	movl	%eax, 12(%esp)
	movl	_text_frame_h, %eax
	addl	_text_frame_x, %eax
	decl	%eax
	movl	%eax, 8(%esp)
	movl	$15, 4(%esp)
	movl	$32, (%esp)
	call	__Z10print_charciii
	leal	-8(%ebp), %eax
	incl	(%eax)
	jmp	L48
L49:
	movl	_curse_x, %eax
	movl	_curse_x, %edx
	movzbl	(%edx), %edx
	decb	%dl
	movb	%dl, (%eax)
L41:
	movl	_curse_x, %edx
	movl	_curse_x, %eax
	movzbl	(%eax), %eax
	incb	%al
	movb	%al, (%edx)
	movl	_curse_y, %edx
	movl	_text_frame_y, %eax
	incb	%al
	movb	%al, (%edx)
	cmpl	$0, 12(%ebp)
	je	L51
	movl	_curse_y, %eax
	movsbl	(%eax),%eax
	movl	%eax, 12(%esp)
	movl	_curse_x, %eax
	movsbl	(%eax),%eax
	movl	%eax, 8(%esp)
	movl	$15, 4(%esp)
	movl	8(%ebp), %eax
	movsbl	%al,%eax
	movl	%eax, (%esp)
	call	__Z10print_charciii
	jmp	L52
L51:
	movl	_curse_y, %eax
	movsbl	(%eax),%eax
	movl	%eax, 12(%esp)
	movl	_curse_x, %eax
	movsbl	(%eax),%eax
	movl	%eax, 8(%esp)
	movl	$15, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	__Z9print_intiiii
L52:
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
	jmp	L39
L40:
	cmpl	$0, 12(%ebp)
	je	L54
	movl	_curse_y, %eax
	movsbl	(%eax),%eax
	movl	%eax, 12(%esp)
	movl	_curse_x, %eax
	movsbl	(%eax),%eax
	movl	%eax, 8(%esp)
	movl	$15, 4(%esp)
	movl	8(%ebp), %eax
	movsbl	%al,%eax
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
	jmp	L39
L54:
	movl	_curse_y, %eax
	movsbl	(%eax),%eax
	movl	%eax, 12(%esp)
	movl	_curse_x, %eax
	movsbl	(%eax),%eax
	movl	%eax, 8(%esp)
	movl	$15, 4(%esp)
	movl	8(%ebp), %eax
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
L39:
	leave
	ret
	.align 2
.globl __Z12clean_screenii
	.def	__Z12clean_screenii;	.scl	2;	.type	32;	.endef
__Z12clean_screenii:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	8(%ebp), %eax
	movl	%eax, -4(%ebp)
L57:
	movl	-4(%ebp), %eax
	cmpl	12(%ebp), %eax
	jge	L56
	movl	$0, -8(%ebp)
L60:
	cmpl	$79, -8(%ebp)
	jg	L59
	movl	-8(%ebp), %eax
	movl	%eax, 12(%esp)
	movl	-4(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	$15, 4(%esp)
	movl	$32, (%esp)
	call	__Z10print_charciii
	leal	-8(%ebp), %eax
	incl	(%eax)
	jmp	L60
L59:
	leal	-4(%ebp), %eax
	incl	(%eax)
	jmp	L57
L56:
	leave
	ret
	.def	__Z8io_delayv;	.scl	3;	.type	32;	.endef
