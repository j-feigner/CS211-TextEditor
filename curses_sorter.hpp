/* This file contains all functions related to sorting the text data in my Text Editor. */
/* All algorithmic code was written by Adam Carter and has been repurposed by me.       */

#pragma once

#include <string>
#include <vector>
#include <fstream>

#include "curses.h"
#include "panel.h"
#include "curspriv.h"

#define PDC_DLL_BUILD 1

using namespace std;

void insertionSortDisplay(WINDOW* input_window, vector<string>& words);
//Insertion sort + curses animation

void selectionSortDisplay(WINDOW* input_window, vector<string>& words);
//Selection sort + curses animation

void quickSortDisplay(WINDOW* input_window, vector<string>& words);
//Quick sort + curses animation

void quickSortHelper(WINDOW* input_window, vector<string>& data, int start_index, int end_index);
//Recursive helper function for quick sort

vector<string> grabWords(const vector<vector<chtype>>& text);
//Creates a vector of all words in text data, currently only grabs letter data

void outputWordsToWindow(WINDOW* input_window, const vector<string>& words);
//Helper function for curses animation, outputs vector of words to screen
//Used in each sorting algorithm after each respective iteration to create "animation"

WINDOW* outputSorterSelections(const vector<string>& sorters, int selection);
//Helper function to output sorter selections to mini window on screen.
//Highlights user selection

bool isLetter(char c);
//Helper function to check whether a char is a letter

char chtypeToChar(chtype c);
//Helper function to convert PDCurses chtype to plain character

void insertionSortDisplay(WINDOW* input_window, vector<string>& words)
{
	for (int i = 1; i < words.size(); i++)
	{
		for (int j = i; j > 0; j--)
		{
			if (words[j] < words[j - 1])
			{
				string temp = words[j];
				words[j] = words[j - 1];
				words[j - 1] = temp;
			}
			else
			{
				break;
			}

			//Outputs current sorted state
			outputWordsToWindow(input_window, words);
		}
	}

	return;
}

void selectionSortDisplay(WINDOW* input_window, vector<string>& words)
{
	for (int i = 0; i < words.size(); i++)
	{
		int smallest_index = i;

		for (int j = i + 1; j < words.size(); j++)
		{
			if (words[j] < words[smallest_index])
			{
				smallest_index = j;
			}

			//Outputs current sorted state
			outputWordsToWindow(input_window, words);
		}

		string temp = words[i];
		words[i] = words[smallest_index];
		words[smallest_index] = temp;
	}

	return;
}

void bubbleSortDisplay(WINDOW* input_window, vector<string>& words)
{
	for (int i = 0; i < words.size(); i++)
	{
		bool has_swapped = false;
		for (int j = 1; j < words.size() - i; j++)
		{
			if (words[j - 1] > words[j])
			{
				string temp = words[j - 1];
				words[j - 1] = words[j];
				words[j] = temp;
				has_swapped = true;
			}

			//Outputs current sorted state
			outputWordsToWindow(input_window, words);
		}
		if (has_swapped == false)
		{
			//data is sortd, break out of loop
			break;
		}
	}

	return;
}

void quickSortDisplay(WINDOW* input_window, vector<string>& words)
{
	quickSortHelper(input_window, words, 0, words.size() - 1);

	return;
}

void quickSortHelper(WINDOW * input_window, vector<string> & words, int start_index, int end_index)
{
	//array of size 1 or smaller
	if (end_index <= start_index)
	{
		return;
	}

	//array of size 2
	if (end_index - start_index == 1)
	{
		if (words[end_index] < words[start_index])
		{
			swap(words[end_index], words[start_index]);
		}
		return;
	}

	//must be size 3 or larger
	//find pivot
	string first_item = words[start_index];
	string last_item = words[end_index];
	int mid_index = (start_index + end_index) / 2;
	string middle_item = words[mid_index];
	int pivot_index = start_index;


	if (
		middle_item > first_item && middle_item < last_item //ex: 1 5 10
		||
		middle_item < first_item && middle_item > last_item //ex: 10 5 1
		)
	{
		pivot_index = mid_index;
	}
	else if (
		last_item > first_item && last_item < middle_item //ex: 1 10 5
		||
		last_item < first_item && last_item > middle_item //ex: 10 1 5
		)
	{
		pivot_index = end_index;
	}

	//swap pivot with end index
	string pivot_value = words[pivot_index];
	swap(words[pivot_index], words[end_index]);

	int i = start_index;
	int j = end_index - 1;
	while (i < j)
	{
		while (words[i] < pivot_value && i < j)
		{
			i++;
		}
		while (words[j] >= pivot_value && i < j)
		{
			j--;
		}
		if (i < j)
		{
			swap(words[i], words[j]);
		}
	}

	//swap pivot back
	swap(words[i], words[end_index]);

	//Output current iteration with 10 millisecond delay
	outputWordsToWindow(input_window, words);
	//delay_output(2);

	//recursively repeat
	quickSortHelper(input_window, words, start_index, i - 1);
	quickSortHelper(input_window, words, i + 1, end_index);

	return;
}

vector<string> grabWords(const vector<vector<chtype>> & text)
{
	vector<string> words{};
	string current_word = "";
	char current_ch = '/0';

	for (int i = 0; i < text.size(); i++)
	{
		for (int j = 0; j < text[i].size(); j++)
		{
			current_ch = chtypeToChar(text[i][j]);

			//If character is a letter, push it to word
			if (isLetter(current_ch))
			{
				current_word.push_back(current_ch);
			}
			//Otherwise push word to vector
			else if (!current_word.empty())
			{
				words.push_back(current_word);
				current_word.clear();
			}
		}
	}

	return words;
}

void outputWordsToWindow(WINDOW* input_window, const vector<string>& words)
{
	wclear(input_window);

	for (int i = 0; i < words.size(); i++)
	{
		waddstr(input_window, words[i].c_str());
		waddch(input_window, ' ');
	}

	wrefresh(input_window);

	return;
}

WINDOW* outputSorterSelections(const vector<string>& sorters, int selection)
{
	WINDOW* sort_selection_window = newwin(6, 20, 1, 0);
	box(sort_selection_window, 0, 0);

	for (int i = 0; i < 4; i++)
	{
		//Highlight the user selection
		if (i == selection)
		{
			wattron(sort_selection_window, WA_REVERSE);
			mvwaddstr(sort_selection_window, i + 1, 1, sorters[i].c_str());
			wattroff(sort_selection_window, WA_REVERSE);
		}
		else
		{
			mvwaddstr(sort_selection_window, i + 1, 1, sorters[i].c_str());
		}
	}

	return sort_selection_window;
}

bool isLetter(char c)
{
	if (c >= 'a' && c <= 'z')
	{
		return true;
	}
	else if (c >= 'A' && c <= 'Z')
	{
		return true;
	}
	return false;
}

char chtypeToChar(chtype c)
{
	return c - A_ATTRIBUTES;
}