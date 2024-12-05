﻿/*
    Egaroucid Project

    @file ai.hpp
        Main algorithm of Egaroucid
    @date 2021-2024
    @author Takuto Yamana
    @license GPL-3.0 license
*/

#pragma once
#include <iostream>
#include <future>
#include <unordered_set>
#include <iomanip>
#include "level.hpp"
#include "setting.hpp"
#include "midsearch.hpp"
#include "book.hpp"
#include "util.hpp"
#include "clogsearch.hpp"
#include "time_management.hpp"

#define SEARCH_BOOK -1

#define AI_TYPE_BOOK 1000

#define IDSEARCH_ENDSEARCH_PRESEARCH_OFFSET 10
#define IDSEARCH_ENDSEARCH_PRESEARCH_OFFSET_TIMELIMIT 10
#define PONDER_ENDSEARCH_PRESEARCH_OFFSET_TIMELIMIT 6

#define NOBOOK_SEARCH_LEVEL 10
#define NOBOOK_SEARCH_MARGIN 1

#define PONDER_START_SELFPLAY_DEPTH 25

struct Lazy_SMP_task {
    uint_fast8_t mpc_level;
    int depth;
    bool is_end_search;
};

struct Ponder_elem {
    Flip flip;
    double value;
    int count;
    int depth;
    uint_fast8_t mpc_level;
    bool is_endgame_search;
    bool is_complete_search;
};

std::vector<Ponder_elem> ai_ponder(Board board, bool show_log, bool *searching);
int ai_self_play_and_analyze(Board board_start, int mid_depth, bool show_log, bool use_multi_thread, bool *searching);

