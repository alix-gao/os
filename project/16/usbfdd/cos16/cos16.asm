        ifndef version
debug   macro
        endm
        endif
        debug S "entry.c"
COS16_TEXT  segment byte public 'CODE'
DGROUP  group   _DATA,_BSS
        assume  cs:COS16_TEXT,ds:DGROUP
COS16_TEXT  ends
_DATA   segment word public 'DATA'
d@      label   byte
d@w     label   word
_DATA   ends
_BSS    segment word public 'BSS'
b@      label   byte
b@w     label   word
        debug   C E91497CA3803692E63
_BSS    ends
COS16_TEXT  segment byte public 'CODE'
;       debug   L 2
_cos16   proc    far
jloc_addr:
        cli
	mov     ax,DGROUP
	mov     ds,ax
	mov     ax,ss
	mov     es,ax
        call    far ptr _main
        ret
_cos16   endp

COS16_TEXT  ends
        debug   C E9
_DATA   segment word public 'DATA'
s@      label   word
_DATA   ends
        extrn   _main:far
COS16_TEXT  segment byte public 'CODE'
COS16_TEXT  ends
        public  _cos16
        end

