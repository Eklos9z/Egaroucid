#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include "./../../engine/board.hpp"
#include "./../../engine/util.hpp"

/*
    @brief array for calculating hash code for book
*/
size_t hash_rand_player_book[4][65536];
size_t hash_rand_opponent_book[4][65536];

/*
    @brief initialize hash array for book randomly
*/
void book_hash_init_rand(){
    int i, j;
    for (i = 0; i < 4; ++i){
        for (j = 0; j < 65536; ++j){
            hash_rand_player_book[i][j] = 0;
            while (pop_count_uint(hash_rand_player_book[i][j]) < 9)
                hash_rand_player_book[i][j] = myrand_ull();
            hash_rand_opponent_book[i][j] = 0;
            while (pop_count_uint(hash_rand_opponent_book[i][j]) < 9)
                hash_rand_opponent_book[i][j] = myrand_ull();
        }
    }
}

/*
    @brief Hash function for book

    @param board                board
    @return hash code
*/
struct Book_hash {
    size_t operator()(Board board) const{
        const uint16_t *p = (uint16_t*)&board.player;
        const uint16_t *o = (uint16_t*)&board.opponent;
        return 
            hash_rand_player_book[0][p[0]] ^ 
            hash_rand_player_book[1][p[1]] ^ 
            hash_rand_player_book[2][p[2]] ^ 
            hash_rand_player_book[3][p[3]] ^ 
            hash_rand_opponent_book[0][o[0]] ^ 
            hash_rand_opponent_book[1][o[1]] ^ 
            hash_rand_opponent_book[2][o[2]] ^ 
            hash_rand_opponent_book[3][o[3]];
    }
};

struct Enumerate_elem{
    std::vector<int> line;
};

std::unordered_map<Board, Enumerate_elem, Book_hash> all_lines;
Enumerate_elem global_elem;

inline void first_update_representative_board(Board *res, Board *sym){
    uint64_t vp = vertical_mirror(sym->player);
    uint64_t vo = vertical_mirror(sym->opponent);
    if (res->player > vp || (res->player == vp && res->opponent > vo)){
        res->player = vp;
        res->opponent = vo;
    }
}

inline void update_representative_board(Board *res, Board *sym){
    if (res->player > sym->player || (res->player == sym->player && res->opponent > sym->opponent))
        sym->copy(res);
    uint64_t vp = vertical_mirror(sym->player);
    uint64_t vo = vertical_mirror(sym->opponent);
    if (res->player > vp || (res->player == vp && res->opponent > vo)){
        res->player = vp;
        res->opponent = vo;
    }
}

inline Board get_representative_board(Board b){
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

void enumerate(Board board, const int n_moves){
    Board rboard = get_representative_board(board);
    if (all_lines.find(rboard) != all_lines.end()){
        return;
    }
    all_lines[rboard] = global_elem;
    if (board.n_discs() < n_moves + 4){
        uint64_t legal = board.get_legal();
        if (legal == 0){
            board.pass();
            legal = board.get_legal();
            if (legal == 0){
                return;
            }
        }
        Flip flip;
        for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)){
            calc_flip(&flip, &board, cell);
            board.move_board(&flip);
            global_elem.line.emplace_back(cell);
                enumerate(board, n_moves);
            global_elem.line.pop_back();
            board.undo_board(&flip);
        }
    }
}

int main(int argc, char *argv[]){
    if (argc < 2){
        std::cerr << "input [n_moves]" << std::endl;
        return 1;
    }

    mobility_init();
    flip_init();
    book_hash_init_rand();

    int n_moves = atoi(argv[1]);

    Board board;
    board.reset();
    enumerate(board, n_moves);
    std::cerr << all_lines.size() << " lines found at depth 0 to " << n_moves << std::endl;
    std::vector<int> n_lines_in_depth;
    for (int i = 0; i < n_moves + 1; ++i){
        n_lines_in_depth.emplace_back(0);
    }
    for (auto itr = all_lines.begin(); itr != all_lines.end(); ++itr){
        ++n_lines_in_depth[itr->first.n_discs() - 4];
        /*
        for (int &cell: itr->second.line){
            std::cout << idx_to_coord(cell);
        }
        std::cout << std::endl;
        */
    }
    for (int i = 0; i < n_moves + 1; ++i){
        std::cerr << "n_moves " << i << " n_lines " << n_lines_in_depth[i] << std::endl;
    }
    return 0;
}