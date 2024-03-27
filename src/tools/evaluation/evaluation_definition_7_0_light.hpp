/*
    Egaroucid Project

    @file evaluation_definition.hpp
        Evaluation Function Definition
    @date 2021-2024
    @author Takuto Yamana
    @license GPL-3.0 license
*/

#pragma once
#ifndef OPTIMIZER_INCLUDE
    #include "./../../engine/board.hpp"
#endif

#define EVAL_DEFINITION_NAME "20240327_1"
#define EVAL_DEFINITION_DESCRIPTION "light pattern + n_discs_of_player + n_surround_of_both"

#ifndef HW
    #define HW 8
#endif

#ifndef HW2
    #define HW2 64
#endif

#ifndef SCORE_MAX
    #define SCORE_MAX 64
#endif

/*
    @brief evaluation pattern definition
*/
// disc pattern
#define ADJ_N_PATTERNS 16
#define ADJ_N_SYMMETRY_PATTERNS 62
#define ADJ_MAX_PATTERN_CELLS 9

// additional features
#define ADJ_N_ADDITIONAL_EVALS 2
#define ADJ_MAX_SURROUND 64
#define ADJ_MAX_STONE_NUM 65

// overall
#define ADJ_MAX_EVALUATE_IDX 19683
#define ADJ_N_EVAL (16 + 2)
#define ADJ_N_FEATURES (62 + 2)
#define N_FLOOR_UNIQUE_FEATURES 16 // floorpow2(ADJ_N_EVAL): 16-31->16 32-63->32

// phase
#define ADJ_N_PHASES 60
#define ADJ_N_PHASE_DISCS 1 // 60 / ADJ_N_PHASES

//#define ADJ_SCORE_MAX HW2

/*
    @brief value definition

    Raw score is STEP times larger than the real score.
*/
#define ADJ_STEP 32
#define ADJ_STEP_2 16

#ifndef PNO
    /*
        @brief 3 ^ N definition
    */
    #define PNO 0
    #define P30 1
    #define P31 3
    #define P32 9
    #define P33 27
    #define P34 81
    #define P35 243
    #define P36 729
    #define P37 2187
    #define P38 6561
    #define P39 19683
    #define P310 59049
#endif

#ifndef COORD_NO
    /*
        @brief coordinate definition
    */
    #define COORD_A1 63
    #define COORD_B1 62
    #define COORD_C1 61
    #define COORD_D1 60
    #define COORD_E1 59
    #define COORD_F1 58
    #define COORD_G1 57
    #define COORD_H1 56

    #define COORD_A2 55
    #define COORD_B2 54
    #define COORD_C2 53
    #define COORD_D2 52
    #define COORD_E2 51
    #define COORD_F2 50
    #define COORD_G2 49
    #define COORD_H2 48

    #define COORD_A3 47
    #define COORD_B3 46
    #define COORD_C3 45
    #define COORD_D3 44
    #define COORD_E3 43
    #define COORD_F3 42
    #define COORD_G3 41
    #define COORD_H3 40

    #define COORD_A4 39
    #define COORD_B4 38
    #define COORD_C4 37
    #define COORD_D4 36
    #define COORD_E4 35
    #define COORD_F4 34
    #define COORD_G4 33
    #define COORD_H4 32

    #define COORD_A5 31
    #define COORD_B5 30
    #define COORD_C5 29
    #define COORD_D5 28
    #define COORD_E5 27
    #define COORD_F5 26
    #define COORD_G5 25
    #define COORD_H5 24

    #define COORD_A6 23
    #define COORD_B6 22
    #define COORD_C6 21
    #define COORD_D6 20
    #define COORD_E6 19
    #define COORD_F6 18
    #define COORD_G6 17
    #define COORD_H6 16

    #define COORD_A7 15
    #define COORD_B7 14
    #define COORD_C7 13
    #define COORD_D7 12
    #define COORD_E7 11
    #define COORD_F7 10
    #define COORD_G7 9
    #define COORD_H7 8

    #define COORD_A8 7
    #define COORD_B8 6
    #define COORD_C8 5
    #define COORD_D8 4
    #define COORD_E8 3
    #define COORD_F8 2
    #define COORD_G8 1
    #define COORD_H8 0

    #define COORD_NO 64
#endif

constexpr int adj_pow3[11] = {P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P310};

/*
    @brief definition of patterns in evaluation function

    pattern -> coordinate

    @param n_cells              number of cells included in the pattern
    @param cells                coordinates of each cell
*/
struct Adj_Feature_to_coord{
    uint_fast8_t n_cells;
    uint_fast8_t cells[ADJ_MAX_PATTERN_CELLS];
};

