    .global sti
sti:
    pushf
    pop %eax
    sti
    ret

    .global cli
cli:
    pushf
    pop %eax
    cli
    ret

    .global restore
restore:
    mov 4(%esp),%eax
    push %eax
    popf
    ret

    # outb(int port, int val)
    .global outb
outb:
    mov 4(%esp),%dx
    mov 8(%esp),%al
    outb %al,%dx
    ret

    # outl(int port, int val)
    .global outl
outl:
    mov 4(%esp),%dx
    mov 8(%esp),%eax
    outl %eax,%dx
    ret

    # int inb(int port)
    .global inb
inb:
    mov 4(%esp),%dx
    inb %dx,%al
    and $0xff,%eax
    ret

    # unsigned long inb(int port)
    .global inl
inl:
    mov 4(%esp),%dx
    inl %dx,%eax
    ret

    .globl rdmsr
    # uint64_t rdmsr(uint32_t id)
rdmsr:
    mov 4(%esp), %ecx
    rdmsr
    ret

    .globl wrmsr
    # wrmsr (uint32_t id, uint64_t value)
wrmsr:
    mov 4(%esp), %ecx
    mov 8(%esp), %eax
    mov 12(%esp), %edx
    wrmsr
    ret

    /* vmm_on(uint32_t pd) */
    .global vmm_on
vmm_on:
    mov 4(%esp),%eax
    mov %eax,%cr3

    mov %cr0,%eax
    or $0x80000000,%eax
    mov %eax,%cr0
    ret

    /* invlpg(uint32_t va) */
    .global invlpg
invlpg:
    mov 4(%esp),%eax
    invlpg (%eax)
    ret

    .global spuriousHandler_
spuriousHandler_:
    iret



    /* memcpy(void* dest, void* src, size_t n) */
    .global memcpy
memcpy:
    mov 4(%esp),%eax       # dest
    mov 8(%esp),%edx       # src
    mov 12(%esp),%ecx      # n
    push %ebx
1:
    add $-1,%ecx
    jl 1f
    movb (%edx),%bl
    movb %bl,(%eax)
    add $1,%edx
    add $1,%eax
    jmp 1b
1:
    pop %ebx
    ret

    /* memset(void* dest, int val, size_t n) */
    .global memset
memset:
    mov 4(%esp),%eax       # dest
    mov 8(%esp),%edx       # val
    mov 12(%esp),%ecx      # n
1:
    add $-1,%ecx
    jl 1f
    movb %dl,(%eax)
    add $1,%eax
    jmp 1b
1:
    ret

    /* bzero(void* dest, size_t n) */
    .global bzero
bzero:
    mov 4(%esp),%eax       # dest
    mov 8(%esp),%ecx      # n
1:
    add $-1,%ecx
    jl 1f
    movb $0,(%eax)
    add $1,%eax
    jmp 1b
1:
    ret

# ltr(uint32_t tr)
    .global ltr
ltr:
    mov 4(%esp),%eax
    ltr %ax
    ret

# switchToUser(pc,esp,eax)
        .global switchToUser
switchToUser:
        mov 4(%esp),%ecx        # pc
        mov 8(%esp),%ebx        # esp
        mov 12(%esp),%eax       # eax

        pushl userSS            # user SS
        pushl %ebx              # user ESP

        pushf                   # ring 3 flags
        orl $0x200,(%esp)       # enable interrupts

        pushl userCS            # user CS
        pushl %ecx              # user EIP

        iret

