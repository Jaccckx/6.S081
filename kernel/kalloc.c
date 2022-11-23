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
  struct run *freelist;
} kmem;

void initialize(uint64);

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
  initialize(0x88000);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
  printf("num: %p\n", (uint64)p / (PGSIZE));
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

  acquire(&kmem.lock);
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
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}


#define MAX_ORDER 11
#define SIZE 11
#define lp struct List*
#define nullptr 0
#define N 1000000
// Size of vector of pairs
int size;

struct List{
	int first;
	int second;
	struct List* next;
	struct List* prev;
};

struct head{
	int sz;
	struct List* prev;
	struct List* next;
};

uint64 page_state[N];
struct head zone[11];
struct List new[N];
int k = 0;

static int log(int v){
  int k = 0;
  while(v)  k++, v /= 2;
  return k;
}
static uint64 pow(int v, int d){
  uint64 res = v;
  while(d)  res *= v, d--;
  return res; 
}
void add_node_list(int index, uint64 first, uint64 second){
	lp pointer = &(new[k++]);
  
	pointer -> first = first, pointer -> second = second;
	if((zone[index]).next != nullptr)
		(zone[index]).next -> prev = pointer;
	pointer -> next = zone[index].next;
	zone[index].next = pointer;  
	return;
}

void pop_node_list(int index){
	if(zone[index].next == nullptr){
    printf("wrong want to free\n");
	}

	zone[index].next = zone[index].next -> next;
	if(zone[index].next != nullptr)	zone[index].next -> prev = nullptr;
	return;
}

void pop_node_list_index(int index, int i){
	if(!i){
		zone[index].next = zone[index].next->next;
		zone[index].next -> prev = nullptr;
	}
	i--;

	lp p = zone[index].next;
	for(int j = 0; j < i; j++) p = p -> next;
	p->next = p -> next -> next;
	if(p->next != nullptr) p -> next -> prev = p;
	return;
}
void pr();



// Map used as hash map to store the starting
// address as key and size of allocated segment
// key as value

void occupy(int first, int second, int line){
	if(page_state[first] != -1){
    printf("alloc wrong, page state is occupied, line:%d\n", line);
	}
	else{
		page_state[first] = second - first;
	}

}

void free(int first, int second, int line){	
	if(page_state[first] == -1){
		printf("alloc wrong, page state is freed, line:%d\n", line);
	}
	else{
		page_state[first] = -1;
	}

}

void initialize(uint64 sz)
{
	memset(page_state, -1, sizeof page_state);
	sz >>= 11;
	uint64 pa_begin = 0;
	for(int i = 0; i < sz; i++){
		add_node_list(10, pa_begin, pa_begin + (1 << 10));
		pa_begin += 1 << 10;
	}
}

void allocate(int sz)
{
	//这里限制 sz 小于4MB
	int n = log(sz) / log(2) ;

	// Block available
	if (zone[n].next != nullptr)
	{
		lp temp = zone[n].next;

		// Remove block from free list
		pop_node_list(n);
	  printf("Memory from %d to %d allocated\n", temp -> first, temp -> second);

		// map starting address with
		// size to make deallocating easy
		occupy(temp -> first, temp -> second, 50);
	}
	else
	{
		int i;

		for(i = n + 1; i < SIZE; i++)
		{
			
			// Find block size greater than request
			if (zone[i].next != nullptr)
				break;
		}


		// If no such block is found
		// i.e., no memory block available
		if (i == SIZE)
		{
      printf("Sorry, failed to allocate memory \n");
		}
		
		// If found
		else
		{
			lp temp = zone[i].next;
			// Remove first block to split it into halves
			pop_node_list(i);
			// free(temp.first, temp.second, 110);
			i--;
			
			for(; i >= n; i--)
			{
				// Divide block into two halves
				add_node_list(i, temp -> first, temp -> first +	(temp -> second - temp -> first) / 2);
				add_node_list(i, temp -> first + (temp -> second - temp -> first + 1) / 2, temp -> second);

				// printf("L:%d mid:%d, R:%d, len:%d\n", temp -> first, temp -> first + (temp -> second - temp -> first) / 2, temp -> second
				// ,temp -> second - temp -> first);

				if(zone[i].next == nullptr){
          printf("kalloc wrong\n");
				}
				temp = zone[i].next;
				pop_node_list(i);
			}
			occupy(temp -> first, temp -> second, 138);
      printf("Memory from %d to %d allocated\n", temp -> first, temp -> second);
		}
	}
}

void deallocate(int id)
{
  if(page_state[id] == -1){
    printf("Sorry, invalid free request\n");
  }

  // Size of block to be searched
  int n = log(page_state[id]) / log(2);
      
  int i, buddyNumber, buddyAddress;

  
  // Add the block in free list
  add_node_list(n, id, id + pow(2, n) - 1);
  printf("Memory block from  %d to %d freed\n", id, id + pow(2, n) - 1);


    // Calculate buddy number
  buddyNumber = id / page_state[id];

  if (buddyNumber % 2 != 0)
      buddyAddress = id - pow(2, n);
  else
      buddyAddress = id + pow(2, n);
          
    // Search in free list to find it's buddy
  i = 0;
  for(lp p = zone[n].next; p != nullptr ; p = p -> next, i++)
  {
    // If buddy found and is also free
    if (p -> first == buddyAddress)
    {
      // Now merge the buddies to make
      // them one large free memory block
      if (buddyNumber % 2 == 0)
      {
        add_node_list(n + 1, id , id + 2 * pow(2, n) - 1);

        printf("Coalescing of blocks starting at %d  and %d was done\n",id, buddyAddress);
      }
      else
      {
        add_node_list(n + 1, buddyAddress, buddyAddress + 2 * pow(2, n) - 1);

        printf("Coalescing of blocks starting at %d  and %d was done\n",id, buddyAddress);
      }
      pop_node_list_index(n, i);
      pop_node_list(n);
      break;
    }
  }

  // Remove the key existence from map
  free(id, id + page_state[id], 217);
}

