#pragma once

#include <string>
#include "trie_node.hpp"

using namespace std;

class Trie
{
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

	vector<string> findMatches(string word)
	{
		vector<string> matches{};
		string current_word = "";
		TrieNode* here = root;

		//Moves through tree until last matching character in word is found
		for (int i = 0; i < word.length(); i++)
		{
			//If current node has a matching child node, move to that child
			if (here->hasChild(word[i]))
			{
				here = here->getChild(word[i]);

				//Push character to current_word if it is not the last character in the entered string
				if (word[i] != word.back())
				{
					current_word.push_back(here->getValue());
				}

				//If new location is end of word, push word to matches
				if (here->isEndOfWord())
				{
					matches.push_back(current_word);
				}
			}
			//If current node does not have a matching child node, the trie has no matching words
			else
			{
				//Returns empty vector
				return matches;
			}
		}

		//Moves through the rest of the trie until all matches are found
		findRemaining(here, current_word, matches);

		return matches;
	}

private:
	TrieNode* root;

	//Recursive subroutine within findMatches()
	void findRemaining(TrieNode* here, string current_word, vector<string>& matches)
	{
		//Add current character to word
		current_word.push_back(here->getValue());

		//If the current location is an end of word, push current word as a possible match
		if (here->isEndOfWord())
		{
			matches.push_back(current_word);
		}

		//Duplicate of current node's children hashtable
		unordered_map<char, TrieNode*> children = here->getChildren();
		//Call recursively on all of these children
		for (auto ch : children)
		{
			findRemaining(ch.second, current_word, matches);
		}

		return;
	}
};