
#include "mem.h"
#include "stdlib.h"
#include "string.h"
#include <pthread.h>
#include <stdio.h>

static BYTE _ram[RAM_SIZE];

static struct {
	uint32_t proc;	// ID of process currently uses this page
	int index;	// Index of the page in the list of pages allocated
			// to the process.
	int next;	// The next page in the list. -1 if it is the last
			// page.
} _mem_stat [NUM_PAGES];

static pthread_mutex_t mem_lock;

void init_mem(void) {
	memset(_mem_stat, 0, sizeof(*_mem_stat) * NUM_PAGES);
	memset(_ram, 0, sizeof(BYTE) * RAM_SIZE);
	pthread_mutex_init(&mem_lock, NULL);
}

/* get offset of the virtual address */
static addr_t get_offset(addr_t addr) {
	return addr & ~((~0U) << OFFSET_LEN);
}

/* get the first layer index */
static addr_t get_first_lv(addr_t addr) {
	return addr >> (OFFSET_LEN + PAGE_LEN);
}

/* get the second layer index */
static addr_t get_second_lv(addr_t addr) {
	return (addr >> OFFSET_LEN) - (get_first_lv(addr) << PAGE_LEN);
}

/* Search for page table table from the a segment table */
static struct trans_table_t * get_trans_table(
		addr_t index, 	// Segment level index
		struct page_table_t * page_table) { // first level table
	
	/*
	 * TODO: Given the Segment index [index], you must go through each
	 * row of the segment table [page_table] and check if the v_index
	 * field of the row is equal to the index
	 *
	 * */


	int i;
	for (i = 0; i < page_table->size; i++) {
		// Enter your code here
		if(page_table->table[i].v_index == index)
			return &page_table->table[i].v_index;
	}
	return NULL;

}

/* Translate virtual address to physical address. If [virtual_addr] is valid,
 * return 1 and write its physical counterpart to [physical_addr].
 * Otherwise, return 0 */
static int translate(
		addr_t virtual_addr, 	// Given virtual address
		addr_t * physical_addr, // Physical address to be returned
		struct pcb_t * proc) {  // Process uses given virtual address

	/* Offset of the virtual address */
	addr_t offset = get_offset(virtual_addr);
	/* The first layer index */
	addr_t first_lv = get_first_lv(virtual_addr);
	/* The second layer index */
	addr_t second_lv = get_second_lv(virtual_addr);
	
	/* Search in the first level */
	struct trans_table_t * trans_table = NULL;
	trans_table = get_trans_table(first_lv, proc->page_table);
	if (trans_table == NULL) {
		return 0;
	}

	int i;
	for (i = 0; i < trans_table->size; i++) {
		if (trans_table->table[i].v_index == second_lv) {
			/* TODO: Concatenate the offset of the virtual addess
			 * to [p_index] field of trans_table->table[i] to 
			 * produce the correct physical address and save it to
			 * [*physical_addr]  */
			// *physical_addr = offset /* (TODo) + translated_based_address */;
			*physical_addr = offset + trans_table->table[i].p_index;

			return 1;
		}
	}
	return 0;	
}
void add_page_table_entry(
		addr_t v_index, 	// Virtual index
		addr_t p_index, 	// Physical index
		struct page_table_t * page_table) { // Page table to add entry
		//add page table entry
		int i;
		for (i = 0; i < page_table->size; i++) {
			if (page_table->table[i].v_index == 0) {
				page_table->table[i].v_index = v_index;
				page_table->table[i].next_lv = p_index;
				return;
			}
		}
		page_table->table[page_table->size].v_index = v_index;
		page_table->table[page_table->size].next_lv = p_index;
		page_table->size++;

}
addr_t alloc_mem(uint32_t size, struct pcb_t * proc) {
	pthread_mutex_lock(&mem_lock);
	addr_t ret_mem = 0;
	/* TODO: Allocate [size] byte in the memory for the
	 * process [proc] and save the address of the first
	 * byte in the allocated memory region to [ret_mem].
	 * */

	uint32_t num_pages = (size % PAGE_SIZE) ? size / PAGE_SIZE :
		size / PAGE_SIZE + 1; // Number of pages we will use
	int mem_avail = 0; // We could allocate new memory region or not?

	/* First we must check if the amount of free memory in
	 * virtual address space and physical address space is
	 * large enough to represent the amount of required 
	 * memory. If so, set 1 to [mem_avail].
	 * Hint: check [proc] bit in each page of _mem_stat
	 * to know whether this page has been used by a process.
	 * For virtual memory space, check bp (break pointer).
	 * */
	for(int i = 0; i < NUM_PAGES; i++){
		if(_mem_stat[i].proc == 0){
			mem_avail = 1;
			break;
		}
	}
	if (proc->bp + num_pages * PAGE_SIZE < RAM_SIZE) {
		mem_avail = 1;
	}
		
	if (mem_avail) {
		/* We could allocate new memory region to the process */
		ret_mem = proc->bp;
		proc->bp += num_pages * PAGE_SIZE;
		/* Update status of physical pages which will be allocated
		 * to [proc] in _mem_stat. Tasks to do:
		 * 	- Update [proc], [index], and [next] field
		 * 	- Add entries to segment table page tables of [proc]
		 * 	  to ensure accesses to allocated memory slot is
		 * 	  valid. */

		// Update _mem_stat by updating proc, index, next and adding entries to segment table and page tables
		int count = 0;
		for(int i = 0; i < NUM_PAGES; i++){
			if(_mem_stat[i].proc == 0){
				_mem_stat[i].proc = proc->pid;
				_mem_stat[i].index = count;
				_mem_stat[i].next = NULL;
				count++;
				if(count == num_pages)
					break;
			}
		}
		// Add entries to segment table and page tables
		addr_t first_lv = get_first_lv(ret_mem);
		addr_t second_lv = get_second_lv(ret_mem);
		addr_t physical_addr = 0;
		for(int i = 0; i < num_pages; i++){
			translate(ret_mem, &physical_addr, proc);
			add_page_table_entry(second_lv, physical_addr, proc->page_table);
			ret_mem += PAGE_SIZE;
			second_lv++;
			if(second_lv == 256){
				first_lv++;
				second_lv = 0;
			}
		}
	}
	pthread_mutex_unlock(&mem_lock);
	return ret_mem;
}

