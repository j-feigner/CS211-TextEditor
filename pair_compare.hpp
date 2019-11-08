#pragma once

/* Written by Adam */

#include <iostream>
#include <queue>
#include <string>

using namespace std;

//Used to overload comparison operator in priority queue
class MaxHeapPairComparer
{
public:
	template<typename K, typename V>
	bool operator()(const pair<K, V>& left, const pair <K, V>& right)
	{
		return left.second < right.second;
	}
};