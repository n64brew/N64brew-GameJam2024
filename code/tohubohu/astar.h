// Freely inspired by Sean Heber's A* implementation: https://github.com/BigZaphod/AStar

#ifndef __ASTAR_H
#define __ASTAR_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    int x;
    int y;
} cell_t;

typedef struct {
    size_t capacity;
    size_t count;
    float* costs;
    cell_t* cells;
} node_list_t;

typedef struct {
    bool closed;
    bool open;
    bool target;
    float estimated_cost;
    float cost;
    size_t open_idx;
    size_t parent_idx;
    cell_t cell;
} node_record_t;

typedef struct {
    size_t records_capacity;
    size_t records_count;
    node_record_t* records;
    size_t* records_indexes;
    size_t open_capacity;
    size_t open_count;
    size_t* open;
} visited_nodes_t;

typedef struct {
    visited_nodes_t* nodes;
    size_t index;
} node_t;

typedef struct {
    size_t capacity;
    size_t count;
    float cost;
    bool incomplete;
    cell_t cells[];
} path_t;

const node_t empty_node = {NULL, -1};


// defined by caller
extern void add_neighbours(node_list_t* list, cell_t cell);
extern float heuristic(cell_t from, cell_t to);


node_t make_node(visited_nodes_t* nodes, size_t index) {
    return (node_t){nodes, index};
}

bool is_empty(node_t n) {
    return (n.nodes == empty_node.nodes) && (n.index == empty_node.index);
}

node_record_t* get_record(node_t node) {
    return &node.nodes->records[node.index];
}

float get_rank(node_t node) {
    node_record_t* record = get_record(node);
    return record->estimated_cost + record->cost;
}

int has_estimated_cost(node_t node) {
    return get_record(node)->estimated_cost != -1;
}

void set_target(node_t node) {
    if (!is_empty(node)) {
        get_record(node)->target = true;
    }
}

int is_target(node_t node) {
    return !is_empty(node) && get_record(node)->target;
}

node_t get_parent(node_t node) {
    node_record_t* record = get_record(node);
    if (record->parent_idx != -1) {
        return make_node(node.nodes, record->parent_idx);
    } else {
        return empty_node;
    }
}

int compare_rank(node_t n1, node_t n2) {
    const float rank1 = get_rank(n1);
    const float rank2 = get_rank(n2);
    return (rank1 < rank2) ? -1 : ((rank1 > rank2) ? 1 : 0);
}

float estimate_cost(node_t n1, node_t n2) {
    if (!is_empty(n1) && !is_empty(n2)) {
        return heuristic(get_record(n1)->cell, get_record(n2)->cell);
    } else {
        return 0;
    }
}

int compare_nodes(node_t node, cell_t n) {
    return memcmp(&get_record(node)->cell, &n, sizeof(cell_t));
}

node_t get_node(visited_nodes_t* nodes, cell_t n) {
    size_t first = 0;
    if (nodes->records_count > 0) {
        size_t last = nodes->records_count-1;
        while (first <= last) {
            const size_t mid = (first + last) / 2;
            const int comp = compare_nodes(make_node(nodes, nodes->records_indexes[mid]), n);
            if (comp < 0) {
                first = mid + 1;
            } else if (comp > 0 && mid > 0) {
                last = mid - 1;
            } else if (comp > 0) {
                break;
            } else {
                return make_node(nodes, nodes->records_indexes[mid]);
            }
        }
    }
    
    if (nodes->records_count == nodes->records_capacity) {
        nodes->records_capacity = 1 + (nodes->records_capacity * 2);
        nodes->records = realloc(nodes->records, nodes->records_capacity * sizeof(node_record_t));
        nodes->records_indexes = realloc(nodes->records_indexes, nodes->records_capacity * sizeof(size_t));
    }
    
    node_t node = make_node(nodes, nodes->records_count);
    nodes->records_count++;
    
    memmove(&nodes->records_indexes[first+1], &nodes->records_indexes[first], (nodes->records_capacity - first - 1) * sizeof(size_t));
    nodes->records_indexes[first] = node.index;
    
    node_record_t* record = get_record(node);
    memset(record, 0, sizeof(node_record_t));
    record->estimated_cost = -1;
    record->parent_idx = -1;
    memcpy(&record->cell, &n, sizeof(cell_t));

    return node;
}

void swap(visited_nodes_t* nodes, size_t index1, size_t index2) {
    if (index1 != index2) {
        node_record_t* record1 = get_record(make_node(nodes, nodes->open[index1]));
        node_record_t* record2 = get_record(make_node(nodes, nodes->open[index2]));
        
        size_t tmp = record1->open_idx;
        record1->open_idx = record2->open_idx;
        record2->open_idx = tmp;
        
        tmp = nodes->open[index1];
        nodes->open[index1] = nodes->open[index2];
        nodes->open[index2] = tmp;
    }
}

void balance_after_remove(visited_nodes_t* nodes, size_t index) {
    size_t smallestIndex = index;
    
    do {
        if (smallestIndex != index) {
            swap(nodes, smallestIndex, index);
            index = smallestIndex;
        }

        const size_t leftIndex = (2 * index) + 1;
        const size_t rightIndex = (2 * index) + 2;
        
        if (leftIndex < nodes->open_count && compare_rank(make_node(nodes, nodes->open[leftIndex]), make_node(nodes, nodes->open[smallestIndex])) < 0) {
            smallestIndex = leftIndex;
        }
        
        if (rightIndex < nodes->open_count && compare_rank(make_node(nodes, nodes->open[rightIndex]), make_node(nodes, nodes->open[smallestIndex])) < 0) {
            smallestIndex = rightIndex;
        }
    } while (smallestIndex != index);
}