void iterative_deepening_search(Board board, int alpha, int beta, int depth, uint_fast8_t mpc_level, bool show_log, std::vector<Clog_result> clogs, uint64_t use_legal, bool use_multi_thread, Search_result *result, bool *searching) {
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
#if USE_LAZY_SMP
    std::vector<Search> searches(thread_pool.size() + 1);
#endif
    while (main_depth <= depth && main_mpc_level <= mpc_level && global_searching && *searching) {
#if USE_LAZY_SMP
        for (Search &search: searches) {
            search.n_nodes = 0;
        }
#endif
        bool main_is_end_search = false;
        if (main_depth >= max_depth) {
            main_is_end_search = true;
            main_depth = max_depth;
        }
        bool is_last_search = (main_depth == depth) && (main_mpc_level == mpc_level);
#if USE_LAZY_SMP
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
            for (int sub_thread_idx = 0; sub_thread_idx < max_thread_size && sub_thread_idx < searches.size() && global_searching && *searching; ++sub_thread_idx) {
                int ntz = ctz_uint32(sub_thread_idx + 1);
                int sub_depth = std::min(max_depth, main_depth + ntz);
                uint_fast8_t sub_mpc_level = sub_max_mpc_level[sub_depth];
                bool sub_is_end_search = (sub_depth == max_depth);
                if (sub_mpc_level <= MPC_100_LEVEL) {
                    //std::cerr << sub_thread_idx << " " << sub_depth << " " << SELECTIVITY_PERCENTAGE[sub_mpc_level] << std::endl;
                    searches[sub_thread_idx] = Search{&board, sub_mpc_level, false, true};
                    bool pushed = false;
                    parallel_tasks.emplace_back(thread_pool.push(&pushed, std::bind(&nega_scout, &searches[sub_thread_idx], alpha, beta, sub_depth, false, LEGAL_UNDEFINED, sub_is_end_search, &sub_searching)));
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
        }
#endif
        Search main_search(&board, main_mpc_level, use_multi_thread, !is_last_search);        
        std::pair<int, int> id_result = first_nega_scout_legal(&main_search, alpha, beta, main_depth, main_is_end_search, clogs, use_legal, strt, searching);
#if USE_LAZY_SMP
        sub_searching = false;
        for (std::future<int> &task: parallel_tasks) {
            task.get();
        }
        for (Search &search: searches) {
            result->nodes += search.n_nodes;
        }
#endif
        result->nodes += main_search.n_nodes;
        if (*searching) {
            if (result->value != SCORE_UNDEFINED && !main_is_end_search) {
                double n_value = (0.9 * result->value + 1.1 * id_result.first) / 2.0;
                result->value = round(n_value);
            } else{
                result->value = id_result.first;
            }
            result->policy = id_result.second;
            result->depth = main_depth;
            result->is_end_search = main_is_end_search;
            result->probability = SELECTIVITY_PERCENTAGE[main_mpc_level];
        }
        result->time = tim() - strt;
        result->nps = calc_nps(result->nodes, result->time);
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
#if USE_LAZY_SMP
            std::cerr << "depth " << result->depth << "@" << SELECTIVITY_PERCENTAGE[main_mpc_level] << "%" << " value " << result->value << " (raw " << id_result.first << ") policy " << idx_to_coord(id_result.second) << " n_worker " << parallel_tasks.size() << " n_nodes " << result->nodes << " time " << result->time << " NPS " << result->nps << std::endl;
#else
            std::cerr << "depth " << result->depth << "@" << SELECTIVITY_PERCENTAGE[main_mpc_level] << "%" << " value " << result->value << " (raw " << id_result.first << ") policy " << idx_to_coord(id_result.second) << " n_nodes " << result->nodes << " time " << result->time << " NPS " << result->nps << std::endl;
#endif
        }
        if (is_end_search && main_depth >= depth - IDSEARCH_ENDSEARCH_PRESEARCH_OFFSET) {
            if (main_depth < depth) {
                main_depth = depth;
                if (depth <= 30 && mpc_level >= MPC_88_LEVEL) {
                    main_mpc_level = MPC_88_LEVEL;
                } else {
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
        } else {
            if (main_depth <= 15 && main_depth < depth - 3) {
                main_depth += 3;
            } else{
                ++main_depth;
            }
        }
    }
}

void iterative_deepening_search_time_limit(Board board, int alpha, int beta, bool show_log, std::vector<Clog_result> clogs, uint64_t use_legal, bool use_multi_thread, Search_result *result, uint64_t time_limit, bool *searching) {
    uint64_t strt = tim();
    result->value = SCORE_UNDEFINED;
    int main_depth = 1;
    int main_mpc_level = MPC_100_LEVEL;
    const int max_depth = HW2 - board.n_discs();
    if (show_log) {
        std::cerr << "thread pool size " << thread_pool.size() << " n_idle " << thread_pool.get_n_idle() << std::endl;
    }
#if USE_LAZY_SMP
    std::vector<Search> searches(thread_pool.size() + 1);
#endif
    int before_raw_value = -100;
    bool policy_changed_before = true;
    while (global_searching && (*searching) && ((tim() - strt < time_limit) || main_depth <= 1)) {
#if USE_LAZY_SMP
        for (Search &search: searches) {
            search.n_nodes = 0;
        }
#endif
        bool main_is_end_search = false;
        if (main_depth >= max_depth) {
            main_is_end_search = true;
            main_depth = max_depth;
        }
#if USE_LAZY_SMP
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
            for (int sub_thread_idx = 0; sub_thread_idx < max_thread_size && sub_thread_idx < searches.size() && global_searching && (*searching); ++sub_thread_idx) {
                int ntz = ctz_uint32(sub_thread_idx + 1);
                int sub_depth = std::min(max_depth, main_depth + ntz);
                uint_fast8_t sub_mpc_level = sub_max_mpc_level[sub_depth];
                bool sub_is_end_search = (sub_depth == max_depth);
                if (sub_mpc_level <= MPC_100_LEVEL) {
                    //std::cerr << sub_thread_idx << " " << sub_depth << " " << SELECTIVITY_PERCENTAGE[sub_mpc_level] << std::endl;
                    searches[sub_thread_idx] = Search{&board, sub_mpc_level, false, true};
                    bool pushed = false;
                    parallel_tasks.emplace_back(thread_pool.push(&pushed, std::bind(&nega_scout, &searches[sub_thread_idx], alpha, beta, sub_depth, false, LEGAL_UNDEFINED, sub_is_end_search, &sub_searching)));
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
        }
#endif
        if (show_log) {
            if (main_is_end_search) {
                std::cerr << "end ";
            } else{
                std::cerr << "mid ";
            }
            std::cerr << "depth " << main_depth << "@" << SELECTIVITY_PERCENTAGE[main_mpc_level] << "% " << std::flush;
        }
        Search main_search(&board, main_mpc_level, use_multi_thread, false);
        std::pair<int, int> id_result;
        bool search_success = true;
        bool main_searching = true;
        std::future<std::pair<int, int>> f = std::async(std::launch::async, first_nega_scout_legal, &main_search, alpha, beta, main_depth, main_is_end_search, clogs, use_legal, strt, &main_searching);
        for (;;) {
            if (f.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                id_result = f.get();
                break;
            }
            if ((tim() - strt >= time_limit && main_depth > 1) || !(*searching) || !global_searching) {
                if (show_log) {
                    std::cerr << "terminated " << tim() - strt << " ms" << std::endl;
                }
                main_searching = false;
                f.get();
                search_success = false;
                break;
            }
        }
#if USE_LAZY_SMP
        sub_searching = false;
        for (std::future<int> &task: parallel_tasks) {
            task.get();
        }
        for (Search &search: searches) {
            result->nodes += search.n_nodes;
        }
#endif
        result->nodes += main_search.n_nodes;
        result->time = tim() - strt;
        result->nps = calc_nps(result->nodes, result->time);
        if (search_success) {
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
#if USE_LAZY_SMP
                std::cerr << "value " << result->value << " (raw " << id_result.first << ") policy " << idx_to_coord(id_result.second) << " n_worker " << parallel_tasks.size() << " n_nodes " << result->nodes << " time " << result->time << " NPS " << result->nps << std::endl;
#else
                std::cerr << "value " << result->value << " (raw " << id_result.first << ") policy " << idx_to_coord(id_result.second) << " n_nodes " << result->nodes << " time " << result->time << " NPS " << result->nps << std::endl;
#endif
            }
            if (
                (!main_is_end_search && main_depth >= 30 && main_depth <= 31) && 
                !policy_changed && 
                !policy_changed_before && 
                main_mpc_level == MPC_74_LEVEL
            ) {
                int nws_alpha = result->value - 6;
                if (nws_alpha >= -SCORE_MAX) {
                    Search nws_search(&board, main_mpc_level, use_multi_thread, false);
                    bool nws_searching = true;
                    uint64_t nws_use_legal = use_legal ^ (1ULL << result->policy);
                    std::future<std::pair<int, int>> nws_f = std::async(std::launch::async, first_nega_scout_legal, &main_search, nws_alpha, nws_alpha + 1, main_depth, main_is_end_search, clogs, nws_use_legal, strt, &nws_searching);
                    int nws_value = SCORE_INF;
                    int nws_move = MOVE_NOMOVE;
                    bool nws_success = false;
                    for (;;) {
                        if (nws_f.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                            std::pair<int, int> nws_result = nws_f.get();
                            nws_value = nws_result.first;
                            nws_move = nws_result.second;
                            nws_success = true;
                            break;
                        }
                        if (tim() - strt >= time_limit || !global_hash_level || !(*searching)) {
                            if (show_log) {
                                std::cerr << "terminate early cut nws by time limit " << tim() - strt << " ms" << std::endl;
                            }
                            nws_searching = false;
                            nws_f.get();
                            break;
                        }
                    }
                    result->nodes += main_search.n_nodes;
                    result->time = tim() - strt;
                    result->nps = calc_nps(result->nodes, result->time);
                    if (nws_success) {
                        if (nws_value <= nws_alpha) {
                            if (show_log) {
                                std::cerr << "early break second best " << idx_to_coord(nws_move) << " value <= " << nws_value << " time " << tim() - strt << std::endl;
                            }
                            break;
                        } else if (nws_searching) {
                            if (show_log) {
                                std::cerr << "no early break second best " << idx_to_coord(nws_move) << " value >= " << nws_value << " time " << tim() - strt << std::endl;
                            }
                        }
                    }
                }
            }
            before_raw_value = id_result.first;
            policy_changed_before = policy_changed;
        }
        //if (main_depth > 10 && pop_count_ull(board.get_legal()) == 1) { // not use_legal
        //    break; // there is only 1 move
        //}
        if (main_depth < max_depth - IDSEARCH_ENDSEARCH_PRESEARCH_OFFSET_TIMELIMIT) { // next: midgame search
            if (main_depth <= 15 && main_depth < max_depth - 3) {
                main_depth += 3;
                if (main_depth > 13 && main_mpc_level == MPC_100_LEVEL) {
                    main_mpc_level = MPC_74_LEVEL;
                }
            } else {
                if (main_depth + 1 < 23) {
                    ++main_depth;
                    if (main_depth > 13 && main_mpc_level == MPC_100_LEVEL) {
                        main_mpc_level = MPC_74_LEVEL;
                    }
                } else {
                    ++main_depth;
                    /*
                    if (main_mpc_level < MPC_88_LEVEL) {
                        ++main_mpc_level;
                    } else {
                        ++main_depth;
                        main_mpc_level = MPC_74_LEVEL;
                    }
                    */
                }
            }
        } else { // next: endgame search
            if (main_depth < max_depth) {
                main_depth = max_depth;
                main_mpc_level = MPC_74_LEVEL;
            } else{
                if (main_mpc_level < MPC_100_LEVEL) {
                    ++main_mpc_level;
                } else{
                    if (show_log) {
                        std::cerr << "completely searched" << std::endl;
                    }
                    break;
                }
            }
        }
    }
}


/*
    @brief Get a result of a search

    Firstly, if using MPC, execute clog search for finding special endgame.
    Then do some pre-search, and main search.

    @param board                board to solve
    @param depth                depth to search
    @param mpc_level            MPC level
    @param show_log             show log?
    @param use_multi_thread     search in multi thread?
    @return the result in Search_result structure
*/
inline Search_result tree_search_legal(Board board, int alpha, int beta, int depth, uint_fast8_t mpc_level, bool show_log, uint64_t use_legal, bool use_multi_thread, uint64_t time_limit, bool *searching) {
    //thread_pool.tell_start_using();
    Search_result res;
    depth = std::min(HW2 - board.n_discs(), depth);
    bool is_end_search = (HW2 - board.n_discs() == depth);
    bool use_time_limit = (time_limit != TIME_LIMIT_INF);
    std::vector<Clog_result> clogs;
    uint64_t clog_nodes = 0;
    uint64_t clog_time = 0;
    if (mpc_level != MPC_100_LEVEL || use_time_limit) {
        uint64_t strt = tim();
        int clog_depth = std::min(depth, CLOG_SEARCH_MAX_DEPTH);
        if (use_time_limit) {
            clog_depth = CLOG_SEARCH_MAX_DEPTH;
        }
        clogs = first_clog_search(board, &clog_nodes, clog_depth, use_legal);
        clog_time = tim() - strt;
        if (show_log) {
            std::cerr << "clog search depth " << clog_depth << " time " << clog_time << " nodes " << clog_nodes << " nps " << calc_nps(clog_nodes, clog_time) << std::endl;
            for (int i = 0; i < (int)clogs.size(); ++i) {
                std::cerr << "clogsearch " << i + 1 << "/" << clogs.size() << " " << idx_to_coord(clogs[i].pos) << " value " << clogs[i].val << std::endl;
            }
        }
        res.clog_nodes = clog_nodes;
        res.clog_time = clog_time;
    }
    if (use_legal) {
        uint64_t time_limit_proc = TIME_LIMIT_INF;
        if (use_time_limit) {
            if (time_limit > clog_time) {
                time_limit_proc = time_limit - clog_time;
            } else {
                time_limit_proc = 1;
            }
        }
        if (use_time_limit) {
            /*
            if (HW2 - board.n_discs() >= 30) {
                uint64_t strt_selfplay = tim();
                uint64_t selfplay_time = time_limit_proc * 0.3;
                time_management_selfplay(board, show_log, use_legal, selfplay_time);
                time_limit_proc -= tim() - strt_selfplay;
            }
            */
            iterative_deepening_search_time_limit(board, alpha, beta, show_log, clogs, use_legal, use_multi_thread, &res, time_limit_proc, searching);
        } else {
            iterative_deepening_search(board, alpha, beta, depth, mpc_level, show_log, clogs, use_legal, use_multi_thread, &res, searching);
        }
    }
    //thread_pool.tell_finish_using();
    //thread_pool.reset_unavailable();
    //delete_tt(&board, 6);
    return res;
}

/*
    @brief Get a result of a search with book or search

    Firstly, check if the given board is in the book.
    Then search the board and get the result.

    @param board                board to solve
    @param level                level of AI
    @param alpha                search window (alpha)
    @param beta                 search window (beta)
    @param use_book             use book?
	@param book_acc_level		book accuracy level
    @param use_multi_thread     search in multi thread?
    @param show_log             show log?
    @return the result in Search_result structure
*/
Search_result ai_common(Board board, int alpha, int beta, int level, bool use_book, int book_acc_level, bool use_multi_thread, bool show_log, uint64_t use_legal, bool use_specified_move_book, uint64_t time_limit, bool *searching) {
    Search_result res;
    int value_sign = 1;
    if (board.get_legal() == 0ULL) {
        board.pass();
        if (board.get_legal() == 0ULL) {
            res.level = MAX_LEVEL;
            res.policy = 64;
            res.value = -board.score_player();
            res.depth = 0;
            res.nps = 0;
            res.is_end_search = true;
            res.probability = 100;
            return res;
        } else{
            if (show_log) {
                std::cerr << "pass found in ai_common!" << std::endl;
            }
            value_sign = -1;
        }
    }
    Book_value book_result;
    if (use_specified_move_book) {
        book_result = book.get_specified_best_move(&board, use_legal);
    } else{
        book_result = book.get_random(&board, book_acc_level, use_legal);
    }
    bool book_move_found = false;
    if (is_valid_policy(book_result.policy) && use_book) {
        bool better_move_maybe_found = false;
        if (book_acc_level == 0) { // accurate book level
            std::vector<Book_value> book_moves = book.get_all_moves_with_value(&board);
            for (const Book_value &move: book_moves) {
                use_legal &= ~(1ULL << move.policy);
            }
            if (use_legal != 0) {
                int nobook_search_level = std::min(level, NOBOOK_SEARCH_LEVEL);
                int nobook_search_depth;
                uint_fast8_t nobook_search_mpc_level;
                bool nobook_search_is_mid_search;
                get_level(nobook_search_level, board.n_discs() - 4, &nobook_search_is_mid_search, &nobook_search_depth, &nobook_search_mpc_level);
                Search_result nobook_search_result = tree_search_legal(board, alpha, beta, nobook_search_depth, nobook_search_mpc_level, show_log, use_legal, use_multi_thread, TIME_LIMIT_INF, searching);
                if (*searching) {
                    if (nobook_search_result.value >= book_result.value + NOBOOK_SEARCH_MARGIN) {
                        better_move_maybe_found = true;
                        if (show_log) {
                            std::cerr << "book used but better move can be found book " << book_result.value << " best move " << nobook_search_result.value << " at level " << nobook_search_level << std::endl;
                        }
                    }
                }
            }
        }
        if (!better_move_maybe_found) {
            if (show_log) {
                std::cerr << "book " << idx_to_coord(book_result.policy) << " " << book_result.value << " at book error level " << book_acc_level << std::endl;
            }
            res.level = LEVEL_TYPE_BOOK;
            res.policy = book_result.policy;
            res.value = value_sign * book_result.value;
            res.depth = SEARCH_BOOK;
            res.nps = 0;
            res.is_end_search = false;
            res.probability = 100;
            book_move_found = true;
        }
    }
    if (!book_move_found) {
        int depth;
        bool is_mid_search;
        uint_fast8_t mpc_level;
        get_level(level, board.n_discs() - 4, &is_mid_search, &depth, &mpc_level);
        if (show_log && time_limit == TIME_LIMIT_INF) {
            std::cerr << "level status " << level << " " << board.n_discs() - 4 << " discs depth " << depth << "@" << SELECTIVITY_PERCENTAGE[mpc_level] << "%" << std::endl;
        }
        //thread_pool.tell_start_using();
        res = tree_search_legal(board, alpha, beta, depth, mpc_level, show_log, use_legal, use_multi_thread, time_limit, searching);
        //thread_pool.tell_finish_using();
        res.level = level;
        res.value *= value_sign;
        if (is_valid_policy(book_result.policy) && use_book) { // check book
            if (res.value <= book_result.value) { // book move is better
                if (show_log) {
                    std::cerr << "book " << idx_to_coord(book_result.policy) << " " << book_result.value << " at book error level " << book_acc_level << std::endl;
                }
                res.level = LEVEL_TYPE_BOOK;
                res.policy = book_result.policy;
                res.value = value_sign * book_result.value;
                res.depth = SEARCH_BOOK;
                res.is_end_search = false;
                res.probability = 100;
            }
        }
    }
    return res;
}

/*
    @brief Get a result of a search with book or search

    Firstly, check if the given board is in the book.
    Then search the board and get the result.

    @param board                board to solve
    @param level                level of AI
    @param use_book             use book?
	@param book_acc_level		book accuracy level
    @param use_multi_thread     search in multi thread?
    @param show_log             show log?
    @return the result in Search_result structure
*/
Search_result ai(Board board, int level, bool use_book, int book_acc_level, bool use_multi_thread, bool show_log) {
    bool searching = true;
    return ai_common(board, -SCORE_MAX, SCORE_MAX, level, use_book, book_acc_level, use_multi_thread, show_log, board.get_legal(), false, TIME_LIMIT_INF, &searching);
}

Search_result ai_searching(Board board, int level, bool use_book, int book_acc_level, bool use_multi_thread, bool show_log, bool *searching) {
    return ai_common(board, -SCORE_MAX, SCORE_MAX, level, use_book, book_acc_level, use_multi_thread, show_log, board.get_legal(), false, TIME_LIMIT_INF, searching);
}


Search_result ai_legal(Board board, int level, bool use_book, int book_acc_level, bool use_multi_thread, bool show_log, uint64_t use_legal) {
    bool searching = true;
    return ai_common(board, -SCORE_MAX, SCORE_MAX, level, use_book, book_acc_level, use_multi_thread, show_log, use_legal, false, TIME_LIMIT_INF, &searching);
}

Search_result ai_legal_window(Board board, int alpha, int beta, int level, bool use_book, int book_acc_level, bool use_multi_thread, bool show_log, uint64_t use_legal) {
    bool searching = true;
    return ai_common(board, alpha, beta, level, use_book, book_acc_level, use_multi_thread, show_log, use_legal, false, TIME_LIMIT_INF, &searching);
}

Search_result ai_specified(Board board, int level, bool use_book, int book_acc_level, bool use_multi_thread, bool show_log) {
    bool searching = true;
    return ai_common(board, -SCORE_MAX, SCORE_MAX, level, use_book, book_acc_level, use_multi_thread, show_log, board.get_legal(), true, TIME_LIMIT_INF, &searching);
}

Search_result ai_time_limit(Board board, bool use_book, int book_acc_level, bool use_multi_thread, bool show_log, uint64_t remaining_time_msec) {
    uint64_t time_limit = calc_time_limit_ply(board, remaining_time_msec, show_log);
    if (show_log) {
        std::cerr << "time limit " << time_limit << " remaining " << remaining_time_msec << std::endl;
    }
    int n_empties = HW2 - board.n_discs();
    if (time_limit > 30000ULL && n_empties >= 34) {
        uint64_t strt = tim();
        bool need_request_more_time = false;
        bool ponder_searching = true;
        uint64_t ponder_tl = 1000ULL;
        std::cerr << "pre search by ponder tl " << ponder_tl << std::endl;
        std::future<std::vector<Ponder_elem>> ponder_future = std::async(std::launch::async, ai_ponder, board, show_log, &ponder_searching);
        while (tim() - strt < ponder_tl && ponder_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready);
        ponder_searching = false;
        std::vector<Ponder_elem> ponder_move_list = ponder_future.get();
        if (ponder_move_list.size()) {
            double best_value = ponder_move_list[0].value;
            int n_good_moves = 0;
            for (const Ponder_elem &elem: ponder_move_list) {
                if (elem.value >= best_value - 1.5) {
                    ++n_good_moves;
                } else {
                    break; // because sorted
                }
            }
            if (n_good_moves >= 2) {
                uint64_t self_play_tl = std::max(25000ULL + ponder_tl, (uint64_t)(time_limit * 0.6));
                std::vector<Board> self_play_boards;
                std::vector<int> self_play_depth_arr;
                for (int i = 0; i < n_good_moves; ++i) {
                    Board n_board = board.copy();
                    n_board.move_board(&ponder_move_list[i].flip);
                    self_play_boards.emplace_back(n_board);
                    self_play_depth_arr.emplace_back(21); // initial depth
                }
                int board_idx = 0;
                int n_searched = 0;
                if (show_log) {
                    std::cerr << "need to see good moves tl " << self_play_tl << " " << n_good_moves << " moves :";
                    for (int i = 0; i < n_good_moves; ++i) {
                        std::cerr << " " << idx_to_coord(ponder_move_list[i].flip.pos);
                    }
                    std::cerr << std::endl;
                }
                for (int i = 0; i < (int)self_play_boards.size(); ++i) {
                    bool depth_updated = true;
                    while (depth_updated && self_play_depth_arr[i] < n_empties - 1) {
                        depth_updated = false;
                        Search tt_search(&self_play_boards[i], MPC_74_LEVEL, true, false);
                        if (transposition_table.has_node(&tt_search, self_play_boards[i].hash(), self_play_depth_arr[i] + 1)) {
                            ++self_play_depth_arr[i];
                            depth_updated = true;
                        }
                    }
                }
                int self_play_n_finished = 0;
                while (tim() - strt < self_play_tl && self_play_n_finished < (int)self_play_boards.size()) {
                    if (self_play_depth_arr[board_idx] <= n_empties - 1) {
                        if (show_log) {
                            std::cerr << idx_to_coord(ponder_move_list[board_idx].flip.pos) << " ";
                        }
                        bool self_play_searching = true;
                        std::future<int> self_play_future = std::async(std::launch::async, ai_self_play_and_analyze, self_play_boards[board_idx], self_play_depth_arr[board_idx], show_log, true, &self_play_searching);
                        while (tim() - strt < self_play_tl && self_play_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready);
                        self_play_searching = false;
                        self_play_future.get();
                        ++self_play_depth_arr[board_idx];
                        ++n_searched;
                    } else {
                        ++self_play_n_finished;
                    }
                    ++board_idx;
                    if (board_idx >= n_good_moves) {
                        board_idx = 0;
                    }
                }
                if (show_log) {
                    std::cerr << "self played " << n_searched << " time " << tim() - strt << std::endl;
                }
                if (show_log) {
                    std::cerr << "after self play scores" << std::endl;
                    for (int i = 0; i < (int)self_play_boards.size(); ++i) {
                        Search tt_search(&self_play_boards[i], MPC_74_LEVEL, true, false);
                        int l = -SCORE_MAX, u = SCORE_MAX;
                        transposition_table.get_bounds_any_level(&tt_search, self_play_boards[i].hash(), &l, &u);
                        std::cerr << idx_to_coord(ponder_move_list[i].flip.pos) << " [" << -u << "," << -l << "]" << std::endl;
                    }
                }
                need_request_more_time = true;

                /*
                bool ponder_searching2 = true;
                uint64_t ponder_tl2 = std::max(100ULL + self_play_tl, (uint64_t)(time_limit * 0.65));
                std::cerr << "pre search by ponder 2 tl " << ponder_tl2 << std::endl;
                std::future<std::vector<Ponder_elem>> ponder_future2 = std::async(std::launch::async, ai_ponder, board, show_log, &ponder_searching2);
                while (tim() - strt < ponder_tl2 && ponder_future2.wait_for(std::chrono::seconds(0)) != std::future_status::ready);
                ponder_searching2 = false;
                ponder_future2.get();
                */
            }
        }
        uint64_t elapsed = tim() - strt;
        if (time_limit > elapsed) {
            time_limit -= elapsed;
        } else {
            time_limit = 1;
        }
        if (need_request_more_time) {
            uint64_t remaining_time_msec_p = 1;
            if (remaining_time_msec > elapsed) {
                remaining_time_msec_p = remaining_time_msec - elapsed;
            }
            time_limit = request_more_time(board, remaining_time_msec_p, time_limit, show_log);
        }
        if (show_log) {
            std::cerr << "additional calculation elapsed " << elapsed << " reduced time limit " << time_limit << std::endl;
        }
    }
    bool searching = true;
    return ai_common(board, -SCORE_MAX, SCORE_MAX, MAX_LEVEL, use_book, book_acc_level, use_multi_thread, show_log, board.get_legal(), false, time_limit, &searching);
}

/*
    @brief Search for analyze command

    @param board                board to solve
    @param level                level of AI
    @param use_multi_thread     search in multi thread?
    @return the result in Search_result structure
*/

Analyze_result ai_analyze(Board board, int level, bool use_multi_thread, uint_fast8_t played_move) {
    int depth;
    bool is_mid_search;
    uint_fast8_t mpc_level;
    get_level(level, board.n_discs() - 4, &is_mid_search, &depth, &mpc_level);
    depth = std::min(HW2 - board.n_discs(), depth);
    bool is_end_search = (HW2 - board.n_discs() == depth);
    std::vector<Clog_result> clogs;
    uint64_t clog_nodes = 0;
    uint64_t clog_time = 0;
    int clog_depth = std::min(depth, CLOG_SEARCH_MAX_DEPTH);
    if (mpc_level != MPC_100_LEVEL) {
        uint64_t clog_strt = tim();
        clogs = first_clog_search(board, &clog_nodes, clog_depth, board.get_legal());
        clog_time = tim() - clog_strt;
    }
    Search search(&board, mpc_level, use_multi_thread, false);
    uint64_t strt = tim();
    bool searching = true;
    //thread_pool.tell_start_using();
    Analyze_result res = first_nega_scout_analyze(&search, -SCORE_MAX, SCORE_MAX, depth, is_end_search, clogs, clog_depth, played_move, strt, &searching);
    //thread_pool.tell_finish_using();
    return res;
}



Search_result ai_accept_loss(Board board, int level, int acceptable_loss) {
    uint64_t strt = tim();
    Flip flip;
    int v = SCORE_UNDEFINED;
    uint64_t legal = board.get_legal();
    std::vector<std::pair<int, int>> moves;
    //thread_pool.tell_start_using();
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)) {
        calc_flip(&flip, &board, cell);
        board.move_board(&flip);
            int g = -ai(board, level, true, 0, true, false).value;
        board.undo_board(&flip);
        v = std::max(v, g);
        moves.emplace_back(std::make_pair(g, cell));
    }
    //thread_pool.tell_finish_using();
    std::vector<std::pair<int, int>> acceptable_moves;
    for (std::pair<int, int> move: moves) {
        if (move.first >= v - acceptable_loss)
            acceptable_moves.emplace_back(move);
    }
    int rnd_idx = myrandrange(0, (int)acceptable_moves.size());
    int use_policy = acceptable_moves[rnd_idx].second;
    int use_value = acceptable_moves[rnd_idx].first;
    Search_result res;
    res.depth = 1;
    res.nodes = 0;
    res.time = tim() - strt;
    res.nps = calc_nps(res.nodes, res.time);
    res.policy = use_policy;
    res.value = use_value;
    res.is_end_search = board.n_discs() == HW2 - 1;
    res.probability = SELECTIVITY_PERCENTAGE[MPC_100_LEVEL];
    return res;
}

