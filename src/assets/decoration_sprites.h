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

    DECO_STICK_TREE_B,
    DECO_STICK_TREE_LG,
    DECO_STICK_TREE_MED,
    DECO_STICK_TREE_SM,

    DECO_ICE_TREE_LG,
    DECO_ICE_TREE_MED,
    DECO_ICE_TREE_SM,
    
    DECO_FLOWER_PURPLE,

    DECO_GREEN_TREE_LG,
    DECO_TREE_GREEN_A,
    DECO_GREEN_PLANT_TREE_A,

    DECO_AUTUMN_TREE_RED_LG,
    DECO_AUTUMN_TREE_RED_MED, 
    DECO_AUTUMN_TREE_RED_SM,

    DECO_AUTUMN_TREE_YELLOW_LG,
    DECO_AUTUMN_TREE_YELLOW_MED, 
    DECO_AUTUMN_TREE_YELLOW_SM,

    DECO_TREE_GREEN_B,

    DECO_CATTAIL_GREEN_LG, DECO_CATTAIL_GREEN_MED, DECO_CATTAIL_GREEN_SM,
    DECO_FERN_GREEN_LG, DECO_FERN_GREEN_MED,
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

#define STICK_TREE_B_W  43
#define STICK_TREE_B_H  96
#define STICK_TREE_LG_W  48
#define STICK_TREE_LG_H  96
#define STICK_TREE_MED_W  26
#define STICK_TREE_MED_H  47
#define STICK_TREE_SM_W  16
#define STICK_TREE_SM_H  32

#define ICE_TREE_LG_W  48
#define ICE_TREE_LG_H  96
#define ICE_TREE_MED_W  25
#define ICE_TREE_MED_H  48
#define ICE_TREE_SM_W  16
#define ICE_TREE_SM_H  32

#define FLOWER_PURPLE_W  26
#define FLOWER_PURPLE_H  31

#define GREEN_TREE_LG_W  48
#define GREEN_TREE_LG_H  96

#define TREE_GREEN_A_W  29
#define TREE_GREEN_A_H  47

#define GREEN_PLANT_TREE_A_W  16
#define GREEN_PLANT_TREE_A_H  31

#define TREE_GREEN_B_W  36
#define TREE_GREEN_B_H  76

#define AUTUMN_TREE_RED_LG_W  48
#define AUTUMN_TREE_RED_LG_H  96
#define AUTUMN_TREE_RED_MED_W  32
#define AUTUMN_TREE_RED_MED_H  62
#define AUTUMN_TREE_RED_SM_W  16
#define AUTUMN_TREE_RED_SM_H  32

#define AUTUMN_TREE_YELLOW_LG_W  48
#define AUTUMN_TREE_YELLOW_LG_H  96
#define AUTUMN_TREE_YELLOW_MED_W  32
#define AUTUMN_TREE_YELLOW_MED_H  62
#define AUTUMN_TREE_YELLOW_SM_W  16
#define AUTUMN_TREE_YELLOW_SM_H  32

#define CATTAIL_GREEN_LG_W  16
#define CATTAIL_GREEN_LG_H  26
#define CATTAIL_GREEN_MED_W  15
#define CATTAIL_GREEN_MED_H  26
#define CATTAIL_GREEN_SM_W  9
#define CATTAIL_GREEN_SM_H  16

#define FERN_GREEN_LG_W  16
#define FERN_GREEN_LG_H  22
#define FERN_GREEN_MED_W  14
#define FERN_GREEN_MED_H  15

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

static const short stick_tree_b[STICK_TREE_B_W * STICK_TREE_B_H];
static const short stick_tree_lg[STICK_TREE_LG_W * STICK_TREE_LG_H];
static const short stick_tree_med[STICK_TREE_MED_W * STICK_TREE_MED_H];
static const short stick_tree_sm[STICK_TREE_SM_W * STICK_TREE_SM_H];

static const short ice_tree_lg[ICE_TREE_LG_W * ICE_TREE_LG_H];
static const short ice_tree_med[ICE_TREE_MED_W * ICE_TREE_MED_H];
static const short ice_tree_sm[ICE_TREE_SM_W * ICE_TREE_SM_H];

static const short flower_purple[806];

static const short green_tree_lg[GREEN_TREE_LG_W * GREEN_TREE_LG_H];
static const short tree_green_a[TREE_GREEN_A_W * TREE_GREEN_A_H];
static const short green_plant_tree_a[GREEN_PLANT_TREE_A_H * GREEN_PLANT_TREE_A_W];
static const short tree_green_b[TREE_GREEN_B_W * TREE_GREEN_B_H];

static const short autumn_tree_red_lg[AUTUMN_TREE_RED_LG_W * AUTUMN_TREE_RED_LG_H];
static const short autumn_tree_red_med[AUTUMN_TREE_RED_MED_W * AUTUMN_TREE_RED_MED_H];
static const short autumn_tree_red_sm[AUTUMN_TREE_RED_SM_W * AUTUMN_TREE_RED_SM_H];

static const short autumn_tree_yellow_lg[AUTUMN_TREE_YELLOW_LG_W * AUTUMN_TREE_YELLOW_LG_H];
static const short autumn_tree_yellow_med[AUTUMN_TREE_YELLOW_MED_W * AUTUMN_TREE_YELLOW_MED_H];
static const short autumn_tree_yellow_sm[AUTUMN_TREE_YELLOW_SM_W * AUTUMN_TREE_YELLOW_SM_H];

static const short cattail_green_lg[CATTAIL_GREEN_LG_W * CATTAIL_GREEN_LG_H];
static const short cattail_green_med[CATTAIL_GREEN_MED_W * CATTAIL_GREEN_MED_H];
static const short cattail_green_sm[CATTAIL_GREEN_SM_W * CATTAIL_GREEN_SM_H];

static const short fern_green_lg[FERN_GREEN_LG_W * FERN_GREEN_LG_H];
static const short fern_green_med[FERN_GREEN_MED_W * FERN_GREEN_MED_H];
#endif