.386p
        ifndef        ??version
?debug  macro
        endm
        endif
        ?debug        S "pm.c"
PM_TEXT segment       use16 byte public 'CODE'
DGROUP  group         _DATA,_BSS
        assume        cs:PM_TEXT,ds:DGROUP
PM_TEXT ends
_DATA   segment       use16 word public 'DATA'
d@      label         byte
d@w     label         word
_DATA   ends
_BSS    segment       use16 word public 'BSS'
b@      label         byte
b@w     label         word
        ?debug        C E92204423B066D61696E2E63
_BSS    ends
_DATA   segment       use16 word public 'DATA'
        align 4
        gdt     dd 0,0
	code16  dw 0ffffH
                dw 0000H
                db 00H
                dw 009aH
                db 00H
        data16  dw 0ffffH
                dw 0000H
                db 00H
                dw 0092H
                db 00H
        code32  dw 0ffffH
                dw 0000H
                db 00H
                dw 0cf9aH
                db 00H
        data32  dw 0ffffH
                dw 0000H
                db 00H
                dw 0cf92H
                db 00H

                        dw 0000H
        gdt_pseudo      dw 256*8-1
                        dd offset gdt
                        dw 0000H
        idt_pseudo      dw 0000H
                        dd 00000000H
_DATA   ends
PM_TEXT segment use16 byte public 'CODE'
;       debug   L 2
_get_4g_gs proc far
        push ds
        cli
        mov ax,seg gdt_pseudo
        mov ds,ax
        mov di,offset gdt_pseudo
        lgdt qword ptr ds:[di]
        mov eax,cr0
        or eax,1
        mov cr0,eax
        db 0e9H,00H,00H
        mov ax,32
        mov gs,ax
        cli
        mov eax,cr0
        and eax,0fffffffeH
        mov cr0,eax
        db 0e9H,00H,00H
        xor ax,ax
        mov gs,ax
        pop ds
        ret
_get_4g_gs endp

_move_1m proc far
        push bp
        mov bp,sp
	call _get_4g_gs
        mov esi,[bp+2+4]
        mov edi,[bp+2+8]
        mov ecx,[bp+2+12]
      mov_1m_loop:
        mov al,gs:[esi]
        add esi,1
        mov gs:[edi],al
        add edi,1
        sub ecx,1
        jnz mov_1m_loop
        pop bp
        ret
_move_1m endp

_goto_asm32 proc far
        push ds
        cli
        mov ax,seg gdt_pseudo
        mov ds,ax
        mov di,offset gdt_pseudo
        lgdt qword ptr ds:[di]
        mov eax,cr0
        or eax,1
        mov cr0,eax
        jmp short clear_instruction
      clear_instruction:
        db 66H,0eaH
        dd 132000H
        dw 18H
        pop ds
        ret
_goto_asm32 endp

PM_TEXT ends
        ?debug        C E9
_DATA   segment use16 word public 'DATA'
s@      label        byte
_DATA   ends
PM_TEXT segment use16 byte public 'CODE'
PM_TEXT ends
        public       _get_4g_gs
        public       _move_1m
        public       _goto_asm32
        end

