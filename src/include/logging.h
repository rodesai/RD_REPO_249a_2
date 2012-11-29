#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>

#define DEBUG 0
#define DEBUG_LOG if(DEBUG) std::cerr << __FILE__ << ":" << __LINE__ << " "

#endif
