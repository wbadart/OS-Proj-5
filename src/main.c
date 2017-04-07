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
int *frame_states
    , frame_queue_index
    , npages        // pages represent program in virtual memory
    , nframes       // frames = physical memory
    , nreads;

typedef struct frame{
    int is_available;
    int is_dirty;
    int entry_order;
    int page_index;
} frame_t;

int open_frame(frame_t *fs, int nframes){
    int i;
    for(i = 0; i < nframes; i++)
        if((fs + i)->is_available) return i;
    return -1;
}

int oldest_frame(frame_t *fs, int nframes){
    int i, min = nreads, min_index;
    for(i = 0; i < nframes; i++)
        if(fs[i].entry_order < min){
            min = fs[i].entry_order;
            min_index = i;
        }
    return min_index;
}

frame_t *frames;

struct disk *disk;

void page_fault_handler( struct page_table *pt, int page )
{

    printf("page fault on page #%d\n",page);

    int frame, bits;
    page_table_get_entry(pt, page, &frame, &bits);

    //printf("INFO: read '%d' bits\n", bits);

    if(bits == 0){
        int available_frame = open_frame(frames, nframes);
        printf("INFO: got available_frame '%d'\n", available_frame);

        if(available_frame >= 0){
            disk_read( disk, page
                     , &(page_table_get_physmem(pt)[page * 4096]) );

            page_table_set_entry(pt, page, available_frame, PROT_READ);

            frames[available_frame].is_available = 0;
            frames[available_frame].page_index   = page;
            frames[available_frame].entry_order  = nreads++;

        } else {
            int eviction_target;
            if(strcmp(algorithm, "rand") == 0)
                eviction_target = rand() % nframes;
            else if(strcmp(algorithm, "fifo") == 0)
                eviction_target = oldest_frame(frames, nframes);
            else if(strcmp(algorithm, "custom") == 0)
                eviction_target = rand() % nframes;
            else{
                printf("ERR: Algorithm '%s' not recognized\n", algorithm);
                exit(2);
            }

            printf("INFO: picked eviction target '%d'\n", frames[eviction_target].page_index);

            int bits_evict, frame_evict;
            page_table_get_entry(pt, frames[eviction_target].page_index, &frame_evict, &bits_evict);
            // just PROT_WRITE        = 010 = 2
            // just PROT_READ         = 100 = 4
            // PROT_READ & PROT_WRITE = 110 = 6
            if(bits_evict == 2 || bits_evict == 6){
                //write target page to disk before kicking it out
                disk_write( disk, frames[eviction_target].page_index
                          , &(page_table_get_virtmem(pt)[eviction_target * 4096]));
            }

            page_table_set_entry(pt, frames[eviction_target].page_index, 0, 0);

            disk_read( disk, page
                     , &(page_table_get_physmem(pt)[eviction_target * 4096]) );

            page_table_set_entry(pt, page, eviction_target, PROT_READ);

            //page_table_print(pt);

            frames[eviction_target].is_available = 0;
            frames[eviction_target].page_index   = page;
            frames[eviction_target].entry_order  = nreads++;

        }
    } else if(bits == PROT_READ){
        page_table_set_entry(pt, page, frame, PROT_READ|PROT_WRITE);
    } else {
        printf("ERR: Bits isn't 0 or PROT_READ\n");
        exit(1);
    }
    printf("Page table at end:\n");
    page_table_print(pt);
    printf("\n");
    /* if(nreads > 2) exit(1); */
    return;
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
    nreads = 0;

    frames = malloc(nframes * sizeof(frame_t));
    int i;
    for(i = 0; i < nframes; i++)
        frames[i].is_available = 1;

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
    /* char *physmem = page_table_get_physmem(pt); */

    if(!strcmp(program,"sort")) {
        sort_program(virtmem,npages*PAGE_SIZE);

    } else if(!strcmp(program,"scan")) {
        scan_program(virtmem,npages*PAGE_SIZE);

    } else if(!strcmp(program,"focus")) {
        focus_program(virtmem,npages*PAGE_SIZE);

    } else {
        fprintf(stderr,"ERR: unknown program: %s\n",argv[3]);
        return 1;
    }

    free(frame_states);
    page_table_delete(pt);
    disk_close(disk);

    return 0;
}
