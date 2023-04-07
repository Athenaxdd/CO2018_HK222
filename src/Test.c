#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<stdint.h>
typedef uint32_t addr_t;
struct pcb_t {
	uint32_t pid;	// PID
	uint32_t priority;
	struct code_seg_t * code;	// Code segment
	addr_t regs[10]; // Registers, store address of allocated regions
	uint32_t pc; // Program pointer, point to the next instruction
	struct page_table_t * page_table; // Page table
	uint32_t bp;	// Break pointer
	uint32_t prio;
};
#define MAX_QUEUE_SIZE 10

struct queue_t {
	struct pcb_t * proc[MAX_QUEUE_SIZE];
	int size;
};

int empty(struct queue_t * q) {
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
	/* TODO: put a new process to queue [q] */
    int i;
    for(i = 0; i < q->size; i++){
        if(q->proc[i]->prio < proc->prio){
            int j;
            for(j = q->size; j > i; j--){
                q->proc[j] = q->proc[j-1];
            }
            q->proc[i] = proc;
            q->size++;
            return;
        }
    }
    q->proc[q->size] = proc;
    q->size++;

}

struct pcb_t * dequeue(struct queue_t * q) {
	/* TODO: return a pcb whose prioprity is the highest
	 * in the queue [q] and remember to remove it from q
	 * */
    if(q->size == 0){
        return NULL;
    }
    struct pcb_t * ret = q->proc[0];
    int i;
    for(i = 0; i < q->size - 1; i++){
        q->proc[i] = q->proc[i+1];
    }
    q->size--;
    return ret;
}