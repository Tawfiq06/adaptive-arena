#include "map_evolution.h"

#include "map.h"
#include "decorations.h"
#include "obstacle_map.h"
#include "tile_sprites.h"
#include "renderer.h"
#include "vga.h"
#include <stdlib.h>


/* Part 1: Dirty tile queue*/
//remember there is a double buffer

TileRedraw tile_redraws[TILE_REDRAW_CAP];
int tile_redraw_count = 0;

//add tile to pending redraw list
void tile_redraw_enqueue (int row, int col){
    //if already queue, make sure both buffers are marked
    for(int i = 0; i < tile_redraw_count; i++){
        if(tile_redraws[i].row == row && tile_redraws[i].col == col){
            tile_redraws[i].draw_b0 = 1;
            tile_redraws[i].draw_b1 = 1;
            return;
        }
    }
    if(tile_redraw_count >= TILE_REDRAW_CAP) return;

    tile_redraws[tile_redraw_count].row = (short) row;
    tile_redraws[tile_redraw_count].col = (short) col;
    tile_redraws[tile_redraw_count].draw_b0 = 1;
    tile_redraws[tile_redraw_count].draw_b1 = 1;
    tile_redraw_count++;
}

//pending deco type-change redraws

int deco_redraw_count = 0;

void deco_redraw_enqueue(int idx){
    const DecoType *old_dt = &DECO_LOOKUP[decorations[idx].type];
    short ec0 = (short) (decorations[idx].x >> 4);
    short er0 = (short) (decorations[idx].y >> 4);
    short ec1 = (short) (((decorations[idx].x + old_dt->w - 1) >> 4) + 1);
    short er1 = (short) (((decorations[idx].y + old_dt->h - 1) >> 4) + 1);

    for (int i = 0; i < deco_redraw_count; i++){
        if (deco_redraws[i].deco_idx == idx){
            /* Already queued — expand erase box to union with previous old box */
            if (ec0 < deco_redraws[i].erase_col0) deco_redraws[i].erase_col0 = ec0;
            if (er0 < deco_redraws[i].erase_row0) deco_redraws[i].erase_row0 = er0;
            if (ec1 > deco_redraws[i].erase_col1) deco_redraws[i].erase_col1 = ec1;
            if (er1 > deco_redraws[i].erase_row1) deco_redraws[i].erase_row1 = er1;
            deco_redraws[i].draw_b0 = 1;
            deco_redraws[i].draw_b1 = 1;
            return;
        }
    }
    if(deco_redraw_count >= DECO_REDRAW_CAP) return;
    
    deco_redraws[deco_redraw_count].deco_idx   = idx;
    deco_redraws[deco_redraw_count].draw_b0    = 1;
    deco_redraws[deco_redraw_count].draw_b1    = 1;
    deco_redraws[deco_redraw_count].erase_col0 = ec0;
    deco_redraws[deco_redraw_count].erase_row0 = er0;
    deco_redraws[deco_redraw_count].erase_col1 = ec1;
    deco_redraws[deco_redraw_count].erase_row1 = er1;
    deco_redraw_count++;
}


/* Map Evolution Draw */

void map_evolution_draw(int cur_buf){
    //tile redraws

    int new_count = 0;
    for(int i = 0; i < tile_redraw_count; i++){
        TileRedraw *tr = &tile_redraws[i];

        if((cur_buf == 0 && tr->draw_b0) || (cur_buf == 1 && tr->draw_b1)){
            int r = tr->row;
            int c = tr->col;
            SpriteID id = map_get_tile(r,c);

            //repaint the tile itself
            draw_sprite(&sprites[id], c << 4, r << 4, 0, 0);

            //repaint decoration
            decoration_redraw_region(r, c, r + 1, c + 1);

            if(cur_buf == 0) tr->draw_b0 = 0;
            else tr->draw_b1 = 0;
        }

        if(tr->draw_b0 || tr->draw_b1)
            tile_redraws[new_count++] = *tr;
    }
    tile_redraw_count = new_count;

    //deco type-chane redraw
    new_count = 0;
    for(int i = 0; i < deco_redraw_count; i++){
        DecoRedraw *dr = &deco_redraws[i];

        if((cur_buf == 0 && dr->draw_b0) || (cur_buf == 1 && dr->draw_b1)){
            int idx = dr->deco_idx;

            //need to erase using old bounding box
            for(int rr = dr->erase_row0; rr < dr->erase_row1 && rr < MAP_HEIGHT; rr++){
                for(int cc = dr->erase_col0; cc < dr->erase_col1 && cc < MAP_WIDTH; cc++){
                     draw_sprite(&sprites[map_get_tile(rr, cc)], cc << 4, rr << 4, 0, 0);
                }
            }

            //now draw new sprite
            const DecoType *dt = &DECO_LOOKUP[decorations[idx].type];

            //now we can draw the decoration
            Sprite s = { dt->w, dt->h, dt->data};
            draw_sprite(&s, decorations[idx].x, decorations[idx].y, 0, 0);

            if(cur_buf == 0) 
                dr->draw_b0 = 0;
            else 
                dr->draw_b1 = 0;
        }

        if(dr->draw_b0 || dr->draw_b1)
            deco_redraws[new_count++] = *dr;
    }
    deco_redraw_count = new_count;
}

