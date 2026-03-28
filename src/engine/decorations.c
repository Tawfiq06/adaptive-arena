#include "decorations.h"
#include "map.h"
#include "vga.h"
#include "tile_sprites.h"
#include "sprite.h"
#include <stdlib.h>
#include "soldier_frames.h"
/**************************************************/
/* DECOR LOOKUP MAP                               */
/* Same dimensions as tile map (15x20)            */
/* Each cell will have up to DECO_CELL_CAP decors */
/* built only ocne during init                    */
/**************************************************/

DecoCell deco_map[MAP_HEIGHT][MAP_WIDTH];

const DecoCell *deco_map_get_cell(int row, int col){
    return &deco_map[row][col];
}

void deco_map_build(void){
    //first cell every cell
    for(int r=0; r < MAP_HEIGHT; r++){
        for (int c = 0; c < MAP_WIDTH; c++){
            deco_map[r][c].count = 0;
        }
    }

    for(int i = 0; i < deco_count; i++){
        const DecoType *d = &DECO_LOOKUP[decorations[i].type];
        //get all the cells it touches
        int c0 = decorations[i].x / TILE_W;
        int r0 = decorations[i].y /TILE_H;
        int c1 = (decorations[i].x + d->w - 1) / TILE_W;
        int r1 = (decorations[i].y + d->h - 1) / TILE_H;

        for(int r = r0; r <= r1 && r < MAP_HEIGHT; r++){
            for(int c = c0; c <= c1 && c < MAP_WIDTH; c++){
                if(r < 0 || c < 0){
                    continue;
                }
                DecoCell *cell = &deco_map[r][c];
                if(cell->count < DECO_CELL_CAP){
                    cell->indices[cell->count++] = (unsigned char)i;
                }
            }
        }
    }
}

/*return 1 is tile is next to water*/
static int is_next_to_water(int row, int col){
    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};
    for(int d = 0; d < 4; d++){
        int r = row + dr[d];
        int c = col + dc[d];
        if(r < 0 || r >= MAP_HEIGHT || c < 0 || c >= MAP_WIDTH) continue;
        if(map_get_tile(r, c) == SPRITE_TILE_WATER) return 1;
    }
    return 0;
}

/*return 1 if any decoration already places is a tree in the radius*/
static int is_near_tree(int px, int py, int radius){
    for(int i = 0; i < deco_count; i++){
        int dtype = decorations[i].type;
        if (dtype != DECO_TREE_GREEN_A && dtype != DECO_TREE_GREEN_B &&
            dtype !=DECO_AUTUMN_TREE_RED_MED && dtype != DECO_AUTUMN_TREE_RED_SM &&
            dtype != DECO_AUTUMN_TREE_YELLOW_MED && dtype != DECO_AUTUMN_TREE_YELLOW_SM &&
            dtype != DECO_STICK_TREE_MED && dtype != DECO_GREEN_PLANT_TREE_A) continue;
        int dx = decorations[i].x - px;
        int dy = decorations[i].y - py;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        if (dx < radius && dy < radius) return 1;
    }
    return 0;
}

//spacing check
static int too_close_to_any(int px, int py, int min_spacing) {
    for (int i = 0; i < deco_count; i++) {
        const DecoType *other = &DECO_LOOKUP[decorations[i].type];
        int dx = decorations[i].x - px;
        int dy = decorations[i].y - py;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        if (dx < other->w + min_spacing && dy < other->h + min_spacing) return 1;
    }
    return 0;
}

/*Place one decor*/
static int place_deco(int type, int px, int py) {
    if (deco_count >= MAX_DECORATIONS) return 0;
    decorations[deco_count++] = (Decoration){ (short)px, (short)py, (unsigned char)type };
    return 1;
}

