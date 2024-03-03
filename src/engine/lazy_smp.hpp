/*
    Egaroucid Project

    @file lazy_smp.hpp
        Parallel search with Lazy SMP
    @date 2021-2024
    @author Takuto Yamana
    @license GPL-3.0 license
*/

#pragma once
#include <iostream>
#include "setting.hpp"
#include "common.hpp"
#include "search.hpp"
#include "midsearch.hpp"
#include "endsearch.hpp"
#include "thread_pool.hpp"

#define N_PARALLEL_MAX 128
#define MAIN_THREAD_IDX 0

std::pair<int, int> first_nega_scout(Search *search, int alpha, int beta, int predicted_value, int depth, bool is_end_search, const bool is_main_search, const std::vector<Clog_result> clogs, uint64_t strt);

Search_result lazy_smp_midsearch(Board board, int depth, uint_fast8_t mpc_level, bool show_log, std::vector<Clog_result> clogs){
    Search searches[N_PARALLEL_MAX];
    for (int i = 0; i < N_PARALLEL_MAX; ++i){
        searches[i].init_board(&board);
        searches[i].n_nodes = 0ULL;
        searches[i].use_multi_thread = false; // <= no need
        searches[i].mpc_level = mpc_level;
    }
    Search_result result;
    result.value = SCORE_UNDEFINED;
    uint64_t strt = tim();
    for (int main_thread_depth = 1; main_thread_depth <= depth; ++main_thread_depth){
        bool sub_thread_searching = true;
        std::vector<std::future<std::pair<int, int>>> parallel_tasks;
        for (int thread_idx = MAIN_THREAD_IDX + 1; thread_idx < N_PARALLEL_MAX && (int)parallel_tasks.size() < thread_pool.size() && thread_pool.get_n_idle(); ++thread_idx){
            int sub_thread_depth = main_thread_depth + (int)(3.0 * log(1.0 + thread_idx));
            bool pushed;
            parallel_tasks.emplace_back(thread_pool.push(&pushed, std::bind(&first_nega_scout, &searches[thread_idx], -SCORE_MAX, SCORE_MAX, SCORE_UNDEFINED, sub_thread_depth, false, false, clogs, strt)));
            if (!pushed){
                parallel_tasks.pop_back();
                break;
            }
        }
        bool is_main_search = main_thread_depth == depth;
        std::pair<int, int> id_result = first_nega_scout(&searches[MAIN_THREAD_IDX], -SCORE_MAX, SCORE_MAX, SCORE_UNDEFINED, main_thread_depth, false, is_main_search, clogs, strt);
        if (main_thread_depth >= depth - 1){
            if (result.value == SCORE_UNDEFINED){
                result.value = id_result.first;
            } else{
                double n_value = (0.9 * result.value + 1.1 * id_result.first) / 2.0;
                result.value = round(n_value);
                id_result.first = result.value;
            }
            result.policy = id_result.second;
        }
        sub_thread_searching = false;
        for (std::future<std::pair<int, int>> &task: parallel_tasks)
            task.get();
        result.depth = main_thread_depth;
        result.nodes = 0;
        for (int i = 0; i < N_PARALLEL_MAX; ++i){
            result.nodes += searches[i].n_nodes;
        }
        result.time = tim() - strt;
        result.nps = calc_nps(result.nodes, result.time);
        if (show_log){
            std::cerr << "depth " << result.depth << "@" << SELECTIVITY_PERCENTAGE[searches[MAIN_THREAD_IDX].mpc_level] << "%" << " value " << id_result.first << " policy " << idx_to_coord(id_result.second) << " n_worker " << parallel_tasks.size() << " n_nodes " << result.nodes << " time " << result.time << " NPS " << result.nps << std::endl;
        }
    }
    result.is_end_search = false;
    result.probability = SELECTIVITY_PERCENTAGE[mpc_level];
    return result;
}
