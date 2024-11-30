/*
    Egaroucid Project

    @file util.hpp
        Utility
    @date 2021-2024
    @author Takuto Yamana
    @license GPL-3.0 license
*/

#pragma once
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include "board.hpp"

inline bool is_black_like_char(char c) {
    return c == 'B' || c == 'b' || c == 'X' || c == 'x' || c == '0' || c == '*';
}

inline bool is_white_like_char(char c) {
    return c == 'W' || c == 'w' || c == 'O' || c == 'o' || c == '1';
}

std::pair<Board, int> convert_board_from_str(std::string board_str) {
    Board board;
    if (!board.from_str(board_str)) {
        return std::make_pair(board, -1); // error
    }
    int player = BLACK;
    if (is_black_like_char(board_str[HW2])) {
        player = BLACK;
    } else if (is_white_like_char(board_str[HW2])) {
        player = WHITE;
    }
    return std::make_pair(board, player);
}

/*
    @brief Input board from console

    '0' as black, '1' as white, '.' as empty

    @return board structure
*/
Board input_board() {
    Board res;
    char elem;
    int player;
    std::cin >> player;
    res.player = 0;
    res.opponent = 0;
    for (int i = 0; i < HW2; ++i) {
        std::cin >> elem;
        if (elem == '0') {
            if (player == BLACK)
                res.player |= 1ULL << (HW2_M1 - i);
            else
                res.opponent |= 1ULL << (HW2_M1 - i);
        } else if (elem == '1') {
            if (player == WHITE)
                res.player |= 1ULL << (HW2_M1 - i);
            else
                res.opponent |= 1ULL << (HW2_M1 - i);
        }
    }
    return res;
}

// use Base81 from https://github.com/primenumber/issen/blob/f418af2c7decac8143dd699c7ee89579013987f7/README.md#base81
/*
    empty square:  0
    player disc:   1
    opponent disc: 2

    board coordinate:
        a  b  c  d  e  f  g  h
        ------------------------
    1| 0  1  2  3  4  5  6  7
    2| 8  9 10 11 12 13 14 15
    3|16 17 18 19 20 21 22 23
    4|24 25 26 27 28 29 30 31
    5|32 33 34 35 36 37 38 39
    6|40 41 42 43 44 45 46 47
    7|48 49 50 51 52 53 54 55
    8|56 57 58 59 60 61 62 63

    the 'i'th character of the string (length=16) is calculated as:
    char c = 
            '!' + 
            board[i * 4] + 
            board[i * 4 + 1] * 3 + 
            board[i * 4 + 2] * 9 + 
            board[i * 4 + 3] * 32
    
*/
bool input_board_base81(std::string board_str, Board *board) {
    if (board_str.length() != 16) {
        std::cerr << "[ERROR] invalid argument" << std::endl;
        return true;
    }
    board->player = 0;
    board->opponent = 0;
    int idx, d;
    char c;
    for (int i = 0; i < 16; ++i) {
        idx = i * 4 + 3;
        c = board_str[i] - '!';
        d = c / 32;
        if (d == 1) {
            board->player |= 1ULL << idx;
        } else if (d == 2) {
            board->opponent |= 1ULL << idx;
        }
        --idx;
        c %= 32;
        d = c / 9;
        if (d == 1) {
            board->player |= 1ULL << idx;
        } else if (d == 2) {
            board->opponent |= 1ULL << idx;
        }
        --idx;
        c %= 9;
        d = c / 3;
        if (d == 1) {
            board->player |= 1ULL << idx;
        } else if (d == 2) {
            board->opponent |= 1ULL << idx;
        }
        --idx;
        c %= 3;
        d = c;
        if (d == 1) {
            board->player |= 1ULL << idx;
        } else if (d == 2) {
            board->opponent |= 1ULL << idx;
        }
    }
    return false;
}

/*
    @brief Generate coordinate in string

    @param idx                  index of the coordinate
    @return coordinate as string
*/
std::string idx_to_coord(int idx) {
    if (idx < 0 || HW2 <= idx)
        return "??";
    int y = HW_M1 - idx / HW;
    int x = HW_M1 - idx % HW;
    const std::string x_coord = "abcdefgh";
    return x_coord[x] + std::to_string(y + 1);
}

/*
    @brief Generate time in string

    @param t                    time in [ms]
    @return time with ms as string
*/
std::string ms_to_time(uint64_t t) {
    std::string res;
    uint64_t hour = t / (1000 * 60 * 60);
    t %= 1000 * 60 * 60;
    uint64_t minute = t / (1000 * 60);
    t %= 1000 * 60;
    uint64_t second = t / 1000;
    uint64_t msecond = t % 1000;
    std::ostringstream hour_s;
    hour_s << std::right << std::setw(3) << std::setfill('0') << hour;
    res += hour_s.str();
    res += ":";
    std::ostringstream minute_s;
    minute_s << std::right << std::setw(2) << std::setfill('0') << minute;
    res += minute_s.str();
    res += ":";
    std::ostringstream second_s;
    second_s << std::right << std::setw(2) << std::setfill('0') << second;
    res += second_s.str();
    res += ".";
    std::ostringstream msecond_s;
    msecond_s << std::right << std::setw(3) << std::setfill('0') << msecond;
    res += msecond_s.str();
    return res;
}