/* Now helpers for all the map evolutions type*/

void map_change_tile (int row, int col, SpriteID new_id, unsigned char new_flags){
    if(row < 0 || row >= MAP_HEIGHT || col < 0 || col >= MAP_WIDTH) return;

    SpriteID old_id = map_get_tile(row, col);
    if(old_id == new_id) return;

    map_set_tile(row, col, new_id);

    //update obstacle flags
    if (old_id == SPRITE_TILE_WATER)
        obstacle_map_clear(row, col, TILE_FLAG_SLOW);
    if(old_id == SPRITE_TILE_LAVA)
        obstacle_map_clear(row, col, TILE_FLAG_DAMAGE);

    if(new_flags)
        obstacle_map_set(row, col, new_flags);

    tile_redraw_enqueue(row, col);
}

/*deco type change */

void deco_change_type(int idx, int new_type){
    if (idx < 0 || idx >= deco_count) return;

    int old_type = decorations[idx].type;
    if(old_type == new_type) return;

    const DecoType *old_dt = &DECO_LOOKUP[old_type];
    const DecoType *new_dt = &DECO_LOOKUP[new_type];

    //obstacle map, remove old solid footprint
    if(deco_is_solid(old_type)){
        int base_row = (decorations[idx].y + old_dt->h - 1) >> 4;
        int cl = decorations[idx].x >> 4;
        int cr = (decorations[idx].x + old_dt->w - 1) >> 4;

        for(int c = cl; c <= cr; c++)
            obstacle_map_clear(base_row, c, TILE_FLAG_SOLID);
    }

    //apply new type now
    decorations[idx].type = new_type;

    /* now stamp new flags into obstacle map*/
    if(deco_is_solid(new_type)){
        int base_row = (decorations[idx].y + new_dt->h - 1) >> 4;
        int cl       =  decorations[idx].x >> 4;
        int cr       = (decorations[idx].x + new_dt->w - 1) >> 4;
        for (int c = cl; c <= cr; c++)
            obstacle_map_set(base_row, c, TILE_FLAG_SOLID);
    }

    //need to rebuld canopy list
    deco_canopy_rebuild();

    deco_redraw_enqueue(idx);
}


/* burn table, what each deco maps to when it burns */

int deco_burn_target(int type){
    switch(type){
        case DECO_GREEN_TREE_LG:    return DECO_STICK_TREE_LG;
        case DECO_AUTUMN_TREE_RED_LG:       return DECO_STICK_TREE_LG;
        case DECO_AUTUMN_TREE_YELLOW_LG:    return DECO_STICK_TREE_LG;
        case DECO_TREE_GREEN_B:             return DECO_STICK_TREE_B;
        case DECO_TREE_GREEN_A:             return DECO_STICK_TREE_MED;
        case DECO_AUTUMN_TREE_RED_MED:      return DECO_STICK_TREE_MED;
        case DECO_AUTUMN_TREE_YELLOW_MED:   return DECO_STICK_TREE_MED;
        case DECO_GREEN_PLANT_TREE_A:       return DECO_STICK_TREE_SM;
        case DECO_AUTUMN_TREE_RED_SM:       return DECO_STICK_TREE_SM;
        case DECO_AUTUMN_TREE_YELLOW_SM:    return DECO_STICK_TREE_SM;
        default: return -1;
    }
}

