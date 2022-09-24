﻿#pragma once
#include <iostream>

using namespace std;

#define GRAPH_IGNORE_VALUE INF

struct History_elem {
	Board board;
	int player;
	int v;
	int policy;
	int next_policy;
	string opening_name;

	History_elem() {
		reset();
	}

	void reset() {
		board.reset();
		player = 0;
		v = GRAPH_IGNORE_VALUE;
		policy = -1;
		next_policy = -1;
	}
};
