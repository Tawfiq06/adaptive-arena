#ifndef MAP_H
#define MAP_H

#include "sprite.h"

#define MAP_WIDTH 20
#define MAP_HEIGHT 15

typedef struct {
    /* Budgets: how much of each category to place*/
    int tree_budget;
    int rock_budget;
    int bush_budget;
    int small_budget;
    int cattail_budget;
    int fern_budget;

    /* Now clusters*/
    /*if > 1 trees/rocks placed in tight groups*/
    // 1 = scattered, 2-4 = cluster
    int tree_cluster_size;
    int rock_cluster_size;

    int prefer_big_trees; // 0 mixed, 1 big trees first, 2 big trees only
    int prefer_big_rocks; //0 mixed, 1 big rocks first, 2 big rocks only

    /* Which decoration variants to use*/
    int use_autumn_trees; //1 = include autumn variants, 2 = only autumn
    int use_stick_trees; // 1 = include stick trees, 2 = only stick trees
    int use_ice_trees;  // 1 = include ice trees, 2 = only ice trees
    int use_grey_rocks; //1 = grey rocks only, 0 = brown only, 2 both
} MapConfig;

//to be indexed by map_index when selecting map
#define NUM_MAPS 4
extern const MapConfig MAP_CONFIGS[NUM_MAPS + 1];

void map_init(int map_index);
SpriteID map_get_tile(int row, int col);
void map_set_tile(int row, int col, SpriteID id);

#endif