void ai_hint(Board board, int level, bool use_book, int book_acc_level, bool use_multi_thread, bool show_log, int n_display, double values[], int hint_types[]) {
    uint64_t legal = board.get_legal();
    if (use_book) {
        std::vector<Book_value> links = book.get_all_moves_with_value(&board);
        for (Book_value &link: links) {
            values[link.policy] = link.value;
            hint_types[link.policy] = AI_TYPE_BOOK;
            legal ^= 1ULL << link.policy;
            --n_display;
        }
    }
    //thread_pool.tell_start_using();
    for (int search_level = 1; search_level <= level && global_searching; ++search_level) {
        //if (show_log) {
        //    std::cerr << "hint level " << search_level << " calculating" << std::endl;
        //}
        uint64_t search_legal = legal;
        for (int i = 0; i < n_display && search_legal && global_searching; ++i) {
            Search_result elem = ai_legal(board, search_level, use_book, book_acc_level, use_multi_thread, false, search_legal);
            if (global_searching && (search_legal & (1ULL << elem.policy))) {
                search_legal ^= 1ULL << elem.policy;
                values[elem.policy] = elem.value;
                if (elem.is_end_search) {
                    hint_types[elem.policy] = elem.probability;
                } else{
                    hint_types[elem.policy] = search_level;
                }
            }
        }
    }
    //thread_pool.tell_finish_using();
}

