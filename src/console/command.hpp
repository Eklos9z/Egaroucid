/*
    Egaroucid Project

    @file command.hpp
        Commands for Egaroucid
    @date 2021-2024
    @author Takuto Yamana
    @license GPL-3.0 license
*/

#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <cctype>
#include <algorithm>
#include "./../engine/engine_all.hpp"
#include "board_info.hpp"
#include "option.hpp"
#include "state.hpp"
#include "close.hpp"
#include "print.hpp"
#include "command_definition.hpp"

#define ANALYZE_MISTAKE_THRESHOLD 4

std::string get_command_line() {
    std::cerr << "> ";
    std::string cmd_line;
    std::getline(std::cin, cmd_line);
    return cmd_line;
}

void split_cmd_arg(std::string cmd_line, std::string *cmd, std::string *arg) {
    std::istringstream iss(cmd_line);
    iss >> *cmd;
    iss.get();
    std::getline(iss, *arg);
}

int get_command_id(std::string cmd) {
    for (int i = 0; i < N_COMMANDS; ++i) {
        if (std::find(command_data[i].names.begin(), command_data[i].names.end(), cmd) != command_data[i].names.end())
            return command_data[i].id;
    }
    return COMMAND_NOT_FOUND;
}

void allocate_time(Options *options, State *state) {
    if (options->time_allocated_seconds != TIME_NOT_ALLOCATED) {
        state->remaining_time_msec_black = 1000ULL * options->time_allocated_seconds;
        state->remaining_time_msec_white = 1000ULL * options->time_allocated_seconds;
    }
}

void update_time(int player, State *state, Options *options, uint64_t elapsed) {
    if (options->time_allocated_seconds != TIME_NOT_ALLOCATED) {
        uint64_t *remaining_time_msec;
        if (player == BLACK) {
            remaining_time_msec = &state->remaining_time_msec_black;
        } else {
            remaining_time_msec = &state->remaining_time_msec_white;
        }
        if (elapsed <= *remaining_time_msec) {
            *remaining_time_msec -= elapsed;
        } else {
            *remaining_time_msec = 0;
        }
    }
}

void init_board(Board_info *board, Options *options, State *state) {
    board->reset();
    allocate_time(options, state);
}

void new_board(Board_info *board, Options *options, State *state) {
    board->board = board->boards[0].copy();
    board->player = board->players[0];
    board->boards.clear();
    board->players.clear();
    board->boards.emplace_back(board->board);
    board->players.emplace_back(board->player);
    board->ply_vec = 0;
    allocate_time(options, state);
}

bool outside(int y, int x) {
    return y < 0 || HW <= y || x < 0 || HW <= x;
}

void play(Board_info *board, std::string transcript) {
    if (transcript.length() % 2) {
        std::cerr << "[ERROR] invalid transcript length" << std::endl;
        return;
    }
    Board_info board_bak = board->copy();
    while (board->ply_vec < (int)board->boards.size() - 1) {
        board->boards.pop_back();
        board->players.pop_back();
    }
    Flip flip;
    for (int i = 0; i < (int)transcript.length(); i += 2) {
        int x = HW_M1 - (int)(transcript[i] - 'a');
        if (x >= HW)
            x = HW_M1 - (int)(transcript[i] - 'A');
        int y = HW_M1 - (int)(transcript[i + 1] - '1');
        if (outside(y, x)) {
            std::cerr << "[ERROR] invalid coordinate " << transcript[i] << transcript[i + 1] << std::endl;
            *board = board_bak;
            return;
        }
        calc_flip(&flip, &board->board, y * HW + x);
        if (flip.flip == 0ULL) {
            std::cerr << "[ERROR] invalid move " << transcript[i] << transcript[i + 1] << std::endl;
            *board = board_bak;
            return;
        }
        board->board.move_board(&flip);
        board->player ^= 1;
        if (board->board.is_end() && i < (int)transcript.length() - 2) {
            std::cerr << "[ERROR] game over found before checking all transcript. remaining codes ignored." << std::endl;
            return;
        }
        if (board->board.get_legal() == 0ULL) {
            board->board.pass();
            board->player ^= 1;
        }
        board->boards.emplace_back(board->board);
        board->players.emplace_back(board->player);
        ++board->ply_vec;
    }
}

