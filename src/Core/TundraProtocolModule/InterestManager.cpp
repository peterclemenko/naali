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
    LogInfo("Creating Interest Manager");

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
    std::vector<MessageFilter*>::iterator it;

    LogInfo("Destroying Interest Manager");

    if (instanceFlag)
    {
        foreach (MessageFilter *m, messageFilters)
            delete m;

        messageFilters.clear();
        instanceFlag = false;
        delete thisPointer;
    }
    else
    {
        return;
    }
}

void InterestManager::Start() { status = RUNNING; }

void InterestManager::Stop() { status = STOPPED; }

IMStatus InterestManager::getStatus() { return status; }

void InterestManager::LoadFilter(MessageFilter* m)
{
    messageFilters.push_back(m);
    LogInfo("Loaded " + m->toString() + " filter succesfully");
}

void InterestManager::UnloadFilter(int id)
{
    messageFilters.erase(messageFilters.begin() + id);
    LogInfo("Unloaded filter succesfully");
}

void InterestManager::ListFilters()
{
    std::vector<MessageFilter*>::iterator it;
    int i = 0;

    for(it = messageFilters.begin(); it != messageFilters.end(); it++)
    {
        LogInfo(i + ": " + (*it)->toString());
        i++;
    }
}

void InterestManager::filterMessages(QList<Entity*> users, QList<Entity*> messages)
{
    std::vector<MessageFilter*>::iterator it;

    switch(status)
    {
        case ERROR:
            LogInfo("Interest Manager encountered an error");
            return;
        break;

        case STOPPED:
            LogInfo("Interest Manager is stopped");
            return;
        break;

        default:
            LogInfo("Filtering...");

            for(it = messageFilters.begin(); it != messageFilters.end(); it++)
            {
                (*it)->filter();
            }

            LogInfo("OK");
        break;
    }

}
