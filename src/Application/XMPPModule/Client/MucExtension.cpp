// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MucExtension.h"
#include "Client.h"
#include "XMPPModule.h"

#include "LoggingFunctions.h"
#include "Framework.h"

#include "qxmpp/QXmppMucManager.h"
#include "qxmpp/QXmppUtils.h"
#include "qxmpp/QXmppMessage.h"
#include "qxmpp/QXmppPresence.h"

#include "MemoryLeakCheck.h"

namespace XMPP
{

QString MucExtension::extension_name_ = "Muc";

MucExtension::MucExtension() :
    Extension(extension_name_),
    qxmpp_muc_manager_(new QXmppMucManager())
{
}

MucExtension::~MucExtension()
{
    foreach(QXmppMucRoom *room, rooms_)
    {
        room->leave("Logging off!");
    }
}

void MucExtension::initialize(Client *client)
{
    client_ = client;
    framework_ = client_->getFramework();

    client_->getQxmppClient()->addExtension(qxmpp_muc_manager_);

    bool check = connect(qxmpp_muc_manager_, SIGNAL(invitationReceived(QString,QString,QString)), this, SLOT(handleInvitationReceived(QString,QString,QString)));
    Q_ASSERT(check);
}

void MucExtension::handleMessageReceived(const QXmppMessage &message)
{
    QXmppMucRoom *room = qobject_cast<QXmppMucRoom*>(sender());
    if(!room)
        return;

    LogInfo("XMPPModule: Received message. From: " + message.from().toStdString() + " Body: " + message.body().toStdString());

}

void MucExtension::handleInvitationReceived(const QString &room, const QString &inviter, const QString &reason)
{
    LogDebug(extension_name_.toStdString() + ": Received invitation (room ="
                         + room.toStdString() + " inviter ="
                         + inviter.toStdString() + " reason = "
                         + reason.toStdString() +")");
    emit invitationReceived(room, inviter, reason);
}



/*void MucExtension::handlePresenceReceived(const QXmppPresence &presence)
{
    QString from_domain = jidToDomain(presence.from());
    if(!from_domain.contains("conference"))
        return;

    QString room_jid = jidToBareJid(presence.from());
    QString nickname = jidToResource(presence.from());

    if(rooms_.keys().contains(room_jid))
        rooms_[room_jid]->setNickname(nickname);
    else
        handleRoomAdded(room_jid, nickname);
}*/

void MucExtension::handleParticipantJoined(const QString &jid)
{
    QXmppMucRoom *room = qobject_cast<QXmppMucRoom*>(sender());
    if(!room)
        return;

    emit userJoinedRoom(room->jid(), jid);
}

void MucExtension::handleParticipantLeft(const QString &jid)
{
    QXmppMucRoom *room = qobject_cast<QXmppMucRoom*>(sender());
    if(!room)
        return;

    emit userLeftRoom(room->jid(), jid);
}

void MucExtension::handleRoomJoined()
{
    QXmppMucRoom *room = qobject_cast<QXmppMucRoom*>(sender());
    if(!room)
        return;

    rooms_.append(room);
    emit roomAdded(room->jid(), room->nickName());
}

bool MucExtension::joinRoom(QString roomJid, QString nickname, QString password)
{
    QXmppMucRoom *room = qxmpp_muc_manager_->addRoom(roomJid);
    room->setNickName(nickname);
    room->setPassword(password);

    bool check = connect(room, SIGNAL(joined()), this, SLOT(handleRoomJoined()));
    Q_ASSERT(check);

    room->join();
}

bool MucExtension::leaveRoom(QString roomJid)
{
    QXmppMucRoom *room = getRoom(roomJid);
    if(!room)
        return false;

    return room->leave("Bye!");
}

bool MucExtension::sendMessage(QString roomJid, QString message)
{
    QXmppMucRoom *room = getRoom(roomJid);
    if(!room)
        return false;

    return room->sendMessage(message);
}

bool MucExtension::invite(QString roomJid, QString peerJid, QString reason)
{
    QXmppMucRoom *room = getRoom(roomJid);
    if(!room)
        return false;

    return room->sendInvitation(peerJid, reason);
}

QStringList MucExtension::getParticipants(QString roomJid)
{
    QXmppMucRoom *room = getRoom(roomJid);
    if(!room)
        return QStringList();

    return room->participants();
}

QStringList MucExtension::getRooms() const
{
    QStringList rooms;
    foreach(QXmppMucRoom *room, rooms_)
    {
        rooms.append(room->jid());
    }
    return rooms;
}

QXmppMucRoom* MucExtension::getRoom(const QString &roomJid)
{
    foreach(QXmppMucRoom *room, rooms_)
    {
        if(room->jid() == roomJid)
            return room;
    }
    LogError("XMPPModule: No such room: " + roomJid.toStdString());
    return 0;
}



} // end of namespace: XMPP