/* frozen table, what each deco maps to when it freezes*/
int deco_ice_target(int type){
    switch (type) {
        case DECO_GREEN_TREE_LG:            return DECO_ICE_TREE_LG;
        case DECO_AUTUMN_TREE_RED_LG:       return DECO_ICE_TREE_LG;
        case DECO_AUTUMN_TREE_YELLOW_LG:    return DECO_ICE_TREE_LG;
        case DECO_TREE_GREEN_B:             return DECO_ICE_TREE_MED; /* no ice B variant */
        case DECO_TREE_GREEN_A:             return DECO_ICE_TREE_MED;
        case DECO_AUTUMN_TREE_RED_MED:      return DECO_ICE_TREE_MED;
        case DECO_AUTUMN_TREE_YELLOW_MED:   return DECO_ICE_TREE_MED;
        case DECO_GREEN_PLANT_TREE_A:       return DECO_ICE_TREE_SM;
        case DECO_AUTUMN_TREE_RED_SM:       return DECO_ICE_TREE_SM;
        case DECO_AUTUMN_TREE_YELLOW_SM:    return DECO_ICE_TREE_SM;
        default: return -1;
        }
}


/* Per-Map evolution State */

#define MAP_EVO_NONE 0
#define MAP_EVO_NONE    0
#define MAP_EVO_WETLAND 1   /* lava creep + trees burn */
#define MAP_EVO_BEACH   2   /* water -> ice            */
#define MAP_EVO_ICE     3   /* lava appears randomly   */
#define MAP_EVO_ROCKY   4   /* flooding                */

int current_map_evo = MAP_EVO_NONE;
 
/* Shared timers — reset on map_evolution_init */
#define EVO_TILE_INTERVAL  90   /* ~1.5 s between each tile change */
#define EVO_DECO_INTERVAL  120  /* ~2 s between each deco change   */

int evo_tile_timer = 0;
int evo_deco_timer = 0;

/* WETLAND map evolution*/

//lava spreads cell-automation style, trees next to lava burn

#define LAVA_MAX 60
typedef struct{
    short row, col;
} LavaCell;

LavaCell lava_cells[LAVA_MAX];
int lava_count = 0;

#define BURN_QUEUE_MAX 20
typedef struct { 
    int deco_idx; 
    int burn_delay; 
} BurnEntry;

BurnEntry burn_queue[BURN_QUEUE_MAX];
int burn_queue_count = 0;
 
void wetland_seed_lava(void) {
    int seeds = 1 + (rand() % 3);
    for (int s = 0; s < seeds && lava_count < LAVA_MAX; s++) {
        for (int attempt = 0; attempt < 200; attempt++) {
            int r = 2 + rand() % (MAP_HEIGHT - 4);
            int c = 2 + rand() % (MAP_WIDTH  - 4);
            SpriteID t = map_get_tile(r, c);
            if (t != SPRITE_TILE_GRASS && t != SPRITE_TILE_DIRT) continue;
            map_change_tile(r, c, SPRITE_TILE_LAVA, TILE_FLAG_DAMAGE);
            lava_cells[lava_count].row = (short)r;
            lava_cells[lava_count].col = (short)c;
            lava_count++;
            break;
        }
    }
}
 
void queue_nearby_tree_burns(int lava_row, int lava_col) {
    for (int i = 0; i < deco_count; i++) {
        if(deco_burn_target(decorations[i].type) == -1) continue;
 
        int already = 0;
        for (int q = 0; q < burn_queue_count; q++)
            if (burn_queue[q].deco_idx == i) { already = 1; break; }
        if (already) continue;
        
        int dtype = decorations[i].type;
        int deco_col = (decorations[i].x + DECO_LOOKUP[dtype].w / 2) >> 4;
        int deco_row = (decorations[i].y + DECO_LOOKUP[dtype].h - 1) >> 4;

        int dist_r   = deco_row - lava_row; 
        if (dist_r < 0) 
            dist_r = -dist_r;

        int dist_c   = deco_col - lava_col; 
        if (dist_c < 0) 
            dist_c = -dist_c;

        if (dist_r > 2 || dist_c > 2) continue;
 
        if (burn_queue_count >= BURN_QUEUE_MAX) return;
        burn_queue[burn_queue_count].deco_idx   = i;
        burn_queue[burn_queue_count].burn_delay = 180 + rand() % 300;
        burn_queue_count++;
    }
}
 
