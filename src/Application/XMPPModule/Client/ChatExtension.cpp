// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ChatExtension.h"
#include "Client.h"
#include "XMPPModule.h"

#include "LoggingFunctions.h"

#include "qxmpp/QXmppMessage.h"
#include "qxmpp/QXmppUtils.h"

#include "MemoryLeakCheck.h"

namespace XMPP
{

QString ChatExtension::extension_name_ = "Chat";

ChatExtension::ChatExtension() :
    Extension(extension_name_)
{
}

ChatExtension::~ChatExtension()
{

}

void ChatExtension::Initialize(Client *client)
{
    client_ = client;

    bool check;
    check = connect(client_->GetQxmppClient(), SIGNAL(messageReceived(QXmppMessage)), this, SLOT(handleMessageReceived(QXmppMessage)));
    Q_ASSERT(check);
}

void ChatExtension::HandleMessageReceived(const QXmppMessage &message)
{   
    if(message.type() == QXmppMessage::GroupChat)
        return;

    QString sender_jid = jidToBareJid(message.from());
    QString msg = message.body();

    LogDebug(extension_name_.toStdString()
                         + "Message (sender = \"" + sender_jid.toStdString()
                         + "\", message =\"" + msg.toStdString() + "\"");

    emit MessageReceived(sender_jid, msg);
}

void ChatExtension::SendMessage(QString receiver, QString message)
{
    if(!client_)
        return;

    client_->GetQxmppClient()->sendMessage(receiver, message);
}

} // end of namespace: XMPP
