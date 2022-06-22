format elf64 executable
segment writeable
align 16
__D0 dq 5.000000
align 16
__D1 dq 1.000000
align 8
hello            db 5    dup ("Hello",'"'," World",10,"")
world            db "World",10
ptr3             rq 1
array            db 10   dup (11,12,13,14,15,16,17,18,19,20)
segment executable
align 16
strlen:
push rbp
mov rbp, rsp
sub rsp, 20
; len                              = rbp - 12
; float                            = rbp - 20
; len = 0
mov [rbp-12], word 0

; 0
mov eax, 0

.__endfn:
add rsp, 20
pop rbp
ret
entry main
main:
push rbp
mov rbp, rsp
sub rsp, 96
; x                                = rbp - 8
; y                                = rbp - 16
; z                                = rbp - 21
; float                            = rbp - 32
; float2                           = rbp - 40
; ptr1                             = rbp - 52
; ptr2                             = rbp - 64
; ptr3                             = rbp - 76
; now                              = rbp - 88
; float + ptr1
pxor xmm0, xmm0
movsd xmm0, qword [rbp-32]
mov rcx, [rbp-52]
pxor xmm1, xmm1
cvtsi2sd xmm1, rcx
addsd xmm0, xmm1

; ptr1 = @ptr2
lea rcx, [rbp-64]
mov [rbp-52], rcx

; ptr2 = @ptr3
lea rcx, [rbp-76]
mov [rbp-64], rcx

; ptr3 = @hello
mov rcx, hello
mov [rbp-76], rcx

; x *= ptr3:1
mov rcx, [rbp-76]
movzx ecx, byte [rcx+1]
imul ecx, [rbp-8]
mov [rbp-8], ecx

; ptr1 + 1
mov rax, [rbp-52]
inc rax

; ptr2~ *= 1
mov rdx, [rbp-64]
imul rax, [rdx], 1
mov [rdx], al

; x *= 0.5
imul eax, [rbp-8], dword 0
mov [rbp-8], eax

; x = 2500
mov [rbp-8], dword 2500

; y = 100
mov [rbp-16], dword 100

; z = 5
mov [rbp-21], byte 5

; hello:1   = 'a'
mov [hello+1], byte 'a'

; ptr3:2    = 'b'
mov rdx, [rbp-76]
mov [rdx+2], byte 'b'

; ptr2~:3   = 'c'
mov rdx, [rbp-64]
mov rdx, [rdx]
mov [rdx+3], byte 'c'

; ptr1~~:4  = 'd'
mov rdx, [rbp-52]
mov rdx, [rdx]
mov rdx, [rdx]
mov [rdx+4], byte 'd'

; ptr1~~:4 -= 32
mov rdx, [rbp-52]
mov rdx, [rdx]
mov rdx, [rdx]
sub [rdx+4], byte 32

; float = 1.8 / (float + z + 5)
movsd [rsp], xmm0

pxor xmm0, xmm0
movsd xmm0, qword [rbp-32]
movzx ecx, byte [rbp-21]
pxor xmm1, xmm1
cvtsi2sd xmm1, ecx
addsd xmm0, xmm1
addsd xmm0, [__D0]
movsd xmm0, [__D1]
divsd xmm0, xmm2
movsd xmm0, xmm2

movsd xmm2, xmm0
movsd xmm0, [rsp]

pxor xmm0, xmm0
movsd [rbp-32], xmm2

; float = x + 5
mov r10d, dword [rbp-8]
add r10d, 5
pxor xmm0, xmm0
pxor xmm2, xmm2
cvtsi2sd xmm2, r10d
movsd [rbp-32], xmm2

.c0_l:
; x /= y
mov ecx, dword [rbp-16]
mov eax, dword [rbp-8]
cdq
idiv ecx
mov [rbp-8], eax

; hello:5 += 1
inc [hello+5]

; x >= 50
mov eax, dword [rbp-8]
cmp eax, 50
setge al
and al, 1
movzx eax, al

cmp eax, 1
jge .c0_l

; x < 0
mov eax, dword [rbp-8]
cmp eax, 0
setl al
and al, 1
movzx eax, al

cmp eax, 0
je .c1_0

; 0
mov eax, 0

jmp .__endfn
.c1_0:
.c1e:

; x >= 5
mov eax, dword [rbp-8]
cmp eax, 5
setge al
and al, 1
movzx eax, al

cmp eax, 0
je .c2_0

mov rsi, hello
mov rdx, 13

jmp .c2e
.c2_0:

;
mov rsi, world
; x > 0
mov eax, dword [rbp-8]
cmp eax, 0
setg al
and al, 1
movzx eax, al

cmp eax, 0
je .c3_0

mov rdx, 5

jmp .c3e
.c3_0:

;
mov rdx, 3

.c3e:


.c2e:

mov rax, 1
mov rdi, 1
syscall
; 0
mov eax, 0

.__endfn:
add rsp, 96
pop rbp
mov rdi, rax
mov rax, 60
syscall


