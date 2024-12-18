/*
===============================================================================
AF_QUADTREE_H
Author: jhall.develop

Description:
This header file implements a quadtree structure, which is useful for spatial 
partitioning in various applications such as broad-phase and narrow-phase 
collision detection in physics simulations. The quadtree allows efficient 
management of objects within a two-dimensional space by subdividing it into 
quadrants, enabling faster queries and updates on object positions and 
interactions.

Key Features:
- QuadTree_Entry: Represents a single entry in the quadtree, holding the 
  position, size, and an associated object pointer.
- QuadTree_Node: Represents a node in the quadtree, containing multiple 
  entries and pointers to child nodes.
- Functions for creating, deleting, and operating on quadtree nodes and 
  entries, facilitating dynamic spatial organization of objects.
Some code inspired by https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/physicstutorials
===============================================================================
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "AF_Vec3.h"
#include "AF_Vec2.h"



/*
====================
QuadTree_Entry
Struct to hold a single entry in the quadtree.

This structure stores the position, size, and a pointer to an associated object
for each entry in the quadtree, enabling efficient spatial partitioning and 
object management.
====================
*/
typedef struct QuadTree_Entry {
    Vec3 pos;         // Position of the entry
    Vec3 size;        // Size of the entry
    void* object;     // Pointer to the associated object
} QuadTree_Entry;

/*
====================
QuadTree_Node
Struct to hold a node in the quadtree.

This structure represents a node in the quadtree, which can contain multiple entries 
and has pointers to its child nodes. The node is defined by its position and size 
in 2D space, along with an array of pointers to its contents.
====================
*/
typedef struct QuadTree_Node {
    QuadTree_Entry** contents;          // Array of pointers to entries contained in this node
    int entryCount;                     // Number of entries in this node
    Vec2 position;                      // Position of the node in 2D space
    Vec2 size;                          // Size of the node
    struct QuadTree_Node* children[4];  // Array of pointers to child nodes (4 for quadtree)
} QuadTree_Node;


// Forward declaration of QuadTreeNode
//typedef struct QuadTreeNode QuadTreeNode;


// Structure representing a QuadTree
typedef struct QuadTree {
    QuadTree_Node* root;  // Pointer to the root node of the quadtree
    int maxDepth;        // Maximum depth of the quadtree
    int maxSize;         // Maximum size of entries in each node
} QuadTree;

/*
====================
AF_QuadTree_CreateQuadTreeEntry
Function to create a new quadtree entry.

This function allocates memory for a new QuadTree_Entry and initializes its 
members with the provided object, position, and size. If memory allocation fails, 
it prints an error message and returns NULL.

Parameters:
    _obj - A pointer to the object to associate with the entry.
    _pos - A Vec3 structure representing the position of the entry.
    _size - A Vec3 structure representing the size of the entry.

Returns:
    A pointer to the newly created QuadTree_Entry, or NULL if allocation fails.
====================
*/
static QuadTree_Entry* AF_QuadTree_CreateQuadTreeEntry(void* _obj, Vec3 _pos, Vec3 _size) {
    // Allocate memory for a new QuadTree_Entry
    QuadTree_Entry* entry = (QuadTree_Entry*)malloc(sizeof(QuadTree_Entry));

    // Check if memory allocation was successful
    if (entry == NULL) {
        perror("AF_QuadTree_CreateQuadTreeEntry: Failed to allocate memory for QuadTreeEntry\n");
        return NULL; // Return NULL if allocation fails
    }

    // Initialize the properties of the new entry
    entry->object = _obj; // Associate the provided object with this entry
    entry->pos = _pos;     // Set the position of the entry
    entry->size = _size;   // Set the size of the entry

    return entry; // Return the pointer to the newly created QuadTree_Entry
}


// Define a function pointer for type of processing lists of entries in a quadtree
typedef void (*QuadTreeFunc)(QuadTree_Entry** entries, int entryCount);

