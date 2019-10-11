//This file will be for refactoring my code at a later date

#pragma once

#define PDC_DLL_BUILD 1

#include <vector>
#include "curses.h"

using namespace std;

struct text_struct
{
	vector<vector<chtype>> data;

	int render_line;
	int render_index;

	int edit_line;
	int edit_index;
};