constexpr Adj_Feature_to_coord adj_feature_to_coord[ADJ_N_SYMMETRY_PATTERNS] = {
    // 0 hv2
    {8, {COORD_A2, COORD_B2, COORD_C2, COORD_D2, COORD_E2, COORD_F2, COORD_G2, COORD_H2, COORD_NO}}, // 0
    {8, {COORD_B1, COORD_B2, COORD_B3, COORD_B4, COORD_B5, COORD_B6, COORD_B7, COORD_B8, COORD_NO}}, // 1
    {8, {COORD_A7, COORD_B7, COORD_C7, COORD_D7, COORD_E7, COORD_F7, COORD_G7, COORD_H7, COORD_NO}}, // 2
    {8, {COORD_G1, COORD_G2, COORD_G3, COORD_G4, COORD_G5, COORD_G6, COORD_G7, COORD_G8, COORD_NO}}, // 3

    // 1 hv3
    {8, {COORD_A3, COORD_B3, COORD_C3, COORD_D3, COORD_E3, COORD_F3, COORD_G3, COORD_H3, COORD_NO}}, // 4
    {8, {COORD_C1, COORD_C2, COORD_C3, COORD_C4, COORD_C5, COORD_C6, COORD_C7, COORD_C8, COORD_NO}}, // 5
    {8, {COORD_A6, COORD_B6, COORD_C6, COORD_D6, COORD_E6, COORD_F6, COORD_G6, COORD_H6, COORD_NO}}, // 6
    {8, {COORD_F1, COORD_F2, COORD_F3, COORD_F4, COORD_F5, COORD_F6, COORD_F7, COORD_F8, COORD_NO}}, // 7

    // 2 hv4
    {8, {COORD_A4, COORD_B4, COORD_C4, COORD_D4, COORD_E4, COORD_F4, COORD_G4, COORD_H4, COORD_NO}}, // 8
    {8, {COORD_D1, COORD_D2, COORD_D3, COORD_D4, COORD_D5, COORD_D6, COORD_D7, COORD_D8, COORD_NO}}, // 9
    {8, {COORD_A5, COORD_B5, COORD_C5, COORD_D5, COORD_E5, COORD_F5, COORD_G5, COORD_H5, COORD_NO}}, // 10
    {8, {COORD_E1, COORD_E2, COORD_E3, COORD_E4, COORD_E5, COORD_E6, COORD_E7, COORD_E8, COORD_NO}}, // 11

    // 3 d5 + corner + X
    {7, {COORD_D1, COORD_E2, COORD_F3, COORD_G4, COORD_H5, COORD_H1, COORD_G2, COORD_NO, COORD_NO}}, // 12
    {7, {COORD_E1, COORD_D2, COORD_C3, COORD_B4, COORD_A5, COORD_A1, COORD_B2, COORD_NO, COORD_NO}}, // 13
    {7, {COORD_A4, COORD_B5, COORD_C6, COORD_D7, COORD_E8, COORD_A8, COORD_B7, COORD_NO, COORD_NO}}, // 14
    {7, {COORD_H4, COORD_G5, COORD_F6, COORD_E7, COORD_D8, COORD_H8, COORD_G7, COORD_NO, COORD_NO}}, // 15

    // 4 d6
    {6, {COORD_C1, COORD_D2, COORD_E3, COORD_F4, COORD_G5, COORD_H6, COORD_NO, COORD_NO, COORD_NO}}, // 16
    {6, {COORD_F1, COORD_E2, COORD_D3, COORD_C4, COORD_B5, COORD_A6, COORD_NO, COORD_NO, COORD_NO}}, // 17
    {6, {COORD_A3, COORD_B4, COORD_C5, COORD_D6, COORD_E7, COORD_F8, COORD_NO, COORD_NO, COORD_NO}}, // 18
    {6, {COORD_H3, COORD_G4, COORD_F5, COORD_E6, COORD_D7, COORD_C8, COORD_NO, COORD_NO, COORD_NO}}, // 19

    // 5 d7 + corner
    {9, {COORD_A1, COORD_B1, COORD_C2, COORD_D3, COORD_E4, COORD_F5, COORD_G6, COORD_H7, COORD_H8}}, // 20
    {9, {COORD_H1, COORD_G1, COORD_F2, COORD_E3, COORD_D4, COORD_C5, COORD_B6, COORD_A7, COORD_A8}}, // 21
    {9, {COORD_A1, COORD_A2, COORD_B3, COORD_C4, COORD_D5, COORD_E6, COORD_F7, COORD_G8, COORD_H8}}, // 22
    {9, {COORD_H1, COORD_H2, COORD_G3, COORD_F4, COORD_E5, COORD_D6, COORD_C7, COORD_B8, COORD_A8}}, // 23

    // 6 d8
    {8, {COORD_A1, COORD_B2, COORD_C3, COORD_D4, COORD_E5, COORD_F6, COORD_G7, COORD_H8, COORD_NO}}, // 24
    {8, {COORD_H1, COORD_G2, COORD_F3, COORD_E4, COORD_D5, COORD_C6, COORD_B7, COORD_A8, COORD_NO}}, // 25

    // 7 corner-edge + 2x
    {8, {COORD_B2, COORD_A1, COORD_B1, COORD_C1, COORD_F1, COORD_G1, COORD_H1, COORD_G2, COORD_NO}}, // 26
    {8, {COORD_B2, COORD_A1, COORD_A2, COORD_A3, COORD_A6, COORD_A7, COORD_A8, COORD_B7, COORD_NO}}, // 27
    {8, {COORD_B7, COORD_A8, COORD_B8, COORD_C8, COORD_F8, COORD_G8, COORD_H8, COORD_G7, COORD_NO}}, // 28
    {8, {COORD_G2, COORD_H1, COORD_H2, COORD_H3, COORD_H6, COORD_H7, COORD_H8, COORD_G7, COORD_NO}}, // 29

    // 8 small triangle
    {8, {COORD_A1, COORD_B1, COORD_C1, COORD_D1, COORD_A2, COORD_B2, COORD_A3, COORD_A4, COORD_NO}}, // 30
    {8, {COORD_H1, COORD_G1, COORD_F1, COORD_E1, COORD_H2, COORD_G2, COORD_H3, COORD_H4, COORD_NO}}, // 31
    {8, {COORD_A8, COORD_B8, COORD_C8, COORD_D8, COORD_A7, COORD_B7, COORD_A6, COORD_A5, COORD_NO}}, // 32
    {8, {COORD_H8, COORD_G8, COORD_F8, COORD_E8, COORD_H7, COORD_G7, COORD_H6, COORD_H5, COORD_NO}}, // 33

    // 9 corner + small-block
    {8, {COORD_A1, COORD_C1, COORD_D1, COORD_E1, COORD_F1, COORD_H1, COORD_C2, COORD_F2, COORD_NO}}, // 34
    {8, {COORD_A1, COORD_A3, COORD_A4, COORD_A5, COORD_A6, COORD_A8, COORD_B3, COORD_B6, COORD_NO}}, // 35
    {8, {COORD_A8, COORD_C8, COORD_D8, COORD_E8, COORD_F8, COORD_H8, COORD_C7, COORD_F7, COORD_NO}}, // 36
    {8, {COORD_H1, COORD_H3, COORD_H4, COORD_H5, COORD_H6, COORD_H8, COORD_G3, COORD_G6, COORD_NO}}, // 37

    // 10 corner8
    {8, {COORD_A1, COORD_B1, COORD_C1, COORD_A2, COORD_B2, COORD_C2, COORD_A3, COORD_B3, COORD_NO}}, // 38
    {8, {COORD_H1, COORD_G1, COORD_F1, COORD_H2, COORD_G2, COORD_F2, COORD_H3, COORD_G3, COORD_NO}}, // 39
    {8, {COORD_A8, COORD_B8, COORD_C8, COORD_A7, COORD_B7, COORD_C7, COORD_A6, COORD_B6, COORD_NO}}, // 40
    {8, {COORD_H8, COORD_G8, COORD_F8, COORD_H7, COORD_G7, COORD_F7, COORD_H6, COORD_G6, COORD_NO}}, // 41

    // 11 corner-stability + 2 corner
    {8, {COORD_A1, COORD_A6, COORD_A7, COORD_A8, COORD_B7, COORD_B8, COORD_C8, COORD_H8, COORD_NO}}, // 42
    {8, {COORD_H8, COORD_H3, COORD_H2, COORD_H1, COORD_G2, COORD_G1, COORD_F1, COORD_A1, COORD_NO}}, // 43
    {8, {COORD_H1, COORD_H6, COORD_H7, COORD_H8, COORD_G7, COORD_G8, COORD_F8, COORD_A8, COORD_NO}}, // 44
    {8, {COORD_A8, COORD_A3, COORD_A2, COORD_A1, COORD_B2, COORD_B1, COORD_C1, COORD_H1, COORD_NO}}, // 45

    // 12 half center-block
    {8, {COORD_C3, COORD_D3, COORD_C4, COORD_D4, COORD_D5, COORD_C5, COORD_D6, COORD_C6, COORD_NO}}, // 46
    {8, {COORD_F6, COORD_E6, COORD_F5, COORD_E5, COORD_E4, COORD_F4, COORD_E3, COORD_F3, COORD_NO}}, // 47
    {8, {COORD_C3, COORD_C4, COORD_D3, COORD_D4, COORD_E4, COORD_E3, COORD_F4, COORD_F3, COORD_NO}}, // 48
    {8, {COORD_F6, COORD_F5, COORD_E6, COORD_E5, COORD_D5, COORD_D6, COORD_C5, COORD_C6, COORD_NO}}, // 49

    // 13 4x2-A
    {8, {COORD_A1, COORD_B1, COORD_A2, COORD_B2, COORD_C3, COORD_D3, COORD_C4, COORD_D4, COORD_NO}}, // 50
    {8, {COORD_H1, COORD_H2, COORD_G1, COORD_G2, COORD_F3, COORD_F4, COORD_E3, COORD_E4, COORD_NO}}, // 51
    {8, {COORD_H8, COORD_G8, COORD_H7, COORD_G7, COORD_F6, COORD_E6, COORD_F5, COORD_E5, COORD_NO}}, // 52
    {8, {COORD_A8, COORD_A7, COORD_B8, COORD_B7, COORD_C6, COORD_C5, COORD_D6, COORD_D5, COORD_NO}}, // 53

    // 14 4x2-B
    {8, {COORD_C1, COORD_D1, COORD_C2, COORD_D2, COORD_A3, COORD_B3, COORD_A4, COORD_B4, COORD_NO}}, // 54
    {8, {COORD_H3, COORD_H4, COORD_G3, COORD_G4, COORD_F1, COORD_F2, COORD_E1, COORD_E2, COORD_NO}}, // 55
    {8, {COORD_F8, COORD_E8, COORD_F7, COORD_E7, COORD_H6, COORD_G6, COORD_H5, COORD_G5, COORD_NO}}, // 56
    {8, {COORD_A6, COORD_A5, COORD_B6, COORD_B5, COORD_C8, COORD_C7, COORD_D8, COORD_D7, COORD_NO}}, // 57

    // 15 4x2-C
    {8, {COORD_A1, COORD_B1, COORD_A2, COORD_B2, COORD_B7, COORD_A7, COORD_B8, COORD_A8, COORD_NO}}, // 58
    {8, {COORD_H1, COORD_H2, COORD_G1, COORD_G2, COORD_B2, COORD_B1, COORD_A2, COORD_A1, COORD_NO}}, // 59
    {8, {COORD_H8, COORD_G8, COORD_H7, COORD_G7, COORD_G2, COORD_H2, COORD_G1, COORD_H1, COORD_NO}}, // 60
    {8, {COORD_A8, COORD_A7, COORD_B8, COORD_B7, COORD_G7, COORD_G8, COORD_H7, COORD_H8, COORD_NO}}  // 61
};