void remove_from_open(node_t node) {
    node_record_t* record = get_record(node);

    if (record->open) {
        record->open = false;
        node.nodes->open_count--;
        const size_t index = record->open_idx;
        swap(node.nodes, index, node.nodes->open_count);
        balance_after_remove(node.nodes, index);
    }
}

void balance_after_insert(visited_nodes_t* nodes, size_t index) {
    while (index > 0) {
        const size_t parent = floorf((index-1) / 2);
        if (compare_rank(make_node(nodes, nodes->open[parent]), make_node(nodes, nodes->open[index])) < 0) {
            break;
        } else {
            swap(nodes, parent, index);
            index = parent;
        }
    }
}

void add_to_open(node_t node, float cost, node_t parent) {
    node_record_t* record = get_record(node);

    if (!is_empty(parent)) {
        record->parent_idx = parent.index;
    } else {
        record->parent_idx = -1;
    }

    if (node.nodes->open_count == node.nodes->open_capacity) {
        node.nodes->open_capacity = 1 + (node.nodes->open_capacity * 2);
        node.nodes->open = realloc(node.nodes->open, node.nodes->open_capacity * sizeof(size_t));
    }

    const size_t open_idx = node.nodes->open_count;
    node.nodes->open[open_idx] = node.index;
    node.nodes->open_count++;

    record->open_idx = open_idx;
    record->open = true;
    record->cost = cost;

    balance_after_insert(node.nodes, open_idx);
}

void add_neighbour(node_list_t* list, cell_t cell, float edgeCost) {
    if (list->count == list->capacity) {
        list->capacity = 1 + (list->capacity * 2);
        list->costs = realloc(list->costs, sizeof(float) * list->capacity);
        list->cells = realloc(list->cells, sizeof(cell_t) * list->capacity);
    }
    list->costs[list->count] = edgeCost;
    memcpy(list->cells + list->count, &cell, sizeof(cell_t));
    list->count++;
}

path_t* find_path(cell_t start, cell_t target, int max_cost, int max_visit) {
    visited_nodes_t* visited_nodes = calloc(1, sizeof(visited_nodes_t));
    node_list_t* neighbours = calloc(1, sizeof(node_list_t));
    node_t current = get_node(visited_nodes, start);
    node_t goal = get_node(visited_nodes, target);
    node_t best = current;
    int remaining = max_visit;
    path_t* path = NULL;
    
    set_target(goal);
    get_record(current)->estimated_cost = estimate_cost(current, goal);
    add_to_open(current, 0, empty_node);

    while ((max_visit == -1 || remaining-- > 0) && visited_nodes->open_count > 0 && !is_target((current = make_node(visited_nodes, visited_nodes->open[0])))) {
        remove_from_open(current);
        get_record(current)->closed = true;
        
        neighbours->count = 0;
        add_neighbours(neighbours, get_record(current)->cell);

        for (size_t n=0; n<neighbours->count; n++) {
            const float cost = get_record(current)->cost + neighbours->costs[n];
            node_t neighbour = get_node(visited_nodes, neighbours->cells[n]);
            float neighbour_estimated_cost = estimate_cost(neighbour, goal);

            // Keep track of the cheapest path
            if (cost + neighbour_estimated_cost <= get_record(best)->cost + get_record(best)->estimated_cost) {
                if (neighbour_estimated_cost < get_record(best)->estimated_cost) {
                    best = neighbour;
                }
            }

            // Stop as soon as the best path reached the targeted cost
            if (max_cost != -1 && get_record(best)->cost >= max_cost) {
                break;
            }
            
            if (!has_estimated_cost(neighbour)) {
                get_record(neighbour)->estimated_cost = neighbour_estimated_cost;
            }
            
            if (get_record(neighbour)->open && cost < get_record(neighbour)->cost) {
                remove_from_open(neighbour);
            }
            
            if (get_record(neighbour)->closed && cost < get_record(neighbour)->cost) {
                get_record(neighbour)->closed = false;
            }
            
            if (!get_record(neighbour)->open && !get_record(neighbour)->closed) {
                add_to_open(neighbour, cost, current);
            }
        }
    }

    // If goal is unreachable, return cheapest path to the closest cell
    if (!is_target(current)) {
        set_target(best);
        current = best;
    }
    
    if (is_empty(goal)) {
        set_target(current);
    }
    
    if (is_target(current)) {
        size_t count = 0;
        node_t n = current;
        
        while (!is_empty(n)) {
            count++;
            n = get_parent(n);
        }
        
        path = malloc(sizeof(path_t) + (count * sizeof(cell_t)));
        path->count = count;
        path->cost = get_record(current)->cost;
        cell_t best_cell = get_record(best)->cell;
        path->incomplete = (target.x != best_cell.x || target.y != best_cell.y);

        n = current;
        for (size_t i=count; i>0; i--) {
            cell_t v = get_record(n)->cell;
            memcpy(path->cells + (i - 1), &v, sizeof(cell_t));
            n = get_parent(n);
        }
    }
    
    free(neighbours->costs);
    free(neighbours->cells);
    free(neighbours);
    free(visited_nodes->records_indexes);
    free(visited_nodes->records);
    free(visited_nodes->open);
    free(visited_nodes);

    return path;
}

void free_path(path_t* path) {
    free(path);
}

size_t get_path_count(path_t* path) {
    return path ? path->count : 0;
}

bool get_path_complete(path_t* path) {
    return path ? !path->incomplete : false;
}

cell_t* get_path_cell(path_t* path, size_t index) {
    return (path && index < path->count) ? (path->cells + index) : NULL;
}

#endif
