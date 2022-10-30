/*
    Egaroucid Project

    @date 2021-2022
    @author Takuto Yamana (a.k.a Nyanyan)
    @license GPL-3.0 license
*/

#include <iostream>
#include "ai.hpp"

inline void init(const int16_t buff[]){
    board_init();
    mobility_init();
    stability_init();
    parent_transpose_table.first_init();
    child_transpose_table.first_init();
    evaluate_init(buff);
    book.init("resources/book.egbk");
}

inline int input_board(Board *bd, const int *arr, const int ai_player){
    int i, j;
    uint64_t b = 0ULL, w = 0ULL;
    int elem;
    int n_stones = 0;
    for (i = 0; i < HW; ++i){
        for (j = 0; j < HW; ++j){
            elem = arr[i * HW + j];
            if (elem != -1){
                b |= (uint64_t)(elem == 0) << (i * HW + j);
                w |= (uint64_t)(elem == 1) << (i * HW + j);
                ++n_stones;
            }
        }
    }
    if (ai_player == 0){
        bd->player = b;
        bd->opponent = w;
    } else{
        bd->player = w;
        bd->opponent = b;
    }
    return n_stones;
}

inline double calc_result_value(int v){
    return (double)v;
}

inline void print_result(int policy, int value){
    cout << policy / HW << " " << policy % HW << " " << calc_result_value(value) << endl;
}

inline void print_result(Search_result result){
    cout << idx_to_coord(result.policy) << " " << calc_result_value(result.value) << endl;
}

inline int output_coord(int policy, int raw_val){
    return 1000 * policy + 100 + raw_val;
}

extern "C" int init_ai(const int16_t buff[]){
    cout << "initializing AI" << endl;
    init(buff);
    cout << "AI iniitialized" << endl;
    return 0;
}

extern "C" int ai_js(int *arr_board, int level, int ai_player){
    cout << "start AI" << endl;
    int i, n_stones, policy;
    Board b;
    Search_result result;
    cout << endl;
    n_stones = input_board(&b, arr_board, ai_player);
    b.print();
    cout << "ply " << n_stones - 3 << endl;
    result = ai(b, level, true, false, true);
    cout << "searched policy " << result.policy << " value " << result.value << " nps " << result.nps << endl;
    int res = output_coord(result.policy, result.value);
    cout << "res " << res << endl;
    return res;
}

extern "C" void calc_value(int *arr_board, int *res, int level, int ai_player){
    int i, n_stones, policy;
    Board b;
    Search_result result;
    n_stones = input_board(&b, arr_board, ai_player ^ 1);
    b.print();
    cout << "ply " << n_stones - 3 << endl;
    int tmp_res[HW2];
    vector<int> moves;
    uint64_t legal = b.get_legal();
    for (i = 0; i < HW2; ++i)
        tmp_res[i] = -1;
    Flip flip;
    uint64_t searched_nodes = 0ULL;
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)){
        calc_flip(&flip, &b, cell);
        b.move_board(&flip);
            tmp_res[cell] = -ai(b, level, true, false, false).value;
        b.undo_board(&flip);
        cerr << idx_to_coord(cell) << " value " << tmp_res[cell] << endl;
    }
    for (i = 0; i < HW2; ++i)
        res[10 + i] = tmp_res[i];
    for (int y = 0; y < HW; ++y){
        for (int x = 0; x < HW; ++x)
            cout << tmp_res[y * HW + x] << " ";
        cout << endl;
    }
}
