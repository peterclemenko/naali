// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Call.h"
#include "XMPPModule.h"

#include "Framework.h"

#include "qxmpp/QXmppRtpChannel.h"
#include "qxmpp/QXmppJingleIq.h"
#include "qxmpp/QXmppUtils.h"

#include "MemoryLeakCheck.h"

namespace XMPP
{
Call::Call(Foundation::Framework *framework, QXmppCall *call) :
    framework_(framework),
    call_(call)
{
    if(call->direction() == QXmppCall::IncomingDirection)
        state_ = Call::RingingState;
    else if(call->direction() == QXmppCall::OutgoingDirection)
        state_ = Call::ConnectingState;

    peer_jid_ = jidToBareJid(call->jid());

    bool check;
    check = connect(call_, SIGNAL(stateChanged(QXmppCall::State)), this, SLOT(handleCallStateChanged(QXmppCall::State)));
    Q_ASSERT(check);

    check = connect(call_, SIGNAL(connected()), this, SLOT(handleCallConnected()));
    Q_ASSERT(check);

    check = connect(call_, SIGNAL(finished()), this, SLOT(handleCallTerminated()));
    Q_ASSERT(check);
}

Call::~Call()
{
    XMPPModule::LogDebug("Destroyed call object, with jid: " + peer_jid_.toStdString());
    if(audio_channel_)
    {
        framework_->Audio()->Stop(audio_channel_);
        audio_channel_.reset();
    }
    delete call_;
}

bool Call::accept()
{
    if(state_ != Call::RingingState)
        return false;

    call_->accept();
    return true;
}

void Call::hangup()
{
    call_->hangup();
    setState(Call::FinishedState);
}

bool Call::suspend()
{
    if(state_ == Call::FinishedState || state_ == Call::SuspendedState)
        return false;

    // QXmpp has no method of actually putting calls on hold like XEP-0167 defines
    // We'll just change the internal state of our call and discard received data.
    // We need to craft the messages ourselves unless this changes.

    setState(Call::SuspendedState);
    return true;
}

bool Call::resume()
{
    if(state_ != Call::SuspendedState)
        return false;

    setState(Call::ActiveState);
    return true;
}

void Call::Update(f64 frametime)
{
    UNREFERENCED_PARAM(frametime);
    handleOutboundVoice();
}

void Call::handleCallTerminated()
{
    setState(Call::FinishedState);
}

void Call::handleCallConnected()
{
    if(!call_)
        return;

    Q_ASSERT(framework_);

    if(!framework_->Audio())
    {
        XMPPModule::LogError("Tundra sound API not initialized, cannot initialize voice call.");
        handleCallTerminated();
    }

    QXmppRtpAudioChannel *channel = call_->audioChannel();

    bool stereo; /// \todo change this to global property of the call
    if(channel->payloadType().channels() == 2)
        stereo = true;
    else
        stereo = false;

    /// \todo change to something proper and define as a class member
    int buffer_size = 16/8*channel->payloadType().clockrate()*200/1000;

    framework_->Audio()->StartRecording("", channel->payloadType().clockrate(), true, stereo, buffer_size);

    bool ok = QObject::connect(channel, SIGNAL(readyRead()), this, SLOT(handleInboundVoice()));
    Q_ASSERT(ok);
}

void Call::handleInboundVoice()
{
    if(state_ == Call::DisconnectingState || state_ == Call::FinishedState)
        return;

    SoundBuffer buffer;
    QXmppRtpAudioChannel *channel = call_->audioChannel();
    QByteArray data = channel->read(channel->bytesAvailable());

    // For now, just discard the data when on hold (wastes downlink, proper implementation pending)
    if(state_ == Call::SuspendedState)
        return;

    buffer.data.resize(data.size());
    memcpy(&buffer.data[0], data.data(), data.size());

    buffer.frequency = channel->payloadType().clockrate();
    buffer.is16Bit = true;
    if(channel->payloadType().channels() == 2)
        buffer.stereo = true;
    else
        buffer.stereo = false;

    if(!audio_channel_)
        audio_channel_ = framework_->Audio()->PlaySoundBuffer(buffer, SoundChannel::Voice);
    else
        framework_->Audio()->PlaySoundBuffer(buffer, SoundChannel::Voice, audio_channel_);
}

void Call::handleOutboundVoice()
{
    if(state_ != Call::ActiveState)
        return;

    QXmppRtpAudioChannel *channel = call_->audioChannel();
    QByteArray buffer;

    int buffer_size = (channel->payloadType().clockrate() * channel->payloadType().channels() * (16 / 8) * 160) / 1000;

    while (framework_->Audio()->GetRecordedSoundSize() > buffer_size)
    {
        buffer.resize(buffer_size);
        char *data = buffer.data();
        int bytes = framework_->Audio()->GetRecordedSoundData(data, buffer_size);
        buffer.resize(bytes);

        call_->audioChannel()->write(buffer);
    }
}


void Call::handleCallStateChanged(QXmppCall::State state)
{
    switch(state)
    {
    case QXmppCall::ConnectingState:
        setState(Call::ConnectingState);
        break;
    case QXmppCall::ActiveState:
        setState(Call::ActiveState);
        break;
    case QXmppCall::DisconnectingState:
        setState(Call::DisconnectingState);
        break;
    case QXmppCall::FinishedState:
        setState(Call::FinishedState);
        break;
    default:
        break;
    }
}

void Call::setState(Call::State state)
{
    state_ = state;
    QString state_string;

    switch(state_)
    {
    case Call::RingingState:
        state_string = "ringing";
        break;
    case Call::ConnectingState:
        state_string = "connecting";
        break;
    case Call::ActiveState:
        state_string = "active";
        break;
    case Call::SuspendedState:
        state_string = "suspended";
        break;
    case Call::DisconnectingState:
        state_string = "disconnecting";
        break;
    case Call::FinishedState:
        state_string = "finished";
        break;
    default:
        state_string = "unknown";
        break;
    }

    XMPPModule::LogInfo("Call with \"" + peer_jid_.toStdString() + "\" " + state_string.toStdString());
    emit stateChanged(state_);
}

} // end of namespace: XMPP
