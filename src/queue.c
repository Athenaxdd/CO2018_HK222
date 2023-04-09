#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
	return (q->size == 0);
}

bool full (struct queue_t *q{
	return (q -> size == MAX_QUEUE_SIZE);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
	/* TODO: put a new process to queue [q] */
	int i;
	if (full(q)){
    		printf("Queue Overflow");
    		exit(1);
    	}
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
	if(q->size <= 0){
        return NULL;
    }
    struct pcb_t * ret = q->proc[0];
    int i;
    for(i = 0; i < q->size; i++){
        q->proc[i] = q->proc[i+1];
    }
    q->size--;
    q->proc[q->size] = NULL;
    return ret;
}

