#pragma once
#include <iostream>

/* Debug flag. Comment in or out to produce debug messages on std::cout.
 * Bewar: Debug messages are a lot. On big instances, this will result in
 * gigabytes of debug messages! */
//#define DEBUG_F

#ifdef DEBUG_F
bool DEBUG_ENABLED = true;
#define TOGGLE_DEBUG()\
/**/DEBUG_ENABLED = !DEBUG_ENABLED;
#define DEBUG(PRINT)\
/**/if(DEBUG_ENABLED) {\
/**/	std::cout << PRINT;\
/**/	std::flush(std::cout);\
/**/}
#else
bool DEBUG_ENABLED = false;
#define TOGGLE_DEBUG()
#define DEBUG(print)
#endif