/* EuclideanDistance.h - Class definition of the EuclideanDistance class.
*  @Author: Kari Vatjus-Anttila
*
*  InterestManager class trys to figure out whether a network message
*  should be sent over the network or not. If not the message is discarded.
*  This saves the network bandwidth if messages can be sent only to the
*  avatars and objects that are really interested about the data.
*/

#pragma once

#include "MessageFilter.h"
#include <string>

class EuclideanDistanceFilter : public MessageFilter
{

public:

    /// The constructor which constructs the EuclideanDistance MessageFilter object
    EuclideanDistanceFilter();

    /// A basic destructor
    ~EuclideanDistanceFilter();

    /// The filtering method where the magic happens. In this class the filtering is done
    /// using the Euclidean Distance algorithm
    void filter();

    /// Prints out information
    std::string toString();
};

