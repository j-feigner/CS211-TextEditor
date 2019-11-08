#ifdef _WIN32
//Windows includes
#include "curses.h"
#include "panel.h"
#include "curspriv.h"
#else
//Linux / MacOS includes
#include <curses.h>
#endif

#include <vector>
#include <string>
#include <fstream>
#include <queue>
#include <bitset>
#include <cmath>
#include <stack>
#include "trie.hpp"

#define PDC_DLL_BUILD 1

#define SPACE       32
#define BACKSPACE   8
#define CTRL_E      5
#define CTRL_S      19
#define CTRL_A      1
#define NEWLINE     10

using namespace std;

WINDOW* setBorders(char* file_name);
//Setter function for borders window.
//Expects command line argument for file name.

WINDOW* setInputWindow();
//Setter function for input window.

WINDOW* setAutoFillWindow(vector<string> matches, int selection, int y, int x);
//Setter function for auto-fill box.
//Called in auto_fill(), expects user selection and cursor position

void readFileToVector(ifstream& input_file, vector<string>& keywords);
//Reads file contents into an empty vector of strings for use in auto-fill.

void readFileTo2DVector(ifstream& input_file, vector<vector<chtype>>& text);
//Reads file contents into a new, empty vector of vectors of chtypes.
//Each sub-vector should be terminated with a newline, except the final line.

void writeOut(ofstream& output_file, const vector<vector<chtype>>& text);
//Outputs text vector to a plain text file.

void writeOutCoded(ofstream& output_file, const vector<vector<chtype>>& text, unordered_map<string, string> word_codes);
//Outputs text vector using unordered map of word codes to text file.

void outputVector(WINDOW* window, int line_num, int line_index, const vector<vector<chtype>>& text);
//Outputs current vector contents to the input window from the coordiantes (line_num, line_index).
//These coordinates do NOT have to corespond to an actual location in the vector, they are an imaginary starting point.
//Input window needs to be refreshed OUTSIDE this function.

void lineTracker(WINDOW* borders_window, int total_lines, int current_line);
//Generates proper values for current line position in file. 
//Currently requires border window refresh OUTSIDE of function.

void insertCharacterIntoLine(vector<vector<chtype>>& text, chtype to_insert, int line_num, int index);
//Inserts a non-newline character into the current line based on user edit location.

void deleteCharacterFromLine(vector<vector<chtype>>& text, int line_num, int index);
//Deletes a non-newline character from the current line based on user edit location.

void insertNewlineIntoVector(vector<vector<chtype>>& text, int line_num, int index);
//Inserts a newline terminated vector into the 2d text vector based on user edit location.
//Will carry over any characters that were ahead of the entered newline to the new line.

void deleteNewlineFromVector(vector<vector<chtype>>& text, int line_num, int index);
//Empty

string auto_fill(Trie source, string word_buffer, int y, int x);
//Main function for auto fill subroutine, y and x are draw coordinates (current cursor pos)

string intToBinaryString(int x);

unordered_map <string, int> createFreqDist(const vector<vector<chtype>>& text);
//Fills a hashtable with the frequencies of strings in text data

unordered_map <string, string> assignValues(unordered_map<string, int> frequency_distribution);
//Converts a frequency distribution to an unordered map with ascending values for most common strings


