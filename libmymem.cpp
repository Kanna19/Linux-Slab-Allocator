#include "libmymem.hpp"
#include <sys/types.h>
#include <sys/mman.h>
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

Slab* initializeSlab(Bucket* bucket)
{
    // Length of the mapping to be created
    int length = 64 * 1024;
    Slab* newSlab = (Slab*)mmap(0, length, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    // Check if mmap was successfull
    if(newSlab == MAP_FAILED)
    {
        std::cerr << "Unable to create a mapped area" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Mark all objects as free
    for(int i = 0; i < length/2; i++)
    {
        newSlab->bitmap[i] = 0;
    }

    /*
    * Set the total number of objects depending on the
    * length of mapped area, slab size, object size
    */

    newSlab->totobj = (length - sizeof(Slab))/(bucket->objSize + sizeof(Object));

    // Set the number of free objects in the slab
    newSlab->freeobj = newSlab->totobj - 1;

    // Set the next slab to NULL
    newSlab->nextslab = NULL;

    // Set the bucket to which this slab belongs to
    newSlab->bucket = bucket;

    // Pointer to first object in the slab
    newSlab->objPtr = (Object*)((char*)(newSlab) + sizeof(Slab));
    newSlab->objPtr->memory = (void*)((char*)newSlab->objPtr + sizeof(Object));
    newSlab->objPtr->slab = newSlab;

    // Initializes the Linked List of Slabs
    Object* temp = newSlab->objPtr;
    for(int i = 0; i < newSlab->freeobj; i++)
    {
        temp->nextObj = (Object*)((char*)temp + bucket->objSize + sizeof(Object));
        temp->nextObj->slab = newSlab;
        temp->nextObj->memory = (void*)((char*)temp->nextObj + sizeof(Object));
        temp = temp->nextObj;
    }

    /* Last element in linked list points to NULL */
    temp->nextObj = NULL;
    return newSlab;
}

void initializeBucket(int size, Bucket* bucket)
{
    // Set the size of the object
    bucket->objSize = size;

    // Initialize the slab
    bucket->firstSlab = initializeSlab(bucket);
}

void createSlabAllocator()
{
    // Initialize the required buckets
    for(int i = 2, j = 0; j < 13; i *= 2, j++)
    {
        initializeBucket(i, &HASH_TABLE[j]);
    }
}

/* Returns NULL if an invalid amount of memory is requested */
void* mymalloc(unsigned size)
{
    // Check if a valid size was provided
    if(size < 1 || size > 8192)
    {
        std::cerr << "Please use a number in the range 1 to 8192" << std::endl;
        return NULL;
    }

    // If slab allocator was not created then, create it
    if(!isCreated)
    {
        createSlabAllocator();
        isCreated = true;
    }

    // Decide which bucket to allocate using the BEST FIT method
    int idx = 0;
    unsigned bucketSize = 2;
    for(; bucketSize < size; bucketSize *= 2, idx++);

    HASH_TABLE[idx].m_mutex.lock();

    // Find the slab in which the object has to be placed
    Slab* curSlab = HASH_TABLE[idx].firstSlab;
    while(curSlab->nextslab != NULL && curSlab->freeobj == 0)
    {
        curSlab = curSlab->nextslab;
    }

    // If there are no free objects then, create a new slab
    if(curSlab->freeobj == 0)
    {
        curSlab->nextslab = initializeSlab(&HASH_TABLE[idx]);
        curSlab = curSlab->nextslab;
    }

    // Select the position in which the object has to be placed
    int offset = 0;
    Object* cur = curSlab->objPtr;
    while(curSlab->bitmap[offset] != 0)
    {
        cur = cur->nextObj;
        offset++;
    }

    // Mark this object as not free
    curSlab->bitmap[offset] = 1;

    // Decrease the number of free objects
    curSlab->freeobj--;

    HASH_TABLE[idx].m_mutex.unlock();
    // Return the allocated memory address
    return cur->memory;
}

void myfree(void* mem)
{
    if(mem == NULL)
    {
        std::cerr << "Please use a valid address" << std::endl;
        return;
    }

    Object* obj = (Object*)((char*)mem - sizeof(Object));

    Slab* slab = obj->slab;

    slab->bucket->m_mutex.lock();

    Object* curObj = slab->objPtr;

    int idx = 0;
    for(; curObj != obj; idx++)
    {
        curObj = curObj->nextObj;
    }

    slab->bitmap[idx] = 0;
    slab->freeobj++;

    slab->bucket->m_mutex.unlock();
}
