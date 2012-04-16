/* InterestManager.cpp - Implementation of the InterestManager class.
*  @Author: Kari Vatjus-Anttila
*
*  InterestManager class trys to figure out whether a network message
*  should be sent over the network or not. This in turn saves the bandwidth
*  if messages can be sent only to the avatars and objects that are really
*  interested about the data.
*/

#include <algorithm>
#include "InterestManager.h"
#include "LoggingFunctions.h"
#include "Entity.h"

InterestManager* InterestManager::thisPointer = NULL;
bool InterestManager::instanceFlag = false;

InterestManager::InterestManager(IMStatus s) : status(s) {}

InterestManager* InterestManager::getInstance() 
{
    std::cout << "Creating Interest Manager" << std::endl;

    if (!instanceFlag)
    {
        thisPointer = new InterestManager(RUNNING);
        instanceFlag = true;
        return thisPointer;
    }
    else
    {
        return thisPointer;
    }
}

void InterestManager::dropInstance()
{
    std::cout << "Destroying Interest Manager" << std::endl;

    if (instanceFlag)
    {
        instanceFlag = false;
        delete thisPointer;
    }
    else
    {
        return;
    }
}

void InterestManager::Start()                       { status = RUNNING; }

void InterestManager::Stop()                        { status = STOPPED; }

IMStatus InterestManager::getStatus()               { return status; }

void InterestManager::LoadFilter(MessageFilter* m)
{
    messageFilters.push_back(m);
    std::cout << "Loaded " << m->toString() << " filter succesfully." << std::endl;
}

void InterestManager::UnloadFilter(int id)
{
    messageFilters.erase(messageFilters.begin() + id);
    std::cout << "Unloaded filter succesfully." << std::endl;
}

void InterestManager::ListFilters()
{
    std::vector<MessageFilter*>::iterator it;

    for(it = messageFilters.begin(); it != messageFilters.end(); it++)
    {
        (*it)->toString();
    }

}

void InterestManager::filterMessages(QList<Entity*> users, QList<Entity*> messages)
{

}
