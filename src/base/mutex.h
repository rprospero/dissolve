/*
	*** Messaging Routines
	*** src/base/messenger.h
	Copyright T. Youngs 2012-2014

	This file is part of dUQ.

	dUQ is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	dUQ is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with dUQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DUQ_MUTEX_H
#define DUQ_MUTEX_H

// Forward Declarations
class QMutex;

/*
 * Within the GUI, running the simulation occurs in a separate thread on the master process.
 * Here we define a general Mutex class to use in either CLI/GUI code, but which will only
 * perform mutex operations if the GUI is being built.
 */
class dUQMutex
{
	public:
	// Constructor / Destructor
	dUQMutex();
	~dUQMutex();


	/*
	// Mutex Pointerr
	*/
	private:
	QMutex* mutex_;
	
	public:
	// Lock mutex
	void lock();
	// Unlock mutex
	void unlock();
};

#endif