void wetland_update(void) {
    /* Tile tick: spread lava */
    evo_tile_timer++;
    if (evo_tile_timer >= EVO_TILE_INTERVAL && lava_count < LAVA_MAX) {
        evo_tile_timer = 0;
 
        int src = rand() % lava_count;
        int sr  = lava_cells[src].row;
        int sc  = lava_cells[src].col;
 
        int dr[4] = {-1, 1, 0, 0};
        int dc[4] = { 0, 0,-1, 1};
        for (int i = 3; i > 0; i--) {  
            int j = rand() % (i + 1);
            int tmp; 

            tmp = dr[i]; 
            dr[i] = dr[j]; 
            dr[j] = tmp;  

            tmp = dc[i]; 
            dc[i] = dc[j]; 
            dc[j] = tmp;
        }
 
        for (int d = 0; d < 4; d++) {
            int nr = sr + dr[d];
            int nc = sc + dc[d];
            SpriteID t = map_get_tile(nr, nc);
            if (t != SPRITE_TILE_GRASS && t != SPRITE_TILE_DIRT && t != SPRITE_TILE_WATER)
                continue;
 
            map_change_tile(nr, nc, SPRITE_TILE_LAVA, TILE_FLAG_DAMAGE);
            lava_cells[lava_count].row = (short)nr;
            lava_cells[lava_count].col = (short)nc;
            lava_count++;
            queue_nearby_tree_burns(nr, nc);
            break;
        }
    }
 
    /* Deco tick: burn one tree per tick */
    evo_deco_timer++;
    if (evo_deco_timer >= EVO_DECO_INTERVAL) {
        evo_deco_timer = 0;
 
        for (int q = 0; q < burn_queue_count; q++) {
            burn_queue[q].burn_delay -= EVO_DECO_INTERVAL;
            if (burn_queue[q].burn_delay <= 0) {
                int idx = burn_queue[q].deco_idx;
                int target = deco_burn_target(decorations[idx].type);
                if(target != -1){
                    deco_change_type(idx, target);
                }
                burn_queue[q] = burn_queue[--burn_queue_count];
                break; //once tree per tick
            }
        }
    }
}

/* Beach map evolution*/

//right now using sand as a place holder because i dont have a ice tile yet
 
#define ICE_MAX 80
typedef struct { 
    short row, col; 
} IceCell;

IceCell ice_cells[ICE_MAX];
int     ice_count = 0;

#define FREEZE_QUEUE_MAX 20

typedef struct{
    int deco_idx;
    int freeze_delay;
} FreezeEntry;

FreezeEntry freeze_queue[FREEZE_QUEUE_MAX];
int freeze_queue_count = 0;

/* Pick seed points: water tiles that are next to a sand tile.
(this is to make it more realistic cuz ice forms on coastlines)*/

void beach_seed_ice(void) {
    int seeds = 2 + rand() % 3;   /* 2-4 seeds */
    for (int s = 0; s < seeds && ice_count < ICE_MAX; s++) {
        for (int attempt = 0; attempt < 400; attempt++) {
            int r = 1 + rand() % (MAP_HEIGHT - 2);
            int c = 1 + rand() % (MAP_WIDTH  - 2);
            if (map_get_tile(r, c) != SPRITE_TILE_WATER) continue;
 
            /* Must be touching sand */
            int near_sand = 0;
            if (r > 0            && map_get_tile(r-1, c) == SPRITE_TILE_SAND) near_sand = 1;
            if (r < MAP_HEIGHT-1 && map_get_tile(r+1, c) == SPRITE_TILE_SAND) near_sand = 1;
            if (c > 0            && map_get_tile(r, c-1) == SPRITE_TILE_SAND) near_sand = 1;
            if (c < MAP_WIDTH-1  && map_get_tile(r, c+1) == SPRITE_TILE_SAND) near_sand = 1;
            if (!near_sand) continue;
 
            map_change_tile(r, c, SPRITE_TILE_ICE, TILE_FLAG_ICE);
            ice_cells[ice_count].row = (short)r;
            ice_cells[ice_count].col = (short)c;
            ice_count++;
            break;
        }
    }
}