void decoration_init(int map_index){
    deco_count = 0;
    
    const MapConfig *cfg = &MAP_CONFIGS[map_index];
    const int MAX_ATTEMPTS = 500;
    const int MIN_SPACING = 0;

    /* ---- Build tree table from config ---- */
    int TREE_TABLE_BIG[12];
    int TREE_TABLE_SMALL[8];
    int TREE_COUNT_BIG = 0;
    int TREE_COUNT_SMALL = 0;

    if(cfg->use_ice_trees != 2 && cfg->use_autumn_trees != 2 && cfg->use_stick_trees != 2){ //if 2 only use ice trees
        TREE_TABLE_BIG[TREE_COUNT_BIG++] = DECO_GREEN_TREE_LG;
        TREE_TABLE_BIG[TREE_COUNT_BIG++] = DECO_TREE_GREEN_B;     
        TREE_TABLE_BIG[TREE_COUNT_BIG++] = DECO_TREE_GREEN_A;
    }
    if ((cfg->use_autumn_trees || cfg->use_autumn_trees == 2) && cfg->use_ice_trees != 2 && cfg->use_stick_trees != 2) {
        TREE_TABLE_BIG[TREE_COUNT_BIG++] = DECO_AUTUMN_TREE_RED_LG;
        TREE_TABLE_BIG[TREE_COUNT_BIG++] = DECO_AUTUMN_TREE_RED_MED;
        TREE_TABLE_BIG[TREE_COUNT_BIG++] = DECO_AUTUMN_TREE_YELLOW_LG;
        TREE_TABLE_BIG[TREE_COUNT_BIG++] = DECO_AUTUMN_TREE_YELLOW_MED;
    }
    if ((cfg->use_ice_trees || cfg->use_ice_trees == 2) && cfg->use_autumn_trees != 2 && cfg->use_stick_trees != 2){
        TREE_TABLE_BIG[TREE_COUNT_BIG++] = DECO_ICE_TREE_LG;
        TREE_TABLE_BIG[TREE_COUNT_BIG++] = DECO_ICE_TREE_MED;
    }
    if ((cfg->use_stick_trees || cfg->use_stick_trees == 2) && cfg->use_ice_trees != 2 && cfg->use_autumn_trees != 2){
        TREE_TABLE_BIG[TREE_COUNT_BIG++] = DECO_STICK_TREE_B;
        TREE_TABLE_BIG[TREE_COUNT_BIG++] = DECO_STICK_TREE_LG;
        TREE_TABLE_BIG[TREE_COUNT_BIG++] = DECO_STICK_TREE_MED;
    }

    if(cfg->use_autumn_trees != 2 && cfg->use_ice_trees != 2 && cfg->use_stick_trees != 2){
        TREE_TABLE_SMALL[TREE_COUNT_SMALL++] = DECO_GREEN_PLANT_TREE_A;
    }
    if ((cfg->use_autumn_trees || cfg->use_autumn_trees == 2) && cfg->use_ice_trees != 2 && cfg->use_stick_trees != 2) {
        TREE_TABLE_SMALL[TREE_COUNT_SMALL++] = DECO_AUTUMN_TREE_RED_SM;
        TREE_TABLE_SMALL[TREE_COUNT_SMALL++] = DECO_AUTUMN_TREE_YELLOW_SM;
    }
    if((cfg->use_ice_trees || cfg->use_ice_trees == 2) && cfg->use_autumn_trees != 2 && cfg->use_stick_trees != 2){
        TREE_TABLE_SMALL[TREE_COUNT_SMALL++] = DECO_ICE_TREE_SM;
    }
    if((cfg->use_stick_trees || cfg->use_stick_trees == 2) && cfg->use_autumn_trees != 2 && cfg->use_ice_trees != 2){
        TREE_TABLE_SMALL[TREE_COUNT_SMALL++] = DECO_STICK_TREE_SM;
    }

    /* ---- Build rock table ---- */
    int ROCK_TABLE_BIG[4];
    int ROCK_TABLE_SMALL[6];
    int ROCK_COUNT_BIG = 0;
    int ROCK_COUNT_SMALL = 0;

    if (cfg->use_grey_rocks != 1) {
        ROCK_TABLE_BIG[ROCK_COUNT_BIG++]     = DECO_ROCK_BIG_BROWN;
        ROCK_TABLE_BIG[ROCK_COUNT_BIG++]     = DECO_ROCK_MED_BROWN;
        ROCK_TABLE_SMALL[ROCK_COUNT_SMALL++] = DECO_ROCK_SM_A_BROWN;
        ROCK_TABLE_SMALL[ROCK_COUNT_SMALL++] = DECO_ROCK_SM_B_BROWN;
        ROCK_TABLE_SMALL[ROCK_COUNT_SMALL++] = DECO_ROCK_SM_C_BROWN;
    }
    if (cfg->use_grey_rocks >= 1) {
        ROCK_TABLE_BIG[ROCK_COUNT_BIG++]     = DECO_ROCK_BIG_GREY;
        ROCK_TABLE_BIG[ROCK_COUNT_BIG++]     = DECO_ROCK_MED_GREY;
        ROCK_TABLE_SMALL[ROCK_COUNT_SMALL++] = DECO_ROCK_SM_A_GREY;
        ROCK_TABLE_SMALL[ROCK_COUNT_SMALL++] = DECO_ROCK_SM_B_GREY;
        ROCK_TABLE_SMALL[ROCK_COUNT_SMALL++] = DECO_ROCK_SM_C_GREY;
    }

    const int BUSH_TABLE[] = {
        DECO_BUSH_GREEN_SM, DECO_BUSH_OLIVE_SM, DECO_BUSH_RED_SM,
        DECO_BUSH_GREEN_LG, DECO_BUSH_OLIVE_LG,
    };
    const int BUSH_COUNT = 5;

    const int SMALL_TABLE[] = {
        DECO_ROCK_SM_A_BROWN, DECO_ROCK_SM_B_BROWN, DECO_ROCK_SM_C_BROWN,
        DECO_ROCK_SM_A_GREY,  DECO_ROCK_SM_B_GREY,  DECO_ROCK_SM_C_GREY,
    };

    const int SMALL_COUNT = 6;

    const int CATTAIL_TABLE[] = {
        DECO_CATTAIL_GREEN_LG, DECO_CATTAIL_GREEN_MED, DECO_CATTAIL_GREEN_SM,
    };
    const int CATTAIL_COUNT = 3;

    const int FERN_TABLE[] = {
        DECO_FERN_GREEN_LG, DECO_FERN_GREEN_MED,
    };
    const int FERN_COUNT = 2;

    /* Build a list of all valid tiles for trees */
    int grass_tiles[MAP_HEIGHT * MAP_WIDTH][2];  /* [i][0] = row, [i][1] = col */
    int grass_count = 0;

    for (int r = 0; r < MAP_HEIGHT; r++) {
        for (int c = 0; c < MAP_WIDTH; c++) {
            SpriteID t = map_get_tile(r, c);
            if (t == SPRITE_TILE_GRASS || t == SPRITE_TILE_SNOW){
                grass_tiles[grass_count][0] = r,
                grass_tiles[grass_count][1] = c,
                grass_count++;
            }
        }
    }

    /* ---- 1. Place trees (with optional clustering) ---- */
    int trees_placed = 0;
    for (int attempts = 0; attempts < MAX_ATTEMPTS && trees_placed < cfg->tree_budget; attempts++) {
        if (grass_count == 0) break;

        int idx = rand() % grass_count;
        int row = grass_tiles[idx][0];
        int col = grass_tiles[idx][1];

        int type;
        if (cfg->prefer_big_trees == 2) {
            /* big only */
            type = TREE_TABLE_BIG[rand() % TREE_COUNT_BIG];
        } else if (cfg->prefer_big_trees == 1) {
            /* big 75% of the time */
            type = (rand() % 4 != 0)
                ? TREE_TABLE_BIG[rand() % TREE_COUNT_BIG]
                : TREE_TABLE_SMALL[rand() % TREE_COUNT_SMALL];
        } else {
            /* mixed — equal chance */
            int all_count = TREE_COUNT_BIG + TREE_COUNT_SMALL;
            int pick = rand() % all_count;
            type = (pick < TREE_COUNT_BIG)
                ? TREE_TABLE_BIG[pick]
                : TREE_TABLE_SMALL[pick - TREE_COUNT_BIG];
        }

        const DecoType *info = &DECO_LOOKUP[type];
        
        int px = col * TILE_W + (rand() % TILE_W);
        int py = row * TILE_H + (rand() % (TILE_H / 2));

        int bot_col = (px + info->w / 2) >> 4;
        int bot_row = (py + info->h - 1) >> 4;
        if (bot_col >= 0 && bot_col < MAP_WIDTH && bot_row >= 0 && bot_row < MAP_HEIGHT) {
            if (map_get_tile(bot_row, bot_col) != SPRITE_TILE_GRASS && map_get_tile(bot_row, bot_col) != SPRITE_TILE_SNOW) continue;
        }
        else{
            continue;
        }
        if (too_close_to_any(px, py, MIN_SPACING)) continue;
        if (!place_deco(type, px, py)) break;
        trees_placed++;

        int cluster_members = cfg->tree_cluster_size - 1;
        for (int ci = 0; ci < cluster_members; ci++) {
            if (trees_placed >= cfg->tree_budget) break;
            int cpx = px + (rand() % 80) - 40;
            int cpy = py + (rand() % 64) - 32;

            int ctype;
            if (cfg->prefer_big_trees == 2) {
                ctype = TREE_TABLE_BIG[rand() % TREE_COUNT_BIG];
            } else if (cfg->prefer_big_trees == 1) {
                ctype = (rand() % 4 != 0)
                    ? TREE_TABLE_BIG[rand() % TREE_COUNT_BIG]
                    : TREE_TABLE_SMALL[rand() % TREE_COUNT_SMALL];
            } else {
                int all_count = TREE_COUNT_BIG + TREE_COUNT_SMALL;
                int pick = rand() % all_count;
                ctype = (pick < TREE_COUNT_BIG)
                    ? TREE_TABLE_BIG[pick]
                    : TREE_TABLE_SMALL[pick - TREE_COUNT_BIG];
            }
            const DecoType *ci_info = &DECO_LOOKUP[ctype];

            if (cpy + ci_info->h <= 0 || cpy >= SCREEN_HEIGHT) continue;
            if (cpx + ci_info->w <= 0 || cpx >= SCREEN_WIDTH)  continue;

            int cbot_col = (cpx + ci_info->w / 2) >> 4;
            int cbot_row = (cpy + ci_info->h - 1) >> 4;

            if (cbot_col >= 0 && cbot_col < MAP_WIDTH && cbot_row >= 0 && cbot_row < MAP_HEIGHT) {
                if (map_get_tile(cbot_row, cbot_col) != SPRITE_TILE_GRASS && map_get_tile(bot_row, bot_col) != SPRITE_TILE_SNOW) continue;
            }
            else{
                continue;
            }
            if (too_close_to_any(cpx, cpy, 4)) continue;
            if (place_deco(ctype, cpx, cpy)) trees_placed++;
        }
    }

    /* ---- 2. Place rocks (with optional clustering) ---- */
    int rocks_placed = 0;
    for (int attempts = 0; attempts < MAX_ATTEMPTS && rocks_placed < cfg->rock_budget; attempts++) {
        //get random row and col
        int row = 1 + (rand() % (MAP_HEIGHT - 2));
        int col = 1 + (rand() % (MAP_WIDTH  - 2));
        SpriteID temp = map_get_tile(row, col);
        if (temp != SPRITE_TILE_GRASS && temp != SPRITE_TILE_STONE && temp != SPRITE_TILE_SNOW) continue;

        //give random offset
        int type;
        if (cfg->prefer_big_rocks == 2) {
            type = ROCK_TABLE_BIG[rand() % ROCK_COUNT_BIG];
        } else if (cfg->prefer_big_rocks == 1) {
            type = (rand() % 4 != 0)
                ? ROCK_TABLE_BIG[rand() % ROCK_COUNT_BIG]
                : ROCK_TABLE_SMALL[rand() % ROCK_COUNT_SMALL];
        } else {
            int all_count = ROCK_COUNT_BIG + ROCK_COUNT_SMALL;
            int pick = rand() % all_count;
            type = (pick < ROCK_COUNT_BIG)
                ? ROCK_TABLE_BIG[pick]
                : ROCK_TABLE_SMALL[pick - ROCK_COUNT_BIG];
        }
        const DecoType *info = &DECO_LOOKUP[type];
        int px = col * TILE_W + (rand() % TILE_W);
        int py = row * TILE_H + (rand() % TILE_H);

        //check bottom of rock
        int bot_col = (px + info->w / 2) >> 4;
        int bot_row = (py + info->h - 1) >> 4;
        if (bot_row < 0 || bot_row >= MAP_HEIGHT || bot_col < 0 || bot_col >= MAP_WIDTH) continue;

        temp = map_get_tile(bot_row, bot_col);
        if (temp != SPRITE_TILE_GRASS && temp != SPRITE_TILE_STONE && temp != SPRITE_TILE_DIRT && temp != SPRITE_TILE_SNOW) continue;
        if (too_close_to_any(px, py, MIN_SPACING)) continue;

        //place rock
        if (!place_deco(type, px, py)) break;
        rocks_placed++;

        //this is for clusters
        for (int ci = 1; ci < cfg->rock_cluster_size; ci++) {
            if (rocks_placed >= cfg->rock_budget) break;
            int cpx = px + (rand() % 24) - 12;
            int cpy = py + (rand() % 16) - 8;

            /* Cluster members are smaller rocks */
            int small_rocks[] = {
                DECO_ROCK_SM_A_BROWN, DECO_ROCK_SM_B_BROWN, DECO_ROCK_SM_C_BROWN,
                DECO_ROCK_SM_A_GREY,  DECO_ROCK_SM_B_GREY,  DECO_ROCK_SM_C_GREY,
            };
            int ctype = small_rocks[(rand() % (cfg->use_grey_rocks >= 1 ? 6 : 3))];
            const DecoType *ci_info = &DECO_LOOKUP[ctype];

            if (cpy + ci_info->h <= 0 || cpy >= SCREEN_HEIGHT) continue;
            if (cpx + ci_info->w <= 0 || cpx >= SCREEN_WIDTH)  continue;

            int cbot_col = (cpx + ci_info->w / 2) >> 4;
            int cbot_row = (cpy + ci_info->h - 1) >> 4;

            if (cbot_row < 0 || cbot_row >= MAP_HEIGHT || cbot_col < 0 || cbot_col >= MAP_WIDTH) continue;
            temp = map_get_tile(cbot_row, cbot_col);
            if (temp != SPRITE_TILE_GRASS && temp != SPRITE_TILE_DIRT && temp != SPRITE_TILE_STONE && temp != SPRITE_TILE_SNOW) continue;
            if (too_close_to_any(cpx, cpy, 3)) continue;
            if (place_deco(ctype, cpx, cpy)) rocks_placed++;
        }
    }

    /* ---- 3. Place bushes ---- */
    int bushes_placed = 0;
    for (int attempts = 0; attempts < MAX_ATTEMPTS && bushes_placed < cfg->bush_budget; attempts++) {
        int row = 1 + (rand() % (MAP_HEIGHT - 2));
        int col = 1 + (rand() % (MAP_WIDTH  - 2));
        if (map_get_tile(row, col) != SPRITE_TILE_GRASS && map_get_tile(row, col) != SPRITE_TILE_SNOW) continue;

        int type = BUSH_TABLE[rand() % BUSH_COUNT];
        const DecoType *info = &DECO_LOOKUP[type];
        int px = col * TILE_W + (rand() % TILE_W);
        int py = row * TILE_H + (rand() % (TILE_H / 2));

        int bot_col = (px + info->w / 2) >> 4;
        int bot_row = (py + info->h - 1) >> 4;

        if (bot_row < 0 || bot_row >= MAP_HEIGHT || bot_col < 0 || bot_col >= MAP_WIDTH) continue;
        if (map_get_tile(bot_row, bot_col) != SPRITE_TILE_GRASS && map_get_tile(bot_row, bot_col) != SPRITE_TILE_SNOW) continue;
        if (too_close_to_any(px, py, MIN_SPACING)) continue;
        if (place_deco(type, px, py)) bushes_placed++;
    }

    /* ---- 4. Place small props ---- */
    int smalls_placed = 0;
    for (int attempts = 0; attempts < MAX_ATTEMPTS && smalls_placed < cfg->small_budget; attempts++) {
        int row = 1 + (rand() % (MAP_HEIGHT - 2));
        int col = 1 + (rand() % (MAP_WIDTH  - 2));
        if (map_get_tile(row, col) != SPRITE_TILE_GRASS && map_get_tile(row, col) != SPRITE_TILE_SNOW) continue;

        int type = SMALL_TABLE[rand() % SMALL_COUNT];
        const DecoType *info = &DECO_LOOKUP[type];
        int px = col * TILE_W + (rand() % TILE_W);
        int py = row * TILE_H + (rand() % TILE_H);
        int bot_col = (px + info->w / 2) >> 4;
        int bot_row = (py + info->h - 1) >> 4;

        if (bot_row < 0 || bot_row >= MAP_HEIGHT || bot_col < 0 || bot_col >= MAP_WIDTH) continue;
        if (map_get_tile(bot_row, bot_col) != SPRITE_TILE_GRASS && map_get_tile(bot_row, bot_col) != SPRITE_TILE_SNOW) continue;
        if (too_close_to_any(px, py, 6)) continue;
        if (place_deco(type, px, py)) smalls_placed++;
    }

    /* ---- 5. Place cattails — grass tiles adjacent to water only ---- */
    int cattails_placed = 0;
    for (int attempts = 0; attempts < MAX_ATTEMPTS && cattails_placed < cfg->cattail_budget; attempts++) {
        int row = 1 + (rand() % (MAP_HEIGHT - 2));
        int col = 1 + (rand() % (MAP_WIDTH  - 2));
        if (map_get_tile(row, col) != SPRITE_TILE_GRASS && map_get_tile(row, col) != SPRITE_TILE_SNOW) continue;
        if (!is_next_to_water(row, col)) continue;  /* <-- the key filter */

        int type = CATTAIL_TABLE[rand() % CATTAIL_COUNT];
        const DecoType *info = &DECO_LOOKUP[type];
        int px = col * TILE_W + (rand() % TILE_W);
        int py = row * TILE_H + (rand() % (TILE_H / 2));

        int bot_col = (px + info->w / 2) >> 4;
        int bot_row = (py + info->h - 1) >> 4;

        if (bot_row < 0 || bot_row >= MAP_HEIGHT || bot_col < 0 || bot_col >= MAP_WIDTH) continue;
        if (map_get_tile(bot_row, bot_col) != SPRITE_TILE_GRASS && map_get_tile(bot_row, bot_col) != SPRITE_TILE_SNOW) continue;
        if (too_close_to_any(px, py, 4)) continue;  /* cattails can be fairly close together */
        if (place_deco(type, px, py)) cattails_placed++;
    }

    /* ---- 6. Place ferns — grass tiles near already-placed trees only ---- */
    int ferns_placed = 0;
    for (int attempts = 0; attempts < MAX_ATTEMPTS && ferns_placed < cfg->fern_budget; attempts++) {
        int row = 1 + (rand() % (MAP_HEIGHT - 2));
        int col = 1 + (rand() % (MAP_WIDTH  - 2));
        if (map_get_tile(row, col) != SPRITE_TILE_GRASS && map_get_tile(row, col) != SPRITE_TILE_SNOW) continue;

        int px = col * TILE_W + (rand() % TILE_W);
        int py = row * TILE_H + (rand() % TILE_H);
        if (!is_near_tree(px, py, 32)) continue;  /* within ~2 tiles of a tree */

        int type = FERN_TABLE[rand() % FERN_COUNT];
        const DecoType *info = &DECO_LOOKUP[type];
        int bot_col = (px + info->w / 2) >> 4;
        int bot_row = (py + info->h - 1) >> 4;

        if (bot_row < 0 || bot_row >= MAP_HEIGHT || bot_col < 0 || bot_col >= MAP_WIDTH) continue;
        if (map_get_tile(bot_row, bot_col) != SPRITE_TILE_GRASS && map_get_tile(bot_row, bot_col) != SPRITE_TILE_SNOW) continue;
        if (too_close_to_any(px, py, 5)) continue;
        if (place_deco(type, px, py)) ferns_placed++;
    }

    canopy_count = 0;
    for (int i = 0; i < deco_count; i++) {
        if (deco_has_canopy(decorations[i].type))
            canopy_indices[canopy_count++] = i;
    }

    deco_map_build();
}

