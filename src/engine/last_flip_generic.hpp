/*
    Egaroucid Project

    @file last_flip_generic.hpp
        calculate number of flipped discs in the last move without AVX2
    @date 2021-2025
    @author Takuto Yamana
    @license GPL-3.0 license
*/

#pragma once
#include "setting.hpp"
#include "common.hpp"
#include "bit.hpp"

/*
    @brief constant of number of flipping discs
*/

// if (i & (1 << j))  n_flip_pre_calc[i][j] = n_flip_pre_calc[i ^ (1 << j)][j];	// to use ~player instead of opponent
constexpr uint8_t n_flip_pre_calc[N_8BIT][HW] = {
    {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 1, 2, 3, 4, 5, 6}, {0, 0, 0, 1, 2, 3, 4, 5}, {0, 0, 0, 1, 2, 3, 4, 5}, {1, 0, 0, 0, 1, 2, 3, 4}, {1, 0, 1, 0, 1, 2, 3, 4}, {0, 0, 0, 0, 1, 2, 3, 4}, {0, 0, 0, 0, 1, 2, 3, 4},
    {2, 1, 0, 0, 0, 1, 2, 3}, {2, 1, 1, 2, 0, 1, 2, 3}, {0, 1, 0, 1, 0, 1, 2, 3}, {0, 1, 0, 1, 0, 1, 2, 3}, {1, 0, 0, 0, 0, 1, 2, 3}, {1, 0, 1, 0, 0, 1, 2, 3}, {0, 0, 0, 0, 0, 1, 2, 3}, {0, 0, 0, 0, 0, 1, 2, 3},
    {3, 2, 1, 0, 0, 0, 1, 2}, {3, 2, 2, 2, 3, 0, 1, 2}, {0, 2, 1, 1, 2, 0, 1, 2}, {0, 2, 1, 1, 2, 0, 1, 2}, {1, 0, 1, 0, 1, 0, 1, 2}, {1, 0, 2, 0, 1, 0, 1, 2}, {0, 0, 1, 0, 1, 0, 1, 2}, {0, 0, 1, 0, 1, 0, 1, 2},
    {2, 1, 0, 0, 0, 0, 1, 2}, {2, 1, 1, 2, 0, 0, 1, 2}, {0, 1, 0, 1, 0, 0, 1, 2}, {0, 1, 0, 1, 0, 0, 1, 2}, {1, 0, 0, 0, 0, 0, 1, 2}, {1, 0, 1, 0, 0, 0, 1, 2}, {0, 0, 0, 0, 0, 0, 1, 2}, {0, 0, 0, 0, 0, 0, 1, 2},
    {4, 3, 2, 1, 0, 0, 0, 1}, {4, 3, 3, 3, 3, 4, 0, 1}, {0, 3, 2, 2, 2, 3, 0, 1}, {0, 3, 2, 2, 2, 3, 0, 1}, {1, 0, 2, 1, 1, 2, 0, 1}, {1, 0, 3, 1, 1, 2, 0, 1}, {0, 0, 2, 1, 1, 2, 0, 1}, {0, 0, 2, 1, 1, 2, 0, 1},
    {2, 1, 0, 1, 0, 1, 0, 1}, {2, 1, 1, 3, 0, 1, 0, 1}, {0, 1, 0, 2, 0, 1, 0, 1}, {0, 1, 0, 2, 0, 1, 0, 1}, {1, 0, 0, 1, 0, 1, 0, 1}, {1, 0, 1, 1, 0, 1, 0, 1}, {0, 0, 0, 1, 0, 1, 0, 1}, {0, 0, 0, 1, 0, 1, 0, 1},
    {3, 2, 1, 0, 0, 0, 0, 1}, {3, 2, 2, 2, 3, 0, 0, 1}, {0, 2, 1, 1, 2, 0, 0, 1}, {0, 2, 1, 1, 2, 0, 0, 1}, {1, 0, 1, 0, 1, 0, 0, 1}, {1, 0, 2, 0, 1, 0, 0, 1}, {0, 0, 1, 0, 1, 0, 0, 1}, {0, 0, 1, 0, 1, 0, 0, 1},
    {2, 1, 0, 0, 0, 0, 0, 1}, {2, 1, 1, 2, 0, 0, 0, 1}, {0, 1, 0, 1, 0, 0, 0, 1}, {0, 1, 0, 1, 0, 0, 0, 1}, {1, 0, 0, 0, 0, 0, 0, 1}, {1, 0, 1, 0, 0, 0, 0, 1}, {0, 0, 0, 0, 0, 0, 0, 1}, {0, 0, 0, 0, 0, 0, 0, 1},
    {5, 4, 3, 2, 1, 0, 0, 0}, {5, 4, 4, 4, 4, 4, 5, 0}, {0, 4, 3, 3, 3, 3, 4, 0}, {0, 4, 3, 3, 3, 3, 4, 0}, {1, 0, 3, 2, 2, 2, 3, 0}, {1, 0, 4, 2, 2, 2, 3, 0}, {0, 0, 3, 2, 2, 2, 3, 0}, {0, 0, 3, 2, 2, 2, 3, 0},
    {2, 1, 0, 2, 1, 1, 2, 0}, {2, 1, 1, 4, 1, 1, 2, 0}, {0, 1, 0, 3, 1, 1, 2, 0}, {0, 1, 0, 3, 1, 1, 2, 0}, {1, 0, 0, 2, 1, 1, 2, 0}, {1, 0, 1, 2, 1, 1, 2, 0}, {0, 0, 0, 2, 1, 1, 2, 0}, {0, 0, 0, 2, 1, 1, 2, 0},
    {3, 2, 1, 0, 1, 0, 1, 0}, {3, 2, 2, 2, 4, 0, 1, 0}, {0, 2, 1, 1, 3, 0, 1, 0}, {0, 2, 1, 1, 3, 0, 1, 0}, {1, 0, 1, 0, 2, 0, 1, 0}, {1, 0, 2, 0, 2, 0, 1, 0}, {0, 0, 1, 0, 2, 0, 1, 0}, {0, 0, 1, 0, 2, 0, 1, 0},
    {2, 1, 0, 0, 1, 0, 1, 0}, {2, 1, 1, 2, 1, 0, 1, 0}, {0, 1, 0, 1, 1, 0, 1, 0}, {0, 1, 0, 1, 1, 0, 1, 0}, {1, 0, 0, 0, 1, 0, 1, 0}, {1, 0, 1, 0, 1, 0, 1, 0}, {0, 0, 0, 0, 1, 0, 1, 0}, {0, 0, 0, 0, 1, 0, 1, 0},
    {4, 3, 2, 1, 0, 0, 0, 0}, {4, 3, 3, 3, 3, 4, 0, 0}, {0, 3, 2, 2, 2, 3, 0, 0}, {0, 3, 2, 2, 2, 3, 0, 0}, {1, 0, 2, 1, 1, 2, 0, 0}, {1, 0, 3, 1, 1, 2, 0, 0}, {0, 0, 2, 1, 1, 2, 0, 0}, {0, 0, 2, 1, 1, 2, 0, 0},
    {2, 1, 0, 1, 0, 1, 0, 0}, {2, 1, 1, 3, 0, 1, 0, 0}, {0, 1, 0, 2, 0, 1, 0, 0}, {0, 1, 0, 2, 0, 1, 0, 0}, {1, 0, 0, 1, 0, 1, 0, 0}, {1, 0, 1, 1, 0, 1, 0, 0}, {0, 0, 0, 1, 0, 1, 0, 0}, {0, 0, 0, 1, 0, 1, 0, 0},
    {3, 2, 1, 0, 0, 0, 0, 0}, {3, 2, 2, 2, 3, 0, 0, 0}, {0, 2, 1, 1, 2, 0, 0, 0}, {0, 2, 1, 1, 2, 0, 0, 0}, {1, 0, 1, 0, 1, 0, 0, 0}, {1, 0, 2, 0, 1, 0, 0, 0}, {0, 0, 1, 0, 1, 0, 0, 0}, {0, 0, 1, 0, 1, 0, 0, 0},
    {2, 1, 0, 0, 0, 0, 0, 0}, {2, 1, 1, 2, 0, 0, 0, 0}, {0, 1, 0, 1, 0, 0, 0, 0}, {0, 1, 0, 1, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
    {6, 5, 4, 3, 2, 1, 0, 0}, {6, 5, 5, 5, 5, 5, 5, 6}, {0, 5, 4, 4, 4, 4, 4, 5}, {0, 5, 4, 4, 4, 4, 4, 5}, {1, 0, 4, 3, 3, 3, 3, 4}, {1, 0, 5, 3, 3, 3, 3, 4}, {0, 0, 4, 3, 3, 3, 3, 4}, {0, 0, 4, 3, 3, 3, 3, 4},
    {2, 1, 0, 3, 2, 2, 2, 3}, {2, 1, 1, 5, 2, 2, 2, 3}, {0, 1, 0, 4, 2, 2, 2, 3}, {0, 1, 0, 4, 2, 2, 2, 3}, {1, 0, 0, 3, 2, 2, 2, 3}, {1, 0, 1, 3, 2, 2, 2, 3}, {0, 0, 0, 3, 2, 2, 2, 3}, {0, 0, 0, 3, 2, 2, 2, 3},
    {3, 2, 1, 0, 2, 1, 1, 2}, {3, 2, 2, 2, 5, 1, 1, 2}, {0, 2, 1, 1, 4, 1, 1, 2}, {0, 2, 1, 1, 4, 1, 1, 2}, {1, 0, 1, 0, 3, 1, 1, 2}, {1, 0, 2, 0, 3, 1, 1, 2}, {0, 0, 1, 0, 3, 1, 1, 2}, {0, 0, 1, 0, 3, 1, 1, 2},
    {2, 1, 0, 0, 2, 1, 1, 2}, {2, 1, 1, 2, 2, 1, 1, 2}, {0, 1, 0, 1, 2, 1, 1, 2}, {0, 1, 0, 1, 2, 1, 1, 2}, {1, 0, 0, 0, 2, 1, 1, 2}, {1, 0, 1, 0, 2, 1, 1, 2}, {0, 0, 0, 0, 2, 1, 1, 2}, {0, 0, 0, 0, 2, 1, 1, 2},
    {4, 3, 2, 1, 0, 1, 0, 1}, {4, 3, 3, 3, 3, 5, 0, 1}, {0, 3, 2, 2, 2, 4, 0, 1}, {0, 3, 2, 2, 2, 4, 0, 1}, {1, 0, 2, 1, 1, 3, 0, 1}, {1, 0, 3, 1, 1, 3, 0, 1}, {0, 0, 2, 1, 1, 3, 0, 1}, {0, 0, 2, 1, 1, 3, 0, 1},
    {2, 1, 0, 1, 0, 2, 0, 1}, {2, 1, 1, 3, 0, 2, 0, 1}, {0, 1, 0, 2, 0, 2, 0, 1}, {0, 1, 0, 2, 0, 2, 0, 1}, {1, 0, 0, 1, 0, 2, 0, 1}, {1, 0, 1, 1, 0, 2, 0, 1}, {0, 0, 0, 1, 0, 2, 0, 1}, {0, 0, 0, 1, 0, 2, 0, 1},
    {3, 2, 1, 0, 0, 1, 0, 1}, {3, 2, 2, 2, 3, 1, 0, 1}, {0, 2, 1, 1, 2, 1, 0, 1}, {0, 2, 1, 1, 2, 1, 0, 1}, {1, 0, 1, 0, 1, 1, 0, 1}, {1, 0, 2, 0, 1, 1, 0, 1}, {0, 0, 1, 0, 1, 1, 0, 1}, {0, 0, 1, 0, 1, 1, 0, 1},
    {2, 1, 0, 0, 0, 1, 0, 1}, {2, 1, 1, 2, 0, 1, 0, 1}, {0, 1, 0, 1, 0, 1, 0, 1}, {0, 1, 0, 1, 0, 1, 0, 1}, {1, 0, 0, 0, 0, 1, 0, 1}, {1, 0, 1, 0, 0, 1, 0, 1}, {0, 0, 0, 0, 0, 1, 0, 1}, {0, 0, 0, 0, 0, 1, 0, 1},
    {5, 4, 3, 2, 1, 0, 0, 0}, {5, 4, 4, 4, 4, 4, 5, 0}, {0, 4, 3, 3, 3, 3, 4, 0}, {0, 4, 3, 3, 3, 3, 4, 0}, {1, 0, 3, 2, 2, 2, 3, 0}, {1, 0, 4, 2, 2, 2, 3, 0}, {0, 0, 3, 2, 2, 2, 3, 0}, {0, 0, 3, 2, 2, 2, 3, 0},
    {2, 1, 0, 2, 1, 1, 2, 0}, {2, 1, 1, 4, 1, 1, 2, 0}, {0, 1, 0, 3, 1, 1, 2, 0}, {0, 1, 0, 3, 1, 1, 2, 0}, {1, 0, 0, 2, 1, 1, 2, 0}, {1, 0, 1, 2, 1, 1, 2, 0}, {0, 0, 0, 2, 1, 1, 2, 0}, {0, 0, 0, 2, 1, 1, 2, 0},
    {3, 2, 1, 0, 1, 0, 1, 0}, {3, 2, 2, 2, 4, 0, 1, 0}, {0, 2, 1, 1, 3, 0, 1, 0}, {0, 2, 1, 1, 3, 0, 1, 0}, {1, 0, 1, 0, 2, 0, 1, 0}, {1, 0, 2, 0, 2, 0, 1, 0}, {0, 0, 1, 0, 2, 0, 1, 0}, {0, 0, 1, 0, 2, 0, 1, 0},
    {2, 1, 0, 0, 1, 0, 1, 0}, {2, 1, 1, 2, 1, 0, 1, 0}, {0, 1, 0, 1, 1, 0, 1, 0}, {0, 1, 0, 1, 1, 0, 1, 0}, {1, 0, 0, 0, 1, 0, 1, 0}, {1, 0, 1, 0, 1, 0, 1, 0}, {0, 0, 0, 0, 1, 0, 1, 0}, {0, 0, 0, 0, 1, 0, 1, 0},
    {4, 3, 2, 1, 0, 0, 0, 0}, {4, 3, 3, 3, 3, 4, 0, 0}, {0, 3, 2, 2, 2, 3, 0, 0}, {0, 3, 2, 2, 2, 3, 0, 0}, {1, 0, 2, 1, 1, 2, 0, 0}, {1, 0, 3, 1, 1, 2, 0, 0}, {0, 0, 2, 1, 1, 2, 0, 0}, {0, 0, 2, 1, 1, 2, 0, 0},
    {2, 1, 0, 1, 0, 1, 0, 0}, {2, 1, 1, 3, 0, 1, 0, 0}, {0, 1, 0, 2, 0, 1, 0, 0}, {0, 1, 0, 2, 0, 1, 0, 0}, {1, 0, 0, 1, 0, 1, 0, 0}, {1, 0, 1, 1, 0, 1, 0, 0}, {0, 0, 0, 1, 0, 1, 0, 0}, {0, 0, 0, 1, 0, 1, 0, 0},
    {3, 2, 1, 0, 0, 0, 0, 0}, {3, 2, 2, 2, 3, 0, 0, 0}, {0, 2, 1, 1, 2, 0, 0, 0}, {0, 2, 1, 1, 2, 0, 0, 0}, {1, 0, 1, 0, 1, 0, 0, 0}, {1, 0, 2, 0, 1, 0, 0, 0}, {0, 0, 1, 0, 1, 0, 0, 0}, {0, 0, 1, 0, 1, 0, 0, 0},
    {2, 1, 0, 0, 0, 0, 0, 0}, {2, 1, 1, 2, 0, 0, 0, 0}, {0, 1, 0, 1, 0, 0, 0, 0}, {0, 1, 0, 1, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}
};

/*
    @brief calculate number of flipped discs in the last move

    @param player               a bitboard representing player
    @param place                a place to put
    @return number of flipping discs
*/
inline int_fast8_t count_last_flip(uint64_t player, const uint_fast8_t place) {
    const int x = place & 7;
    const int y = place >> 3;
    return
        n_flip_pre_calc[join_h_line(player, y)][x] + 
        n_flip_pre_calc[join_v_line(player, x)][y] + 
        n_flip_pre_calc[join_d7_line(player, x + y)][std::min(y, 7 - x)] + 
        n_flip_pre_calc[join_d9_line(player, x + 7 - y)][std::min(y, x)];
}

inline void last_flip_init() {
}