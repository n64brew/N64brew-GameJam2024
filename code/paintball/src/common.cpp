#include "common.hpp"

int randomRange(int min, int max){
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
};