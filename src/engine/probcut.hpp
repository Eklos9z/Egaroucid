/*
    Egaroucid Project

    @file probcut.hpp
        MPC (Multi-ProbCut)
    @date 2021-2024
    @author Takuto Yamana
    @license GPL-3.0 license
*/

#pragma once
#include <iostream>
#include "board.hpp"
#include "evaluate.hpp"
#include "search.hpp"
#include "midsearch.hpp"
#include "util.hpp"

#define USE_MPC_DEPTH 3
#define MAX_MPC_DEPTH_PRESEARCH 22

#if USE_ALL_NODE_PREDICTION
    #define ALL_NODE_CHECK_MPCT 1.8
#endif

// constants from standard normal distribution table
// constexpr int SELECTIVITY_PERCENTAGE[N_SELECTIVITY_LEVEL] = {74, 88, 93, 98, 99, 100};
// two-sided test
// for normal MPC                                                74.0  88.0  93.0  98.0  99.0
constexpr double SELECTIVITY_MPCT_SINGLE[N_SELECTIVITY_LEVEL] = {1.13, 1.55, 1.81, 2.32, 2.57, 9.99};
// for multi-MPC                                                54.8  77.4  86.5  96.0  98.0
constexpr double SELECTIVITY_MPCT_MULTI[N_SELECTIVITY_LEVEL] = {0.75, 1.21, 1.49, 2.05, 2.32, 9.99};

/*
    @brief constants for ProbCut error calculation
*/
#define probcut_a 0.93740805837003
#define probcut_b -7.340323137961951
#define probcut_c 1.1401695320187872
#define probcut_d 0.700567733735339
#define probcut_e 2.662003673678691
#define probcut_f 3.0554301965778063
#define probcut_g 2.0942574977708674

#define probcut_end_a 1.0991215402719048
#define probcut_end_b 2.1818828716953917
#define probcut_end_c -5.222054343259216
#define probcut_end_d 5.266556448300817
#define probcut_end_e -3.406095066993692
#define probcut_end_f 8.038123155263888

#if USE_MPC_PRE_CALCULATION
    int mpc_error_single0[N_SELECTIVITY_LEVEL][HW2 + 1][HW2 - 3];
    int mpc_error_end_single0[N_SELECTIVITY_LEVEL][HW2 + 1];
    int mpc_error_multi[N_SELECTIVITY_LEVEL][HW2 + 1][HW2 - 3][HW2 - 3];
    int mpc_error_end_multi[N_SELECTIVITY_LEVEL][HW2 + 1][HW2 - 3];
#endif

constexpr int mpc_search_depth_arr[2][61] = {
    { // midgame
         0,  0,  0,  1,  2,  3,  2,  3,  4,  5, 
         4,  5,  6,  7,  6,  7,  8,  9,  8,  9, 
        10, 11, 10, 11, 12, 13, 12, 13, 14, 15, 
        14, 15, 16, 17, 16, 17, 18, 19, 18, 19, 
        20, 21, 20, 21, 22, 23, 22, 23, 24, 25, 
        24, 25, 26, 27, 26, 27, 28, 29, 28, 29, 
        30
    }, { // endgame
         0,  1,  0,  1,  2,  3,  2,  3,  2,  3, 
         2,  3,  2,  3,  2,  3,  4,  5,  4,  5, 
         4,  5,  4,  5,  6,  7,  6,  7,  6,  7, 
         6,  7,  8,  9,  8,  9,  8,  9,  8,  9, 
        10, 11, 10, 11, 10, 11, 10, 11, 12, 13, 
        12, 13, 12, 13, 12, 13, 14, 15, 14, 15, 
        14
    }
};

/*
    @brief ProbCut error calculation for midgame

    @param n_discs              number of discs on the board
    @param depth1               depth of shallow search
    @param depth2               depth of deep search
    @return expected error
*/
inline double probcut_sigma(int n_discs, int depth1, int depth2) {
    double res = probcut_a * ((double)n_discs / 64.0) + probcut_b * ((double)depth1 / 60.0) + probcut_c * ((double)depth2 / 60.0);
    res = probcut_d * res * res * res + probcut_e * res * res + probcut_f * res + probcut_g;
    return res;
}

