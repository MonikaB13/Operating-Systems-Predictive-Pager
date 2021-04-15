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
#define TICKS_AHEAD 100 /*number of ticks predictive algorithm should predict ahead*/

int debug = 0;/*for all processes except markov chain*/
int debug1 = 0;/*for markov chain*/
int debug2 = 0; /*for building the matrix*/

void build_matrix(int pred_matrix[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES], int proc, int page, int next_page){
	int current = page;
	if(current == -1){ /*if current page = -1 no entry to be made because no curr/next relationship, i.e. it's the first page*/
		return;
	}
	if(current == next_page){ /*exclude page entries with itself*/
		return;
	}
	pred_matrix[proc][current][next_page]++; /*update matrix counts*/
	if(debug2){
		printf("BUILD MATRIX proc: %d, page: %d, next_page: %d, count: %d\n", proc, current, next_page, pred_matrix[proc][current][next_page]);

	}
}


int pred_pager(int pred_matrix[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES], int proc, int pc){
	int page = pc/PAGESIZE;
	int *nexts = pred_matrix[proc][page];
	int val = 0;
	int most_likely_page = -1;
	for(int i = 0; i < MAXPROCPAGES; i++){
		if(nexts[i] > val){
			most_likely_page = i;
			val = nexts[i];
		}	
	}
	if(most_likely_page == -1){
		return page;
	}	
	return most_likely_page;
}


int pred_pager2(int pred_matrix[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES], int proc, int pc, int pred1){
	int page = pc/PAGESIZE;
	int *nexts = pred_matrix[proc][page];
	int val = 0;
	int most_likely_page = -1;
	for(int i = 0; i < MAXPROCPAGES; i++){
		if(nexts[i] > val && nexts[i] < nexts[pred1]){
			most_likely_page = i;
			val = nexts[i];		

		}
	}
	if(most_likely_page == -1){
		most_likely_page = page;
	}
	return most_likely_page;
}

int pred_pager3(int pred_matrix[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES], int proc, int pc, int pred1, int pred2){
	int page = pc/PAGESIZE;
	int *nexts = pred_matrix[proc][page];
	int val = 0;
	int most_likely_page = -1;
	for(int i = 0; i < MAXPROCPAGES; i++){
		if(nexts[i] > val && i != pred1 && i != pred2){
			most_likely_page = i;
			val = nexts[i];		

		}
	}
	if(most_likely_page == -1){
		most_likely_page = page;
	}
	return most_likely_page;
}
void lru_pager(Pentry q[MAXPROCESSES], int proc, int page, int tick, int timestamps[MAXPROCESSES][MAXPROCPAGES], int next, int next2, int next3){
	int old_time = 0;
	int oldpage;
	int pagetmp;
	if(!q[proc].pages[page]){
            /*try to swap in*, if not in, pagein*/
                if(!pagein(proc,page)){
                    old_time = tick;
                    for(pagetmp = 0; pagetmp < MAXPROCPAGES; pagetmp++){
			if(pagetmp != next && pagetmp != next2 && pagetmp != next3)
			if (q[proc].pages[pagetmp]){
                        	if(timestamps[proc][pagetmp] < old_time){
                            		old_time = timestamps[proc][pagetmp];
                            		oldpage = pagetmp;
				}
                        }
                    }
                    pageout(proc, oldpage);
                }  
            }
            timestamps[proc][page] = tick;
}


void pageit(Pentry q[MAXPROCESSES]) {
    if(debug){
	printf("IN PAGE IT FUNCTION");
    }

    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];
    static int pred_matrix[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES];
    /* Local vars */
    int proctmp;
    int pagetmp;
    int pc;
    static int counter = 0;
    static int curr_page;
    int prev;
    int next;
    int next2;
    int next3;
    int prev2;
    static int prev_page[MAXPROCESSES];

    /* initialize static vars on first run */
    if(!initialized){
	for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
	    prev_page[proctmp] = -1;
	    for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
		timestamps[proctmp][pagetmp] = 0;
	    }
	}
	for(int i = 0; i < MAXPROCESSES; i++){
		for(int j = 0; j < MAXPROCPAGES; j++){
			for(int k = 0; k < MAXPROCPAGES; k++){
				pred_matrix[i][j][k] = 0;
			}
		}

	}
	initialized = 1;
    }
    for(proctmp = 0; proctmp < MAXPROCESSES; proctmp++){
        if(q[proctmp].active){
            /*dedicate work to the active process*/
	    prev = prev_page[proctmp];
	    pc = q[proctmp].pc; /*program counter for the process*/
            curr_page = pc/PAGESIZE; /*page the program counter needs*/
            /*is the page swapped out?*/
	    if(debug){
		printf("pc: %d, curr: %d, pc2: %d, page2: %d\n", pc, curr_page, pc + 100, (pc + 100)/PAGESIZE);
		printf("IN PAGEIT PROC LOOP proc: %d, curr_page: %d, prev_page: %d\n", proctmp, curr_page, prev_page[proctmp]);
	    }
	    build_matrix(pred_matrix, proctmp, prev, curr_page);
	   
	    next = pred_pager(pred_matrix, proctmp, pc + 100);
	    next2 = pred_pager2(pred_matrix, proctmp, pc + 100, next);
	    next3 = pred_pager3(pred_matrix, proctmp, pc + 100, next, next2);
	    if(prev != curr_page && next != prev && next2 != prev && next3 != prev){
			pageout(proctmp, prev);
	    }
	    lru_pager(q, proctmp, next, tick, timestamps, next2, curr_page, next3);
	    lru_pager(q, proctmp, next2, tick, timestamps, next, curr_page, next3);
	    lru_pager(q, proctmp, curr_page, tick, timestamps, next, next2, next3);
	    lru_pager(q, proctmp, curr_page, tick, timestamps, next, next2, next3);
	    prev_page[proctmp] = curr_page;
        }
    } 
    /* advance time for next pageit iteration */
    tick++;
}