int free_mem(addr_t address, struct pcb_t * proc) {
	/*TODO: Release memory region allocated by [proc]. The first byte of
	 * this region is indicated by [address]. Task to do:
	 * 	- Set flag [proc] of physical page use by the memory block
	 * 	  back to zero to indicate that it is free.
	 * 	- Remove unused entries in segment table and page tables of
	 * 	  the process [proc].
	 * 	- Remember to use lock to protect the memory from other
	 * 	  processes.  */
	// Set flag [proc] of physical page use by the memory block back to zero
	pthread_mutex_lock(&mem_lock);
	addr_t first_lv = get_first_lv(address);
	addr_t second_lv = get_second_lv(address);
	addr_t physical_addr;
	if(translate(address, &physical_addr, proc)){
		_mem_stat[physical_addr].proc = 0;
		_mem_stat[physical_addr].index = 0;
		_mem_stat[physical_addr].next = 0;
	}
	// Remove unused entries in segment table and page tables of the process [proc]
	int i = 0;
	int j = 0;
	int k = 0;
	int count = 0;
	while(count < proc->page_table->size){
		if(proc->page_table->table[i].v_index == physical_addr){
			proc->page_table->table[i].v_index = 0;
			proc->page_table->table[i].next_lv = 0;
			count++;
			j++;
			k++;
		}
		else{
			j++;
		}
		i++;
	}
	proc->page_table->size = proc->page_table->size - k;
	pthread_mutex_unlock(&mem_lock);
	return 0;
}

int read_mem(addr_t address, struct pcb_t * proc, BYTE * data) {
	addr_t physical_addr;
	if (translate(address, &physical_addr, proc)) {
		*data = _ram[physical_addr];
		return 0;
	}else{
		return 1;
	}
}

int write_mem(addr_t address, struct pcb_t * proc, BYTE data) {
	addr_t physical_addr;
	if (translate(address, &physical_addr, proc)) {
		_ram[physical_addr] = data;
		return 0;
	}else{
		return 1;
	}
}

void dump(void) {
	int i;
	for (i = 0; i < NUM_PAGES; i++) {
		if (_mem_stat[i].proc != 0) {
			printf("%03d: ", i);
			printf("%05x-%05x - PID: %02d (idx %03d, nxt: %03d)\n",
				i << OFFSET_LEN,
				((i + 1) << OFFSET_LEN) - 1,
				_mem_stat[i].proc,
				_mem_stat[i].index,
				_mem_stat[i].next
			);
			int j;
			for (	j = i << OFFSET_LEN;
				j < ((i+1) << OFFSET_LEN) - 1;
				j++) {
				
				if (_ram[j] != 0) {
					printf("\t%05x: %02x\n", j, _ram[j]);
				}
					
			}
		}
	}
}


