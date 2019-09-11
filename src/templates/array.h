/*
	*** Array Classes
	*** src/templates/array.h
	Copyright T. Youngs 2012-2019

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

#ifndef DISSOLVE_ARRAY_H
#define DISSOLVE_ARRAY_H

#include <vector>
#include "base/messenger.h"
#include "templates/listitem.h"
#include "templates/vector3.h"
#include <new>

using namespace std;

// Array
template <class A> class Array : public ListItem< Array<A> >
{
	public:
	// Constructors
	Array(int initialSize = 0) : ListItem< Array<A> >()
	{
		array_ = NULL;
		if (initialSize > 0) initialise(initialSize);
	}
        Array(const Array<A>& source, int firstIndex, int lastIndex) : ListItem< Array<A> >()
        {
                array_ = NULL;
                copy(source, firstIndex, lastIndex);
        }
	// Destructor
	~Array()
	{
		if (array_ != NULL) delete[] array_;
	}
	// Copy Constructor
	Array(const Array<A>& source)
	{
		array_ = NULL;
		initialise(source.size_);
		for (int n=0; n<nItems_; ++n) array_[n] = source.array_[n];
	}
	// Assignment Operator
	void operator=(const Array<A>& source)
	{
		clear();
		resize(source.size_);
		for (int n=0; n<nItems_; ++n) array_[n] = source.array_[n];
	}
	// Conversion operator (to standard array)
	operator A*()
	{
		return array_;
	}


	/*
	 * Array Data
	 */
	private:
	// Array data
        vector<A> array_;

	private:
	// Resize array 
	void resize(int newSize)
	{
		// Array large enough already?
	        if (newSize <= array_.capacity()) return;
		try
		{
	        	array_.reserve(newSize);
		}
		catch (length_error& alloc)
		{
			Messenger::error("Array<T>() - Failed to allocate sufficient memory for array_. Exception was : %s\n", alloc.what());
			return;
		}
	}

	public:
	// Return number of items in array
	int nItems() const
	{
	  return array_.size()_;
	}
	// Return current maximum size of array
	int size() const
	{
	  return array_.capacity();
	}
	// Return data array
        Array<A> array()
	{
		return array_;
	}
	// Return const data array
	const A* constArray() const
	{
		return array_;
	}
	// Create array of specified size
	void initialise(int size)
	{
		// First, resize array...
		resize(size);
		
		// ...and finally set all elements to default value
		for (int n=0; n<nItems_; ++n) array_[n] = A();
	}
	// Create empty array of specified size
	void createEmpty(int size)
	{
		clear(); 

		resize(size);
	}
        // Copy data from source array
        void copy(const Array<A>& source, int firstIndex, int lastIndex)
        {
                clear();

                int nItemsToCopy = (lastIndex - firstIndex) + 1;
                if (nItemsToCopy > 0)
                {
                        resize(nItemsToCopy);
                        nItems_ = nItemsToCopy;
                        for (int n=0; n<nItems_; ++n) array_[n] = source.array_[n+firstIndex];
                }
        }


	/*
	 * Add / Remove
	 */
	public:
	// Add new element to array
	void add(A data)
	{
		array_.push_back(data);
	}
	// Insert new element in array
	void insert(A data, const int position)
	{
#ifdef CHECKS
		if ((position < 0) || (position >= nItems_))
		{
			Messenger::print("OUT_OF_RANGE - Position index %i is out of range in Array::insert() (nItems = %i).\n", position, nItems_);
			return;
		}
		array_.insert(array_.begin()+position, data);
#endif
	}
	// Clear array
	void clear()
	{
	        array_.clear();
	}
	// Drop the first item from the array
	void removeFirst()
	{
	        if (array_.empty())
		{
			Messenger::warn("Tried to drop the first item of an empty array...\n");
			return;
		}

		for (int n=0; n<nItems_-1; ++n) array_[n] = array_[n+1];
	}
	// Drop the last item from the array
	void removeLast()
	{
		if (nItems_ == 0)
		{
			Messenger::warn("Tried to drop the last item of an empty array...\n");
			return;
		}
		--nItems_;
	}
	// Remove the specified index
	void remove(const int position)
	{
#ifdef CHECKS
		if ((position < 0) || (position >= nItems_))
		{
			Messenger::print("OUT_OF_RANGE - Array index %i is out of range in Array::remove() (nItems = %i).\n", position, nItems_);
			return;
		}
#endif
		for (int n=position; n<nItems_-1; ++n) array_[n] = array_[n+1];

		--nItems_;
	}

	/*
	 * Set / Get
	 */
	public:
	// Return reference to nth item in array
	A& operator[](int n)
	{
#ifdef CHECKS
		if ((n < 0) || (n >= nItems_))
		{
			static A dummy;
			Messenger::print("OUT_OF_RANGE - Array index %i is out of range in Array::operator[] (nItems = %i).\n", n, nItems_);
			return dummy;
		}
#endif
		return array_[n];
	}
	// Return single value
	A& at(int n)
	{
#ifdef CHECKS
		if ((n < 0) || (n >= nItems_))
		{
			static A dummy;
			Messenger::print("OUT_OF_RANGE - Array index %i is out of range in Array::at() (nItems = %i).\n", n, nItems_);
			return dummy;
		}
#endif
		return array_[n];
	}
	// Return nth item as const reference
	A& constAt(int n) const
	{
#ifdef CHECKS
		if ((n < 0) || (n >= nItems_))
		{
			static A dummy;
			Messenger::print("OUT_OF_RANGE - Array index %i is out of range in Array::constAt() (nItems = %i).\n", n, nItems_);
			return dummy;
		}
#endif
		return array_[n];
	}
	// Return first value in array
	A firstValue() const
	{
	        if (array_.empty())
		{
			Messenger::print("OUT_OF_RANGE - No first item to return in Array.\n");
			return A();
		}
		return array_[0];
	}
	// Return last value in array
	A lastValue() const
	{
	        if (nItems_ == 0)
		{
			Messenger::print("OUT_OF_RANGE - No last item to return in Array.\n");
			return A();
		}
		return array_[nItems_-1];
	}
	// Return first item in array
	A& first()
	{
		if (nItems_ == 0)
		{
			static A dummy;
			Messenger::print("OUT_OF_RANGE - No first item to return in Array.\n");
			return dummy;
		}
		return array_[0];
	}
	// Return last item in array
	A& last()
	{
		if (nItems_ == 0)
		{
			static A dummy;
			Messenger::print("OUT_OF_RANGE - No last item to return in Array.\n");
			return dummy;
		}
		return array_[nItems_-1];
	}


	/*
	 * Move
	 */
	public:
	// Shift item up in the array (towards higher indices)
	void shiftUp(int position)
	{
#ifdef CHECKS
		if ((position < 0) || (position >= nItems_))
		{
			Messenger::print("OUT_OF_RANGE - Array index %i is out of range in Array::shiftUp() (nItems = %i).\n", position, nItems_);
			return;
		}
#endif
		// If this item is already last in the list, return now
		if (position == (nItems_-1)) return;

		A temporary(array_[position+1]);
		array_[position+1] = array_[position];
		array_[position] = temporary;
	}
	// Shift item down in the array (towards lower indices)
	void shiftDown(int position)
	{
#ifdef CHECKS
		if ((position < 0) || (position >= nItems_))
		{
			Messenger::print("OUT_OF_RANGE - Array index %i is out of range in Array::shiftDown() (nItems = %i).\n", position, nItems_);
			return;
		}
#endif
		// If this item is already first in the list, return now
		if (position == 0) return;

		A temporary(array_[position-1]);
		array_[position-1] = array_[position];
		array_[position] = temporary;
	}


	/*
	 * Operators
	 */
	public:
	// Operator= (set all)
	void operator=(const A value) { for (int n=0; n<nItems_; ++n) array_[n] = value; }
	// Operator+= (add to all)
	void operator+=(const A value) { for (int n=0; n<nItems_; ++n) array_[n] += value; }
	void operator+=(const Array<A> array) { for (int n=0; n<nItems_; ++n) array_[n] += array.constAt(n); }
	// Operator-= (subtract from all)
	void operator-=(const A value) { for (int n=0; n<nItems_; ++n) array_[n] -= value; }
	// Operator*= (multiply all)
	void operator*=(const A value) { for (int n=0; n<nItems_; ++n) array_[n] *= value; }
	// Operator/= (divide all)
	void operator/=(const A value) { for (int n=0; n<nItems_; ++n) array_[n] /= value; }
	// Operator- (subtraction)
	Array<A> operator-(const A value) { Array<A> result = *this; result -= value; return result; }
	Array<A> operator-(const Array<A> array) { Array<A> result(nItems_); for (int n=0; n<nItems_; ++n) result[n] = array_[n] - array.constAt(n); return result; }
	// Operator+ (addition)
	Array<A> operator+(const A value) { Array<A> result = *this; result += value; return result; }
	Array<A> operator+(const Array<A> array) { Array<A> result(nItems_); for (int n=0; n<nItems_; ++n) result[n] = array_[n] + array.constAt(n); return result; }
	// Operator* (multiply all and return new)
	Array<A> operator*(const A value) { Array<A> result = *this; result *= value; return result; }
	// Operator/ (divide all and return new)
	Array<A> operator/(const A value) { Array<A> result = *this; result /= value; return result; }


	/*
	 * Functions
	 */
	public:
	// Return sum of elements in array
	A sum()
	{
		A result = 0.0;
		for (int n=0; n<nItems_; ++n) result += array_[n];
		return result;
	}
	// Return maximal absolute value in array
	A maxAbs() const
	{
		A result = 0.0;
		A temp;
		for (int n=0; n<nItems_; ++n)
		{
			temp = array_[n];
			if (temp < 0) temp = -temp;
			if (temp > result) result = temp;
		}
		return result;
	};
};

#endif
