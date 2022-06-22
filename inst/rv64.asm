macro setrv64 %0, %1
{
	match %0, 'b'
	\{
		fp equ bpl
		sp equ spl
		
		a0 equ al
		a1 equ bl
		a2 equ cl
		a3 equ dl
		a4 equ dil
		a5 equ sil
		
		t0 equ r8b
		t1 equ r9b
		t2 equ r10b
		t3 equ r11b
		t4 equ r12b
		t5 equ r13b
		t6 equ r14b
		t7 equ r15b
	\}
	match %0, 'w'
	\{
		fp equ bp
		
		a0 equ ax
		a1 equ bx
		a2 equ cx
		a3 equ dx
		a4 equ di
		a5 equ si
		
		t0 equ r8w
		t1 equ r9w
		t2 equ r10w
		t3 equ r11w
		t4 equ r12w
		t5 equ r13w
		t6 equ r14w
		t7 equ r15w
	\}
	
	match %0, 'd'
	\{
		sp equ esp
		fp equ ebp
		
		a0 equ eax
		a1 equ ebx
		a2 equ ecx
		a3 equ edx
		a4 equ edi
		a5 equ esi
		
		t0 equ r8d
		t1 equ r9d
		t2 equ r10d
		t3 equ r11d
		t4 equ r12d
		t5 equ r13d
		t6 equ r14d
		t7 equ r15d
	\}
	
	match %0, 'q'
	\{
		sp equ rsp
		fp equ rbp
		
		a0 equ rax
		a1 equ rbx
		a2 equ rcx
		a3 equ rdx
		a4 equ rdi
		a5 equ rsi
		
		t0 equ r8
		t1 equ r9
		t2 equ r10
		t3 equ r11
		t4 equ r12
		t5 equ r13
		t6 equ r14
		t7 equ r15
	\}
}
macro resetrv64
{
	sp equ sp
	fp equ fp
	
	a0 equ a0
	a1 equ a1
	a2 equ a2
	a3 equ a3
	
	t0 equ t0
	t1 equ t1
	t2 equ t2
	t3 equ t3
	t4 equ t4
	t5 equ t5
	t6 equ t6
	t7 equ t7
}

;Load
macro lb %0, %1
{
	setrv64 'b', %0
	mov %0, %1
	resetrv64
}
macro lw %0, %1
{
	setrv64 'w'
	mov %0, %1
	resetrv64
}
macro ld %0, %1
{
	setrv64 'd'
	mov %0, %1
	resetrv64
}
macro ld %0, %1
{
	setrv64 'd'
	mov %0, %1
	resetrv64
}
macro li %0, %1
{
	setrv64 'q'
	mov %0, %1
	resetrv64
}
macro la %0, %1
{
	setrv64 'q'
	lea %0, [ %1 ]
	resetrv64
}

;Store
macro sb %0, %1
{
	setrv64 'b'
	mov %1, %0
	resetrv64
}
macro sw %0, %1
{
	setrv64 'w'
	mov %1, %0
	resetrv64
}
macro sd %0, %1
{
	setrv64 'd'
	mov %1, %0
	resetrv64
}
macro sq %0, %1
{
	setrv64 'q'
	mov %1, %0
	resetrv64
}