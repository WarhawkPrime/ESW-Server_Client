/***************************************************************************
 *  openICM-framework
 ***************************************************************************
 *  Copyright 2010 Joachim Wietzke and Manh Tien Tran
 *
 *  Contact and informations:
 *  University of Applied Sciences Darmstadt, Germany
 *  	http://www.fbi.h-da.de/~j.wietzke
 *  or
 *  	http://www.fbi.h-da.de/~openicm
 *
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  	http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
*****************************************************************************/

#ifndef _FWASSERTION_H
#define _FWASSERTION_H

#include <assert.h>

/**
 * Run time assertion handling
 */
#define ASSERTION(cond)   assert(cond)

/**
 * Compile time assertion handling (only enabled for debug builds)
 */
#ifdef DEBUG_MODE
	#define CTASSERTION(x,z) _CTASSERT(x, __LINE__, z)
	#define _CTASSERT(x, y, z) __CTASSERT(x, y, z)
	#define __CTASSERT(x, y, z) typedef char __ct_assert__LINE_ ## y ## __ ## z [(x) ? 1 : -1]
#else
	#define CTASSERTION(x,z)
#endif


#endif // end of _FWASSERTION_H