int calc_remain(std::string arg) {
    int remain = 1;
    try{
        remain = std::stoi(arg);
    } catch (const std::invalid_argument& ex) {
        remain = 1;
    } catch (const std::out_of_range& ex) {
        remain = 1;
    }
    if (remain <= 0)
        remain = 1;
    return remain;
}

void undo(Board_info *board, int remain) {
    if (remain == 0)
        return;
    if (board->ply_vec <= 0) {
        std::cerr << "[ERROR] can't undo" << std::endl;
        return;
    }
    --board->ply_vec;
    board->board = board->boards[board->ply_vec].copy();
    board->player = board->players[board->ply_vec];
    undo(board, remain - 1);
}

void redo(Board_info *board, int remain) {
    if (remain == 0)
        return;
    if (board->ply_vec >= (int)board->boards.size() - 1) {
        std::cerr << "[ERROR] can't redo" << std::endl;
        return;
    }
    ++board->ply_vec;
    board->board = board->boards[board->ply_vec].copy();
    board->player = board->players[board->ply_vec];
    redo(board, remain - 1);
}

#define TIME_MANAGEMENT_REMAINING_TIME_OFFSET 200 // ms / move
//#define TIME_MANAGEMENT_REMAINING_MOVES_OFFSET 12 // moves (fast complete search = 24 moves)
#define TIME_MANAGEMENT_REMAINING_MOVES_OFFSET 5

uint64_t calc_time_limit_ply(const Board board, uint64_t remaining_time_msec) {
    // try complete search
    // Nodes(depth) = a * exp(b * depth)
    constexpr double const_a = 0.5;
    constexpr double const_b = 0.8;
    constexpr double nps = 120000000.0;
    double complete_search_depth = log((double)remaining_time_msec / 1000.0 * nps / const_a) / const_b;
    std::cerr << "complete search depth " << complete_search_depth << std::endl;
    int n_empties = HW2 - board.n_discs();
    if (n_empties <= complete_search_depth) {
        return remaining_time_msec * 0.8;
    }
    // midgame search
    int remaining_moves = (n_empties + 1) / 2;
    if (remaining_time_msec > TIME_MANAGEMENT_REMAINING_TIME_OFFSET * remaining_moves) {
        uint64_t remaining_time_msec_proc = remaining_time_msec - TIME_MANAGEMENT_REMAINING_TIME_OFFSET * remaining_moves;
        int remaining_moves_proc = std::max(2, remaining_moves - TIME_MANAGEMENT_REMAINING_MOVES_OFFSET);
        if (remaining_time_msec > 60000 && board.n_discs() >= 30) { // last 34 moves (60 sec)
            remaining_moves_proc = std::max(2, remaining_moves_proc - 1);
        } else if (remaining_time_msec > 30000 && board.n_discs() >= 32) { // last 32 moves (30 sec)
            remaining_moves_proc = std::max(2, remaining_moves_proc - 1);
        } else if (remaining_time_msec > 10000 && board.n_discs() >= 30) { // last 30 moves (10 sec)
            remaining_moves_proc = std::max(2, remaining_moves_proc - 1);
        }
        return remaining_time_msec_proc / remaining_moves_proc;
    }
    return 1;
}

Search_result go_noprint(Board_info *board, Options *options, State *state) {
    if (board->board.is_end()) {
        std::cerr << "[ERROR] game over" << std::endl;
        Search_result res;
        return res;
    }
    Search_result result;
    if (options->time_allocated_seconds == TIME_NOT_ALLOCATED) {
        result = ai(board->board, options->level, true, 0, true, options->show_log);
    } else {
        uint64_t start_time = tim();
        uint64_t *remaining_time_msec;
        if (board->player == BLACK) {
            remaining_time_msec = &state->remaining_time_msec_black;
        } else {
            remaining_time_msec = &state->remaining_time_msec_white;
        }
        uint64_t time_limit_ply = calc_time_limit_ply(board->board, *remaining_time_msec);
        if (options->show_log) {
            std::cerr << "time limit: " << time_limit_ply << std::endl;
        }
        result = ai_time_limit(board->board, options->level, true, 0, true, options->show_log, time_limit_ply);
    }
    Flip flip;
    calc_flip(&flip, &board->board, result.policy);
    board->board.move_board(&flip);
    board->player ^= 1;
    if (board->board.get_legal() == 0ULL) {
        board->board.pass();
        board->player ^= 1;
    }
    while (board->ply_vec < (int)board->boards.size() - 1) {
        board->boards.pop_back();
        board->players.pop_back();
    }
    board->boards.emplace_back(board->board);
    board->players.emplace_back(board->player);
    ++board->ply_vec;
    return result;
}

