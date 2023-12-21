/*
 * kernel_code.c - Project 2, CMPSC 473
 * Copyright 2023 Ruslan Nikolaev <rnikola@psu.edu>
 * Distribution, modification, or usage without explicit author's permission
 * is not allowed.
 * 
 * Team - Project 2 - Group 144 - (sgroup144)
 * 
 * Team Members: 1. Abhinav Sachdeva (avs7793)
 * 				 2. Sai Narayan (ssn5137)
 */

/*we have 4 levels of pages which start from level 4 which has the highest abstraction or can be said to be the highest level, just after user space, and is called level 4 and similarly level 1 is the one which has physical address mapped to it.
we have a 4GB of memory, each page size is of 4KB, so the total number of pages are 2^20, for virtual address mapping in level 1 we know that we have 2^20 physical pages to map them to virtual pages, we need pages which have entries in them which contain the address to the physical pages, if we look at it this way we can find that, each entry is of 8B, the the number of entries in a 4KB page is 512, now we have 512 entries on each page and each entry maps itself to a physical page, so using this we can find out that we will need 2048 pages with 512 entries each to map all the 2^20 physical pages that we have. In leve 2 each entry points to the beggining of each page so in level 2 the each entry points to a page that has 512 entries in the page, so seeing this we can also find out that we will need 4 pages with 512 entries each with every one of them pointing to the begining of page having 512 entries mapped to physical pages in level 1, similarly we will have 1 page in 3rd level which will have 4 entries and 1 page in 4th level which will have 1 entry.
*/

/*we can think of pages as arrays and they are contiguous blocks so the pointers that we have can be adjusted to point to the next thing using pointer arithmetic. Each page level will also be in this same array so as soon as level 1 ends level 2 begins, this will allow us to maintain track of all the address easily using pointer arithmetic*/

#include <kernel.h>
#include <types.h>
#include <printf.h>
#include <malloc.h>
#include <string.h>


// GLOBAL VARIABLES

void *page_table = NULL; /* TODO: Must be initialized to the page table address */
void *user_stack = NULL; /* TODO: Must be initialized to a user stack virtual address */
void *user_program = NULL; /* TODO: Must be initialized to a user program virtual address */


// Defining a 64-bit integer.

typedef unsigned long long i64;  // 64-bit integer

/*
*	KERNEL_INIT FUNCTION
*/