int main(int argc, char* argv[])
{
	//Initialize screen, begin curses mode, stdscr settings
	initscr();
	noecho();
	keypad(stdscr, TRUE);
	refresh();

	//File streams
	ifstream input_file{ argv[1] };
	ifstream keyword_file{ "cpp_keywords.txt" };
	ofstream output_file{ "test_output.txt" };
	ofstream coded_output_file{ "test_output.compressed.txt" };
	
	//Initialize and fill keywords vector and trie
	vector<string> cpp_keywords{};
	readFileToVector(keyword_file, cpp_keywords);
	Trie cpp_trie(cpp_keywords);

	//Initialize and fill main text vector
	vector<vector<chtype>> text{};
	readFileTo2DVector(input_file, text);

	//Set windows
	WINDOW* borders_window = setBorders(argv[1]);
	WINDOW* input_window = setInputWindow();

	//Output vector into input window
	outputVector(input_window, 0, 0, text);

	//Draw all windows
	wrefresh(borders_window);
	wrefresh(input_window);

	/* Input Variables */
	//Grabs input char from user
	int user_input = 0;
	//Tracks location of user cursor IN WINDOW
	int input_cursor_y = 0;
	int input_cursor_x = 0;
	//Tracks location of user input location IN VECTOR (i, j)
	int current_line_num = 0;
	int current_line_index = 0;
	//These variables track the starting coordinate of the output
	int render_line_start = 0;
	int render_index_start = 0;
	//String to track current word user is entering for auto-fill functionality
	string word_buffer = "";
	//String returned from auto-fill function to be inserted into window
	string word_to_insert = "";
	//Hashtable for storing frequency distribution of words
	unordered_map<string, int> freq_dist = createFreqDist(text);
	//Hashtable for storing word codes
	unordered_map<string, string> word_codes = assignValues(freq_dist);

	/* INPUT LOOP */
	wmove(input_window, 0, 0);
	while (user_input != CTRL_E)
	{
		//Window resizing
		if (is_termresized())
		{
			resize_term(0, 0);

			borders_window = setBorders(argv[1]);
			wrefresh(borders_window);

			input_window = setInputWindow();
			outputVector(input_window, render_line_start, render_index_start, text);
			wrefresh(input_window);
			wmove(input_window, input_cursor_y, input_cursor_x);
		}
		//Grab char from user
		user_input = wgetch(input_window);
		//Grab cursor position in window
		getyx(input_window, input_cursor_y, input_cursor_x);

		switch (user_input)
		{
			case KEY_UP:
				//If cursor is in the first line of vector, do nothing
				if (current_line_num == 0)
				{
					break;
				}
				//TEMPORARY CHECK: If line above is smaller than current line, do nothing
				else if (text[current_line_num - 1].size() <= current_line_index)
				{
					break;
				}
				//If cursor is not in the first line and is at the top of the window,
				//scroll up
				else if (input_cursor_y == 0)
				{
					render_line_start--;                                                     //FOR ALL ARROW KEYS: refactor these lines to a scroll function later
					current_line_num--;                                                      //--
					outputVector(input_window, render_line_start, render_index_start, text); //--
					wmove(input_window, input_cursor_y, input_cursor_x);                     //--

					wrefresh(input_window);
					break;
				}
				//If cursor is not at the first line and is not at the top of the window,
				//move cursor up by one line
				else
				{
					input_cursor_y--;                                    //FOR ALL ARROW KEYS: refactor these lines to a move function later
					current_line_num--;                                  //--
					wmove(input_window, input_cursor_y, input_cursor_x); //--

					break;
				}

			case KEY_DOWN:
				//If cursor is in the last line of vector, do nothing
				if (current_line_num == text.size() - 1)
				{
					break;
				}
				//NOT WORKING PROPERLY
				//TODO: move cursor approriately in this case
				else if (text[current_line_num + 1].size() <= current_line_index)
				{
					break;
				}
				//If cursor is not in the last line and is at the bottom of the window,
				//scroll down by one line
				else if (input_cursor_y == input_window->_maxy - 1)
				{
					render_line_start++;
					current_line_num++;
					outputVector(input_window, render_line_start, render_index_start, text);
					wmove(input_window, input_cursor_y, input_cursor_x);
					wrefresh(input_window);
					break;
				}
				//If cursor is not in the last line, and is not at the bottom of the window,
				//move cursor down one line
				else
				{
					input_cursor_y++;
					current_line_num++;
					wmove(input_window, input_cursor_y, input_cursor_x);
					break;
				}

			case KEY_LEFT:
				//If cursor is in the first position of the line, do nothing.
				if (current_line_index == 0)
				{
					break;
				}
				//If cursor is not in the first position of the line,
				//and is at the left edge of the window,
				//scroll left
				else if (input_cursor_x == 0)
				{
					render_index_start--;
					current_line_index--;
					outputVector(input_window, render_line_start, render_index_start, text);
					wmove(input_window, input_cursor_y, input_cursor_x);
					wrefresh(input_window);
					break;
				}
				//If cursor is not in the first position of the line,
				//and is not at the left edge of the window,
				//move cursor to the left by one character
				else
				{
					input_cursor_x--;
					current_line_index--;
					wmove(input_window, input_cursor_y, input_cursor_x);
					break;
				}

			case KEY_RIGHT:
				//If cursor is in the last position of the line, not including newline terminator,
				//do nothing.
				if (current_line_index == text[current_line_num].size() - 1)
				{
					break;
				}
				//If cursor is not in the last position of the line,
				//and is at the right edge of the window,
				//scroll right
				else if (input_cursor_x == input_window->_maxx - 1)
				{
					render_index_start++;
					current_line_index++;
					outputVector(input_window, render_line_start, render_index_start, text);
					wmove(input_window, input_cursor_y, input_cursor_x);
					wrefresh(input_window);
					break;
				}
				//If cursor is not in the last position of the line,
				//and is not at the right edge of the window,
				//move cursor to the right by one character
				else
				{
					input_cursor_x++;
					current_line_index++;
					wmove(input_window, input_cursor_y, input_cursor_x);
					break;
				}

			case BACKSPACE:
				//Delete last char from buffer if not empty
				if (!word_buffer.empty())
				{
					word_buffer.pop_back();
				}

				//If cursor is at the beginning of the line, delete line
				if (current_line_index == 0)
				{
					//TODO: Add delete line functionality
					break;
				}
				//If cursor is not at the beginning of the line,
				//and the cursor is at the left of the window,
				//delete character and scroll left
				else if (input_cursor_x == 0)
				{
					deleteCharacterFromLine(text, current_line_num, current_line_index);

					//Scroll left
					render_index_start--;
					current_line_index--;
					outputVector(input_window, render_line_start, render_index_start, text);
					wmove(input_window, input_cursor_y, input_cursor_x);
				}
				//If cursor is not at the beginning of the line, delete character
				else
				{
					//Delete character and output vector
					deleteCharacterFromLine(text, current_line_num, current_line_index);
					outputVector(input_window, render_line_start, render_index_start, text);

					//Move cursor to the left
					input_cursor_x--;
					current_line_index--;
					wmove(input_window, input_cursor_y, input_cursor_x);
				}
				break;

			case NEWLINE:
				//Clear buffer (new word)
				word_buffer.clear();

				//TODO: add scroll down functionality if in last line of window
				//Insert and output
				insertNewlineIntoVector(text, current_line_num, current_line_index);
				render_index_start = 0;
				outputVector(input_window, render_line_start, render_index_start, text);

				//Move cursor appropriately
				input_cursor_y++;
				current_line_num++;
				input_cursor_x = 0;
				current_line_index = 0;
				wmove(input_window, input_cursor_y, input_cursor_x);
				break;

			//SAVE COMMAND
			case CTRL_S:
				writeOut(output_file, text);
				writeOutCoded(coded_output_file, text, word_codes);
				break;
			
			//AUTOFILL COMMAND
			case CTRL_A:
				//Run auto-fill subroutine and grab user selection
				word_to_insert = auto_fill(cpp_trie, word_buffer, input_cursor_y, input_cursor_x);

				//If auto_fill() returned a word, insert selection
				if (!word_to_insert.empty())
				{
					wclear(input_window);

					//Insert remaining characters from user selection one at a time
					for (int i = 0; i < word_to_insert.length(); i++)
					{
						insertCharacterIntoLine(text, word_to_insert[i], current_line_num, current_line_index);
						outputVector(input_window, render_line_start, render_index_start, text);

						//Move right by one
						input_cursor_x++;
						current_line_index++;
						wmove(input_window, input_cursor_y, input_cursor_x);
					}
				}

				break;

			//DEFAULT: insert character		
			default:
				//TEMPORARY: if user_input is a space, empty buffer for new word
				if (user_input == SPACE)
				{
					word_buffer.clear();
				}
				//Otherwise add character to word buffer
				else
				{
					word_buffer.push_back(user_input);
				}

				//If cursor is at the right of the window,
				//insert character to line and scroll right
				if (input_cursor_x == input_window->_maxx - 1)
				{
					insertCharacterIntoLine(text, user_input, current_line_num, current_line_index);

					//Scroll right
					render_index_start++;
					current_line_index++;
					outputVector(input_window, render_line_start, render_index_start, text);
					wmove(input_window, input_cursor_y, input_cursor_x);

					wrefresh(input_window);
					break;
				}
				//If curse is not at the right of the window,
				//insert character and move cursor to the right.
				else
				{
					insertCharacterIntoLine(text, user_input, current_line_num, current_line_index);
					outputVector(input_window, render_line_start, render_index_start, text);

					//Move right
					input_cursor_x++;
					current_line_index++;
					wmove(input_window, input_cursor_y, input_cursor_x);

					break;
				}
		}
		//Update and draw line counter
		lineTracker(borders_window, text.size(), current_line_num + 1);
		wrefresh(borders_window);
	}

	//End curses
	endwin();

	return 0;
}

