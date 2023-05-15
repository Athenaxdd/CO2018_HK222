#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<stdint.h>
typedef uint32_t addr_t;
struct pcb_t {
	uint32_t pid;	// PID
	uint32_t priority; //priority of process
	struct code_seg_t * code;	// Code segment
	addr_t regs[10]; // Registers, store address of allocated regions
	uint32_t pc; // Program pointer, point to the next instruction
	struct page_table_t * page_table; // Page table
	uint32_t bp;	// Break pointer
    #ifdef MLQ_SCHED
	    uint32_t prio;    // Priority on execution (if supported), this value overwrites the default priority when it existed
    #endif
};
#define MAX_QUEUE_SIZE 10

struct queue_t {
	struct pcb_t * proc[MAX_QUEUE_SIZE];
	int size;
};
int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}



void enqueue(struct queue_t * q, struct pcb_t * proc) {
    /* TODO: put a new process to queue [q] */
    int i;
    // Find the correct position to insert the new process based on its priority
    for (i = 0; i < q->size; i++) {
        if (q->proc[i]->priority <= proc->priority) {
            int j;
            // Shift all processes with lower or equal priority to the right
            for (j = q->size - 1; j >= i; j--) {
                q->proc[j + 1] = q->proc[j];
            }
            // Insert the new process at the correct position
            q->proc[i] = proc;
            q->size++;
            return;
        }
    }
    // If the new process has lower priority than all existing processes, insert it at the end
    q->proc[q->size] = proc;
    q->size++;
}

struct pcb_t * dequeue(struct queue_t * q) {
    /* TODO: return a pcb whose priority is the highest
     * in the queue [q] and remember to remove it from q
     */
    if (empty(q)) {
        return NULL; // Return NULL if the queue is empty
    }
    // Find the process with the highest priority
    int i, max_idx = 0;
    for (i = 1; i < q->size; i++) {
        if (q->proc[i]->priority > q->proc[max_idx]->priority) {
            max_idx = i;
        }
    }
    // Remove the process from the queue
    struct pcb_t *ret = q->proc[max_idx];
    for (i = max_idx; i < q->size - 1; i++) {
        q->proc[i] = q->proc[i + 1];
    }
    q->proc[q->size - 1] = NULL;
    q->size--;
    return ret;
}

