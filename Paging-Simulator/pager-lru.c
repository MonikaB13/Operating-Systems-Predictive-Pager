/*
 * File: pager-lru.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h>
#include <stdlib.h>

#include "simulator.h"
int debug = 1;

void pageit(Pentry q[MAXPROCESSES]) {
    if(debug){
       printf("PAGEIT CALLED");
    }
    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];
    /* Local vars */
    int proctmp;
    int pagetmp;
    int pc;
    int page;
    int oldpage;
    int old_time;
    /* initialize static vars on first run */
    if(!initialized){
	for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
	    for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
		timestamps[proctmp][pagetmp] = 0;
	    }
	}
	initialized = 1;
    }
    for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
        if(q[proctmp].active){
            /*dedicate work to the active process*/
            pc = q[proctmp].pc; /*program counter for the process*/
            page = pc/PAGESIZE; /*page the program counter needs*/
            /*is the page swapped out?*/
	    if(debug){
                printf("proc: %d, curr_page: %d\n", proctmp, page);
            }
            if(!q[proctmp].pages[page]){
            /*try to swap in*/
                if(!pagein(proctmp,page)){
                    old_time = tick;
                    for(pagetmp = 0; pagetmp < MAXPROCPAGES; pagetmp++){
			if (q[proctmp].pages[pagetmp]){
                        	if(timestamps[proctmp][pagetmp] < old_time){
                            		old_time = timestamps[proctmp][pagetmp];
                            		oldpage = pagetmp;
				}
                        }
                    }
                    pageout(proctmp, oldpage);
		    pagein(proctmp, page);
                }  
            }
            timestamps[proctmp][page] = tick;
        }
    } 
    /* advance time for next pageit iteration */
    tick++;
}
