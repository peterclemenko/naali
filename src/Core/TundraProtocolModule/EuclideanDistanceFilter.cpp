/* EuclideanDistance.cpp - Implementation of the EuclideanDistance class.
*  @Author: Kari Vatjus-Anttila
*
*  EuclideanDistance is an MessageFilter typed object which main purpose is
*  to filter out unnecessary messages based on Euclidean Distance.
*/

#include "EuclideanDistanceFilter.h"

EuclideanDistanceFilter::EuclideanDistanceFilter() { std::cout << "Euclidean Distance Filter Created" << std::endl; }

EuclideanDistanceFilter::~EuclideanDistanceFilter() { std::cout << "Euclidean Distance Filter Destroyed" << std::endl; }

void EuclideanDistanceFilter::filter()
{
    std::cout << "Filtering Messages" << std::endl;
}

std::string EuclideanDistanceFilter::toString()
{
    return "Euclidean Distance Filter";
}

