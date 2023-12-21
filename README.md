# Final Submission Work - MiniOS - CMPSC 473 Project 2

## Project 2 - Group 144 (sgroup144)

### Team Members
- Abhinav Sachdeva (avs7793)
- Sai Narayan (ssn5137)

## Overview
This repository contains the implementation for Project 2 of CMPSC 473. Our team has extended a small operating system, MiniOS, to set up a simple system call and initialize a simple page table.

____

## Implementation Details

### 1.1 Kernel Initialization
This function is the core of our kernel setup. It is responsible for initializing the page tables and setting up the environment for the user stack and program. It takes pointers to the user stack, user program, and a block of memory to be used for page table initialization.

#### Parameters:
- `ustack`: Pointer to the user stack.
- `uprogram`: Pointer to the user program.
- `memory`: Pointer to the memory block for page table setup.
- `memorySize`: Size of the memory block.

#### functionality:
- Sets up the page tables with identity mapping for the kernel space.
- Initializes page table entries for the user stack and program.


________

### 1.2 Page Table Setup
We have implemented a four-level page table setup as per the x86-64 architecture requirements. The page tables are initialized with identity mapping and appropriate flags.

#### Functionality:
- Level 1 page table entries are set up with identity mapping and appropriate flags.
- Level 2 to Level 4 page tables are set up to point to the beginning of each page that has entries mapped to physical pages in the previous level.
- Pointer arithmetic is used to navigate through contiguous blocks of page tables.

_______

### 1.3 System Call Entry
The `syscall_entry` function serves as the entry point for all system calls. Currently, it supports a single system call (n = 1) to print a user-supplied message. It is designed to handle system calls by checking the system call number and executing the corresponding function.

#### Parameters:
- `n`: System call number.
- `a1` to `a5`: Arguments for the system call.

#### functionality:
- Checks if the system call number `n` is equal to 1, which corresponds to our print message system call.
- If `n` is 1, it casts the first argument `a1` to a character pointer and prints the message pointed to by `a1`.
- Returns 0 for a successful system call execution, -1 otherwise.


________

### 1.4 User Space Support
#### Global Pointers:

- `user_stack`: This global pointer is set to point to the top of the user stack. It is initialized to a virtual address that is mapped to the appropriate physical memory location for the user stack.
- `user_program`: Similarly, this global pointer is set to the starting virtual address of the user program, ensuring that the program is correctly mapped into the user space memory region.

#### Page Table Configuration for User Space:

- The last entries of the level 1 page table are mapped to the user stack and user program. and the last entries of each level are mapped to the page address of the next level
- These entries are set with the necessary permissions to allow user mode access, that is the first 3 bits (0,1,2) are set to 1, ensuring that the user program can operate on its stack without causing page faults due to permission errors.

#### Functionality:

- The user space setup is crucial for the separation of kernel and user space memory, which is a fundamental security feature in modern operating systems.
- It allows user programs to execute in a controlled environment where they have access to their own memory regions but are restricted from accessing kernel memory, thus preventing accidental or malicious interference with the kernel.

_____

## Extra Credit is also finished, Malloc is successfully implemented
- Source code from the malloc implementation done by Abhinav Sachdeva, (avs7793)
- Malloc of Sai's was also giving the same score but we decided to go with Abhinav's implementation.
