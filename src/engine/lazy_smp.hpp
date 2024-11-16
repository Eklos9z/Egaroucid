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

#define LAZYSMP_ENDSEARCH_PRESEARCH_OFFSET 10

struct Lazy_SMP_task {
    uint_fast8_t mpc_level;
    int depth;
    bool is_end_search;
};

void lazy_smp(Board board, int depth, uint_fast8_t mpc_level, bool show_log, std::vector<Clog_result> clogs, uint64_t use_legal, bool use_multi_thread, Search_result *result) {
    uint64_t strt = tim();
    result->value = SCORE_UNDEFINED;
    int main_depth = 1;
    int main_mpc_level = mpc_level;
    const int max_depth = HW2 - board.n_discs();
    depth = std::min(depth, max_depth);
    bool is_end_search = (depth == max_depth);
    if (is_end_search) {
        main_mpc_level = MPC_74_LEVEL;
    }
    if (show_log) {
        std::cerr << "thread pool size " << thread_pool.size() << " n_idle " << thread_pool.get_n_idle() << std::endl;
    }
    std::vector<Search> searches(thread_pool.size() + 1);
    while (main_depth <= depth && main_mpc_level <= mpc_level && global_searching) {
        for (Search &search: searches) {
            search.n_nodes = 0;
        }
        bool main_is_end_search = false;
        if (main_depth >= max_depth) {
            main_is_end_search = true;
            main_depth = max_depth;
        }
        bool is_last_search = (main_depth == depth) && (main_mpc_level == mpc_level);
        std::vector<std::future<int>> parallel_tasks;
        std::vector<int> sub_depth_arr;
        int sub_max_mpc_level[61];
        bool sub_searching = true;
        int sub_depth = main_depth;
        if (use_multi_thread && !(is_end_search && main_depth == depth) && main_depth <= 10) {
            int max_thread_size = thread_pool.size();
            for (int i = 0; i < main_depth - 14; ++i) {
                max_thread_size *= 0.9;
            }
            sub_max_mpc_level[main_depth] = main_mpc_level + 1;
            for (int i = main_depth + 1; i < 61; ++i) {
                sub_max_mpc_level[i] = MPC_74_LEVEL;
            }
            for (int sub_thread_idx = 0; sub_thread_idx < max_thread_size && sub_thread_idx < searches.size() && global_searching; ++sub_thread_idx) {
                int ntz = ctz_uint32(sub_thread_idx + 1);
                int sub_depth = std::min(max_depth, main_depth + ntz);
                uint_fast8_t sub_mpc_level = sub_max_mpc_level[sub_depth];
                bool sub_is_end_search = (sub_depth == max_depth);
                if (sub_mpc_level <= MPC_100_LEVEL) {
                    //std::cerr << sub_thread_idx << " " << sub_depth << " " << SELECTIVITY_PERCENTAGE[sub_mpc_level] << std::endl;
                    searches[sub_thread_idx] = Search{&board, sub_mpc_level, false, true, true};
                    bool pushed = false;
                    parallel_tasks.emplace_back(thread_pool.push(&pushed, std::bind(&nega_scout, &searches[sub_thread_idx], -SCORE_MAX, SCORE_MAX, sub_depth, false, LEGAL_UNDEFINED, sub_is_end_search, &sub_searching)));
                    sub_depth_arr.emplace_back(sub_depth);
                    ++sub_max_mpc_level[sub_depth];
                    if (!pushed) {
                        parallel_tasks.pop_back();
                        sub_depth_arr.pop_back();
                    }
                }
            }
            int max_sub_search_depth = -1;
            int max_sub_main_mpc_level = 0;
            bool max_is_only_one = false;
            for (int i = 0; i < (int)parallel_tasks.size(); ++i) {
                if (sub_depth_arr[i] > max_sub_search_depth) {
                    max_sub_search_depth = sub_depth_arr[i];
                    max_sub_main_mpc_level = searches[i].mpc_level;
                    max_is_only_one = true;
                } else if (sub_depth_arr[i] == max_sub_search_depth && max_sub_main_mpc_level < searches[i].mpc_level) {
                    max_sub_main_mpc_level = searches[i].mpc_level;
                    max_is_only_one = true;
                } else if (sub_depth_arr[i] == max_sub_search_depth && searches[i].mpc_level == max_sub_main_mpc_level) {
                    max_is_only_one = false;
                }
            }
            if (max_is_only_one) {
                for (int i = 0; i < (int)parallel_tasks.size(); ++i) {
                    if (sub_depth_arr[i] == max_sub_search_depth && searches[i].mpc_level == max_sub_main_mpc_level) {
                        searches[i].need_to_see_tt_loop = false; // off the inside-loop tt lookup in the max level thread
                    }
                }
            }
        }
        Search main_search(&board, main_mpc_level, use_multi_thread, parallel_tasks.size() != 0, !is_last_search);
        bool searching = true;
        std::pair<int, int> id_result = first_nega_scout_legal(&main_search, -SCORE_MAX, SCORE_MAX, main_depth, main_is_end_search, clogs, use_legal, strt, &searching);
        sub_searching = false;
        for (std::future<int> &task: parallel_tasks) {
            task.get();
        }
        for (Search &search: searches) {
            result->nodes += search.n_nodes;
        }
        result->nodes += main_search.n_nodes;
        if (result->value != SCORE_UNDEFINED && !main_is_end_search) {
            double n_value = (0.9 * result->value + 1.1 * id_result.first) / 2.0;
            result->value = round(n_value);
        } else{
            result->value = id_result.first;
        }
        result->policy = id_result.second;
        result->depth = main_depth;
        result->time = tim() - strt;
        result->nps = calc_nps(result->nodes, result->time);
        result->is_end_search = main_is_end_search;
        result->probability = SELECTIVITY_PERCENTAGE[main_mpc_level];
        if (show_log) {
            if (is_last_search) {
                std::cerr << "main ";
            } else{
                std::cerr << "pre ";
            }
            if (main_is_end_search) {
                std::cerr << "end ";
            } else{
                std::cerr << "mid ";
            }
            std::cerr << "depth " << result->depth << "@" << SELECTIVITY_PERCENTAGE[main_mpc_level] << "%" << " value " << result->value << " (raw " << id_result.first << ") policy " << idx_to_coord(id_result.second) << " n_worker " << parallel_tasks.size() << " n_nodes " << result->nodes << " time " << result->time << " NPS " << result->nps << std::endl;
        }
        if (!is_end_search || main_depth < depth - LAZYSMP_ENDSEARCH_PRESEARCH_OFFSET) {
            if (main_depth <= 15 && main_depth < depth - 3) {
                main_depth += 3;
            } else{
                ++main_depth;
            }
        } else{
            if (main_depth < depth) {
                main_depth = depth;
                if (depth <= 30 && mpc_level >= MPC_88_LEVEL) {
                    main_mpc_level = MPC_88_LEVEL;
                } else{
                    main_mpc_level = MPC_74_LEVEL;
                }
            } else{
                if (main_mpc_level < mpc_level) {
                    if (
                        (main_mpc_level >= MPC_74_LEVEL && mpc_level > MPC_74_LEVEL && depth <= 22) || 
                        (main_mpc_level >= MPC_88_LEVEL && mpc_level > MPC_88_LEVEL && depth <= 25) || 
                        (main_mpc_level >= MPC_93_LEVEL && mpc_level > MPC_93_LEVEL && depth <= 29) || 
                        (main_mpc_level >= MPC_98_LEVEL && mpc_level > MPC_98_LEVEL)
                        ) {
                        main_mpc_level = mpc_level;
                    } else{
                        ++main_mpc_level;
                    }
                } else{
                    break;
                }
            }
        }
    }
}