/*
    @brief ProbCut error calculation for midgame (when shallow depth = 0)

    @param n_discs              number of discs on the board
    @param depth2               depth of deep search
    @return expected error
*/
inline double probcut_sigma_depth0(int n_discs, int depth2) {
    double res = probcut_a * ((double)n_discs / 64.0) + probcut_c * ((double)depth2 / 60.0);
    res = probcut_d * res * res * res + probcut_e * res * res + probcut_f * res + probcut_g;
    return res;
}

/*
    @brief ProbCut error calculation for endgame

    @param n_discs              number of discs on the board
    @param depth                depth of shallow search
    @return expected error
*/
inline double probcut_sigma_end(int n_discs, int depth) {
    double res = probcut_end_a * ((double)n_discs / 64.0) + probcut_end_b * ((double)depth / 60.0);
    res = probcut_end_c * res * res * res + probcut_end_d * res * res + probcut_end_e * res + probcut_end_f;
    return res;
}

/*
    @brief ProbCut error calculation for endgame (when shallow depth = 0)

    @param n_discs              number of discs on the board
    @return expected error
*/
inline double probcut_sigma_end_depth0(int n_discs) {
    double res = probcut_end_a * ((double)n_discs / 64.0);
    res = probcut_end_c * res * res * res + probcut_end_d * res * res + probcut_end_e * res + probcut_end_f;
    return res;
}

#if USE_NEGA_ALPHA_ORDERING
    int nega_alpha_ordering(Search *search, int alpha, int beta, int depth, bool skipped, uint64_t legal, const bool is_end_search, bool *searching);
#endif
int nega_alpha_ordering_nws(Search *search, int alpha, int depth, bool skipped, uint64_t legal, const bool is_end_search, bool *searching);

/*
    @brief Multi-ProbCut for normal search

    @param search               search information
    @param alpha                alpha value
    @param beta                 beta value
    @param depth                depth of deep search
    @param legal                for use of previously calculated legal bitboard
    @param is_end_search        search till the end?
    @param v                    an integer to store result
    @param searching            flag for terminating this search
    @return cutoff occurred?
*/
inline bool mpc(Search* search, int alpha, int beta, int depth, uint64_t legal, const bool is_end_search, int* v, bool *searching) {
    if (search->mpc_level == MPC_100_LEVEL || depth < USE_MPC_DEPTH || (search->is_presearch && depth >= MAX_MPC_DEPTH_PRESEARCH))
        return false;
    int search_depth = mpc_search_depth_arr[is_end_search][depth];
    if (search_depth == 0) {
        int error;
        #if USE_MPC_PRE_CALCULATION
            if (is_end_search) {
                error = mpc_error_end_single0[search->mpc_level][search->n_discs];
            } else{
                error = mpc_error_single0[search->mpc_level][search->n_discs][depth];
            }
        #else
            double mpct = SELECTIVITY_MPCT_SINGLE[search->mpc_level];
            if (is_end_search) {
                error = ceil(mpct * probcut_sigma_end(search->n_discs, 0));
            }else{
                error = ceil(mpct * probcut_sigma(search->n_discs, 0, depth));
            }
        #endif
        int d0value = mid_evaluate_diff(search);
        if (d0value >= beta + error) {
            *v = beta;
            if (is_end_search)
                *v += beta & 1;
            return true;
        }
        if (d0value <= alpha - error) {
            *v = alpha;
            if (is_end_search)
                *v -= alpha & 1;
            return true;
        }
    } else{
        int error_search, error_0;
        uint_fast8_t mpc_level = search->mpc_level;
        #if USE_MPC_PRE_CALCULATION
            if (is_end_search) {
                error_search = mpc_error_end_multi[mpc_level][search->n_discs][search_depth];
                error_0 = mpc_error_end_multi[mpc_level][search->n_discs][0];
            } else{
                error_search = mpc_error_multi[mpc_level][search->n_discs][search_depth][depth];
                error_0 = mpc_error_multi[mpc_level][search->n_discs][0][depth];
            }
        #else
            double mpct = SELECTIVITY_MPCT_MULTI[mpc_level];
            if (is_end_search) {
                error_search = ceil(mpct * probcut_sigma_end(search->n_discs, search_depth));
                error_0 = ceil(mpct * probcut_sigma_end(search->n_discs, 0));
            }else{
                error_search = ceil(mpct * probcut_sigma(search->n_discs, search_depth, depth));
                error_0 = ceil(mpct * probcut_sigma(search->n_discs, 0, depth));
            }
        #endif
        int d0value = mid_evaluate_diff(search);
        search->mpc_level = MPC_100_LEVEL;
        if (d0value >= beta + error_0) {
            int pc_beta = beta + error_search;
            if (pc_beta <= SCORE_MAX) {
                if (nega_alpha_ordering_nws(search, pc_beta - 1, search_depth, false, legal, false, searching) >= pc_beta) {
                    *v = beta;
                    if (is_end_search)
                        *v += beta & 1;
                    search->mpc_level = mpc_level;
                    return true;
                }
            }
        }
        if (d0value <= alpha - error_0) {
            int pc_alpha = alpha - error_search;
            if (pc_alpha >= -SCORE_MAX) {
                if (nega_alpha_ordering_nws(search, pc_alpha, search_depth, false, legal, false, searching) <= pc_alpha) {
                    *v = alpha;
                    if (is_end_search)
                        *v -= alpha & 1;
                    search->mpc_level = mpc_level;
                    return true;
                }
            }
        }
        search->mpc_level = mpc_level;
    }
    return false;
}