void decoration_draw_all(void){
    for(int i = 0; i < deco_count; i++){
        const DecoType *d = &DECO_LOOKUP[decorations[i].type];
        Sprite s = {
            d->w,
            d->h,
            d->data
        };
        draw_sprite(&s, decorations[i].x, decorations[i].y, 0, 0);
    }
}

//need to redraw decor if a sprite overlaps it
void decoration_redraw_region(int row0, int col0, int row1, int col1) {
    int clip_x0 = col0 * TILE_W;
    int clip_x1 = col1 * TILE_W - 1;
    int clip_y0 = row0 * TILE_H;
    int clip_y1 = row1 * TILE_H - 1;

    for (int i = 0; i < deco_count; i++) {
        const DecoType *d = &DECO_LOOKUP[decorations[i].type];
        int sx = decorations[i].x;
        int sy = decorations[i].y;

        /* Broad phase — skip entirely if no overlap */
        if (sx + d->w <= clip_x0 || sx > clip_x1) continue;
        if (sy + d->h <= clip_y0 || sy > clip_y1) continue;

        /* Tight row bounds — only iterate rows inside the clip rect */
        int row_start = clip_y0 - sy;
        int row_end   = clip_y1 - sy + 1;
        if (row_start < 0) row_start = 0;
        if (row_end > d->h) row_end = d->h;

        /* Tight col bounds */
        int col_start = clip_x0 - sx;
        int col_end   = clip_x1 - sx + 1;
        if (col_start < 0) col_start = 0;
        if (col_end > d->w) col_end = d->w;

        for (int row = row_start; row < row_end; row++) {
            int py = sy + row;
            if (py < 0 || py >= SCREEN_HEIGHT) continue;
            for (int col = col_start; col < col_end; col++) {
                int px = sx + col;
                if (px < 0 || px >= SCREEN_WIDTH) continue;
                short colour = d->data[row * d->w + col];
                if ((unsigned short)colour != (unsigned short)TRANSPARENT)
                    plot_pixel(px, py, colour);
            }
        }
    }
}