/*
    @brief Generate time in string

    @param t                    time in [ms]
    @return time as string
*/
std::string ms_to_time_short(uint64_t t) {
    std::string res;
    uint64_t hour = t / (1000 * 60 * 60);
    t -= hour * 1000 * 60 * 60;
    uint64_t minute = t / (1000 * 60);
    t -= minute * 1000 * 60;
    uint64_t second = t / 1000;
    t -= second * 1000;
    std::ostringstream hour_s;
    hour_s << std::right << std::setw(3) << std::setfill('0') << hour;
    res += hour_s.str();
    res += ":";
    std::ostringstream minute_s;
    minute_s << std::right << std::setw(2) << std::setfill('0') << minute;
    res += minute_s.str();
    res += ":";
    std::ostringstream second_s;
    second_s << std::right << std::setw(2) << std::setfill('0') << second;
    res += second_s.str();
    return res;
}


inline int convert_coord_from_representative_board(int cell, int idx) {
    int res;
    int y = cell / HW;
    int x = cell % HW;
    switch (idx) {
        case 0:
            res = cell;
            break;
        case 1:
            res = (HW_M1 - y) * HW + x; // vertical
            break;
        case 2:
            res = (HW_M1 - x) * HW + (HW_M1 - y); // black line
            break;
        case 3:
            res = (HW_M1 - x) * HW + y; // black line + vertical ( = rotate 90 clockwise)
            break;
        case 4:
            res = x * HW + (HW_M1 - y); // black line + horizontal ( = rotate 90 counterclockwise)
            break;
        case 5:
            res = x * HW + y; // black line + horizontal + vertical ( = white line)
            break;
        case 6:
            res = y * HW + (HW_M1 - x); // horizontal
            break;
        case 7:
            res = (HW_M1 - y) * HW + (HW_M1 - x); // horizontal + vertical ( = rotate180)
            break;
        default:
            std::cerr << "converting coord error in book" << std::endl;
            break;
    }
    return res;
}

inline int convert_coord_to_representative_board(int cell, int idx) {
    int res;
    int y = cell / HW;
    int x = cell % HW;
    switch (idx) {
        case 0:
            res = cell;
            break;
        case 1:
            res = (HW_M1 - y) * HW + x; // vertical
            break;
        case 2:
            res = (HW_M1 - x) * HW + (HW_M1 - y); // black line
            break;
        case 3:
            res = x * HW + (HW_M1 - y); // black line + vertical ( = rotate 90 clockwise)
            break;
        case 4:
            res = (HW_M1 - x) * HW + y; // black line + horizontal ( = rotate 90 counterclockwise)
            break;
        case 5:
            res = x * HW + y; // black line + horizontal + vertical ( = white line)
            break;
        case 6:
            res = y * HW + (HW_M1 - x); // horizontal
            break;
        case 7:
            res = (HW_M1 - y) * HW + (HW_M1 - x); // horizontal + vertical ( = rotate180)
            break;
        default:
            std::cerr << "converting coord error in book" << std::endl;
            break;
    }
    return res;
}

inline void first_update_representative_board(Board *res, Board *sym) {
    uint64_t vp = vertical_mirror(sym->player);
    uint64_t vo = vertical_mirror(sym->opponent);
    if (res->player > vp || (res->player == vp && res->opponent > vo)) {
        res->player = vp;
        res->opponent = vo;
    }
}

inline void update_representative_board(Board *res, Board *sym) {
    if (res->player > sym->player || (res->player == sym->player && res->opponent > sym->opponent))
        sym->copy(res);
    uint64_t vp = vertical_mirror(sym->player);
    uint64_t vo = vertical_mirror(sym->opponent);
    if (res->player > vp || (res->player == vp && res->opponent > vo)) {
        res->player = vp;
        res->opponent = vo;
    }
}

inline void first_update_representative_board(Board *res, Board *sym, int *idx, int *cnt) {
    uint64_t vp = vertical_mirror(sym->player);
    uint64_t vo = vertical_mirror(sym->opponent);
    ++(*cnt);
    if (res->player > vp || (res->player == vp && res->opponent > vo)) {
        res->player = vp;
        res->opponent = vo;
        *idx = *cnt;
    }
}

inline void update_representative_board(Board *res, Board *sym, int *idx, int *cnt) {
    ++(*cnt);
    if (res->player > sym->player || (res->player == sym->player && res->opponent > sym->opponent)) {
        sym->copy(res);
        *idx = *cnt;
    }
    uint64_t vp = vertical_mirror(sym->player);
    uint64_t vo = vertical_mirror(sym->opponent);
    ++(*cnt);
    if (res->player > vp || (res->player == vp && res->opponent > vo)) {
        res->player = vp;
        res->opponent = vo;
        *idx = *cnt;
    }
}

inline Board get_representative_board(Board b) {
    Board res = b;
    first_update_representative_board(&res, &b);
    b.board_black_line_mirror();
    update_representative_board(&res, &b);
    b.board_horizontal_mirror();
    update_representative_board(&res, &b);
    b.board_white_line_mirror();
    update_representative_board(&res, &b);
    return res;
}

inline Board get_representative_board(Board b, int *idx) {
    Board res = b;
    *idx = 0;
    int cnt = 0;
    first_update_representative_board(&res, &b, idx, &cnt);
    b.board_black_line_mirror();
    update_representative_board(&res, &b, idx, &cnt);
    b.board_horizontal_mirror();
    update_representative_board(&res, &b, idx, &cnt);
    b.board_white_line_mirror();
    update_representative_board(&res, &b, idx, &cnt);
    return res;
}

inline Board get_representative_board(Board *b, int *idx) {
    return get_representative_board(b->copy(), idx);
}

inline Board get_representative_board(Board *b) {
    return get_representative_board(b->copy());
}
