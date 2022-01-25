#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  //I'm lazy !!!(理直气壮,手动叉腰)
  extern pte_t *walk(pagetable_t pagetable, uint64 va, int alloc);

  // lab pgtbl: your code here.
  uint64 va;
  int npages;
  uint64 dst;
  unsigned int bitmask;
  pagetable_t pagetable;

  if(argaddr(0, &va) < 0)
    return -1;
  if(argint(1, &npages) < 0)
    return -1;
  if(argaddr(2, &dst) < 0)
    return -1;
  bitmask = 0;
  pagetable = myproc()->pagetable;

  for(int i=0; i<npages && i<(sizeof(bitmask)<<3); ++i){//prevent bitmask overflows
    pte_t *pte = walk(pagetable, va, 0);
    if(pte == 0)
      break;
    if(*pte & PTE_A){
      bitmask |= (1U << i);
      *pte = (*pte ^ PTE_A);
    }
    va += PGSIZE;
  }
  if(copyout(pagetable, dst, (char*)&bitmask, sizeof(bitmask)) < 0)
    return -1;
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
