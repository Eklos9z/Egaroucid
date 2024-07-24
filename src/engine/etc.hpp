/*
    Egaroucid Project

    @file etc.hpp
        Enhanced Transposition Cutoff
    @date 2021-2024
    @author Takuto Yamana
    @license GPL-3.0 license
*/
#pragma once
#include "setting.hpp"
#include "search.hpp"
#include "transposition_table.hpp"
#include "move_ordering.hpp"

/*
    @brief Enhanced Transposition Cutoff (ETC)

    @param hash_level_failed    hash level used when failed
    @param hash_level           new hash level
    @return hash resized?
*/
inline bool etc(Search *search, std::vector<Flip_value> &move_list, int depth, int *alpha, int *beta, int *v, int *etc_done_idx){
    *etc_done_idx = 0;
    int l, u, n_beta = *alpha;
    for (Flip_value &flip_value: move_list){
        l = -SCORE_MAX;
        u = SCORE_MAX;
        search->move(&flip_value.flip);
            transposition_table.get_bounds(search, search->board.hash(), depth - 1, &l, &u);
        search->undo(&flip_value.flip);
        if (*beta <= -u){ // alpha < beta <= -u <= -l
            *v = -u;
            return true; // fail high
        } else if (*alpha <= -u && -u < *beta){ // alpha <= -u <= beta <= -l or alpha <= -u <= -l <= beta
            *alpha = -u; // update alpha (alpha <= -u)
            *v = -u;
            if (-l <= *v || u == l){ // better move already found or this move is already done
                flip_value.flip.flip = 0ULL; // make this move invalid
                ++(*etc_done_idx);
            }
        } else if (-l <= *alpha){ // -u <= -l <= alpha < beta
            *v = std::max(*v, -l); // this move is worse than alpha
            flip_value.flip.flip = 0ULL; // make this move invalid
            ++(*etc_done_idx);
        }
    }
    return false;
}

/*
    @brief Enhanced Transposition Cutoff (ETC) for NWS

    @param hash_level_failed    hash level used when failed
    @param hash_level           new hash level
    @return hash resized?
*/
inline bool etc_nws(Search *search, std::vector<Flip_value> &move_list, int depth, int alpha, int *v, int *etc_done_idx){
    *etc_done_idx = 0;
    int l, u;
    for (Flip_value &flip_value: move_list){
        l = -SCORE_MAX;
        u = SCORE_MAX;
        search->move(&flip_value.flip);
            transposition_table.get_bounds(search, search->board.hash(), depth - 1, &l, &u);
        search->undo(&flip_value.flip);
        if (alpha < -u){ // fail high at parent node
            *v = -u;
            return true;
        }
        if (-alpha <= l){ // fail high at child node
            if (*v < -l)
                *v = -l;
            flip_value.flip.flip = 0ULL; // make this move invalid
            ++(*etc_done_idx);
        }
    }
    return false;
}
