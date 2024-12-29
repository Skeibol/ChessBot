/********************************************
 *                 RANDOM BULLSHIT          *
 *        Random bullshit generator         *
 *        For magic number generation       *
 ********************************************/
#ifndef MAGIC_H
#define MAGIC_H
#include "utils.h"

// XORSHIFT pseudo random number generator
// prng state
unsigned int seed = 1804289383; // 32 bit

inline unsigned int getRandomU32Number() {
    // get current state
    unsigned int number = seed;

    // XOR shift algorithm
    number = number ^ (number << 13);
    number = number ^ (number >> 17);
    number = number ^ (number << 5);

    // update rnd state
    seed = number;

    return number;
}

// generate 64 bit pseudo random number
inline U64 getRandomU64Number() {
    const U64 n1 = (U64) getRandomU32Number() & 0xFFFF;
    const U64 n2 = (U64) getRandomU32Number() & 0xFFFF;
    const U64 n3 = (U64) getRandomU32Number() & 0xFFFF;
    const U64 n4 = (U64) getRandomU32Number() & 0xFFFF;

    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

inline U64 generateMagicNumberCandidate() {
    return getRandomU64Number() & getRandomU64Number() & getRandomU64Number();
}


#endif