void lazy_smp_time_limit(Board board, bool show_log, std::vector<Clog_result> clogs, uint64_t use_legal, bool use_multi_thread, Search_result *result, uint64_t time_limit) {
    uint64_t strt = tim();
    result->value = SCORE_UNDEFINED;
    int main_depth = 1;
    int main_mpc_level = MPC_100_LEVEL;
    const int max_depth = HW2 - board.n_discs();
    if (show_log) {
        std::cerr << "thread pool size " << thread_pool.size() << " n_idle " << thread_pool.get_n_idle() << std::endl;
    }
    std::vector<Search> searches(thread_pool.size() + 1);
    int before_raw_value = -100;
    while (global_searching && tim() - strt < time_limit) {
        for (Search &search: searches) {
            search.n_nodes = 0;
        }
        bool main_is_end_search = false;
        if (main_depth >= max_depth) {
            main_is_end_search = true;
            main_depth = max_depth;
        }
        std::vector<std::future<int>> parallel_tasks;
        std::vector<int> sub_depth_arr;
        int sub_max_mpc_level[61];
        bool sub_searching = true;
        int sub_depth = main_depth;
        if (use_multi_thread && main_depth <= 10) {
            int max_thread_size = thread_pool.size();
            for (int i = 0; i < main_depth - 14; ++i) {
                max_thread_size *= 0.9;
            }
            sub_max_mpc_level[main_depth] = main_mpc_level + 1;
            for (int i = main_depth + 1; i < 61; ++i) {
                sub_max_mpc_level[i] = MPC_74_LEVEL;
            }
            for (int sub_thread_idx = 0; sub_thread_idx < max_thread_size && sub_thread_idx < searches.size() && global_searching; ++sub_thread_idx) {
                int ntz = ctz_uint32(sub_thread_idx + 1);
                int sub_depth = std::min(max_depth, main_depth + ntz);
                uint_fast8_t sub_mpc_level = sub_max_mpc_level[sub_depth];
                bool sub_is_end_search = (sub_depth == max_depth);
                if (sub_mpc_level <= MPC_100_LEVEL) {
                    //std::cerr << sub_thread_idx << " " << sub_depth << " " << SELECTIVITY_PERCENTAGE[sub_mpc_level] << std::endl;
                    searches[sub_thread_idx] = Search{&board, sub_mpc_level, false, true, true};
                    bool pushed = false;
                    parallel_tasks.emplace_back(thread_pool.push(&pushed, std::bind(&nega_scout, &searches[sub_thread_idx], -SCORE_MAX, SCORE_MAX, sub_depth, false, LEGAL_UNDEFINED, sub_is_end_search, &sub_searching)));
                    sub_depth_arr.emplace_back(sub_depth);
                    ++sub_max_mpc_level[sub_depth];
                    if (!pushed) {
                        parallel_tasks.pop_back();
                        sub_depth_arr.pop_back();
                    }
                }
            }
            int max_sub_search_depth = -1;
            int max_sub_main_mpc_level = 0;
            bool max_is_only_one = false;
            for (int i = 0; i < (int)parallel_tasks.size(); ++i) {
                if (sub_depth_arr[i] > max_sub_search_depth) {
                    max_sub_search_depth = sub_depth_arr[i];
                    max_sub_main_mpc_level = searches[i].mpc_level;
                    max_is_only_one = true;
                } else if (sub_depth_arr[i] == max_sub_search_depth && max_sub_main_mpc_level < searches[i].mpc_level) {
                    max_sub_main_mpc_level = searches[i].mpc_level;
                    max_is_only_one = true;
                } else if (sub_depth_arr[i] == max_sub_search_depth && searches[i].mpc_level == max_sub_main_mpc_level) {
                    max_is_only_one = false;
                }
            }
            if (max_is_only_one) {
                for (int i = 0; i < (int)parallel_tasks.size(); ++i) {
                    if (sub_depth_arr[i] == max_sub_search_depth && searches[i].mpc_level == max_sub_main_mpc_level) {
                        searches[i].need_to_see_tt_loop = false; // off the inside-loop tt lookup in the max level thread
                    }
                }
            }
        }
        Search main_search(&board, main_mpc_level, use_multi_thread, parallel_tasks.size() != 0, false);
        bool searching = true;
        std::pair<int, int> id_result;
        bool search_success = true;
        if (time_limit == TIME_LIMIT_INF) {
            id_result = first_nega_scout_legal(&main_search, -SCORE_MAX, SCORE_MAX, main_depth, main_is_end_search, clogs, use_legal, strt, &searching);
        } else {
            std::future<std::pair<int, int>> f = std::async(std::launch::async, first_nega_scout_legal, &main_search, -SCORE_MAX, SCORE_MAX, main_depth, main_is_end_search, clogs, use_legal, strt, &searching);
            for (;;) {
                if (f.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    id_result = f.get();
                    break;
                }
                if (tim() - strt >= time_limit) {
                    if (show_log) {
                        std::cerr << "terminate search by time limit" << std::endl;
                    }
                    searching = false;
                    f.get();
                    search_success = false;
                    break;
                }
            }
        }
        sub_searching = false;
        for (std::future<int> &task: parallel_tasks) {
            task.get();
        }
        for (Search &search: searches) {
            result->nodes += search.n_nodes;
        }
        result->nodes += main_search.n_nodes;
        result->time = tim() - strt;
        result->nps = calc_nps(result->nodes, result->time);
        if (search_success) {
            result->level = get_level_from_depth_mpc_level(board.n_discs(), main_depth, main_mpc_level);
            if (result->value != SCORE_UNDEFINED && !main_is_end_search) {
                double n_value = (0.9 * result->value + 1.1 * id_result.first) / 2.0;
                result->value = round(n_value);
            } else{
                result->value = id_result.first;
            }
            bool policy_changed = result->policy != id_result.second;
            result->policy = id_result.second;
            result->depth = main_depth;
            result->is_end_search = main_is_end_search;
            result->probability = SELECTIVITY_PERCENTAGE[main_mpc_level];
            if (show_log) {
                if (main_is_end_search) {
                    std::cerr << "end ";
                } else{
                    std::cerr << "mid ";
                }
                std::cerr << "depth " << result->depth << "@" << SELECTIVITY_PERCENTAGE[main_mpc_level] << "%" << " value " << result->value << " (raw " << id_result.first << ") policy " << idx_to_coord(id_result.second) << " n_worker " << parallel_tasks.size() << " n_nodes " << result->nodes << " time " << result->time << " NPS " << result->nps << std::endl;
            }
            if (
                !main_is_end_search && 
                main_depth >= 23 && 
                tim() - strt > time_limit * 0.35 && 
                result->nodes >= 100000000ULL && 
                !policy_changed && 
                abs(before_raw_value - id_result.first) <= 0
            ) {
                if (show_log) {
                    std::cerr << "early break" << std::endl;
                }
                break;
            }
            before_raw_value = id_result.first;
        }
        if (main_depth < max_depth - LAZYSMP_ENDSEARCH_PRESEARCH_OFFSET) { // next: midgame search
            if (main_depth <= 15 && main_depth < max_depth - 3) {
                main_depth += 3;
            } else{
                ++main_depth;
            }
            if (main_depth > 13 && main_mpc_level == MPC_100_LEVEL) {
                main_mpc_level = MPC_74_LEVEL;
            }
            if (main_depth > 23) {
                if (main_mpc_level < MPC_93_LEVEL) {
                    --main_depth;
                    ++main_mpc_level;
                } else {
                    main_mpc_level = MPC_74_LEVEL;
                }
            }
        } else{ // next: endgame search
            if (main_depth < max_depth) {
                main_depth = max_depth;
                main_mpc_level = MPC_74_LEVEL;
            } else{
                if (main_mpc_level < MPC_100_LEVEL) {
                    ++main_mpc_level;
                } else{
                    if (show_log) {
                        std::cerr << "all searched" << std::endl;
                    }
                    break;
                }
            }
        }
    }
}