int ai_self_play_and_analyze(Board board_start, int mid_depth, bool show_log, bool use_multi_thread, bool *searching) { // used for ponder
    Flip flip;
    Board board = board_start.copy();
    std::vector<Board> boards;
    int depth_arr[HW2];
    uint_fast8_t mpc_level_arr[HW2];
    int start_n_discs = board_start.n_discs();
    for (int n_discs = 4; n_discs < HW2; ++n_discs) {
        int n_empties = HW2 - n_discs;
        if (n_empties <= 24) { // complete
            depth_arr[n_discs] = n_empties;
            mpc_level_arr[n_discs] = MPC_100_LEVEL;
        } else if (n_empties <= 26) { // 99%
            depth_arr[n_discs] = n_empties;
            mpc_level_arr[n_discs] = MPC_99_LEVEL;
        } else if (n_empties <= 28) { // 98%
            depth_arr[n_discs] = n_empties;
            mpc_level_arr[n_discs] = MPC_98_LEVEL;
        } else if (n_empties <= 30) { // 93%
            depth_arr[n_discs] = n_empties;
            mpc_level_arr[n_discs] = MPC_93_LEVEL;
        } else {
            depth_arr[n_discs] = std::min(n_empties, std::max(21, mid_depth - (n_discs - start_n_discs)));
            mpc_level_arr[n_discs] = MPC_74_LEVEL;
        }
    }
    if (show_log) {
        std::cerr << "mid_depth " << mid_depth << " " << board_start.to_str() << " ";
    }
    std::vector<Clog_result> clogs;
    bool is_player = false; // opponent first
    while (*searching) { // self play
        if (board.get_legal() == 0) {
            board.pass();
            is_player ^= 1;
            if (board.get_legal() == 0) {
                break;
            }
        }
        int n_discs = board.n_discs();
        int n_empties = HW2 - n_discs;
        if (n_empties >= 22) {
            boards.emplace_back(board);
        }
        Search search(&board, mpc_level_arr[n_discs], use_multi_thread, false);
        std::pair<int, int> result = first_nega_scout(&search, -SCORE_MAX, SCORE_MAX, depth_arr[n_discs], depth_arr[n_discs] == n_empties, clogs, tim(), searching);
        if (show_log) {
            std::cerr << idx_to_coord(result.second);
        }
        calc_flip(&flip, &board, result.second);
        board.move_board(&flip);
        is_player ^= 1;
    }
    int score = SCORE_UNDEFINED;
    if (*searching) {
        score = board.score_player();
        if (!is_player) {
            score *= -1;
        }
    }
    if (show_log) {
        if (!(*searching)) {
            std::cerr << " terminated";
        } else {
            std::cerr << " " << score;
        }
        std::cerr << std::endl;
    }
    for (int i = (int)boards.size() - 1; i >= 0 && (*searching); --i) { // analyze
        int n_discs = board.n_discs();
        int n_empties = HW2 - n_discs;
        Search search(&board, mpc_level_arr[n_discs], use_multi_thread, false);
        first_nega_scout(&search, -SCORE_MAX, SCORE_MAX, depth_arr[n_discs], depth_arr[n_discs] == n_empties, clogs, tim(), searching);
    }
    return score;
}

