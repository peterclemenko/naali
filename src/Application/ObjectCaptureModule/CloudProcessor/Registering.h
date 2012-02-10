// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "ObjectCaptureModuleDefines.h"

#include <QObject>
#include <QList>
#include <QString>

namespace ObjectCapture
{

class RegisterInterface : public QObject
{
    Q_OBJECT

public:
    RegisterInterface();
    virtual ~RegisterInterface();

    void addCloud(PointCloud::Ptr cloud);

    void reset();

    virtual PointCloud::Ptr registerClouds() = 0;

protected:
    QList<PointCloud::Ptr> clouds_;

};

class DummyRegister : public RegisterInterface
{
    Q_OBJECT

public:
    DummyRegister();
    ~DummyRegister();

    virtual PointCloud::Ptr registerClouds();
};

}