WINDOW* setBorders(char* file_name)
{
	WINDOW* borders;

	//Creates new window
	borders = newwin(LINES, COLS, 0, 0);

	//Sets text to highlighted
	wattron(borders, WA_REVERSE);

	//Fills window with highlighted white space
	for (int i = 0; i < COLS; i++)
	{
		for (int j = 0; j < LINES; j++)
		{
			waddch(borders, ' ');
		}
	}

	//Status bar strings
	mvwaddstr(borders, 0, 0, "Jordan Feigner's 211 Text Editor");
	mvwaddstr(borders, 0, COLS / 2, file_name);

	//Command Bar Strings
	mvwaddstr(borders, LINES - 2, 0, "^S Save \t ^C Copy \t ^V Paste");
	mvwaddstr(borders, LINES - 2, COLS - 8, "Line#");

	mvwaddstr(borders, LINES - 1, 0, "^E Exit \t ^O Open \t ^A Fill");
	mvwaddch(borders, LINES - 1, COLS - 6, '/');

	return borders;
}

WINDOW* setInputWindow()
{
	WINDOW* input;
	input = newwin(LINES - 3, COLS, 1, 0);
	keypad(input, TRUE);
	return input;
}

WINDOW* setAutoFillWindow(vector<string> matches, int selection, int y, int x)
{
	//Adjust window offset here
	y += 2;

	WINDOW* auto_fill;

	auto_fill = newwin(7, 20, y, x);
	box(auto_fill, 0, 0);

	//Output the first five matches
	for (int i = 0; i < 5 && i < matches.size(); i++)
	{
		//Highlight the user selection
		if (i == selection)
		{
			wattron(auto_fill, WA_REVERSE);
			mvwaddstr(auto_fill, i + 1, 1, matches[i].c_str());
			wattroff(auto_fill, WA_REVERSE);
		}
		else
		{
			mvwaddstr(auto_fill, i + 1, 1, matches[i].c_str());
		}
	}

	return auto_fill;
}