/*
====================
AF_QuadTree_CreateQuadTreeNode

Allocates and initializes a new QuadTree_Node with the specified position 
and size. The function sets the initial contents to NULL and the entry 
count to zero, indicating that the node is empty upon creation. 

Parameters:
    _pos - A Vec2 structure representing the position of the new QuadTree node.
    _size - A Vec2 structure representing the size of the new QuadTree node.

Returns:
    A pointer to the newly created QuadTree_Node, or NULL if memory allocation fails.
====================
*/
static QuadTree_Node* AF_QuadTree_CreateQuadTreeNode(Vec2 _pos, Vec2 _size) {
    // Allocate memory for the new QuadTree_Node
    QuadTree_Node* node = (QuadTree_Node*)malloc(sizeof(QuadTree_Node));

    // Check if memory allocation was successful
    if (node == NULL) {
        perror("AF_QuadTree_CreateQuadTreeNode: Failed to allocate memory for QuadTreeNode\n");
        return NULL; // Return NULL if allocation failed
    }

    // Initialize the properties of the new node
    node->position = _pos;         // Set the position of the node
    node->size = _size;           // Set the size of the node
    node->contents = NULL;        // Initialize contents pointer to NULL (indicating no contents)
    node->entryCount = 0;         // Set the initial entry count to zero (no entries yet)
    // node->children = NULL;      // Optionally, initialize children pointer to NULL (not used here)

    return node; // Return the pointer to the newly created QuadTree_Node
}


/*
====================
AF_QuadTree_DeleteQuadTreeNode

Recursively deletes a QuadTreeNode and all its child nodes, freeing any 
associated memory. This function first frees the contents stored in the 
node, then checks for child nodes and recursively deletes them if they exist. 
Finally, it frees the memory allocated to the node itself.

Parameters:
    _node - Pointer to the QuadTree_Node to be deleted. This function will 
            recursively delete all children if the node has any.
====================
*/
static void AF_QuadTree_DeleteQuadTreeNode(QuadTree_Node* _node) {
    // Check if the node is NULL before attempting deletion
    if (!_node) {
        return;
    }

    // Free each individual entry in the node's contents array
    for (int i = 0; i < _node->entryCount; i++) {
        free(_node->contents[i]); // Free memory for each content entry
    }
    free(_node->contents); // Free the memory allocated for the contents array itself

    // Recursively delete each child node if children exist
    //if (_node->children != NULL) {
        for (int i = 0; i < 4; i++) {
            AF_QuadTree_DeleteQuadTreeNode(_node->children[i]); // Recursively delete each child node
        }
        free(_node->children); // Free the memory allocated for the children array after all are deleted
    //}

    // Finally, free the memory allocated for the node itself
    free(_node);
}


/*
====================
AF_QuadTree_OperateOnNodeContents

Recursively traverses the quadtree from the given node, applying a function 
to the contents of each leaf node (nodes without children). If the current 
node has children, the function is called recursively on each child. If the 
node has no children, the function operates directly on the node’s contents.

Parameters:
    _node - Pointer to the QuadTree_Node to be processed. 
            If this node has children, the function will operate on each child.
    _func - Function pointer of type QuadTreeFunc, which takes the contents
            and entry count of a node as arguments.
====================
*/
static void AF_QuadTree_OperateOnNodeContents(QuadTree_Node* _node, QuadTreeFunc _func) {
    // Check if the current node has children
    if (_node->children[0] != NULL) {
        // Iterate through each child of the current node
        for (int i = 0; i < 4; ++i) {
            if (_node->children[i] != NULL) {
                // Recursively apply the function on each child node's contents
                AF_QuadTree_OperateOnNodeContents(_node->children[i], _func);
            }
        }
    } else {
        // If the node has no children, it is a leaf node with contents to process
        if (_node->contents > 0) {
            // Apply the provided function to the node's contents and entry count
            _func(_node->contents, _node->entryCount);
        }
    }
}


