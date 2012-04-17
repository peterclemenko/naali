/* EuclideanDistance.cpp - Implementation of the EuclideanDistance class.
*  @Author: Kari Vatjus-Anttila
*
*  EuclideanDistance is an MessageFilter typed object which main purpose is
*  to filter out unnecessary messages based on Euclidean Distance.
*/

#include "EuclideanDistanceFilter.h"
#include "LoggingFunctions.h"

EuclideanDistanceFilter::EuclideanDistanceFilter() { LogInfo("Euclidean Distance Filter Created"); }

EuclideanDistanceFilter::~EuclideanDistanceFilter() {  LogInfo("Euclidean Distance Filter Destroyed"); }

void EuclideanDistanceFilter::filter()
{
     LogInfo("EC: Filtering Messages");
}

std::string EuclideanDistanceFilter::toString()
{
    return "Euclidean Distance";
}