#if USE_ALL_NODE_PREDICTION_NWS
    inline bool predict_all_node(Search* search, int alpha, int depth, uint64_t legal, const bool is_end_search, bool *searching) {
        uint_fast8_t mpc_level = MPC_93_LEVEL;
        int search_depth = mpc_search_depth_arr[is_end_search][depth];
        int error_search, error_0;
        #if USE_MPC_PRE_CALCULATION
            if (is_end_search) {
                error_search = mpc_error_end[mpc_level][search->n_discs][search_depth];
                error_0 = mpc_error_end[mpc_level][search->n_discs][0];
            } else{
                error_search = mpc_error[mpc_level][search->n_discs][search_depth][depth];
                error_0 = mpc_error[mpc_level][search->n_discs][0][depth];
            }
        #else
            double mpct = SELECTIVITY_MPCT[mpc_level];
            if (is_end_search) {
                error_search = ceil(mpct * probcut_sigma_end(search->n_discs, search_depth));
                error_0 = ceil(mpct * probcut_sigma_end(search->n_discs, 0));
            }else{
                error_search = ceil(mpct * probcut_sigma(search->n_discs, search_depth, depth));
                error_0 = ceil(mpct * probcut_sigma(search->n_discs, 0, depth));
            }
        #endif
        int d0value = mid_evaluate_diff(search);
        if (d0value <= alpha - (error_search + error_0) / 2) {
            int pc_alpha = alpha - error_search;
            if (pc_alpha > -SCORE_MAX) {
                if (nega_alpha_ordering_nws(search, pc_alpha, search_depth, false, legal, false, searching) <= pc_alpha) {
                    return true;
                }
            }
        }
        return false;
    }
#endif



#if USE_MPC_PRE_CALCULATION
    void mpc_init() {
        int mpc_level, n_discs, depth1, depth2;
        for (mpc_level = 0; mpc_level < N_SELECTIVITY_LEVEL; ++mpc_level) {
            for (n_discs = 0; n_discs < HW2 + 1; ++n_discs) {
                mpc_error_end_single0[mpc_level][n_discs] = ceil(SELECTIVITY_MPCT_SINGLE[mpc_level] * probcut_sigma_end(n_discs, 0));
                for (depth2 = 0; depth2 < HW2 - 3; ++depth2)
                    mpc_error_single0[mpc_level][n_discs][depth2] = ceil(SELECTIVITY_MPCT_SINGLE[mpc_level] * probcut_sigma(n_discs, 0, depth2));
                for (depth1 = 0; depth1 < HW2 - 3; ++depth1) {
                    mpc_error_end_multi[mpc_level][n_discs][depth1] = ceil(SELECTIVITY_MPCT_MULTI[mpc_level] * probcut_sigma_end(n_discs, depth1));
                    for (depth2 = 0; depth2 < HW2 - 3; ++depth2)
                        mpc_error_multi[mpc_level][n_discs][depth1][depth2] = ceil(SELECTIVITY_MPCT_MULTI[mpc_level] * probcut_sigma(n_discs, depth1, depth2));
                }
            }
        }
    }