void beach_queue_nearby_tree_freeze(int ice_row, int ice_col){
    for(int i = 0; i < deco_count; i++){
        if(deco_ice_target(decorations[i].type) == -1) continue;

         int already = 0;
        for (int q = 0; q < freeze_queue_count; q++)
            if (freeze_queue[q].deco_idx == i) { already = 1; break; }
        if (already) continue;
        
        int dtype = decorations[i].type;
        int deco_col = (decorations[i].x + DECO_LOOKUP[dtype].w / 2) >> 4;
        int deco_row = (decorations[i].y + DECO_LOOKUP[dtype].h - 1) >> 4;

        int dist_r   = deco_row - ice_row; 
        if (dist_r < 0) 
            dist_r = -dist_r;

        int dist_c   = deco_col - ice_col; 
        if (dist_c < 0) 
            dist_c = -dist_c;

        if (dist_r > 2 || dist_c > 2) continue;
 
        if (freeze_queue_count >= FREEZE_QUEUE_MAX) return;
        freeze_queue[freeze_queue_count].deco_idx   = i;
        freeze_queue[freeze_queue_count].freeze_delay = 180 + rand() % 300;
        freeze_queue_count++;
    }
}
 
void beach_update(void) {
    if (ice_count == 0) return;   /* nothing seeded yet — safety guard */
 
    evo_tile_timer++;
    if (evo_tile_timer >= EVO_TILE_INTERVAL && ice_count < ICE_MAX){
         evo_tile_timer = 0;
 
        /* Pick a random existing ice cell to spread from */
        int src = rand() % ice_count;
        int sr  = ice_cells[src].row;
        int sc  = ice_cells[src].col;
    
        //Shuffled neighbour order
        int dr[4] = {-1,  1,  0,  0};
        int dc[4] = { 0,  0, -1,  1};

        for (int i = 3; i > 0; i--) {
            int j   = rand() % (i + 1);
            int tmp;

            tmp = dr[i];
            dr[i] = dr[j]; 
            dr[j] = tmp;

            tmp = dc[i]; 
            dc[i] = dc[j]; 
            dc[j] = tmp;
        }
    
        for (int d = 0; d < 4; d++) {
            int nr = sr + dr[d];
            int nc = sc + dc[d];
            /* Ice spreads only into water — not over sand/stone/grass */
            if (map_get_tile(nr, nc) != SPRITE_TILE_WATER) continue;
    
            map_change_tile(nr, nc, SPRITE_TILE_ICE, TILE_FLAG_ICE);
            ice_cells[ice_count].row = (short)nr;
            ice_cells[ice_count].col = (short)nc;
            ice_count++;
            beach_queue_nearby_tree_freeze(nr, nc);
            break;
        }
    }
   
    /* Deco Tick: freeze one tree per tick */
    evo_deco_timer++;
    if (evo_deco_timer >= EVO_DECO_INTERVAL) {
        evo_deco_timer = 0;
 
        for (int q = 0; q < freeze_queue_count; q++) {
            freeze_queue[q].freeze_delay -= EVO_DECO_INTERVAL;
            if (freeze_queue[q].freeze_delay <= 0) {
                int idx = freeze_queue[q].deco_idx;
                int target = deco_ice_target(decorations[idx].type);
                if(target != -1){
                    deco_change_type(idx, target);
                }
                freeze_queue[q] = freeze_queue[--freeze_queue_count];
                break; //once tree per tick
            }
        }
    }
}
 
/* Call this from map_evolution_init for the beach map: */
void beach_init(void) {
    ice_count = 0;
    /* Delay: ice starts forming after ~20 s */
    evo_tile_timer = -(1200 - EVO_TILE_INTERVAL);
    beach_seed_ice();
}

/* Ice map evolution*/
//isolated lava vents appear and melt surrounding ice after a timer

