/*
    CS16BTECH11005
    CS16BTECH11011
*/

#ifndef LIBMYMEM_HPP
#define LIBMYMEM_HPP

#include <mutex>

/* Structure to represent a slab */
typedef struct Slab
{
    int totobj;             // Total number of objects in the slab
    int freeobj;            // Number of free objects in the slab
    bool bitmap[32*1024];   // Bitmap to represent which objects are free
    struct Bucket* bucket;  // Points to the bucket to which this slab belongs
    struct Slab* nextslab;  // Points to the next slab
    struct Object* objPtr;  // Pointer to object
} Slab;

typedef struct Bucket
{
    int objSize;                // Object Size
    Slab* firstSlab;            // Points to the first slab
    std::mutex m_mutex;         // Mutex to lock bucket
} Bucket;

typedef struct Object
{
    Slab* slab;         // Points to parent slab
    void* memory;       // Memory location
    Object* nextObj;    // Pointer to next object
} Object;

const int BUCKETS = 13;     // Number of entries in the hash table
Bucket HASH_TABLE[BUCKETS]; // HASH_TABLE[i] corresponds to (2 ^ (i +2))B Bucket

bool isCreated = false;     // Indicates whether slab allocator was created or not

void createSlabAllocator();             // Creates slab allocator
void initializeBucket(int, Bucket*);    // Initializes the bucket
Slab* initializeSlab(Bucket*);          // Initializes the slab

void* mymalloc(unsigned size);
void myfree(void* ptr);

#endif