void readFileToVector(ifstream& input_file, vector<string>& keywords)
{
	string current = "";

	if (input_file.is_open())
	{
		for (int i = 0; input_file.good(); i++)
		{
			getline(input_file, current);
			keywords.push_back(current);
		}
	}

	return;
}

void readFileTo2DVector(ifstream& input_file, vector<vector<chtype>>& text)
{
	vector<chtype> line{};
	char current = NULL;

	if (input_file.is_open())
	{
		while (input_file.good())
		{
			//Grab char from txt file
			current = input_file.get();
			//If char is newline, clear current line and push to main vector
			if (current == '\n')
			{
				line.push_back(current);
				text.push_back(line);
				line.clear();
			}
			//If any other non-EOF char, push to current line
			else if (current != EOF)
			{
				line.push_back(current);
			}
		}
		//Push last line of file
		text.push_back(line);
	}
	return;
}

void writeOut(ofstream& output_file, const vector<vector<chtype>>& text)
{
	char chtype_to_char = NULL;

	//Loop through text and output one character at a time.
	if (output_file.good())
	{
		for (int i = 0; i < text.size(); i++)
		{
			for (int j = 0; j < text[i].size(); j++)
			{
				                 //raw        //conversion factor
				chtype_to_char = text[i][j] - A_ATTRIBUTES;
				output_file << chtype_to_char;
			}
		}
	}
	return;
}

