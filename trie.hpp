#pragma once

#include <string>
#include "trie_node.hpp"

using namespace std;

/*
class Trie
{
private:

	TrieNode* root = nullptr;

public:

	Trie()
	{
		root = new TrieNode{};
	}

	void insert(string word)
	{
		TrieNode* here = root;

		for (int i = 0; i < word.length(); i++)
		{
			if(here->getChild(i) != here->getChild)
		}
	}
};
*/

void insert(string word, TrieNode* root)
{
	TrieNode* here = root;
	char current = '/0';

	for (int i = 0; i < word.length(); i++)
	{
		current = word[i];
		//If current node does not have a child with the current char, create one
		if (here->hasChild(current) == false)
		{
			TrieNode* to_add = new TrieNode(current);
			here->setChild(current, to_add);
			here = here->getChild(current);
		}
	}

	return;
}