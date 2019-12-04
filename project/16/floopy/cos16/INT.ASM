        ifndef version
debug   macro
        endm
        endif
        debug S "int.c"
INT_TEXT  segment byte public 'CODE'
DGROUP  group   _DATA,_BSS
        assume  cs:INT_TEXT,ds:DGROUP
INT_TEXT  ends
_DATA   segment word public 'DATA'
d@      label   byte
d@w     label   word
_DATA   ends
_BSS    segment word public 'BSS'
b@      label   byte
b@w     label   word
        debug   C E91497CA3803692E63
_BSS    ends
INT_TEXT  segment byte public 'CODE'
;       debug   L 2
_time_int       proc    far
        mov     DGROUP:_temp_reg_sp,sp
        mov     DGROUP:_temp_reg_ss,ss
	;point to old process
	mov      sp,word ptr DGROUP:_sche_context
        mov      ss,word ptr DGROUP:_sche_context+2
	add      sp,22 ;22 is size of task_struct.regs
        ;save context
        push     DGROUP:_temp_reg_sp
        push     DGROUP:_temp_reg_ss
        push     ax
        push     cx
        push     dx
        push     bx
        push     bp
        push     si
        push     di
        push     ds
        push     es
        ;int_stack
        mov      ss,word ptr DGROUP:_temp_reg_sp
        mov      sp,word ptr DGROUP:_temp_reg_sp
        ;scheule
        call     far ptr _time_int_func
        ;point to new pt.reg
	;mov sp,word ptr ds:0x63ae
	;mov ss,word ptr ds:0x63b0
        mov      sp,word ptr DGROUP:_sche_context
        mov      ss,word ptr DGROUP:_sche_context+2
        ;fresh
        pop      es
        pop      ds
        pop      di
        pop      si
        pop      bp
        pop      bx
        pop      dx
        pop      cx
        pop      ax
        pop      DGROUP:_temp_reg_ss
	pop      DGROUP:_temp_reg_sp

	;for support different segment
	mov      _temp_reg,ax
	mov      ax,word ptr DGROUP:_temp_reg_ss
	mov      ss,ax
	mov      ax,word ptr DGROUP:_temp_reg_sp
        mov      sp,ax
	mov      ax,_temp_reg

        ;send eoi
        mov      al,020H
        mov      dx,020H
        out      dx,al

        iret
_time_int       endp

_keyboard_int   proc    far
        call    far ptr _keyboard_int_func
        ;send eoi
        mov      al,020H
        mov      dx,020H
        out      dx,al
        iret
_keyboard_int   endp
INT_TEXT  ends
        debug  C E9
_DATA   segment word public 'DATA'
s@      label  byte
_DATA   ends
        extrn   _time_int_func
        extrn   _temp_reg_sp
        extrn   _temp_reg_ss
        extrn   _temp_reg
	extrn   _sche_context
        extrn   _keyboard_int_func
INT_TEXT  segment byte public 'CODE'
INT_TEXT  ends
        public _time_int
        public _keyboard_int
        end

