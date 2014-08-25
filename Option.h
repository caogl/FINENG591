#ifndef OPTION_H
#define OPTION_H

/* Option.h
 *
 * Representation of ticks of an option
 *
 */

#include "Tick.h"
#include <vector>
#include <string>


using namespace std;

class Option 
{
	public:
		vector<Tick> ticks;
		//start:  ticks[0]
		//end: ticks[ticks.size()-1]
};

#endif