constexpr int adj_pattern_n_cells[ADJ_N_PATTERNS] = {
    8, 8, 8, 7, 
    6, 9, 8, 8, 
    8, 8, 8, 8, 
    8, 8, 8, 8
};

constexpr int adj_rev_patterns[ADJ_N_PATTERNS][ADJ_MAX_PATTERN_CELLS] = {
    {7, 6, 5, 4, 3, 2, 1, 0}, // 0 hv2
    {7, 6, 5, 4, 3, 2, 1, 0}, // 1 hv3
    {7, 6, 5, 4, 3, 2, 1, 0}, // 2 hv4
    {4, 3, 2, 1, 0}, // 3 d5
    {5, 4, 3, 2, 1, 0}, // 4 d6
    {6, 5, 4, 3, 2, 1, 0}, // 5 d7
    {7, 6, 5, 4, 3, 2, 1, 0}, // 6 d8
    {0, 3, 6, 1, 4, 7, 2, 5, 8}, // 7 corner9
    {9, 8, 7, 6, 5, 4, 3, 2, 1, 0}, // 8 edge + 2x
    {0, 4, 7, 9, 1, 5, 8, 2, 6, 3}, // 9 triangle
    {5, 4, 3, 2, 1, 0, 9, 8, 7, 6}, // 10 corner + block
    {0, 1, 2, 3, 7, 8, 9, 4, 5, 6}, // 11 cross
    {9, 8, 7, 6, 5, 4, 3, 2, 1, 0}, // 12 edge + y
    {0, 5, 7, 8, 9, 1, 6, 2, 3, 4}, // 13 narrow triangle
    {0, 2, 1, 3, 6, 8, 4, 7, 5, 9}, // 14 fish
    {0, 2, 1, 3, 7, 8, 9, 4, 5, 6}  // 15 kite
};

