# Virtual Memory Allocator

### First Project (SDA)  
**#BIRLEANU TEODOR MATEI 314CA 2022-2023**

---

### Overview
The main program reads commands to determine the operations to perform. For each entered command, necessary parameters like starting addresses and sizes are handled using subprograms. The solution involves implementing doubly linked lists and their specific operations. To simplify the structure, a `*tail` node is introduced, maintaining the characteristics of the last node in the list.

---

### Functions for List Operations

#### 1. **`dll_create`**  
   Allocates memory for the list and initializes its components.

#### 2. **`dll_get_nth_node`**  
   Traverses nodes individually to locate the node at a specific index. If no next node exists, the list is considered empty.

#### 3. **Delete Node**  
   Deletes a specified node and returns a pointer to it.  
   - Handles error cases.  
   - Positions the pointer at the given index and adjusts neighboring nodes accordingly.  
   - Special cases:  
     - Adding or deleting at the head or tail: Complexity `O(1)`.  
     - General case (iterative): Complexity `O(n)`.

#### 4. **`dll_get_size`**  
   Returns the number of elements in the list with a complexity of `O(1)`.

---

### Subprograms for Problem Commands

#### 1. **`alloc_arena`**
   - Initializes the arena size and the block list.  
   - Uses a doubly linked list structure for initialization.

#### 2. **Resource Reallocation**  
   - Iterates through all blocks and checks if they contain mini-blocks.  
   - Deletes mini-blocks one by one, ensuring to free the allocated memory.  
   - Deallocates all memory for mini-blocks, blocks, and the list itself.

#### 3. **Block Allocation**  
   - Checks for error cases before adding new entities to the list.  
   - Handles special cases for adding nodes at the head or tail efficiently.  
   - **`free_block` Function**: Frees memory based on the node’s position in the list.

#### 4. **Block Size Calculation**  
   - Uses the `count` subprogram to sum the sizes of mini-blocks in a given block.

#### 5. **Read Function**  
   - Reads the required size from `miniblock->rw_buffer` and displays it character by character.  
   - Iterates through the mini-block list until the end of the text.

#### 6. **Write Function**  
   - Similar to the read function but performs memory allocation and writes the provided information to the buffer.

#### 7. **`pmap` Function**  
   - Iterates through lists, displaying information about blocks and mini-blocks.  
   - At the end, compares and displays permissions.  
   - The `mini` subroutine adds the size of each mini-block list corresponding to each block in the arena.

#### 8. **`mprotect` Function**  
   - Iterates through lists to access each mini-block.  
   - Changes permissions by adding specific values for each permission.  
   - Initializes permissions to `0` to ensure no overflow beyond the maximum value `7 (RWX)`.

---

### Complexity Summary
- Special cases (head/tail operations): **`O(1)`**.  
- General cases (iteration through lists): **`O(n)`**.

This project demonstrates efficient memory management and operations using doubly linked lists tailored to a virtual memory allocator.
