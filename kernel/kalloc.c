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

int list_len = 0, st;
struct run {
  struct run *next;
};

struct run* kmemfl[NPROC];
struct spinlock kmemlock[NPROC];

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  for(int i = 0; i < NPROC; i++){
    char buf[100];
    snprintf(buf, 10,"kmem%d", i);
    initlock(&kmemlock[i], buf);
  }

  st = 1;
  freerange(end, (void*)PHYSTOP);
  st = 0;
  struct run* head = kmem.freelist;
  while(head) list_len++, head = head -> next;
  for(int i = 0; i < NPROC; i++){
    int len = list_len / NPROC;
    kmemfl[i] = kmem.freelist; 
    for(int j = 0; j < len - 1; j++)  kmem.freelist = kmem.freelist -> next;  
    if(i != NPROC - 1){
      struct run* r = kmem.freelist -> next;
      kmem.freelist -> next = 0;
      kmem.freelist = r;
    }    
  }
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
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

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  // acquire(&kmem.lock);
  if(st){
    r->next = kmem.freelist;
    kmem.freelist = r;
  }else{
    push_off();
    int id = cpuid();
    pop_off();
    acquire(&kmemlock[id]);
    // printf("kfree acquire: %d\n", id);
    r -> next = kmemfl[id];
    kmemfl[id] = r;
    release(&kmemlock[id]);
    // printf("kfree release: %d\n", id); 
  }
  // release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  // acquire(&kmem.lock);
  push_off();
  int id = cpuid();
  pop_off();
  acquire(&kmemlock[id]);
  // printf("kalloc acquire: %d\n", id);
  r = kmemfl[id];

  if(r){
    kmemfl[id] = r -> next;
    release(&kmemlock[id]);
  }
  else{ 
    release(&kmemlock[id]);
    // acquire(&kmem.lock);
    for(int i = 0; i < NPROC; i++){
      if(i == id) continue;
      acquire(&kmemlock[i]);
      if(kmemfl[i]){
        r = kmemfl[i];
        kmemfl[i] = r -> next;
        release(&kmemlock[i]);
        break;
      }
      release(&kmemlock[i]);
    }
  }
  /*
   r = kmem.freelist;
   if(r)
   kmem.freelist = r->next;
  */

  // release(&kmem.lock);
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
} 