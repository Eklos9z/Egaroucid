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
#include "level.hpp"
#include "setting.hpp"
#include "midsearch.hpp"
#include "book.hpp"
#include "util.hpp"
#include "clogsearch.hpp"
#include "lazy_smp.hpp"

#define SEARCH_BOOK -1

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
inline Search_result tree_search_legal(Board board, int depth, uint_fast8_t mpc_level, bool show_log, uint64_t use_legal, bool use_multi_thread){
    uint64_t strt;
    depth = std::min(HW2 - board.n_discs(), depth);
    bool is_end_search = (HW2 - board.n_discs() == depth);
    std::vector<Clog_result> clogs;
    uint64_t clog_nodes = 0;
    uint64_t clog_time = 0;
    if (mpc_level != MPC_100_LEVEL){
        strt = tim();
        clogs = first_clog_search(board, &clog_nodes, std::min(depth, CLOG_SEARCH_MAX_DEPTH));
        clog_time = tim() - strt;
        if (show_log){
            std::cerr << "clog search time " << clog_time << " nodes " << clog_nodes << " nps " << calc_nps(clog_nodes, clog_time) << std::endl;
            for (int i = 0; i < (int)clogs.size(); ++i){
                std::cerr << "clogsearch " << i + 1 << "/" << clogs.size() << " " << idx_to_coord(clogs[i].pos) << " value " << clogs[i].val << std::endl;
            }
        }
    }
    Search_result res;
    res = lazy_smp(board, depth, mpc_level, show_log, clogs, use_legal, use_multi_thread);
    res.clog_nodes = clog_nodes;
    res.clog_time = clog_time;
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
Search_result ai_legal(Board board, int level, bool use_book, int book_acc_level, bool use_multi_thread, bool show_log, uint64_t use_legal){
    Search_result res;
    int value_sign = 1;
    if (board.get_legal() == 0ULL){
        board.pass();
        if (board.get_legal() == 0ULL){
            res.policy = 64;
            res.value = -board.score_player();
            res.depth = 0;
            res.nps = 0;
            res.is_end_search = true;
            res.probability = 100;
            return res;
        } else{
            value_sign = -1;
        }
    }
    Book_value book_result = book.get_random_specified_moves(&board, book_acc_level, use_legal);
    if (book_result.policy != -1 && use_book){
        if (show_log)
            std::cerr << "book " << idx_to_coord(book_result.policy) << " " << book_result.value << " at book error level " << book_acc_level << std::endl;
        res.policy = book_result.policy;
        res.value = value_sign * book_result.value;
        res.depth = SEARCH_BOOK;
        res.nps = 0;
        res.is_end_search = false;
        res.probability = 100;
    } else if (level == 0){
        uint64_t legal = use_legal;
        std::vector<int> move_lst;
        for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal))
            move_lst.emplace_back(cell);
        res.policy = move_lst[myrandrange(0, (int)move_lst.size())];
        res.value = value_sign * mid_evaluate(&board);
        res.depth = 0;
        res.nps = 0;
        res.is_end_search = false;
        res.probability = 0;
    } else{
        int depth;
        bool is_mid_search;
        uint_fast8_t mpc_level;
        get_level(level, board.n_discs() - 4, &is_mid_search, &depth, &mpc_level);
        if (show_log)
            std::cerr << "level status " << level << " " << board.n_discs() - 4 << " discs depth " << depth << "@" << SELECTIVITY_PERCENTAGE[mpc_level] << "%" << std::endl;
        res = tree_search_legal(board, depth, mpc_level, show_log, use_legal, use_multi_thread);
        res.value *= value_sign;
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
Search_result ai(Board board, int level, bool use_book, int book_acc_level, bool use_multi_thread, bool show_log){
    return ai_legal(board, level, use_book, book_acc_level, use_multi_thread, show_log, board.get_legal());
}

/*
    @brief Search for analyze command

    @param board                board to solve
    @param level                level of AI
    @param use_multi_thread     search in multi thread?
    @return the result in Search_result structure
*/
/*
Analyze_result ai_analyze(Board board, int level, bool use_multi_thread, uint_fast8_t played_move){
    Analyze_result res;
    res.played_move = played_move;
    int got_depth, depth;
    bool is_mid_search;
    uint_fast8_t mpc_level;
    get_level(level, board.n_discs() - 4, &is_mid_search, &got_depth, &mpc_level);
    if (got_depth - 1 >= 0)
        depth = got_depth - 1;
    else
        depth = got_depth;
    Search search;
    search.init_board(&board);
    search.n_nodes = 0ULL;
    search.mpc_level = mpc_level;
    #if USE_SEARCH_STATISTICS
        for (int i = 0; i < HW2; ++i)
            search.n_nodes_discs[i] = 0;
    #endif
    search.use_multi_thread = use_multi_thread;
    Book_elem book_elem = book.get(&search.board);
    std::vector<Book_value> links = book.get_all_moves_with_value(&search.board);
    Flip flip;
    calc_flip(&flip, &search.board, played_move);
    search.move(&flip);
        res.played_score = book_elem.value;
        if (res.played_score != SCORE_UNDEFINED){
            res.played_depth = SEARCH_BOOK;
            res.played_probability = SELECTIVITY_PERCENTAGE[MPC_100_LEVEL];
        } else{
            res.played_score = -first_nega_scout_value(&search, -SCORE_MAX, SCORE_MAX, depth, !is_mid_search, false, false, search.board.get_legal());
            res.played_depth = got_depth;
            res.played_probability = SELECTIVITY_PERCENTAGE[mpc_level];
        }
    search.undo(&flip);
    uint64_t legal = search.board.get_legal() ^ (1ULL << played_move);
    if (legal){
        uint64_t legal_copy = legal;
        bool book_found = false;
        Flip flip;
        int g, v = -INF, best_move = -1;
        for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)){
            calc_flip(&flip, &search.board, cell);
            search.board.move_board(&flip);
                g = SCORE_UNDEFINED;
                for (Book_value book_value: links){
                    if (book_value.policy == cell){
                        g = book_value.value;
                        break;
                    }
                }
                if (g != SCORE_UNDEFINED){
                    book_found = true;
                    if (v < g){
                        v = g;
                        best_move = flip.pos;
                    }
                }
            search.board.undo_board(&flip);
        }
        if (book_found){
            res.alt_move = best_move;
            res.alt_score = v;
            res.alt_depth = SEARCH_BOOK;
            res.alt_probability = SELECTIVITY_PERCENTAGE[MPC_100_LEVEL];
        } else{
            std::vector<Clog_result> clogs;
            uint64_t strt = tim();
            std::pair<int, int> nega_scout_res = first_nega_scout_legal(&search, -SCORE_MAX, SCORE_MAX, SCORE_UNDEFINED, got_depth, !is_mid_search, false, clogs, legal_copy, strt);
            res.alt_move = nega_scout_res.second;
            res.alt_score = nega_scout_res.first;
            res.alt_depth = got_depth;
            res.alt_probability = SELECTIVITY_PERCENTAGE[mpc_level];
        }
    } else{
        res.alt_move = -1;
        res.alt_score = SCORE_UNDEFINED;
        res.alt_depth = -1;
        res.alt_probability = -1;
    }
    return res;
}
*/

Search_result ai_accept_loss(Board board, int level, int acceptable_loss){
    uint64_t strt = tim();
    Flip flip;
    int v = SCORE_UNDEFINED;
    uint64_t legal = board.get_legal();
    std::vector<std::pair<int, int>> moves;
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)){
        calc_flip(&flip, &board, cell);
        board.move_board(&flip);
            int g = -ai(board, level, true, 0, true, false).value;
        board.undo_board(&flip);
        v = std::max(v, g);
        moves.emplace_back(std::make_pair(g, cell));
    }
    std::vector<std::pair<int, int>> acceptable_moves;
    for (std::pair<int, int> move: moves){
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