bool comp_ponder_elem(Ponder_elem &a, Ponder_elem &b) {
    if (a.count == b.count) {
        return a.value > b.value;
    }
    return a.count > b.count;
}

std::vector<Ponder_elem> ai_ponder(Board board, bool show_log, bool *searching) {
    uint64_t strt = tim();
    uint64_t legal = board.get_legal();
    if (legal == 0) {
        board.pass();
        legal = board.get_legal();
        if (legal == 0) {
            if (show_log) {
                std::cerr << "no ponder needed because of game over" << std::endl;
            }
            std::vector<Ponder_elem> empty_list;
            return empty_list;
        } else {
            std::cerr << "ponder pass found" << std::endl;
        }
    }
    const int canput = pop_count_ull(legal);
    std::vector<Ponder_elem> move_list(canput);
    int idx = 0;
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)) {
        calc_flip(&move_list[idx].flip, &board, cell);
        move_list[idx].value = INF;
        move_list[idx].count = 0;
        move_list[idx].depth = 0;
        move_list[idx].mpc_level = MPC_74_LEVEL;
        move_list[idx].is_endgame_search = false;
        move_list[idx].is_complete_search = false;
        ++idx;
    }
    const int max_depth = HW2 - board.n_discs() - 1;
    int n_searched_all = 0;
    while (*searching) {
        int selected_idx = -1;
        double max_ucb = -INF - 1;
        for (int i = 0; i < canput; ++i) {
            double ucb = -INF;
            if (n_searched_all == 0) { // not searched at all
                ucb = move_list[i].value;
            } else if (move_list[i].count == 0) { // this node is not searched
                ucb = INF;
            } else if (move_list[i].is_complete_search) { // fully searched
                ucb = -INF;
            } else {
                double depth_weight = (double)std::min(10, move_list[i].depth) / (double)std::min(10, max_depth);
                ucb = (double)(move_list[i].value + SCORE_MAX) / (double)(SCORE_MAX * 2) * depth_weight + 0.5 * sqrt(log(2.0 * (double)n_searched_all) / (double)move_list[i].count) * (1.5 - depth_weight);
                //std::cerr << move_list[i].value << "," << move_list[i].count << "," << sqrt(log(2.0 * (double)n_searched_all) / (double)move_list[i].count) << "," << ucb << " ";
            }
            if (ucb > max_ucb) {
                selected_idx = i;
                max_ucb = ucb;
            }
        }
        //std::cerr << std::endl;
        if (selected_idx == -1) {
            if (show_log) {
                std::cerr << "ponder: no move selected n_moves " << canput << std::endl;
            }
            break;
        }
        if (move_list[selected_idx].is_complete_search) {
            if (show_log) {
                std::cerr << "ponder completely searched" << std::endl;
            }
            break;
        }
        Board n_board = board.copy();
        n_board.move_board(&move_list[selected_idx].flip);
        int max_depth = HW2 - n_board.n_discs();
        int new_depth = move_list[selected_idx].depth + 1;
        uint_fast8_t new_mpc_level = move_list[selected_idx].mpc_level;
        if (new_depth > max_depth) {
            new_depth = max_depth;
            ++new_mpc_level;
        } else if (new_depth > max_depth - PONDER_ENDSEARCH_PRESEARCH_OFFSET_TIMELIMIT) {
            new_depth = max_depth;
        }
        bool new_is_end_search = (new_depth == max_depth);
        bool new_is_complete_search = new_is_end_search && new_mpc_level == MPC_100_LEVEL;
        Search search(&n_board, new_mpc_level, true, false);
        int v = -nega_scout(&search, -SCORE_MAX, SCORE_MAX, new_depth, false, LEGAL_UNDEFINED, new_is_end_search, searching);
        if (new_depth >= PONDER_START_SELFPLAY_DEPTH && !new_is_complete_search) { // additional search (selfplay)
            int v2 = ai_self_play_and_analyze(n_board, move_list[selected_idx].depth, false, true, searching); // no -1 (opponent first)
            if (*searching) {
                //double nv = ((double)v * 1.1 + (double)v2 * 0.9) / 2.0;
                //std::cerr << idx_to_coord(move_list[selected_idx].flip.pos) << " depth " << new_depth << "@" << SELECTIVITY_PERCENTAGE[new_mpc_level] << "%" << " v " << v << " v2 " << v2 << " new_v " << nv << std::endl;
                //v = nv;
                v = ((double)v * 1.1 + (double)v2 * 0.9) / 2.0;
            }
        }
        if (*searching) {
            if (move_list[selected_idx].value == INF || new_is_end_search) {
                move_list[selected_idx].value = v;
            } else {
                move_list[selected_idx].value = (0.9 * move_list[selected_idx].value + 1.1 * v) / 2.0;
            }
            move_list[selected_idx].depth = new_depth;
            move_list[selected_idx].mpc_level = new_mpc_level;
            move_list[selected_idx].is_endgame_search = new_is_end_search;
            move_list[selected_idx].is_complete_search = new_is_complete_search;
            ++move_list[selected_idx].count;
            ++n_searched_all;
        }
    }
    if (show_log && n_searched_all) {
        std::cerr << "ponder loop " << n_searched_all << " in " << tim() - strt << " ms" << std::endl;
        std::cerr << "ponder board " << board.to_str() << std::endl;
        std::sort(move_list.begin(), move_list.end(), comp_ponder_elem);
        for (int i = 0; i < canput; ++i) {
            std::cerr << "pd " << idx_to_coord(move_list[i].flip.pos) << " value " << std::fixed << std::setprecision(2) << move_list[i].value;
            std::cerr << " count " << move_list[i].count << " depth " << move_list[i].depth << "@" << SELECTIVITY_PERCENTAGE[move_list[i].mpc_level] << "%";
            if (move_list[i].is_complete_search) {
                std::cerr << " complete";
            } else if (move_list[i].is_endgame_search) {
                std::cerr << " endgame";
            }
            std::cerr << std::endl;
        }
    }
    return move_list;
}
