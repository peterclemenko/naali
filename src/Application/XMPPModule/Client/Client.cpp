// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Client.h"
#include "XMPPModule.h"
#include "UserItem.h"
#include "Extension.h"

#include "CallExtension.h"
#include "ChatExtension.h"
#include "MucExtension.h"

#include "AudioAPI.h"
#include "LoggingFunctions.h"

#include "qxmpp/QXmppClient.h"
#include "qxmpp/QXmppReconnectionManager.h"
#include "qxmpp/QXmppVCardManager.h"
#include "qxmpp/QXmppRosterManager.h"
#include "qxmpp/QXmppMessage.h"
#include "qxmpp/QXmppUtils.h"

#include "MemoryLeakCheck.h"

namespace XMPP
{
    Client::Client(Framework* framework, QXmppConfiguration &configuration) :
        framework_(framework),
        xmpp_client_(new QXmppClient()),
        log_stream_(false)
    {
        // Bit of a hackish way to store extensions. Feel free to implement better.
        available_extensions_.append(new CallExtension());
        available_extensions_.append(new ChatExtension());
        available_extensions_.append(new MucExtension());

        xmpp_client_->logger()->setLoggingType(QXmppLogger::SignalLogging);

        // -----Client signals-----
        connect(xmpp_client_, SIGNAL(messageReceived(QXmppMessage)), this, SLOT(handleMessageReceived(QXmppMessage)));
        connect(xmpp_client_, SIGNAL(presenceReceived(QXmppPresence)), this, SLOT(handlePresenceReceived(QXmppPresence)));
        connect(xmpp_client_, SIGNAL(connected()), this, SIGNAL(connected()));
        connect(xmpp_client_, SIGNAL(disconnected()), this, SLOT(disconnect()));

        // -----Rostermanager signals-----
        connect(&xmpp_client_->rosterManager(), SIGNAL(rosterReceived()), this, SLOT(handleRosterReceived()));
        connect(&xmpp_client_->rosterManager(), SIGNAL(rosterChanged(QString)), this, SLOT(handleRosterChanged(QString)));
        connect(&xmpp_client_->rosterManager(), SIGNAL(presenceChanged(QString,QString)), this, SLOT(handlePresenceChanged(QString,QString)));

        // -----vCardmanager signals-----
        connect(&xmpp_client_->vCardManager(), SIGNAL(vCardReceived(QXmppVCardIq)), this, SLOT(handleVCardReceived(QXmppVCardIq)));

        // -----Logger signals-----
        connect(QXmppLogger::getLogger(), SIGNAL(message(QXmppLogger::MessageType,QString)), this, SLOT(handleLogMessage(QXmppLogger::MessageType,QString)));

        xmpp_client_->connectToServer(configuration, QXmppPresence::Available);
    }

    Client::~Client()
    {
        disconnect();
        SAFE_DELETE(xmpp_client_);
    }

    void Client::Update(f64 frametime)
    {
        Extension *extension;
        foreach(extension, extensions_)
            extension->Update(frametime);
    }

    bool Client::addExtension(Extension *extension)
    {
        if(extensions_.contains(extension))
        {
            LogError("XMPPModule: Extension already initialized: " + extension->name().toStdString());
            return false;
        }

        extension->setParent(this);
        extension->initialize(this);

        extensions_.append(extension);

        return true;
    }

    QObject* Client ::addExtension(const QString &extensionName)
    {
        Extension *extension = 0;
        LogError("XMPPModule: addExtension called");
	
        for(int i = 0; i < available_extensions_.size(); i++)
        {
            if(available_extensions_[i]->name() == extensionName)
            {
                extension = available_extensions_[i];
                available_extensions_.removeAt(i);
            }
        }

        if(!extension)
        {
            LogError("XMPPModule: No extension found: " + extensionName.toStdString());
            return 0;
        }

        if(!addExtension(extension))
            return 0;

        LogError("XMPPModule: addExtension found " + extensionName.toStdString());
        return dynamic_cast<QObject*>(extension);
    }

    QObject* Client::getExtension(QString extensionName)
    {
      LogError("XMPPModule: getExtension called, #extensions: " + ToString(extensions_.size()));
        for(int i = 0; i < extensions_.size(); i++)
        {
	  if(extensions_[i]->name() == extensionName)
                return dynamic_cast<QObject*>(extensions_[i]);
	  else
            LogError("XMPPModule: Looking for " + extensionName.toStdString() + " but this is " + extensions_[i]->name().toStdString());
        }
        return 0;
    }

    void Client::disconnect()
    {
        //if(xmpp_client_->state() == QXmppClient::ConnectedState) // state() method only available in bleeding edge QXmpp
        xmpp_client_->disconnectFromServer();
        emit disconnected();
    }


