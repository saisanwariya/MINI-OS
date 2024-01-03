# MiniOS

# Academic Integrity Statement

Please note that all work included in this project is the original work of the authors, and any external sources or references have been properly cited and credited. It is strictly prohibited to copy, reproduce, or use any part of this work without permission from the authors.

If you choose to use any part of this work as a reference or resource, you are responsible for ensuring that you do not plagiarize or violate any academic integrity policies or guidelines. The author of this work cannot be held liable for any legal or academic consequences resulting from the misuse or misappropriation of this work.

Any unauthorized copying or use of this work may result in serious consequences, including but not limited to academic penalties, legal action, and damage to personal and professional reputation. Therefore, please use this work only as a reference and always ensure that you properly cite and attribute any sources or references used.

---


### Team Members
- Abhinav Sachdeva
- Sai Narayan

## Overview
Our team has extended a small operating system, MiniOS, to set up a simple system call and initialize a simple page table.

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
