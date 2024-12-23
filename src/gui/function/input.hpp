/*
    Egaroucid Project

    @file input.hpp
        Input Functions
    @date 2021-2024
    @author Takuto Yamana
    @license GPL-3.0 license
*/

#pragma once
#include "const/gui_common.hpp"


std::vector<History_elem> import_transcript_processing(std::vector<History_elem> n_history, History_elem strt_elem, std::string transcript, bool* failed) {
    Board h_bd = strt_elem.board;
    String transcript_str = Unicode::Widen(transcript).replace(U"\r", U"").replace(U"\n", U"").replace(U" ", U"");
    if (transcript_str.size() % 2 != 0 && transcript_str.size() >= 120) {
        *failed = true;
        return n_history;
    }
    int y, x;
    uint64_t legal;
    Flip flip;
    History_elem history_elem;
    int player = strt_elem.player;
    //history_elem.set(h_bd, player, GRAPH_IGNORE_VALUE, -1, -1, -1, "");
    //n_history.emplace_back(history_elem);
    bool passed = false;
    for (int i = 0; i < (int)transcript_str.size(); i += 2) {
        if (is_pass_like_str(transcript_str.narrow().substr(i, 2)) && passed) {
            continue;
        }
        x = (int)transcript_str[i] - (int)'a';
        if (x < 0 || HW <= x) {
            x = (int)transcript_str[i] - (int)'A';
            if (x < 0 || HW <= x) {
                *failed = true;
                break;
            }
        }
        y = (int)transcript_str[i + 1] - (int)'1';
        if (y < 0 || HW <= y) {
            *failed = true;
            break;
        }
        y = HW_M1 - y;
        x = HW_M1 - x;
        legal = h_bd.get_legal();
        if (1 & (legal >> (y * HW + x))) {
            calc_flip(&flip, &h_bd, y * HW + x);
            h_bd.move_board(&flip);
            player ^= 1;
            passed = false;
            if (h_bd.get_legal() == 0ULL) {
                h_bd.pass();
                player ^= 1;
                passed = true;
                if (h_bd.get_legal() == 0ULL) {
                    h_bd.pass();
                    player ^= 1;
                    if (i != transcript_str.size() - 2) {
                        *failed = true;
                        break;
                    }
                }
            }
        } else {
            *failed = true;
            break;
        }
        n_history.back().next_policy = y * HW + x;
        history_elem.set(h_bd, player, GRAPH_IGNORE_VALUE, -1, y * HW + x, -1, "");
        n_history.emplace_back(history_elem);
    }
    return n_history;
}


std::pair<Board, int> import_board_processing(std::string board_str, bool *failed) {
    String board_str_str = Unicode::Widen(board_str).replace(U"\r", U"").replace(U"\n", U"").replace(U" ", U"");
    *failed = false;
    int bd_arr[HW2];
    Board bd;
    int player = -1;
    if (board_str_str.size() != HW2 + 1) {
        *failed = true;
    } else {
        for (int i = 0; i < HW2; ++i) {
            if (is_black_like_char(board_str_str[i])) {
                bd_arr[i] = BLACK;
            } else if (is_white_like_char(board_str_str[i])) {
                bd_arr[i] = WHITE;
            } else if (is_vacant_like_char(board_str_str[i])) {
                bd_arr[i] = VACANT;
            } else {
                *failed = true;
                break;
            }
        }
        if (is_black_like_char(board_str_str[HW2])) {
            player = BLACK;
        } else if (is_white_like_char(board_str_str[HW2])) {
            player = WHITE;
        } else {
            *failed = true;
        }
    }
    Board board;
    if (!(*failed)) {
        board.translate_from_arr(bd_arr, player);
        if (!board.is_end() && board.get_legal() == 0) {
            board.pass();
        }
    }
    return std::make_pair(board, player);
}




std::vector<History_elem> import_ggf_processing(std::string ggf, bool* failed) {
    std::vector<History_elem> n_history;
    String ggf_str = Unicode::Widen(ggf).replace(U"\r", U"").replace(U"\n", U"").replace(U" ", U"");
    int board_start_idx = ggf_str.indexOf(U"BO[8");
    if (board_start_idx == std::string::npos) {
        *failed = true;
        return n_history;
    }
    board_start_idx += 4;
    if (ggf_str.size() < board_start_idx + 65) {
        *failed = true;
        return n_history;
    }
    std::string start_board_str = ggf_str.substr(board_start_idx, 65).narrow();
    std::cerr << "start board " << start_board_str << std::endl;
    std::pair<Board, player> board_player = import_board_processing(start_board_str, failed);
    if (*failed) {
        return n_history;
    }
    History_elem start_board;
    start_board.board = board_player.first;
    start_board.player = board_player.second;
    n_history.emplace_back(start_board);
    std::string transcript;
    int offset = board_start_idx + 65;
    int coord_start_idx;
    while (true) {
        coord_start_idx = ggf_str.indexOf(U"B[", offset);
        if (coord_start_idx == std::string::npos) {
            coord_start_idx = ggf_str.indexOf(U"W[", offset);
            if (coord_start_idx == std::string::npos) {
                break;
            }
        }
        coord_start_idx += 2;
        std::string coord = ggf_str.substr(coord_start_idx, 2).narrow();
        transcript += coord;
        std::cerr << coord << std::endl;
        offset = coord_start_idx + 2;
    }
    n_history = import_transcript_processing(n_history, start_board, transcript, failed);
    return n_history;
}