#endif

#if TUNE_PROBCUT_MID
    inline Search_result tree_search_legal(Board board, int alpha, int beta, int depth, uint_fast8_t mpc_level, bool show_log, uint64_t use_legal, bool use_multi_thread, uint64_t time_limit);
    void get_data_probcut_mid() {
        std::ofstream ofs("probcut_mid.txt");
        Board board;
        Flip flip;
        Search_result short_ans, long_ans;
        for (int i = 0; i < 1000; ++i) {
            for (int depth = 2; depth < 15; ++depth) {
                for (int n_discs = 4; n_discs < HW2 - depth - 5; ++n_discs) {
                    board.reset();
                    for (int j = 4; j < n_discs && board.check_pass(); ++j) { // random move
                        uint64_t legal = board.get_legal();
                        int random_idx = myrandrange(0, pop_count_ull(legal));
                        int t = 0;
                        for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)) {
                            if (t == random_idx) {
                                calc_flip(&flip, &board, cell);
                                break;
                            }
                            ++t;
                        }
                        board.move_board(&flip);
                    }
                    if (board.check_pass()) {
                        int short_depth = myrandrange(1, depth - 1);
                        short_depth &= 0xfffffffe;
                        short_depth |= depth & 1;
                        //int short_depth = mpc_search_depth_arr[0][depth];
                        if (short_depth == 0) {
                            short_ans.value = mid_evaluate(&board);
                        } else{
                            short_ans = tree_search_legal(board, -SCORE_MAX, SCORE_MAX, short_depth, MPC_100_LEVEL, false, board.get_legal(), true, TIME_LIMIT_INF);
                        }
                        long_ans = tree_search_legal(board, -SCORE_MAX, SCORE_MAX, depth, MPC_100_LEVEL, false, board.get_legal(), true, TIME_LIMIT_INF);
                        // n_discs short_depth long_depth error
                        std::cerr << i << " " << n_discs << " " << short_depth << " " << depth << " " << long_ans.value - short_ans.value << std::endl;
                        ofs << n_discs << " " << short_depth << " " << depth << " " << long_ans.value - short_ans.value << std::endl;
                    }
                }
            }
        }
    }
#endif

#if TUNE_PROBCUT_END
    inline Search_result tree_search_legal(Board board, int alpha, int beta, int depth, uint_fast8_t mpc_level, bool show_log, uint64_t use_legal, bool use_multi_thread, uint64_t time_limit);
    void get_data_probcut_end() {
        std::ofstream ofs("probcut_end.txt");
        Board board;
        Flip flip;
        Search_result short_ans, long_ans;
        for (int i = 0; i < 1000; ++i) {
            for (int depth = 24; depth < 26; ++depth) {
                board.reset();
                for (int j = 0; j < HW2 - 4 - depth && board.check_pass(); ++j) { // random move
                    uint64_t legal = board.get_legal();
                    int random_idx = myrandrange(0, pop_count_ull(legal));
                    int t = 0;
                    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)) {
                        if (t == random_idx) {
                            calc_flip(&flip, &board, cell);
                            break;
                        }
                        ++t;
                    }
                    board.move_board(&flip);
                }
                if (board.check_pass()) {
                    int short_depth = myrandrange(2, std::min(15, depth - 1));
                    short_depth &= 0xfffffffe;
                    short_depth |= depth & 1;
                    //int short_depth = mpc_search_depth_arr[1][depth];
                    if (short_depth == 0) {
                        short_ans.value = mid_evaluate(&board);
                    } else{
                        short_ans = tree_search_legal(board, -SCORE_MAX, SCORE_MAX, short_depth, MPC_100_LEVEL, false, board.get_legal(), true, TIME_LIMIT_INF);
                    }
                    long_ans = tree_search_legal(board, -SCORE_MAX, SCORE_MAX, depth, MPC_100_LEVEL, false, board.get_legal(), true, TIME_LIMIT_INF);
                    // n_discs short_depth error
                    std::cerr << i << " " << HW2 - depth << " " << short_depth << " " << long_ans.value - short_ans.value << std::endl;
                    ofs << HW2 - depth << " " << short_depth << " " << long_ans.value - short_ans.value << std::endl;
                }
            }
        }
    }
#endif