void go(Board_info *board, Options *options, State *state, uint64_t start_time) {
    int before_player = board->player;
    Search_result result = go_noprint(board, options, state);
    update_time(before_player, state, options, tim() - start_time);
    if (options->quiet) {
        print_search_result_quiet(result);
    } else {
        print_search_result(result, options, state);
    }
}

void setboard(Board_info *board, Options *options, State *state, std::string board_str) {
    board_str.erase(std::remove_if(board_str.begin(), board_str.end(), ::isspace), board_str.end());
    if (board_str.length() != HW2 + 1) {
        std::cerr << "[ERROR] invalid argument" << std::endl;
        return;
    }
    Board new_board;
    int player = BLACK;
    new_board.player = 0ULL;
    new_board.opponent = 0ULL;
    for (int i = 0; i < HW2; ++i) {
        if (board_str[i] == 'B' || board_str[i] == 'b' || board_str[i] == 'X' || board_str[i] == 'x' || board_str[i] == '0' || board_str[i] == '*')
            new_board.player |= 1ULL << (HW2_M1 - i);
        else if (board_str[i] == 'W' || board_str[i] == 'w' || board_str[i] == 'O' || board_str[i] == 'o' || board_str[i] == '1')
            new_board.opponent |= 1ULL << (HW2_M1 - i);
    }
    if (board_str[HW2] == 'B' || board_str[HW2] == 'b' || board_str[HW2] == 'X' || board_str[HW2] == 'x' || board_str[HW2] == '0' || board_str[HW2] == '*')
        player = BLACK;
    else if (board_str[HW2] == 'W' || board_str[HW2] == 'w' || board_str[HW2] == 'O' || board_str[HW2] == 'o' || board_str[HW2] == '1')
        player = WHITE;
    else{
        std::cerr << "[ERROR] invalid player argument" << std::endl;
        return;
    }
    if (player == WHITE)
        std::swap(new_board.player, new_board.opponent);
    board->board = new_board.copy();
    board->player = player;
    board->boards.clear();
    board->players.clear();
    board->boards.emplace_back(board->board);
    board->players.emplace_back(board->player);
    board->ply_vec = 0;
    allocate_time(options, state);
}

void set_level(Options *options, std::string level_str) {
    try {
        int level = std::stoi(level_str);
        if (1 <= level && level < N_LEVEL) {
            options->level = level;
            if (options->show_log)
                std::cerr << "level set to " << options->level << std::endl;
        } else
            std::cerr << "[ERROR] level out of range" << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "[ERROR] invalid level" << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "[ERROR] level invalid argument" << std::endl;
    }
}

void set_mode(Options *options, std::string mode_str) {
    try {
        int mode = std::stoi(mode_str);
        if (0 <= mode && mode < 4) {
            options->mode = mode;
            if (options->show_log)
                std::cerr << "mode set to " << options->mode << std::endl;
        } else
            std::cerr << "[ERROR] mode out of range" << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "[ERROR] invalid mode" << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "[ERROR] mode invalid argument" << std::endl;
    }
}

void hint(Board_info *board, Options *options, State *state, std::string arg) {
    int n_show = 1;
    try {
        n_show = std::stoi(arg);
        if (n_show < 1)
            n_show = 1;
    } catch (const std::invalid_argument& e) {
        n_show = 1;
    } catch (const std::out_of_range& e) {
        n_show = 1;
    }
    uint64_t legal = board->board.get_legal();
    if (n_show > pop_count_ull(legal))
        n_show = pop_count_ull(legal);
    std::vector<Book_value> result_book_value = book.get_all_moves_with_value(&board->board);
    std::vector<Search_result> result;
    for (Book_value elem: result_book_value)
        result.emplace_back(elem.to_search_result());
    if ((int)result.size() < n_show) {
        for (const Search_result &elem: result)
            legal ^= 1ULL << elem.policy;
        int n_show_ai = n_show - (int)result.size();
        for (int i = 0; i < n_show_ai; ++i) {
            Search_result elem = ai_legal(board->board, options->level, true, 0, true, false, legal);
            result.emplace_back(elem);
            legal ^= 1ULL << elem.policy;
        }
    }
    std::sort(result.rbegin(), result.rend());
    print_search_result_head();
    for (int i = 0; i < n_show; ++i)
        print_search_result_body(result[i], options, state);
}