/*
====================
AF_QuadTree_Split

Splits a given quadtree node into four child nodes, dividing its size in half
and positioning each child in one of the four quadrants (top-left, top-right,
bottom-left, bottom-right) relative to the original node. Memory is dynamically
allocated for each child node, and their positions are calculated accordingly.

Parameters:
    _node - Pointer to the QuadTree_Node to be split. This node will have
            four children after this function is called.
====================
*/
static void AF_QuadTree_Split(QuadTree_Node* _node) {
    // Calculate half of the current node's size for positioning child nodes
    Vec2 halfSize = {_node->size.x / 2.0f, _node->size.y / 2.0f};
    
    // Allocate memory for each of the 4 child nodes
    for(int i = 0; i < 4; ++i) {
        _node->children[i] = (QuadTree_Node*)malloc(sizeof(QuadTree_Node));
    }

    // Initialize each child node with appropriate position and size

    // Child 0: Top-left quadrant, offset by -halfSize.x and +halfSize.y
    Vec2 halfSizeAdj = {-halfSize.x, halfSize.y};
    _node->children[0]->position = Vec2_ADD(_node->position, halfSizeAdj);
    _node->children[0]->size = halfSize;

    // Child 1: Top-right quadrant, offset by +halfSize.x and +halfSize.y
    _node->children[1]->position = Vec2_ADD(_node->position, halfSize);
    _node->children[1]->size = halfSize;

    // Child 2: Bottom-left quadrant, offset by -halfSize.x and -halfSize.y
    _node->children[2]->position = Vec2_ADD(_node->position, (Vec2){-halfSize.x, -halfSize.y});
    _node->children[2]->size = halfSize;

    // Child 3: Bottom-right quadrant, offset by +halfSize.x and -halfSize.y
    _node->children[3]->position = Vec2_ADD(_node->position, (Vec2){halfSize.x, -halfSize.y});
    _node->children[3]->size = halfSize;
}


// Function prototype for the collision detection test
bool AABBTest(const Vec3* objectPos, const Vec3* nodePos, 
              const Vec3* objectSize, const Vec3* nodeSize);

// Function to insert an object into the quadtree
static void AF_QuadTree_Insert(QuadTree_Node* _node, void* _object, const Vec3* objectPos, 
            const Vec3* objectSize, int depthLeft, int maxSize) {
    
    // Check if the object's bounding box intersects with the _node's bounding box
    if (!AABBTest(objectPos, &(Vec3){_node->position.x, 0, _node->position.y}, 
                  objectSize, &(Vec3){_node->size.x, 1000.0f, _node->size.y})) {
        return; // Exit if there is no collision
    }

    //if (_node->children) { // Not a leaf _node; descend the tree
        for (int i = 0; i < 4; ++i) {
            AF_QuadTree_Insert(_node->children[i], _object, objectPos, objectSize, depthLeft - 1, maxSize);
        }
    //} else { // Currently a leaf _node; can add the entry
        // Allocate memory for the new QuadTreeEntry and add it to contents
        QuadTree_Entry quadTreeEntry = {*objectPos, *objectSize, _object};
        *_node->contents[_node->entryCount++] = quadTreeEntry;

        // Check if the number of contents exceeds the max size
        if (_node->entryCount > maxSize && depthLeft > 0) {
            //if (!_node->children) {
                AF_QuadTree_Split(_node); // Split the _node into child _nodes
                
                // Reinsert the contents into the appropriate child _nodes
                for (int i = 0; i < _node->entryCount; ++i) {
                    QuadTree_Entry entry = *_node->contents[i];
                    for (int j = 0; j < 4; ++j) {
                        AF_QuadTree_Insert(_node->children[j], entry.object, &entry.pos, &entry.size, depthLeft - 1, maxSize);
                    }
                }
                // Clear contents as they have been distributed to children
                _node->entryCount = 0; 
            }
        //}
    //}
}

// Function to create a new QuadTree
static QuadTree* QuadTree_Create(Vec2 _size, int maxDepth, int _maxSize) {
    QuadTree* tree = (QuadTree*)malloc(sizeof(QuadTree));
    if (tree == NULL) {
        fprintf(stderr, "Memory allocation failed for QuadTree\n");
        return NULL;
    }
    
    tree->root = (QuadTree_Node*)malloc(sizeof(QuadTree_Node));
    if (tree->root == NULL) {
        fprintf(stderr, "Memory allocation failed for QuadTreeNode\n");
        free(tree);
        return NULL;
    }
    
    // Initialize root node
    Vec2 quadTreePosition = {0,0};
    tree->root->position = quadTreePosition; // Assuming Vec2 has appropriate member initialization
    tree->root->size = _size;
    tree->root->entryCount = 0;
    tree->root->contents = NULL; // Initialize contents as needed
    for (int i = 0; i < 4; i++) {
        tree->root->children[i] = NULL; // Initialize children pointers to NULL
    }

    tree->maxDepth = maxDepth;
    tree->maxSize = _maxSize;
    return tree;
}