constexpr int adj_eval_sizes[ADJ_N_EVAL] = {
    P38, P38, P38, P35, P36, P37, P38, P39, 
    P310, P310, P310, P310, P310, P310, P310, P310, 
    ADJ_MAX_STONE_NUM, 
    ADJ_MAX_SURROUND * ADJ_MAX_SURROUND
};

constexpr int adj_feature_to_eval_idx[ADJ_N_FEATURES] = {
    0, 0, 0, 0, 
    1, 1, 1, 1, 
    2, 2, 2, 2, 
    3, 3, 3, 3, 
    4, 4, 4, 4, 
    5, 5, 5, 5, 
    6, 6, 
    7, 7, 7, 7, 
    8, 8, 8, 8, 
    9, 9, 9, 9, 
    10, 10, 10, 10, 
    11, 11, 11, 11, 
    12, 12, 12, 12, 
    13, 13, 13, 13, 
    14, 14, 14, 14, 
    15, 15, 15, 15, 
    16, 
    17
};

int adj_pick_digit3(int num, int d, int n_digit){
    num /= adj_pow3[n_digit - 1 - d];
    return num % 3;
}

int adj_pick_digit2(int num, int d){
    return 1 & (num >> d);
}

uint16_t adj_calc_rev_idx(int feature, int idx){
    uint16_t res = 0;
    if (feature < ADJ_N_PATTERNS){
        for (int i = 0; i < adj_pattern_n_cells[feature]; ++i){
            res += adj_pick_digit3(idx, adj_rev_patterns[feature][i], adj_pattern_n_cells[feature]) * adj_pow3[adj_pattern_n_cells[feature] - 1 - i];
        }
    } else{
        res = idx;
    }
    return res;
}