inline void analyze(Board_info *board, Options *options, State *state) {
    print_transcript(board->boards);
    print_analyze_head();
    Analyze_summary summary[2];
    for (int i = (int)board->boards.size() - 2; i >= 0; --i) {
        Board n_board = board->boards[i].copy();
        uint64_t played_board = (n_board.player | n_board.opponent) ^ (board->boards[i + 1].player | board->boards[i + 1].opponent);
        if (pop_count_ull(played_board) == 1) {
            uint_fast8_t played_move = ctz(played_board);
            Analyze_result result = ai_analyze(n_board, options->level, true, played_move);
            std::string judge = "";
            ++summary[board->players[i]].n_ply;
            if (result.alt_score > result.played_score) {
                if (result.alt_score - result.played_score >= ANALYZE_MISTAKE_THRESHOLD) {
                    ++summary[board->players[i]].n_mistake;
                    summary[board->players[i]].sum_mistake += result.alt_score - result.played_score;
                    judge = "Mistake";
                } else{
                    ++summary[board->players[i]].n_disagree;
                    summary[board->players[i]].sum_disagree += result.alt_score - result.played_score;
                    judge = "Disagree";
                }
            }
            int ply = n_board.n_discs() - 3;
            print_analyze_body(result, ply, board->players[i], judge);
        }
    }
    print_analyze_foot(summary);
}

void generate_problems(Options *options, std::string arg) {
    int pos = arg.find(' ');
    if (pos == std::string::npos) {
        std::cerr << "[ERROR] please input <n_empties> <n_problems>" << std::endl;
    }else{
        std::string n_empties_str = arg.substr(0, pos);
        std::string n_problems_str = arg.substr(pos + 1);
        try{
            int n_empties = std::stoi(n_empties_str);
            int n_problems = std::stoi(n_problems_str);
            problem_generator(n_empties, n_problems, options->level);
        } catch (const std::invalid_argument& e) {
            std::cerr << "[ERROR] invalid argument" << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "[ERROR] out of range" << std::endl;
        }
    }
}

void check_command(Board_info *board, State *state, Options *options) {
    uint64_t start_time = tim();
    std::string cmd_line = get_command_line();
    std::string cmd, arg;
    split_cmd_arg(cmd_line, &cmd, &arg);
    int cmd_id = get_command_id(cmd);
    int player_before = board->player;
    switch (cmd_id) {
        case COMMAND_NOT_FOUND:
            std::cout << "[ERROR] command `" << cmd << "` not found" << std::endl;
            break;
        case CMD_ID_HELP:
            print_commands_list();
            break;
        case CMD_ID_EXIT:
            close(state, options);
            break;
        case CMD_ID_VERSION:
            print_version();
            break;
        case CMD_ID_INIT:
            init_board(board, options, state);
            break;
        case CMD_ID_NEW:
            new_board(board, options, state);
            break;
        case CMD_ID_PLAY:
            play(board, arg);
            update_time(player_before, state, options, tim() - start_time);
            break;
        case CMD_ID_UNDO:
            undo(board, calc_remain(arg));
            break;
        case CMD_ID_REDO:
            redo(board, calc_remain(arg));
            break;
        case CMD_ID_GO:
            go(board, options, state, start_time);
            break;
        case CMD_ID_SETBOARD:
            setboard(board, options, state, arg);
            break;
        case CMD_ID_LEVEL:
            set_level(options, arg);
            break;
        case CMD_ID_LEVELINFO:
            print_level_info();
            break;
        case CMD_ID_MODE:
            set_mode(options, arg);
            break;
        case CMD_ID_HINT:
            hint(board, options, state, arg);
            break;
        case CMD_ID_ANALYZE:
            analyze(board, options, state);
            break;
        case CMD_ID_CLEARCACHE:
            transposition_table.init();
            break;
        case CMD_ID_GENPROBLEM:
            generate_problems(options, arg);
            break;
        default:
            break;
    }
}