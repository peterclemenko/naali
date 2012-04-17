/* MessageFilter.h - Class definition of the MessageFilter class.
*  @Author: Kari Vatjus-Anttila
*
*  MessageFilter class is an abstract class from which one cannot create an object.
*  The class has only a pure virtual method filter which is implemented in its child classes
*  MessageFilters are filters that remove unnecessary network traffic. The filtering is done
*  based on different rules. It depends on what kind of algorithm is used.
*/

#pragma once

#include "Entity.h"
#include <string>
#include <iostream>
#include "LoggingFunctions.h"

class MessageFilter
{

public:

    /// Virtual destructor for proper derived/base chain dismantling.
    virtual ~MessageFilter() {  LogInfo("MessageFilter destruction."); }

    /// Pure virtual method filter which is implemented in MessageFilters child classes
    virtual void filter() = 0;

    /// Prints out information
    virtual std::string toString() = 0;
};