/* Which types have a tall canopy that should draw over players */
int deco_has_canopy(int deco_type) {
    switch (deco_type) {
        case DECO_TREE_GREEN_A:
        case DECO_TREE_GREEN_B:
        case DECO_GREEN_TREE_LG:

        case DECO_AUTUMN_TREE_RED_LG:
        case DECO_AUTUMN_TREE_RED_MED:
        case DECO_AUTUMN_TREE_YELLOW_LG:
        case DECO_AUTUMN_TREE_YELLOW_MED:

        case DECO_STICK_TREE_B:
        case DECO_STICK_TREE_LG:
        case DECO_STICK_TREE_MED:

        case DECO_ICE_TREE_LG:
        case DECO_ICE_TREE_MED:

        case DECO_ROCK_BIG_GREY:
        case DECO_ROCK_BIG_BROWN:
        case DECO_ROCK_MED_BROWN:
        case DECO_ROCK_MED_GREY:

        case DECO_BUSH_GREEN_LG:
        case DECO_BUSH_OLIVE_LG:
            return 1;
        default:
            return 0;
    }
}

void decoration_draw_canopies_near(int px1, int py1, int px2, int py2) {
    for (int i = 0; i < canopy_count; i++) {
        const Decoration *d = &decorations[canopy_indices[i]];
        const DecoType *dt = &DECO_LOOKUP[d->type];
        int sx = d->x;
        int sy = d->y;

        /* Only redraw canopy if a player overlaps this decoration's x range */
        int near = 0;
        if (px1 >= sx - SOLDIER_H && px1 <= sx + dt->w) near = 1;
        if (px2 >= sx - SOLDIER_W && px2 <= sx + dt->w) near = 1;
        if (!near) continue;

        int cutoff_py = sy + dt->h - 6;
        for (int row = 0; row < dt->h; row++) {
            int py = sy + row;
            if (py >= cutoff_py) break;
            if (py < 0 || py >= SCREEN_HEIGHT) continue;
            for (int col = 0; col < dt->w; col++) {
                int ppx = sx + col;
                if (ppx < 0 || ppx >= SCREEN_WIDTH) continue;
                short colour = dt->data[row * dt->w + col];
                if ((unsigned short)colour != (unsigned short)TRANSPARENT)
                    plot_pixel(ppx, py, colour);
            }
        }
    }
}

void deco_canopy_rebuild(void){
    canopy_count = 0;
    for (int i = 0; i < deco_count; i++){
        if (deco_has_canopy(decorations[i].type))
            canopy_indices[canopy_count++] = i;
    }
}