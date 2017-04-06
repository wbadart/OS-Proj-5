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
int *frame_states;
int npages;
int nframes;

struct disk *disk;

void page_fault_handler( struct page_table *pt, int page )
{
    printf("page fault on page #%d\n",page);

    //desired page is not in physical memory
    int open_frame;
    for(open_frame = 0; open_frame < nframes; open_frame++){
        if(frame_states[open_frame] == 0)
            break;
    }

    // read something from disk into page
    disk_read(disk, page, page_table_get_physmem(pt) + page);
    page_table_set_entry(pt, page, open_frame, PROT_READ);
    frame_states[open_frame] = 1;

    // if physical memory is full use one of the algorithms to kick out a page
    if(open_frame == npages){
        int target_page;
        if(strncmp(algorithm, "rand", 7) == 0){
            target_page = rand() % npages;
            printf("chose page %d\n", target_page);

        } else if(strncmp(algorithm, "fifo", 7) == 0){
             //target_page = pop_front(&pq);
             printf("chose page %d\n", target_page);

        } else if(strncmp(algorithm, "custom", 7) == 0){

        } else{
            printf("algorithm not recognized\n");
            exit(1);
        }

    } else printf("page_fault_handler: open_frame != npages\n");

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

    frame_states = malloc(nframes * sizeof(int));
    int i;
    for(i = 0; i < nframes; i++) frame_states[i] = 0;

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

    free(frame_states);
	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