void writeOutCoded(ofstream& output_file, const vector<vector<chtype>>& text, unordered_map<string, string> word_codes)
{
	char current_ch = NULL;
	string current_word = "";

	if (output_file.good())
	{
		//Loop through text file, find words, output corresponding code
		for (int i = 0; i < text.size(); i++)
		{
			for (int j = 0; j < text[i].size(); j++)
			{
                                 //raw        //conversion factor
				current_ch = text[i][j] - A_ATTRIBUTES;

				//If character is a lowercase letter, push it to word
				if (current_ch >= 'a' && current_ch <= 'z')
				{
					current_word.push_back(current_ch);
				}
				//Output current word
				else if (!current_word.empty())
				{
					output_file << word_codes[current_word] << " ";
					current_word.clear();
				}
			}
		}
	}
	return;
}

void outputVector(WINDOW* window, int line_num, int line_index, const vector<vector<chtype>>& text)
{
	wmove(window, 0, 0);
	int output_cursor_y = 0;
	int output_cursor_x = 0;
	//Bounds check for number of lines in window and number of lines in vector
	for (int i = line_num; i < text.size() && output_cursor_y < window->_maxy; i++)
	{
		//Bounds check for number of columns in window and size of current line
		for (int j = line_index; j < text[i].size() && output_cursor_x < window->_maxx; j++)
		{
			mvwaddch(window, output_cursor_y, output_cursor_x, text[i][j]);
			output_cursor_x++;
		}
		output_cursor_x = 0;
		output_cursor_y++;
	}
	//Prevents last line from retaining any leftover character data
	wclrtoeol(window);
	return;
}

void lineTracker(WINDOW* borders_window, int total_lines, int current_line)
{
	string line_total = to_string(total_lines);
	string current_line_num = to_string(current_line);

	const char* line_total_str = line_total.c_str();
	const char* current_line_str = current_line_num.c_str();

	mvwaddstr(borders_window, LINES - 1, COLS - 9, current_line_str);
	mvwaddstr(borders_window, LINES - 1, COLS - 3, line_total_str);

	return;
}

void insertCharacterIntoLine(vector<vector<chtype>>& text, chtype to_insert, int line_num, int index)
{
	const vector<chtype>::iterator here = text[line_num].begin() + index;
	text[line_num].insert(here, to_insert);
	return;
}

void deleteCharacterFromLine(vector<vector<chtype>>& text, int line_num, int index)
{
	const vector<chtype>::iterator here = text[line_num].begin() + index - 1;
	text[line_num].erase(here);
	return;
}

