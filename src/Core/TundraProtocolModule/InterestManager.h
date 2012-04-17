/* InterestManager.h - Class definition of the InterestManager class.
*  @Author: Kari Vatjus-Anttila
*
*  InterestManager class trys to figure out whether a network message
*  should be sent over the network or not. If not the message is discarded.
*  This saves the network bandwidth if messages can be sent only to the 
*  avatars and objects that are really interested about the data.
*/

#pragma once

#include <vector>
#include <iostream>
#include "Entity.h"
#include "MessageFilter.h"

typedef enum {
    RUNNING = 1,
    STOPPED = 0,
    ERROR = -1
} IMStatus;


class InterestManager
{

public:

    /// Destructor, sets instanceFlag to false so getInstance creates new on request
    ~InterestManager() { instanceFlag = false; }
    
    /// Returns instance of this class
    static InterestManager* getInstance();

    /// Drops the instance of this class
    void dropInstance();

    /// Starts the InterestManager service
    void Start();
    
    /// Stops the InterestManager service
    void Stop();
    
    /// Returns the current status of the service
    IMStatus getStatus();
    
    /// Adds a new filter to the list of messagefilters
    void LoadFilter(MessageFilter *m);
    
    /// Unload a filter from the list of messagefilters
    void UnloadFilter(int id);
    
    /// Print out the current filter configuration
    void ListFilters();
    
    /// The main filtering method where the magic happens
    /// TODO: Change return value from void to the messagebuffer
    void filterMessages(QList<Entity*> users, QList<Entity*> messages);

private:

    /// Private constructor because singleton class pointer is obtained from getInstance()
    InterestManager(IMStatus s);
    
    /// Singleton instance created flag
    static bool instanceFlag;
    
    /// Pointer to store InterestManager class if some class wants it
    static InterestManager *thisPointer;

    /// Stores the status of the object
    /// TODO: Change to enum for example 0 = RUNNING 1 = STOPPED 2 = ERROR or something like that
    IMStatus status;
    
    /// Vector which contains all of the MessageFilters
    std::vector<MessageFilter*> messageFilters;

    /// Couple of lists where the other one contains the list of users inhabiting the environment and the 
    /// other one containing the current messagebuffer which is going to be sent over the network
    QList<Entity*> userList; //List of users inhabiting the scene
    QList<Entity*> messageList;

};
