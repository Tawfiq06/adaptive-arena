#ifndef DECORATION_SPRITES_H
#define DECORATIONS_SPRTIES_H

/*Rocks/bushes/flowers on grass only. Trees on grass only.*/

typedef enum {
    DECO_ROCK_BIG_BROWN, 
    DECO_ROCK_MED_BROWN, 
    DECO_ROCK_SM_A_BROWN, 
    DECO_ROCK_SM_B_BROWN,
    DECO_ROCK_SM_C_BROWN,
    DECO_ROCK_BIG_GREY,
    DECO_ROCK_MED_GREY, 
    DECO_ROCK_SM_A_GREY,
    DECO_ROCK_SM_B_GREY, 
    DECO_ROCK_SM_C_GREY,
    DECO_BUSH_GREEN_SM, 
    DECO_BUSH_OLIVE_SM, 
    DECO_BUSH_RED_SM,
    DECO_BUSH_GREEN_LG,
    DECO_BUSH_OLIVE_LG, 
    DECO_STICK_TREE,
    DECO_FLOWER_PURPLE, DECO_PLANT_TALL, DECO_TREE_GREEN_A,
    DECO_TREE_GREEN_B, DECO_TREE_AUTUMN_A, DECO_TREE_AUTUMN_B,
    DECO_COUNT
} DecoID;

typedef struct{
    const short* data;
    int w, h;
} DecoType;

static const DecoType DECO_LOOKUP[DECO_COUNT];

#define ROCK_BIG_BROWN_W  28
#define ROCK_BIG_BROWN_H  43
#define ROCK_MED_BROWN_W  26
#define ROCK_MED_BROWN_H  27
#define ROCK_SM_A_BROWN_W  14
#define ROCK_SM_A_BROWN_H  11
#define ROCK_SM_B_BROWN_W  15
#define ROCK_SM_B_BROWN_H  10
#define ROCK_SM_C_BROWN_W  7
#define ROCK_SM_C_BROWN_H  7

#define ROCK_BIG_GREY_W  28
#define ROCK_BIG_GREY_H  43
#define ROCK_MED_GREY_W  26
#define ROCK_MED_GREY_H  27
#define ROCK_SM_A_GREY_W  14
#define ROCK_SM_A_GREY_H  11
#define ROCK_SM_B_GREY_W  15
#define ROCK_SM_B_GREY_H  10
#define ROCK_SM_C_GREY_W  7
#define ROCK_SM_C_GREY_H  7

#define BUSH_GREEN_SM_W  29
#define BUSH_GREEN_SM_H  26

#define BUSH_OLIVE_SM_W  29
#define BUSH_OLIVE_SM_H  26

#define BUSH_RED_SM_W  29
#define BUSH_RED_SM_H  26

#define BUSH_GREEN_LG_W  42
#define BUSH_GREEN_LG_H  32

#define BUSH_OLIVE_LG_W  42
#define BUSH_OLIVE_LG_H  32

#define BUSH_OLIVE_LG_W  30
#define BUSH_OLIVE_LG_H  43
#define STICK_TREE_W  34
#define STICK_TREE_H  47
#define FLOWER_PURPLE_W  26
#define FLOWER_PURPLE_H  31
#define PLANT_TALL_W  47
#define PLANT_TALL_H  28
#define TREE_GREEN_A_W  29
#define TREE_GREEN_A_H  95
#define TREE_GREEN_B_W  29
#define TREE_GREEN_B_H  79
#define TREE_AUTUMN_A_W  29
#define TREE_AUTUMN_A_H  95
#define TREE_AUTUMN_B_W  29
#define TREE_AUTUMN_B_H  79

static const short rock_big_brown[ROCK_BIG_BROWN_W * ROCK_BIG_BROWN_W];
static const short rock_med_brown[ROCK_MED_BROWN_W * ROCK_MED_BROWN_H];
static const short rock_sm_a_brown[ROCK_SM_A_BROWN_W * ROCK_SM_A_BROWN_H];
static const short rock_sm_b_brown[ROCK_SM_B_BROWN_W * ROCK_SM_B_BROWN_H];
static const short rock_sm_c_brown[ROCK_SM_C_BROWN_W * ROCK_SM_C_BROWN_H];

static const short rock_big_grey[ROCK_BIG_GREY_W * ROCK_BIG_GREY_H];
static const short rock_med_grey[ROCK_MED_GREY_W * ROCK_BIG_GREY_H];
static const short rock_sm_a_grey[ROCK_SM_A_GREY_W * ROCK_SM_A_GREY_H];
static const short rock_sm_b_grey[ROCK_SM_B_GREY_W * ROCK_SM_B_GREY_H];
static const short rock_sm_c_grey[ROCK_SM_C_GREY_W * ROCK_SM_C_GREY_H];

static const short bush_green_sm[BUSH_GREEN_SM_W * BUSH_GREEN_SM_H];
static const short bush_olive_sm[BUSH_OLIVE_SM_W * BUSH_OLIVE_SM_H];
static const short bush_red_sm[BUSH_RED_SM_W * BUSH_RED_SM_H];

static const short bush_green_lg[BUSH_GREEN_LG_W * BUSH_GREEN_LG_H];
static const short bush_olive_lg[BUSH_OLIVE_LG_W * BUSH_OLIVE_LG_H];

static const short stick_tree[1598];
static const short flower_purple[806];
static const short plant_tall[1316];
static const short tree_green_a[2755];
static const short tree_green_b[2291];
static const short tree_autumn_a[2755];
static const short tree_autumn_b[2291];

#endif