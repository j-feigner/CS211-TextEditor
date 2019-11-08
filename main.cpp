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
#include "trie.hpp"

#define PDC_DLL_BUILD 1

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

WINDOW* setAutoFillWindow(int selection, int y, int x);
//Setter function for auto-fill box.
//Called in auto_fill(), expects user selection and cursor position

void readFileToVector(ifstream& input_file, vector<string>& keywords);
//Reads file contents into an empty vector of strings for use in auto-fill.

void readFileTo2DVector(ifstream& input_file, vector<vector<chtype>>& text);
//Reads file contents into a new, empty vector of vectors of chtypes.
//Each sub-vector should be terminated with a newline, except the final line.

void writeVectorToFile(ofstream& output_file, const vector<vector<chtype>>& text);
//Outputs text vector to a plain text file with edits madde by the user.
//Currently converts chtypes to plain chars so file is human readable for debugging.

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

void auto_fill(queue<char> word_buffer, int y, int x);
//Main function for auto fill subroutine, y and x are draw coordinates (current cursor pos)

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
	//Queue to track current word user is entering for auto-fill functionality
	queue<char> word_buffer{};

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
				writeVectorToFile(output_file, text);
				break;
			
			//AUTOFILL COMMAND
			case CTRL_A:
				//Run auto-fill subroutine
				auto_fill(word_buffer, input_cursor_y, input_cursor_x);

				//Return input window to its previous state
				wclear(input_window);
				outputVector(input_window, render_line_start, render_index_start, text);
				wrefresh(input_window);

				wmove(input_window, input_cursor_y, input_cursor_x);

				break;

			//DEFAULT: insert character		
			default:
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

WINDOW* setAutoFillWindow(int selection, int y, int x)
{
	//Adjust window offset here
	y += 2;

	WINDOW* auto_fill;

	auto_fill = newwin(7, 20, y, x);
	box(auto_fill, 0, 0);

	for (int i = 1; i < 6; i++)
	{
		if (i == selection)
		{
			wattron(auto_fill, WA_REVERSE);
			mvwaddstr(auto_fill, i, 1, "test");
			wattroff(auto_fill, WA_REVERSE);
		}
		else
		{
			mvwaddstr(auto_fill, i, 1, "test");
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

void writeVectorToFile(ofstream& output_file, const vector<vector<chtype>>& text)
{
	//Value to convert chtype to char for human readability.
	//May delete later for preserving chtype attributes
	char chtype_to_char = NULL;

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

void auto_fill(queue<char> word_buffer, int y, int x)
{
	//Open fill box
	WINDOW* auto_fill_window = setAutoFillWindow(1, y, x);
	wrefresh(auto_fill_window);

	int selection = 1;
	int user_input = 0;

	//Loop until newline is entered
	while (user_input != NEWLINE)
	{
		user_input = getch();

		switch(user_input)
		{
			//If not in first position, move selection up
			case KEY_UP:
				if (selection == 1)
				{
					break;
				}
				else
				{
					selection--;
					auto_fill_window = setAutoFillWindow(selection, y, x);
					wrefresh(auto_fill_window);
					break;
				}
			
			//If not in last position, move selection down
			case KEY_DOWN:
				if (selection == 5)
				{
					break;
				}
				else
				{
					selection++;
					auto_fill_window = setAutoFillWindow(selection, y, x);
					wrefresh(auto_fill_window);
					break;
				}
		}
	}

	return;
}