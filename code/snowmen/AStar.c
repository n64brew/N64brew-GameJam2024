#include "AStar.h"

void AStarRun(node* start, node* destination, NodeDynamicArray* path)//maybe no need for AllNodes?
{
    NodeDynamicArray SearchList;
    NodeDA_Create(&SearchList);
    NodeDA_Add(&SearchList, start);
    // = start;
    NodeDynamicArray ProcessedList;
    NodeDA_Create(&ProcessedList);
    //Loop
    while(true)//searchList is not empty
    {
        //debugf("weeeeeeeeeeeeeeee\n");
        //Get best node in searchList
        node* current = NodeDA_GetAtIndex(&SearchList, 0);
        for (int i = 1; i < SearchList.length; i++)//for i start at 1 is better than 0 I think
        {
            //best node is the one with the smallest F value
            if ((NodeDA_GetAtIndex(&SearchList, i)->G + NodeDA_GetAtIndex(&SearchList, i)->H) < (current->G + current->H))
            {
                current = NodeDA_GetAtIndex(&SearchList, i);
            }
        }
        //take current node out of search list and put into processed list
        //processed.Add
        NodeDA_Add(&ProcessedList, current);
        //toSearch.remove
        NodeDA_Remove(&SearchList, current);

        if (current == destination)
        {
           //debugf("WE'RE HERE! %d\n", current->id);
            //Reached the end, get path back
            //NodeDynamicArray path;
            NodeDA_Create(path);
            //debugf("uh 1\n");
            //create dynamic array
            node* currentPathNode = destination;
            //debugf("uh 2\n");
            while (currentPathNode != start)
            {
                //debugf("this id: %d\n", currentPathNode->id);
                //Path.Add(currentPathNode)
                NodeDA_Add(path, currentPathNode);
                //debugf("woah 2\n");
                currentPathNode = currentPathNode->backConnection;
                //debugf("woah 3\n");
            }

            free(SearchList.nodeArray);
            free(ProcessedList.nodeArray);
            return; //&path;
        }
        
        for(int i = 0; i < current->neighbors.length; i++)
        {
            //does this neighbor exist in the search list?
            if(NodeDA_Contains(&ProcessedList, NodeDA_GetAtIndex(&current->neighbors, i)))
            {
                continue;
            }
            //does this neighbor exist in the search list?
            bool inSearch = NodeDA_Contains(&SearchList, NodeDA_GetAtIndex(&current->neighbors, i));

            float costToNeighbor = current->G + t3d_vec3_distance2(&current->location, &NodeDA_GetAtIndex(&current->neighbors, i)->location);//current.GetDistanced(current->neighbors[i])!!!!;//Distance^2 between current node and this neighbor node
                //this G value is the current one's + whatever it takes to travel there from current
            if(!inSearch || costToNeighbor < NodeDA_GetAtIndex(&current->neighbors, i)->G)
            {
                //set G and add back connection
                NodeDA_GetAtIndex(&current->neighbors, i)->G = costToNeighbor;
                NodeDA_GetAtIndex(&current->neighbors, i)->backConnection = current;

                if(!inSearch)
                {
                    //add to searchlist and set H
                    NodeDA_Add(&SearchList, NodeDA_GetAtIndex(&current->neighbors, i));
                    NodeDA_GetAtIndex(&current->neighbors, i)->H = t3d_vec3_distance2(&NodeDA_GetAtIndex(&current->neighbors, i)->location, &destination->location);//neighbor.GetDistance(targetNode)!!!!!!!!!!!!!!!!!!//Distance^2 between this neighbor and the target node
                }
            }
        }
        //debugf("new node: %d\n", current->id);
        for (int i = 0; i < SearchList.length; i++)
        {
            //debugf("In search List: %d\n",SearchList.nodeArray[i]->id);
        }
    }
    //Check if current node is destination
        //If current node is destination:
        //Create path from dest to start
    //Current node is in "processed" list, do not search
    //Add all adjecent nodes to search list
    //Find the lowest F values among the search list
    //    If there is a tie, choose one with lowest H cost
    //Chosen node is new current node
    free(SearchList.nodeArray);
    free(ProcessedList.nodeArray);
}