#define ICE_LAVA_MAX     30   /* max simultaneous active vents */
#define ICE_VENT_INTERVAL (EVO_TILE_INTERVAL * 3)  /* new vent every ~4.5 s */
 
/* How long a vent stays active before cooling: 8-18 seconds */
#define ICE_MELT_DELAY_MIN  480
#define ICE_MELT_DELAY_MAX 1080
 
typedef struct {
    short row, col;
    int   melt_timer;   /* frames remaining until this vent cools */
} IceLavaCell;
 
IceLavaCell ice_lava_cells[ICE_LAVA_MAX];
int ice_lava_count = 0;
 
void ice_burn_adjacent_trees(int lava_row, int lava_col){
    for (int i = 0; i < deco_count; i++) {
        int target = deco_burn_target(decorations[i].type);
        if (target == -1) continue;
 
        int dtype = decorations[i].type;
        int deco_col = (decorations[i].x + DECO_LOOKUP[dtype].w / 2) >> 4;
        int deco_row = (decorations[i].y + DECO_LOOKUP[dtype].h - 1) >> 4;

        int dist_r = deco_row - lava_row; 
        if (dist_r < 0){
            dist_r = -dist_r;
        }

        int dist_c = deco_col - lava_col; 
        if (dist_c < 0){
            dist_c = -dist_c;
        }

        if (dist_r > 1 || dist_c > 1) continue; /* only directly adjacent */
 
        deco_change_type(i, target);
    }
}


/* Spawn one new vent on a random ice tile, far enough from existing vents */
void ice_spawn_vent(void) {
    for (int attempt = 0; attempt < 120; attempt++) {
        int r = 1 + rand() % (MAP_HEIGHT - 2);
        int c = 1 + rand() % (MAP_WIDTH  - 2);
 
        if (map_get_tile(r, c) != SPRITE_TILE_SNOW) continue;
 
        /* Enforce a minimum gap of 2 tiles between vents */
        int too_close = 0;
        for (int q = 0; q < ice_lava_count; q++) {
            int dr = r - ice_lava_cells[q].row; 
            if (dr < 0){
                dr = -dr;
            }

            int dc = c - ice_lava_cells[q].col; 
            if (dc < 0){
                dc = -dc;
            }

            if (dr <= 2 && dc <= 2) { 
                too_close = 1; break; 
            }
        }
        if (too_close) continue;
 
        map_change_tile(r, c, SPRITE_TILE_LAVA, TILE_FLAG_DAMAGE);
        ice_lava_cells[ice_lava_count].row = (short)r;
        ice_lava_cells[ice_lava_count].col = (short)c;
        ice_lava_cells[ice_lava_count].melt_timer = ICE_MELT_DELAY_MIN + rand() % (ICE_MELT_DELAY_MAX - ICE_MELT_DELAY_MIN);
        ice_lava_count++;
        ice_burn_adjacent_trees(r, c);
        return;
    }
    // If no valid spot found this tick, just wait for the next interval
}
 
void ice_map_update(void) {
    /* Tile tick: maybe spawn a new vent*/
    evo_tile_timer++;
    if (evo_tile_timer >= ICE_VENT_INTERVAL) {
        evo_tile_timer = 0;
        if (ice_lava_count < ICE_LAVA_MAX)
            ice_spawn_vent();
    }
 
    // Cooldown tick: advance every vent's timer by 1 frame */
    int new_count = 0;
    for (int q = 0; q < ice_lava_count; q++) {
        ice_lava_cells[q].melt_timer--;
 
        if (ice_lava_cells[q].melt_timer <= 0) {
            // Vent cools: lava → scorched dirt
            int lr = ice_lava_cells[q].row;
            int lc = ice_lava_cells[q].col;
            map_change_tile(lr, lc, SPRITE_TILE_STONE, 0);
 
            // Melt orthogonal ice neighbours → warm water 
            const int dr[4] = {-1,  1,  0,  0};
            const int dc[4] = { 0,  0, -1,  1};
            for (int d = 0; d < 4; d++) {
                int nr = lr + dr[d];
                int nc = lc + dc[d];
                if (map_get_tile(nr, nc) == SPRITE_TILE_SNOW)
                    map_change_tile(nr, nc, SPRITE_TILE_WATER_BRIGHT, TILE_FLAG_SLOW);
            }
            // Entry expires — don't carry it forward 
        } else {
            ice_lava_cells[new_count++] = ice_lava_cells[q];
        }
    }
    ice_lava_count = new_count;
}
 
