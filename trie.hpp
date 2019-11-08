#pragma once

#include <string>
#include "trie_node.hpp"

using namespace std;

class Trie
{
private:
	TrieNode* root;

public:
	//Default constructor
	Trie()
	{
		root = new TrieNode;
	}

	//Constructs Trie using a vector of strings as data source
	Trie(const vector<string>& words)
	{
		root = new TrieNode;

		for (int i = 0; i < words.size(); i++)
		{
			insert(words[i]);
		}

		return;
	}

	//Inserts string into Trie character by character
	void insert(string word)
	{
		TrieNode* here = root;
		char current = '/0';

		for (int i = 0; i < word.length(); i++)
		{
			current = word[i];
			//If current node does not have a child with the current char: create new child
			if (here->hasChild(current) == false)
			{
				TrieNode* to_add = new TrieNode(current);
				here->setChild(current, to_add);
			}
			here = here->getChild(current);
		}

		//Set last character as end of word
		here->setEndOfWord(true);

		return;
	}

	vector<string> findMatches(string word, TrieNode* root)
	{
		TrieNode* here = root;
		vector<string> matches{};

		for (int i = 0; i < word.length(); i++)
		{

		}

		return matches;
	}

	string search(char to_find, TrieNode* location)
	{
		string word = "";

		TrieNode* here = location;

		return word;
	}
};