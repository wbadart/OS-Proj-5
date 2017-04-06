/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"
#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char *algorithm;
int npages;         //pages represent program in virtual memory
int nframes;        //frames = physical memory
struct disk *disk;
int frame_state[];  //keeps track of whether a frame is empty or not

struct page_queue{
    int *data;
    size_t head;
    size_t tail;
};
//page_queue functions
void push_back(struct page_queue *pq, int n){
    pq->data[pq->tail++] = n;
}
int pop_front(struct page_queue *pq){
    return pq->head <= pq->tail ? pq->data[pq->head++] : -1;
}

//global page_queue object keeps track of order frames are used
struct page_queue pq = {malloc(4096*sizeof(int)), 0, 0};

void page_fault_handler( struct page_table *pt, int page )
{
    //desired page is not in physical memory
    int open_frame;
    for(open_frame = 0; open_frame < nframes; open_frame++){
        if(frame_state[open_frame] == 0)
            break;
    }
    //if there is an open frame, read from disk into frame and set entry
    if(open_frame < nframes){
        disk_read(disk, page, page_table_get_physmem(pt) + open_frame);
        //push page to queue
        push_back(&pq, open_frame);
        frame_state[open_frame] = 1;
        page_table_set_entry(pt, page, open_frame, PROT_READ);
    }
    //else physical memory is full, use one of the algorithms to kick out a page
    int target_page = 0;
    if(open_frame == nframes){
        if(strncmp(algorithm, "rand", 7) == 0){
            target_page = rand() % npages;
            printf("chose page %d\n", target_page);
        }else if(strncmp(algorithm, "fifo", 7) == 0){
            target_page = pop_front(&pq);
            printf("chose page %d\n", target_page);
        }else if(strncmp(algorithm, "custom", 7) == 0){
            //implementation of LDF algorithm
            printf("chose page %d\n", target_page);
        }else{
            printf("algorithm not recognized\n");
            exit(1);
        }
    }
    //once target page is selected, check if it has been written to
    int *bits = 0;
    int *frame = 0;
    page_table_get_entry(pt, target_page, frame, bits);
    //just PROT_WRITE        = 010 = 2
    //just PROT_READ         = 100 = 4
    //PROT_READ & PROT_WRITE = 110 = 6
    if(*bits == 2 || *bits == 6){
        //write target page to disk before kicking it out
        disk_write(disk, target_page, page_table_get_physmem(pt) + *frame);
    }
    //set frame to new page that got read in
    page_table_set_entry(pt, target_page, *frame, PROT_READ);
    frame_state[*frame] = 1;
    push_back(&pq, *frame);
    printf("page fault on page #%d\n",page);
	exit(1);
}

int main( int argc, char *argv[] )
{
	if(argc!=5) {
		printf("use: virtmem <npages> <nframes> <rand|fifo|lru|custom> <sort|scan|focus>\n");
		return 1;
	}

	npages = atoi(argv[1]);
    nframes = atoi(argv[2]);
	algorithm = argv[3];
    const char *program = argv[4];

	disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}

	struct page_table *pt = page_table_create( npages, nframes, page_fault_handler );
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}

    int i;
    for(i = 0; i < nframes; i++) frame_state[i] = 0;
	char *virtmem = page_table_get_virtmem(pt);

	char *physmem = page_table_get_physmem(pt);

	if(!strcmp(program,"sort")) {
		sort_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"scan")) {
		scan_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"focus")) {
		focus_program(virtmem,npages*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[3]);
		return 1;
	}

	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
