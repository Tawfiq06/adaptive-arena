#ifndef DECORATION_SPRITES_H
#define DECORATIONS_SPRTIES_H

/*Rocks/bushes/flowers on grass only. Trees on grass only.*/

typedef enum {
    DECO_ROCK_BIG_BROWN, DECO_ROCK_MED_BROWN, DECO_ROCK_BIG_GREY,
    DECO_ROCK_MED_GREY, DECO_ROCK_SM_A, DECO_ROCK_SM_B, DECO_ROCK_SM_C,
    DECO_BUSH_GREEN_SM, DECO_BUSH_OLIVE_SM, DECO_BUSH_RED_SM,
    DECO_BUSH_GREEN_LG, DECO_BUSH_OLIVE_LG, DECO_STICK_TREE,
    DECO_FLOWER_PURPLE, DECO_PLANT_TALL, DECO_TREE_GREEN_A,
    DECO_TREE_GREEN_B, DECO_TREE_AUTUMN_A, DECO_TREE_AUTUMN_B,
    DECO_COUNT
} DecoID;

typedef struct{
    const short* data;
    int w, h;
} DecoType;

static const DecoType DECO_LOOKUP[DECO_COUNT];

#define ROCK_BIG_BROWN_W  31
#define ROCK_BIG_BROWN_H  30
#define ROCK_MED_BROWN_W  28
#define ROCK_MED_BROWN_H  29
#define ROCK_BIG_GREY_W  31
#define ROCK_BIG_GREY_H  13
#define ROCK_MED_GREY_W  31
#define ROCK_MED_GREY_H  27
#define ROCK_SM_A_W  9
#define ROCK_SM_A_H  13
#define ROCK_SM_B_W  13
#define ROCK_SM_B_H  13
#define ROCK_SM_C_W  13
#define ROCK_SM_C_H  14
#define BUSH_GREEN_SM_W  27
#define BUSH_GREEN_SM_H  32
#define BUSH_OLIVE_SM_W  27
#define BUSH_OLIVE_SM_H  32
#define BUSH_RED_SM_W  27
#define BUSH_RED_SM_H  32
#define BUSH_GREEN_LG_W  30
#define BUSH_GREEN_LG_H  43
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

static const short rock_big_brown[930];
static const short rock_med_brown[812];
static const short rock_big_grey[403];
static const short rock_med_grey[837];
static const short rock_sm_a[117];
static const short rock_sm_b[169];
static const short rock_sm_c[182];
static const short bush_green_sm[864];
static const short bush_olive_sm[864];
static const short bush_red_sm[864];
static const short bush_green_lg[1290];
static const short bush_olive_lg[1290];
static const short stick_tree[1598];
static const short flower_purple[806];
static const short plant_tall[1316];
static const short tree_green_a[2755];
static const short tree_green_b[2291];
static const short tree_autumn_a[2755];
static const short tree_autumn_b[2291];

#endif