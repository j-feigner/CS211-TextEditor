#pragma once

#include <unordered_map>
#include <string>

using namespace std;

class TrieNode
{
private:

	char data;
	unordered_map <char, TrieNode*> children;
	bool end;

public:

	//Default constructor
	TrieNode()
	{
		data = '/0';
	}

	//Constructor
	TrieNode(char value)
	{
		setValue(value);
	}

	//Setter: data
	void setValue(char value)
	{
		data = value;
	}

	//Setter: child
	void setChild(char index, TrieNode* child)
	{
		children[index] = child;
	}

	//Setter: end of word marker
	void setEndOfWord(bool value)
	{
		end = value;
	}

	//Getter: data
	char getValue()
	{
		return data;
	}

	//Getter: child pointer
	TrieNode* getChild(char index)
	{
		return children[index];
	}

	//Getter: end of word marker
	bool getEndOfWord()
	{
		return end;
	}

	//Returns true if current node has a child with the specified value
	bool hasChild(char index)
	{
		if (children.find(index) != children.end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};