    void Client::handleLogMessage(QXmppLogger::MessageType type, const QString &message)
    {
        QString prefix;
        switch(type)
        {
        case QXmppLogger::SentMessage:
            if(!log_stream_)
                return;
            prefix = ">>>";
            break;
        case QXmppLogger::ReceivedMessage:
            if(!log_stream_)
                return;
            prefix = "<<<";
            break;
        case QXmppLogger::WarningMessage:
            prefix = "!!!";
            break;
        default:
            prefix = "---";
            break;
        }

        LogInfo(prefix.toStdString() + " " + message.toStdString());
    }

    QString Client::getHost()
    {
        return xmpp_client_->configuration().host();
    }

    void Client::setStreamLogging(bool state)
    {
        log_stream_ = state;
    }

    QObject* Client::getUser(QString userJid)
    {
        if(users_.contains(userJid))
            return dynamic_cast<QObject*>(users_[userJid]);
        return 0;
    }

    QStringList Client::getRoster()
    {
        return users_.keys();
    }

    void Client::handleRosterReceived()
    {
        LogDebug(getHost().toStdString() + ": Received roster.");
        QStringList roster = xmpp_client_->rosterManager().getRosterBareJids();
        QString roster_user;
        foreach(roster_user, roster)
        {
            if(!users_.contains(roster_user))
            {
                QXmppRosterIq::Item item = xmpp_client_->rosterManager().getRosterEntry(roster_user);
                UserItem *user = new UserItem(item.bareJid());
                users_[roster_user] = user;
                xmpp_client_->vCardManager().requestVCard(roster_user);
            }
        }
        emit rosterChanged();
    }

    void Client::handleRosterChanged(const QString &userJid)
    {
        QXmppRosterIq::Item item = xmpp_client_->rosterManager().getRosterEntry(userJid);
        if(!users_.contains(userJid))
        {
            UserItem *user = new UserItem(item.bareJid());
            users_[userJid] = user;
            /// \todo notify user added (should this logic be moved to a separate function?)
        }
        else
        {
            users_[userJid]->updateRosterItem(item);
        }
        emit rosterChanged();
    }

    void Client::handleMessageReceived(const QXmppMessage &message)
    {
    }

    void Client::handlePresenceReceived(const QXmppPresence &presence)
    {
        // Filter Muc messages coming from room@conference.host.com
        QString from_domain = jidToDomain(presence.from());
        if(from_domain.contains("conference"))
            return;

        QString from_jid = jidToBareJid(presence.from());
        QString from_resource = jidToResource(presence.from());

        if(xmpp_client_->configuration().jidBare() == from_jid)
            return;

        LogDebug("XMPPModule: Received presence (jid=\"" + from_jid.toStdString() + "\", resource=\"" + from_resource.toStdString() + "\")");

        // Some XMPP implementations send presence data before sending roster,
        // create UserItems before receiving roster if this happens.
        if(!users_.contains(from_jid))
        {
            UserItem *user = new UserItem(from_jid);
            users_[from_jid] = user;
        }

        users_[from_jid]->updatePresence(from_resource, presence);
    }

    void Client::handlePresenceChanged(const QString &userJid, const QString &resource)
    {
        if(xmpp_client_->configuration().jidBare() == userJid)
            return;

        if(!users_.contains(userJid))
            return;

        //LogDebug("XMPPModule: Presence changed (user=\"" + userJid.toStdString() + "\", resource=\"" + resource.toStdString() + "\")");

        QMap<QString, QXmppPresence> presences = xmpp_client_->rosterManager().getAllPresencesForBareJid(userJid);
        QXmppPresence& presence = presences[resource];

        if(presence.type() == QXmppPresence::Available)
        {
            if(presence.vCardUpdateType() == QXmppPresence::VCardUpdateNone && !users_[userJid]->hasVCard())
            {
                xmpp_client_->vCardManager().requestVCard(userJid);
            }
        }

        //users_[userJid]->updatePresence(resource, presence);
        //emit presenceChanged(presence.from());
    }

    void Client::handleVCardReceived(const QXmppVCardIq& vcard)
    {
        QString userJid = vcard.from();
        LogDebug(getHost().toStdString() + ": Received vCard from: " + userJid.toStdString());
        if(userJid == xmpp_client_->configuration().jidBare())
            return;

        if(!users_.contains(userJid))
            return;

        users_[userJid]->updateVCard(vcard);
        emit vCardChanged(userJid);
    }

    bool Client::addContact(QString userJid)
    {
        // Check if the given Jid is in proper format
        QRegExp re("^[^@]+@[^@]+$");
        if(userJid.isEmpty() || !re.exactMatch(userJid))
            return false;

        LogDebug(getHost().toStdString() + ": Sending suscribe request to:" + userJid.toStdString());

        QXmppPresence subscribe;
        subscribe.setTo(userJid);
        subscribe.setType(QXmppPresence::Subscribe);

        return xmpp_client_->sendPacket(subscribe);
    }

} // end of namespace: XMPP
