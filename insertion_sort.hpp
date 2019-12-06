#pragma once

#include "sorter.hpp"

/* Written by Adam Carter */

template <typename T>
class InsertionSort : public Sorter<T>
{
public:
	virtual void sort(vector<T>& data)
	{
		for (int i = 1; i < data.size(); i++)
		{
			for (int j = i; j > 0; j--)
			{
				if (data[j] < data[j - 1])
				{
					T temp = data[j];
					data[j] = data[j - 1];
					data[j - 1] = temp
				}
				else
				{
					break;
				}
			}
		}
	}
};