void insertNewlineIntoVector(vector<vector<chtype>>& text, int line_num, int index)
{
	const vector<vector<chtype>>::iterator insert_location = text.begin() + line_num + 1;

	const vector<chtype>::iterator start = text[line_num].begin() + index;
	const vector<chtype>::iterator end = text[line_num].end() - 1;

	//Copy chars past newline into new vector and add newline terminator
	vector<chtype> past_newline(start, end);
	past_newline.push_back('\n');
	//Erase all chars in original line
	text[line_num].erase(start, end);
	//Insert new line into vector
	text.insert(insert_location, past_newline);

	return;
}

void deleteNewlineFromVector(vector<vector<chtype>>& text, int line_num, int index)
{


	return;
}

string auto_fill(Trie source, string word_buffer, int y, int x)
{
	vector<string> matches = source.findMatches(word_buffer);

	//If not matches found, exit function
	if (matches.empty())
	{
		return "";
	}

	int selection = 0;
	int user_input = 0;

	WINDOW* auto_fill_window = setAutoFillWindow(matches, selection, y, x);
	wrefresh(auto_fill_window);

	//Loop until newline is entered
	while (user_input != NEWLINE)
	{
		user_input = getch();

		switch(user_input)
		{
			//If not in first position, move selection up
			case KEY_UP:
				if (selection == 0)
				{
					break;
				}
				else
				{
					selection--;
					auto_fill_window = setAutoFillWindow(matches, selection, y, x);
					wrefresh(auto_fill_window);
					break;
				}
			
			//If not in last position, move selection down
			case KEY_DOWN:
				if (selection == 4 || selection == matches.size())
				{
					break;
				}
				else
				{
					selection++;
					auto_fill_window = setAutoFillWindow(matches, selection, y, x);
					wrefresh(auto_fill_window);
					break;
				}

			default:
				break;
		}
	}

	//Grab user selected string
	string to_insert = matches[selection];

	//Erase characters already entered by user
	to_insert.erase(0, word_buffer.length());

	return to_insert;
}

string intToBinaryString(int x)
{
	//Base Case: most common word will return 0
	if (x == 0)
	{
		return "0";
	}

	stack<char> rem_stack{};
	int rem = 0;
	char binary_char = NULL;

	//Convert integer to shortest possible binary string
	while (x > 0)
	{
		rem = x % 2;
		x /= 2;
		binary_char = '0' + rem;
		rem_stack.push(binary_char);
	}

	string binary_string = "";

	while (!rem_stack.empty())
	{
		binary_string.push_back(rem_stack.top());
		rem_stack.pop();
	}

	return binary_string;
}

unordered_map <string, int> createFreqDist(const vector<vector<chtype>>& text)
{
	char current_ch = '/0';
	string current_word = "";

	unordered_map<string, int> FD{};

	for (int i = 0; i < text.size(); i++)
	{
		for (int j = 0; j < text[i].size(); j++)
		{
			//Convert chtype to plain char
			current_ch = text[i][j] - A_ATTRIBUTES;

			//If character is a lowercase letter, push it to word
			if (current_ch >= 'a' && current_ch <= 'z')
			{
				current_word.push_back(current_ch);
			}
			//Otherwise increment frequency of current word (if non-empty)
			else if(!current_word.empty())
			{
				FD[current_word]++;
				current_word.clear();
			}
		}
	}

	return FD;
}

unordered_map <string, string> assignValues(unordered_map<string, int> freq_dist)
{
	unordered_map<string, string> word_codes{};
	string s = "";

	//Finds the minimum number of binary digits to represent all words
	const int bit_field_size = log2(freq_dist.size());

	//Creating priority queue from unordered map
	priority_queue<pair<string, int>> words_q{};
	for (auto i : freq_dist)
	{
		words_q.push(i);
	}

	int num_words = words_q.size();

	//Create a new unordered map with corresponding unique int values
	for (int i = 0; i < num_words; i++)
	{
		//Converts i to a binary string of size 8
		s = intToBinaryString(i);
		word_codes[words_q.top().first] = s;
		words_q.pop();
	}

	return word_codes;
}