/*
	*** Dynamic Array
	*** src/templates/dynamicarray.h
	Copyright T. Youngs 2012-2020

	This file is part of Dissolve.

	Dissolve is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Dissolve is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Dissolve.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DISSOLVE_DYNAMICARRAY_H
#define DISSOLVE_DYNAMICARRAY_H

#include "templates/list.h"
#include "base/messenger.h"
#include <algorithm>
#include <deque>
#include <new>
#include <vector>
#include <stdint.h>

using namespace std;

// Dynamic Array Class
template <class T> class DynamicArray : std::deque<T>
{
	public:
	using std::deque<T>::back;
	using std::deque<T>::begin;
	using std::deque<T>::clear;
	using std::deque<T>::emplace_back;
	using std::deque<T>::end;
	using std::deque<T>::erase;
	using std::deque<T>::resize;
	using std::deque<T>::size;

	// Initialise array to specified size, creating objects from factory
	void initialise(int nItems)
	{
		resize(nItems);
	}


	/*
	 * Set / Get
	 */
	public:
	// Add single item to end of array
	T* add()
	{
		emplace_back();
		return &(back());
	}
	// Add multiple items to end of array
	void add(int count)
	{
		resize(size()+count);
	}
	// Remove item at specified index and reorder so we still have a contiguous array
	void removeWithReorder(int index)
	{
		erase(begin()+index);
	}
	// Return size of item array
	int nItems() const
	{
		return size();
	}
	// Element access operator
	T* operator[](int index)
	{
#ifdef CHECKS
		if ((index < 0) || (index >= size()))
		{
			Messenger::error("Array index %i is out of bounds (array size = %i).\n", index, array_.nItems());
			return NULL;
		}
#endif
		return &std::deque<T>::operator[](index);
	}
	// Const element access
	const T* constValue(int index) const
	{
		if ((index < 0) || (index >= size()))
		{
			Messenger::error("Array index %i is out of bounds (array size = %i).\n", index, std::deque<T>::size());
			return NULL;
		}

		return &std::deque<T>::operator[](index);
	}
	// Return whether the specified object pointer is in the array
	bool contains(const T* object)
	{
		return end() != std::find_if(begin(), end(), [&object](const T& item){return &item == object;});

	}
	// Return index of the specified object pointer (if it exists in the array)
	bool indexOf(const T* object)
	{
		auto it = std::find_if(begin(), end(), [&object](const T& item){return &item == object;});
		if (it == end()) {return -1;}
		return it - end();
	}
};

// Iterator
template <class T> class DynamicArrayIterator
{
	public:
	// Constructor
	DynamicArrayIterator<T>(DynamicArray<T>& target)
	{
		index_ = 0;
		it_ = target.begin();
		end_ = target.end();
	}

	private:
	// Target DynamicArray
	typename std::deque<T>::iterator it_;
	typename std::deque<T>::iterator end_;
	// Current index for iterator
	int index_;

	public:
	// Iterate
	T* iterate()
	{
		if (it_ == end_) return nullptr;
		++index_;
		++it_;
		return &*it_;
	}
	// Return index of current item
	int currentIndex() const
	{
		// We return index-1, since the index_ is incremented after iterate() is called.
		return index_ - 1;
	}
};

// Const Iterator
template <class T> class DynamicArrayConstIterator
{
	public:
	// Constructor
	DynamicArrayConstIterator<T>(const DynamicArray<T>& target) : arrayTarget_(target)
	{
		index_ = 0;
		result_ = NULL;
	}

	private:
	// Target DynamicArray
	const DynamicArray<T>& arrayTarget_;
	// Current index for iterator
	int index_;
	// Result to return
	const T* result_;

	public:
	// Iterate
	const T* iterate()
	{
		if (index_ < arrayTarget_.nItems())
		{
			result_ = arrayTarget_.constValue(index_);
			++index_;
		}
		else return NULL;
		
		return result_;
	}
};

#endif