#ifndef OPTIMIZER_INCLUDE

int adj_calc_num_feature(Board *board){
    return pop_count_ull(board->player); // board->opponent is unnecessary because of the 60 phases
}

/*
    @brief calculate surround value used in evaluation function

    @param player               a bitboard representing player
    @param empties              a bitboard representing empties
    @return surround value
*/
inline int calc_surround(uint64_t discs, uint64_t empties){
    uint64_t hmask = discs & 0x7E7E7E7E7E7E7E7EULL;
    uint64_t vmask = discs & 0x00FFFFFFFFFFFF00ULL;
    uint64_t hvmask = discs & 0x007E7E7E7E7E7E00ULL;
    uint64_t res = 
        (hmask << 1) | (hmask >> 1) | 
        (vmask << HW) | (vmask >> HW) | 
        (hvmask << HW_M1) | (hvmask >> HW_M1) | 
        (hvmask << HW_P1) | (hvmask >> HW_P1);
    return pop_count_ull(empties & res);
}

int adj_calc_surround_feature(Board *board){
    return calc_surround(board->player, ~(board->player | board->opponent)) * ADJ_MAX_SURROUND + calc_surround(board->opponent, ~(board->player | board->opponent));
}

inline int adj_pick_pattern(const uint_fast8_t b_arr[], int pattern_idx){
    int res = 0;
    for (int i = 0; i < adj_feature_to_coord[pattern_idx].n_cells; ++i){
        res *= 3;
        res += b_arr[adj_feature_to_coord[pattern_idx].cells[i]];
    }
    return res;
}

void adj_calc_features(Board *board, uint16_t res[]){
    uint_fast8_t b_arr[HW2];
    board->translate_to_arr_player(b_arr);
    int idx = 0;
    for (int i = 0; i < ADJ_N_SYMMETRY_PATTERNS; ++i)
        res[idx++] = adj_pick_pattern(b_arr, i);
    res[idx++] = adj_calc_num_feature(board);
    res[idx++] = adj_calc_surround_feature(board);
}

int calc_phase(Board *board, int16_t player){
    return (pop_count_ull(board->player | board->opponent) - 4) / ADJ_N_PHASE_DISCS;
}

void evaluation_definition_init(){
    mobility_init();
}

#endif