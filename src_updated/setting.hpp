/** search **/


/* cutting */

// multi prob cut
#define USE_MID_MPC true
#define USE_END_MPC true
#define USE_HUMAN_MPC true

// stability cut
#define USE_MID_SC  false
#define USE_END_SC  true

// transpose table cut
#define USE_MID_TC true
#define USE_END_TC true


/* ordering */

// parity ordering
#define USE_END_PO true



/* multi threading*/

// multi thread
#define USE_MULTI_THREAD false

// parallel early getting
// 0: none 1: use tt contains bug 2: use early getting contains bug
#define MULTI_THREAD_EARLY_GETTING_MODE 0







/** book **/
#define USE_BOOK false








/** other **/

// flip calculating
#define FLIP_CALC_MODE 2

// mobility calculating
#define MOBILITY_CALC_MODE 0

// nodes statistics
#define STATISTICS_MODE true

// MPC calculation mode
#define MPC_MODE false

// creatring evaluation data mode
#define EVAL_MODE false

// book mode
#define BOOK_MODE false

// logging
#define USE_LOG false

// boost library
#define USE_BOOST false

// adjust move ordering
#define MOVE_ORDERING_ADJUST false