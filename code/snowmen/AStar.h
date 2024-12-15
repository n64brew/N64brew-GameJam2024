#ifndef ASTAR_HEADER
#define ASTAR_HEADER

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "../../core.h"
#include "../../minigame.h"

//#include "DynamicArray.h"

//#include "actor.h"

//Locations are normal, distances are squared

struct node;

typedef struct{
    struct node** nodeArray;
    int length;
    int AllocatedLength;
} NodeDynamicArray;


typedef struct node{
  //int16_t v[3];
  NodeDynamicArray neighbors;
  struct node* backConnection;
  T3DVec3 location;
  float G;//true distance travelled till this point
  float H;//estimated distance to the destination from this point
  int id;
} node;

void AStarRun(node* start, node* destination, NodeDynamicArray* path);





//Create
void NodeDA_Create(NodeDynamicArray* NodeDA);

//add
void NodeDA_Add(NodeDynamicArray* NodeDA, struct node* nodeToAdd);

//remove
void NodeDA_Remove(NodeDynamicArray* NodeDA, struct node* nodeToRemove);

//get at index
node* NodeDA_GetAtIndex(NodeDynamicArray* NodeDA, int index);

//get index
int NodeDA_GetIndexOfNode(NodeDynamicArray* NodeDA, struct node* node);

//contains
bool NodeDA_Contains(NodeDynamicArray* NodeDA, struct node* nodeToCheck);

node* NodeDA_GetClosestNode(NodeDynamicArray* AllNodes, T3DVec3 Position);




#endif