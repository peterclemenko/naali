// For conditions of distribution and use, see copyright notice in LICENSE

#include "Registering.h"

#include <pcl/io/pcd_io.h>

#include "LoggingFunctions.h"

namespace ObjectCapture
{

RegisterInterface::RegisterInterface()
{
}

RegisterInterface::~RegisterInterface()
{
    clouds_.clear();
}

void RegisterInterface::addCloud(PointCloud::Ptr cloud)
{
    clouds_.append(cloud);
}

void RegisterInterface::reset()
{
    clouds_.clear();
}

DummyRegister::DummyRegister() :
    RegisterInterface()
{
}

DummyRegister::~DummyRegister()
{
}

PointCloud::Ptr DummyRegister::registerClouds()
{
    LogInfo("ObjectCapture: Registering clouds with method: DummyRegister..");

    PointCloud::Ptr output(new PointCloud);

    if(clouds_.isEmpty())
        return output;

    return clouds_.at(0);
}

}