/* Call this from map_evolution_init for the ice map: */
void ice_map_init(void) {
    ice_lava_count = 0;
    /* First vent appears after ~10 s */
    evo_tile_timer = -(600 - ICE_VENT_INTERVAL);
}
 
// Rocky Map Evolution
//rings of land flood inward over time
#define ROCKY_MAX_FLOOD_RINGS  8
#define ROCKY_FLOOD_INTERVAL  450   /* flood one ring every 30 s */
 
int rocky_flood_ring  = 0;
int rocky_flood_timer = 0;
 
void rocky_flood_ring_tiles(int ring) {
    /* Top and bottom rows of this ring */
    for (int c = ring; c < MAP_WIDTH - ring; c++) {
        SpriteID t;
        t = map_get_tile(ring, c);
        if (t != SPRITE_TILE_WATER && t != SPRITE_TILE_STONE)
            map_change_tile(ring, c, SPRITE_TILE_WATER, TILE_FLAG_SLOW);
 
        t = map_get_tile(MAP_HEIGHT - 1 - ring, c);
        if (t != SPRITE_TILE_WATER && t != SPRITE_TILE_STONE)
            map_change_tile(MAP_HEIGHT - 1 - ring, c, SPRITE_TILE_WATER, TILE_FLAG_SLOW);
    }
    /* Left and right columns (skip corners already done above) */
    for (int r = ring + 1; r < MAP_HEIGHT - 1 - ring; r++) {
        SpriteID t;
        t = map_get_tile(r, ring);
        if (t != SPRITE_TILE_WATER && t != SPRITE_TILE_STONE)
            map_change_tile(r, ring, SPRITE_TILE_WATER, TILE_FLAG_SLOW);
 
        t = map_get_tile(r, MAP_WIDTH - 1 - ring);
        if (t != SPRITE_TILE_WATER && t != SPRITE_TILE_STONE)
            map_change_tile(r, MAP_WIDTH - 1 - ring, SPRITE_TILE_WATER, TILE_FLAG_SLOW);
    }
}
 
void rocky_update(void) {
    rocky_flood_timer++;
    if (rocky_flood_timer >= ROCKY_FLOOD_INTERVAL &&
        rocky_flood_ring  <  ROCKY_MAX_FLOOD_RINGS) {
        rocky_flood_timer = 0;
        rocky_flood_ring_tiles(rocky_flood_ring);
        rocky_flood_ring++;
    }
}
 
 
void map_evolution_init(int map_index) {
    /* Reset all state */
    evo_tile_timer    = 0;
    evo_deco_timer    = 0;
    tile_redraw_count = 0;
    deco_redraw_count = 0;
 
    lava_count        = 0;
    burn_queue_count  = 0;
    ice_count         = 0;
    ice_lava_count    = 0;
    rocky_flood_ring  = 0;
    rocky_flood_timer = 0;
 
    switch (map_index) {
        case 1:
            current_map_evo = MAP_EVO_WETLAND;
            /* Pre-load a negative timer offset so lava starts ~30 s in */
            evo_tile_timer  = -(1800 - EVO_TILE_INTERVAL);
            wetland_seed_lava();
            break;
        case 2:
            current_map_evo = MAP_EVO_BEACH;
            beach_init();
            break;
        /* Add cases 3 (ICE) and 4 (BEACH) when those maps are built */
        case 3:
            current_map_evo = MAP_EVO_ICE;
            ice_map_init();
            break;
        case 4:
            current_map_evo = MAP_EVO_ROCKY;
            rocky_flood_timer = -450;   /* first flood 30 s after start */
            break;
        default:
            current_map_evo = MAP_EVO_NONE;
            break;
    }
}
 
void map_evolution_update(void) {
    switch (current_map_evo) {
        case MAP_EVO_WETLAND: wetland_update();  break;
        case MAP_EVO_BEACH:   beach_update();    break;
        case MAP_EVO_ICE:     ice_map_update();  break;
        case MAP_EVO_ROCKY:   rocky_update();    break;
        default: break;
    }
}