#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)

# 
# syscall.h에 정의된 번호를 이용하여 syscall을 호출하도록 구현
# setnice -> 위 SYSCALL에 정의된 매크로를 통해 movl $SYS_setnice %eax -> movl $22 %eax가 됨
# trap은 항상 T_SYSCALL(64)로 발생하고, syscall.c에서 eax를 확인하여 어떤 syscall인지 확인 후 처리 
SYSCALL(setnice)
SYSCALL(getnice)
SYSCALL(ps)