void NodeDA_Create(NodeDynamicArray* NodeDA)
{
    *NodeDA = (NodeDynamicArray){
        .nodeArray = NULL,
        .length = 0,
        .AllocatedLength = 10
    };
    NodeDA->nodeArray = malloc(sizeof(node*) * NodeDA->AllocatedLength);
}

void NodeDA_Add(NodeDynamicArray* NodeDA, struct node* nodeToAdd)
{
    if (NodeDA->length == NodeDA->AllocatedLength)
    {
        NodeDA->AllocatedLength += 10;
        NodeDA->nodeArray = realloc(NodeDA->nodeArray, sizeof(node*) * NodeDA->AllocatedLength);  
    }
    NodeDA->length++;
    NodeDA->nodeArray[NodeDA->length - 1] = nodeToAdd;
}


void NodeDA_Remove(NodeDynamicArray* NodeDA, node* nodeToRemove)
{
    //bruh how
    //maybe, since this is an unordered list, just take last element, put it in removed spot, and reduce length?
    if (NodeDA->length == 1)
    {
        //debugf("Array only has one item, no special stuff.\n");
        NodeDA->nodeArray[0] = NULL;
        NodeDA->length = 0;
        return;
    }
    //debugf("we're gonna remove %d\n", nodeToRemove->id);
    int indexToRemove = NodeDA_GetIndexOfNode(NodeDA, nodeToRemove);
    if (indexToRemove < 0 || indexToRemove >= NodeDA->length)
    {
        debugf("ERROR with index!\n");
        return;
    }
    //debugf("AKA: %d\n", NodeDA->nodeArray[indexToRemove]->id);
    NodeDA->nodeArray[indexToRemove] = NodeDA_GetAtIndex(NodeDA, (NodeDA->length - 1));//replaced node get at index with direct.
    NodeDA->length--;
    //debugf("And in it's place is: %d\n", NodeDA->nodeArray[indexToRemove]->id);
}


node* NodeDA_GetAtIndex(NodeDynamicArray* NodeDA, int index)
{
    return NodeDA->nodeArray[index];
}

int NodeDA_GetIndexOfNode(NodeDynamicArray* NodeDA, node* node)
{
    //this is really naive but it should be fine.
    for (int i = 0; i < NodeDA->length; i++)
    {
        if(NodeDA_GetAtIndex(NodeDA, i) == node)
        {
            return i;
        }
    }
    return -1;
}


bool NodeDA_Contains(NodeDynamicArray* NodeDA, node* nodeToCheck)
{
    //this is really naive but it should be fine.
    for (int i = 0; i < NodeDA->length; i++)
    {
        if(NodeDA_GetAtIndex(NodeDA, i) == nodeToCheck)
        {
            return true;
        }
    }
    return false;
}

node* NodeDA_GetClosestNode(NodeDynamicArray* AllNodes, T3DVec3 Position)//PROBLEM could find one that's closest but behind a wall...
{
    node* currentBest = AllNodes->nodeArray[0];
    T3DVec3 currentBestDiff;
    t3d_vec3_diff(&currentBestDiff, &AllNodes->nodeArray[0]->location, &Position);
    float currentBestMag = t3d_vec3_len2(&currentBestDiff);

    for (int i = 1; i < AllNodes->length; i++)
    {
        T3DVec3 newDiff;
        t3d_vec3_diff(&newDiff, &AllNodes->nodeArray[i]->location, &Position);
        float newMag = t3d_vec3_len2(&newDiff);
        if (newMag < currentBestMag)
        {
            currentBestMag = newMag;
            currentBest = AllNodes->nodeArray[i];
        }
    }



    return currentBest;
}

void NodeDA_Free(NodeDynamicArray* NodeDA)
{
    node **array = NodeDA->nodeArray;
    int length = NodeDA->length;
    NodeDA->nodeArray = NULL;
    NodeDA->length = 0;
    NodeDA->AllocatedLength = 0;

    if (length > 0) {
        for (int i = 0; i < length; i++)
        {
            NodeDA_Free(&array[i]->neighbors);
        }
        free(array);
    }
}
