// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "CallExtension.h"
#include "Client.h"
#include "XMPPModule.h"
#include "UserItem.h"

#include "LoggingFunctions.h"

#include "qxmpp/QXmppUtils.h"

#include "MemoryLeakCheck.h"

namespace XMPP
{

QString CallExtension::extension_name_ = "Call";

CallExtension::CallExtension() :
    Extension(extension_name_),
    qxmpp_call_manager_(0)
{
}

CallExtension::~CallExtension()
{
    QString call;
    foreach(call, calls_.keys())
    {
        delete calls_[call];
        calls_.remove(call);
    }
}

void CallExtension::initialize(Client *client)
{
    qxmpp_call_manager_ = new QXmppCallManager();

    client_ = client;
    client_->getQxmppClient()->addExtension(qxmpp_call_manager_);
    framework_ = client_->getFramework();

    bool check;
    check = connect(qxmpp_call_manager_, SIGNAL(callReceived(QXmppCall*)), this, SLOT(handleCallReceived(QXmppCall*)));
    Q_ASSERT(check);
}

void CallExtension::Update(f64 frametime)
{
    QString call;
    foreach(call, calls_.keys())
        calls_[call]->Update(frametime);
}

bool CallExtension::acceptCall(QString peerJid)
{
    if(!calls_.keys().contains(peerJid) || calls_[peerJid]->state() != Call::RingingState)
        return false;

    return calls_[peerJid]->accept();
}

// callType is ignored becouse videochannel is not implemented in QXmpp 0.3.0
bool CallExtension::callUser(QString peerJid, QString peerResource, int callType)
{
    if(!client_ || !client_->getUser(peerJid))
        return false;

    //UserItem* user_item = static_cast<UserItem*>(client_->getUser(peerJid));
    //if(!user_item->getCapabilities(peerResource).contains("voice-v1"))
    //    return false;

    QString full_jid = peerJid + "/" + peerResource;

    QXmppCall *qxmpp_call = qxmpp_call_manager_->call(full_jid);

    if(!qxmpp_call)
        return false;

    /// \todo Check if we miss a signal becouse QXmppCall signals are suscribed inside XMPP::Call constructor
    Call *call = new Call(framework_, qxmpp_call);

    bool check = connect(call, SIGNAL(stateChanged(Call::State)), this, SLOT(handleCallStateChanged(Call::State)));
    Q_ASSERT(check);

    calls_.insert(peerJid, call);
    return true;
}

bool CallExtension::callUser(QString peerJid, QString peerResource, QStringList callType)
{
    int flags = 0;

    if(callType.size() == 0)
    {
        flags ^= 1;
    }
    else
    {
        if(callType.contains("Voice", Qt::CaseInsensitive))
            flags ^= 1;
        if(callType.contains("Video", Qt::CaseInsensitive))
            flags ^= 2;
    }

    return callUser(peerJid, peerResource, flags);
}

bool CallExtension::disconnectCall(QString peerJid)
{
    if(!calls_.keys().contains(peerJid))
        return false;

    calls_[peerJid]->hangup();
    return true;
}

bool CallExtension::suspendCall(QString peerJid)
{
    if(!calls_.contains(peerJid))
        return false;

    return calls_[peerJid]->suspend();
}

QString CallExtension::getActiveCall()
{
    QString call;
    foreach(call, calls_.keys())
    {
        if(calls_[call]->state() == Call::ActiveState)
            return calls_[call]->peerJid();
    }
    return "";
}

bool CallExtension::setActiveCall(QString peerJid)
{
    if(!calls_.keys().contains(peerJid))
        return false;

    if(calls_[peerJid]->state() != Call::SuspendedState)
        return false;

    suspendCall(getActiveCall());
    return calls_[peerJid]->resume();
}

void CallExtension::handleCallReceived(QXmppCall *qxmppCall)
{
    QString from_jid = jidToBareJid(qxmppCall->jid());

    LogDebug(extension_name_.toStdString()
                         + "Incoming call (from = \"" + from_jid.toStdString() + "\")");

    Call *call = new Call(framework_, qxmppCall);
    calls_.insert(from_jid, call);

    emit callIncoming(from_jid);
}

void CallExtension::handleCallDisconnected(Call *call)
{
    emit callDisconnected(call->peerJid());
    calls_.remove(call->peerJid());
    delete call;
}

void CallExtension::handleCallStateChanged(Call::State state)
{
    Call *call = qobject_cast<Call*>(sender());
    if(!call)
        return;

    switch(state)
    {
    case Call::RingingState:
    case Call::ConnectingState:
        break;
    case Call::ActiveState:
        emit callActive(call->peerJid());
        break;
    case Call::SuspendedState:
        emit callSuspended(call->peerJid());
        break;
    case Call::DisconnectingState:
        break;
    case Call::FinishedState:
        handleCallDisconnected(call);
        break;
    }
}

} // end of namespace: XMPP
