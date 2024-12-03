global _main
_main:
        ; save the stack frame
        ; dlopen needs it in its '__builtin_return_address' call
        push    rbp
        mov     rbp, rsp

        mov     rax, rbx
        push    rbx

        ; rbx = dyld::vtable
        mov     rax, [rax]
        mov     rax, [rax + 0x70]

        pop     rbx
        mov     rdi, rbx
        lea     rsi, [rel __lib]
        mov     rdx, 0x2

        pop     rbp
        jmp     rax

__lib:
        db "./lib.dylib", 0x00

; nasm -f macho64 ./dl.s
; otool -tvVj ./dl.o
