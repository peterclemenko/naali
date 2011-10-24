/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   MucExtension.h
 *  @brief  Extension for XMPP:Client, provides multi-user chatroom support (XEP-0045).
 */

#ifndef incl_XMPP_MucExtension_h
#define incl_XMPP_MucExtension_h

#include "Extension.h"

#include <QObject>
#include <QString>
#include <QList>

class QXmppMucManager;
class QXmppMucRoom;
class QXmppMessage;
class QXmppPresence;

namespace XMPP
{
class Client;

//! Handles multiuser chat messaging as defined in XEP-0045
class MucExtension : public Extension
{
    Q_OBJECT

public:
    MucExtension();
    virtual ~MucExtension();
    virtual void initialize(Client *client);

public slots:
    //! Join multiuser chatroom on the server
    //! \param room Full JabberID for the room (room@conference.host.com)
    //! \param nickname Nickname to be used in the room
    //! \param password Optional password for the room
    //! \return bool true for succesful join request
    //! \note Succesful join request does not mean the actual join was succesful
    bool joinRoom(QString roomJid, QString nickname, QString password = QString());

    //! Leave muc chatroom
    //! \param room Full JabberID for the room (room@conference.host.com)
    //! \param bool true for room found and left
    bool leaveRoom(QString roomJid);

    //! Send message to muc chatroom
    //! \param room Full JabberID for the room (room@conference.host.com)
    //! \param message Message to sent
    //! \return bool true for succesfully sent message
    bool sendMessage(QString roomJid, QString message);

    //! Get list of currently active rooms
    //! \return QStringList containing full JabberIDs of the rooms
    QStringList getRooms() const;

    //! Get participants for given room
    //! \param room Full JabberID for the room
    //! \return QStringList containing participant nicknames for the room
    QStringList getParticipants(QString roomJid);

    //! Invite user to chatroom
    //! \param room Room the user is invited to
    //! \param peerJid JabberID of the remote user
    //! \param reason Optional reason message
    //! \return bool true for succesful invite
    bool invite(QString roomJid, QString peerJid, QString reason = QString());

private slots:
    void handleMessageReceived(const QXmppMessage &message);
    void handleInvitationReceived(const QString &room, const QString &inviter, const QString &reason);
    void handleParticipantJoined(const QString &jid);
    void handleParticipantLeft(const QString &jid);
    void handleRoomJoined();

private:
    static QString extension_name_;
    QXmppMucManager* qxmpp_muc_manager_;
    //QMap<QString, MucRoom*> rooms_;
    QList<QXmppMucRoom*> rooms_;
    Foundation::Framework *framework_;
    Client *client_;

protected:
    QXmppMucRoom *getRoom(const QString &roomJid);

signals:
    void messageReceived(QString room, QString sender, QString message);
    void invitationReceived(QString room, QString from, QString reason);
    void roomAdded(QString room, QString nickname);
    void roomRemoved(QString room, QString reason);
    void userJoinedRoom(QString room, QString user);
    void userLeftRoom(QString room, QString user);
};

} // end of namespace: XMPP

#endif // incl_XMPP_MucExtension_h
