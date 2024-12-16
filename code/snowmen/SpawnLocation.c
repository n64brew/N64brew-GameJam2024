#include "SpawnLocation.h"



/*void SetSpawnLocations(SpawnLocation locations[10])
{
    //hard code location values
    locations[0] = (SpawnLocation){
        .isOccupied = false,
        .pickupPtr = NULL,
        .Pos = (T3DVec3) {{-188.414597f, 0.f, 17.354418f}}
    };
    locations[1] = (SpawnLocation){
        .isOccupied = false,
        .pickupPtr = NULL,
        .Pos = (T3DVec3) {{190.647125f, 0.f, 14.896403f}}
    };
    locations[2] = (SpawnLocation){
        .isOccupied = false,
        .pickupPtr = NULL,
        .Pos = (T3DVec3) {{0.f, 0.f, 170.798645f}}
    };
    locations[3] = (SpawnLocation){
        .isOccupied = false,
        .pickupPtr = NULL,
        .Pos = (T3DVec3) {{0.f, 0.f, 100.798645f}}
    };
    locations[4] = (SpawnLocation){
        .isOccupied = false,
        .pickupPtr = NULL,
        .Pos = (T3DVec3) {{0.f, 0.f, -15.798645f}}
    };
}*/

SpawnLocation* GetRandomLocation(SpawnLocation locations[6], int seed)
{
    //get location from hard coded random array, must take in a seed of sorts
    //number of times the a button has been pressed? time between presses?
    //add to a number, then % 50 for 50 hard coded random values?
    //then, instead of zero here, use the random value.
    //BUT must first check if location is filled. If not, try next spot.
    //debugf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ All Spawn Locations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    for (int i = 0; i < 6; i++)
    {
        //debugf("%f, %f, %f\n", locations[i].Pos.v[0], locations[i].Pos.v[1], locations[i].Pos.v[2]);
    }
    return &locations[(int) fabs(seed % 6)];
}