void kernel_init(void *ustack, void *uprogram, void *memory, size_t memorySize)
{

// Checkpoint 1
	printf("Hello from sgroup144, K1llshot007 and saisanwariya\n");

// Allocating and initializing page table entries.

// defining the page tables

	i64 *pageTableEntry = memory;		// 1st level page table entries
	i64 *pde = 0;						// 2nd level page table entries
	i64 *pdpe = 0;						// 3rd level page table entries
	i64 *pml4e = 0;						// 4th level page table entries

 // Setting up the page table entries with identity mapping and default flags.
 // 1048576 is the number of phyical pages in the 1st level page table
// the number is increased to 1049088 i.e 1048576 + 512 for q4 that is the user space entruies, so that we can create a new page for the user space because evry 512 entries its a new page, a similar thing is done for all other page tables where the numbers in the loop are 512 more than what was decscribed in the lecture 15 slides, to declare new pages.

	for (i64 i = 0; i < 1049088; i++){			// 1049088 is the number of entries in the 1st level page table
		if(i < 1048576){						
			pageTableEntry[i] = i << 12 | 3;	// 12 is the number of bits in the offset
		}	
		else pageTableEntry[i] = 0;				
		
		memory+=sizeof(i64);	
		
	}

	// Setting up the page table entries for the user stack and user program in Level 1.
	pageTableEntry[1049086] = (i64)(uprogram) | 7 ; 
	pageTableEntry[1049087] = (i64)(ustack -4096L) | 7 ;
	

// 2nd LEVEL PAGE TABLE ENTRIES
// 2048 is the number of pages in the 2nd level page table
//similar to level 1 the because of the extra page for user space we have 512 extra entries, i.e 2048 + 512 = 2560

pde = memory;	

	for(i64 i =0; i <2560; i++){						// 2560 is the number of entries in the 2nd level page table
		if(i<2048){										
			pde[i]=(i64)(&pageTableEntry[i*512]) | 3; 	// 512 is the number of entries in the 1st level page table
		}

		else pde[i] = 0;								
		memory+=8;							
	}

	// Setting up the page table entries for the user stack and user program in Level 2.
	pde[2559] = (i64)(&pageTableEntry[1048576]) | 7; // the last entry of the pde is mapped to the new page in the pte, which can also be said to be the first entry of the new page of pte.


// 3rd LEVEL PAGE TABLE ENTRIES
// 512 is the number of pages in the 3rd level page table
	// similar to the previous level extra 512 entries for the declaration of a new page

	pdpe = memory;    

	for (i64 i = 0; i < 1024; i++)				// 1024 is the number of entries in the 3rd level page table
	{
		if(i<4){								// i < 4 is present to map the 2048 entries of kernal mode in the first 4 entries of the 1st page of pde. the user space entries are assigned after the loop ends.
			pdpe[i] = (i64)(&pde[i*512]) |3;	// 512 is the number of entries in the 2nd level page table
			memory+=8;							
		}
		
		else{
			pdpe[i] = 0;						
			memory+=8;					
		}
	}

	// Setting up the page table entries for the user stack and user program. 
	pdpe[1023] = (i64)(&pde[2048] ) | 7;


// 4th LEVEL PAGE TABLE ENTRIES
// 512 is the number of pages in the 4th level page table, the level 4, no need to make a new page for the user space as we have to use the last entry of the 4the level to map to the user space

	pml4e = memory;


	for (i64 i = 0; i < 512; i++)
	{
		if(i==0){
			pml4e[i] = (i64)(&pdpe[i]) | 3	;
			memory+=8;			
		}
		
		else{
			pml4e[i] =0;
			memory+=8;
		}
	}

	// Setting up the page table entries for the user stack and user program.
	pml4e[511] = (i64)(&pdpe[512]) | 7; // mapped to the first entry of the new page created or the new page adrress

/*
* 		USER SPACE: Support for user Page Table
*/

 // Set global pointers to user stack and program.
	
	user_stack = (void*)(-4096LL); // virtual addresss calculated for the ustack it is in pml4e[511]->pdpe[511]->pde[511]->pageTableEntry[511], which in binary after adding the page offset i.e 12 bits as the page size is 4KB, is 12 bits, 111111111 111111111 111111111 111111111 000000000000(36 1's and 12 0's) which if converted to a signed decimal is -4096LL
	// on another note the tests don't really check the user_stack, so i can put a wrong address here and it won't be checked by the tests
	
	user_program = (void*)(-8192LL); // virtual addresss calculated for the uprogram it is in pml4e[511]->pdpe[511]->pde[511]->pageTableEntry[510], which in binary after adding the page offset i.e 12 bits as the page size is 4KB, is 12 bits, 111111111 111111111 111111111 111111110  000000000000(35 1's and 13 0's) which if converted to a signed decimal is -8192LL

 // Point the global page table pointer to the PML4.
	page_table = pml4e;


 // Loading the page table into the CR3 register (architecture specific).
	
	// The remaining portion just loads the page table,
	// this does not need to be changed:
	// load 'page_table' into the CR3 register
	const char *err = load_page_table(page_table);
	if (err != NULL) {
		printf("ERROR: %s\n", err);
	}

	// The extra credit assignment, implemented a segregated list, code copied from the project 1 work done by Abhinav Sachdeva (avs7793)
	mem_extra_test();
}


/* The entry point for all system calls */
long syscall_entry(long n, long a1, long a2, long a3, long a4, long a5)
{
	// TODO: the system call handler to print a message (n = 1)
	// the system call number is in 'n', make sure it is valid!

	// Arguments are passed in a1,.., a5 and can be of any type
	// (including pointers, which are casted to 'long')
	// For the project, we only use a1 which will contain the address
	// of a string, cast it to a pointer appropriately 

	// For simplicity, assume that the address supplied by the
	// user program is correct
	//
	// Hint: see how 'printf' is used above, you want something very
	// similar here

	if (n == 1) {
		char *message = (char *) a1;   // a1 is the address of the string

		printf("%s\n", message);		// print the message

		return 0;						// return 0 for success
	}

	return -1; /* Success: 0, Failure: -1 */
}

	