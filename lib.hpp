

#include <stddef.h>


namespace pos_vel {

static const size_t N_POS_VEL = 100000;
static const size_t N_POS = 900000;
static const size_t N_SPARSE = 10000;
static const size_t N_MAX = N_POS_VEL + N_POS;

struct Position {
    float x, y;
};

struct Velocity {
    float dx, dy;
};

}

struct Sparse {
    int type, w, x, y, z;
};



namespace parallel {

static const size_t N = 1000000;

struct R {
    float x;
};

struct W1 {
    float x;
};

struct W2 {
    float x;
};

}
