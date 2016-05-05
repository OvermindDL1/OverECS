

#include <stddef.h>


namespace pos_vel {

static const size_t N_POS_VEL = 1000;
static const size_t N_POS = 9000;

struct Position {
    float x, y;
};

struct Velocity {
    float dx, dy;
};

}


namespace parallel {

static const size_t N = 10000;

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
