#ifndef incl_XMPP_Call_h
#define incl_XMPP_Call_h

#include "qxmpp/QXmppCallManager.h"

#include "AudioAPI.h"

#include <QObject>

class Framework;

namespace XMPP
{

//! Presents single p2p audio call. Basicly acts as a wrapper for QXmppCall.
//! Passes audio through Tundra AudioAPI.
class Call : public QObject
{
        Q_OBJECT
public:
    enum State { RingingState = 0, ConnectingState, ActiveState, SuspendedState, DisconnectingState, FinishedState };

    Call(Framework *framework, QXmppCall *call);
    ~Call();

    void Update(f64 frametime);

public slots:
    //! Accept call
    //! bool true on success
    bool accept();

    //! Disconnect current call
    //! \return bool true for success
    void hangup();

    //! Puts the call in suspended state
    //! \return bool if call was succesfully suspended
    bool suspend();

    //! Continues call from suspended state
    //! \return bool if call was succesfully continued
    bool resume();

    //! Get the Jabber ID for the other party of the conversation
    //! \return QString containing callees/callers Jabber ID
    QString peerJid() const { return peer_jid_; }

    //! Returns state of the call
    State state() const { return state_; };


private slots:
    void handleCallTerminated();
    void handleCallConnected();
    void handleCallStateChanged(QXmppCall::State state);
    void handleInboundVoice();
    void handleOutboundVoice();

private:
    void setState(Call::State state);

    Framework *framework_;
    SoundChannelPtr audio_channel_;
    QString peer_jid_;
    QXmppCall *call_;
    State state_;

signals:
    void stateChanged(Call::State state);
};

} // end of namespace: XMPP

#endif // incl_XMPP_Call_h
