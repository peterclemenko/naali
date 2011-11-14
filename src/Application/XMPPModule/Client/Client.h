/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Client.h
 *  @brief  Provides single client-server XMPP connection.
 */


#ifndef incl_XMPP_Client_h
#define incl_XMPP_Client_h

#include "CoreTypes.h"

#include "qxmpp/QXmppPresence.h"
#include "qxmpp/QXmppLogger.h"

#include <QObject>
#include <QThread>

class QXmppClient;
class QXmppMessage;
class QXmppVCardIq;
class QXmppConfiguration;
class QXmppPresence;

class Framework;

namespace XMPP
{
class Extension;
class UserItem;
//! Represents a single connection to a XMPP Server
//!
//! Provides interface for sending and receiveing messages
//! and voice calls. Also keeps track of users' presence
//! status' and capabilities
//!
//! Uses QXmpp library for connection handling
class Client : public QObject
{
    Q_OBJECT
public:
    Client(Framework* framework);
    ~Client();

    void Update(f64 frametime);

    Framework *getFramework() { return framework_; }
    QXmppClient *getQxmppClient() { return xmpp_client_; }

    //! Get extension pointer
    //! \return pointer for the extensions if found, void otherwise
    template<typename T>
    T* getExtension()
    {
        for (int i = 0; i < extensions_.size(); ++i)
        {
            T* extension = qobject_cast<T*>(extensions_[i]);
            if(extension)
                return extension;
        }
        return 0;
    }

public slots:

    //! Add and initialize extension for the client
    //! \param extension Pointer for object inherited from XMPP:Extension
    //! \return bool true for succesfully initialized extension
    bool addExtension(Extension *extension);

    //! Script friendly overload for adding extensions
    //! \param extensionName name of the extension
    //! \return QObject pointer for the extension, if succesfully initialized.
    //!         NULL for failure.
    QObject *addExtension(const QString &extensionName);

    //! Get extension as a QObject pointer
    //! \return QObject pointer to the object if found, null otherwise
    QObject *getExtension(QString extensionName);

    //! Set own presence availability
    //! \param QString
    void setAvailability(QString availability);

    //! Connect to a XMPP Server, script friendly overload
    //! \param userJid User's Jabber ID
    //! \param userPassword User's password
    //! \param xmppServer XMPP Server (host:port)
    void connectToServer(const QString &xmppServer, const QString &userJid, const QString &userPassword);

    //! Get host associated with this connection
    //! \return QString current host (host)
    QString getHost();

    //! Get UserItem
    //! \param userJid Jabber ID for the user
    QObject* getUser(QString userJid);   

    //! Get current roster
    //! \return QStringList containing known Jabber ID's
    QStringList getRoster();

    //! Disconnect from server
    //! \note Requesting a disconnect destroys this Client object
    void disconnect();

    //! Add contact to roster
    //! \param userJid Jabber ID to be added to roster
    //! \return bool describing if the request was succesfully sent
    bool addContact(QString userJid);

    //! Set logging of the XML stream on/off
    //! \param state Boolean for on/off
    void setStreamLogging(bool state);

private slots:
    void handlePresenceChanged(const QString& userJid, const QString& resource);
    void handlePresenceReceived(const QXmppPresence& presence);
    void handleSetPresence(QXmppPresence::Type presenceType);
    void handleMessageReceived(const QXmppMessage &message);
    void handleRosterReceived();
    void handleRosterChanged(const QString& userJid);
    void handleVCardReceived(const QXmppVCardIq& vcard);
    void handleLogMessage(QXmppLogger::MessageType type, const QString& message);

private:
    QXmppClient *xmpp_client_;
    QList<Extension*> extensions_;
    QXmppConfiguration *current_configuration_;
    QMap<QString, UserItem*> users_;
    Framework* framework_;
    bool log_stream_;

    //! Connect to a XMPP Server using QXmppConfiguration
    //! \param configuration Configuration containing connection details
    void ConnectToServer(const QXmppConfiguration& configuration);

signals:
    //! Signals changes in current roster
    void rosterChanged();

    //! Signals changes in users presence
    //! Can indicate that capabilities were received
    void presenceChanged(QString UserJid);

    //! Signals changes in user's vCard
    void vCardChanged(QString UserJid);

    //! Signals disconnect by request
    void disconnected();

    //! Signals connected status
    //! \note This signal gets emitted when the underlying QXmppClient signals connected state
    void connected();
};

} // end of namespace: XMPP

#endif // incl_XMPP_Client_h
