// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  short ref_cnt[PHYSTOP/PGSIZE];
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    kmem.ref_cnt[(uint64)p/PGSIZE] = 1;
    kfree(p);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&kmem.lock);
  //handle the duplicate allocated
  int ref_cnt = kmem.ref_cnt[(uint64)pa/PGSIZE] - 1;
  if(ref_cnt < 0)
    panic("kfree ref cnt");

  kmem.ref_cnt[(uint64)pa/PGSIZE] = ref_cnt;
  //if still mapped by a pte, return immediately
  if(ref_cnt > 0){
    release(&kmem.lock);
    return;
  }

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    kmem.ref_cnt[(uint64)r/PGSIZE] = 1;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

int get_ref_cnt(void *pa){
  return kmem.ref_cnt[(uint64)pa/PGSIZE];
}

//Duplicate allocate a physical page(increment the reference count)
// which normally should have been returned by a
// call to kalloc(). 
void dup_kalloc(void *pa){
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("dup_kalloc");
  acquire(&kmem.lock);
  const int ref = kmem.ref_cnt[(uint64)pa/PGSIZE];
  //error when physcial page hasn't allocated or ref_cnt overflows 
  if(ref <= 0 || ref == __INT32_MAX__)
    panic("dup_kalloc reference");
  kmem.ref_cnt[(uint64)pa/PGSIZE] = ref + 1;
  release(&kmem.lock);
}