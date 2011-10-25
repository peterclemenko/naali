/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   CallExtension.h
 *  @brief  Extension for XMPP:Client, provides p2p sip calls (XEP-0166, XEP-0167, XEP-0176)
 */

#ifndef incl_XMPP_CallExtension_h
#define incl_XMPP_CallExtension_h

#include "Extension.h"
#include "Call.h"

#include "qxmpp/QXmppCallManager.h"

#include <QObject>
#include <QString>

class Framework;

namespace XMPP
{
class Client;

//! Provides peer2peer SIP calls. Implements XEP-0166, XEP-0167 & XEP-0176.
class CallExtension : public Extension
{
    Q_OBJECT

public:
    CallExtension();
    virtual ~CallExtension();
    virtual void initialize(Client *client);
    void Update(f64 frametime);

    enum CallTypeFlag { VoiceCall = 1, VideoCall = 2 };

public slots:
    /// Call remote user.
    /// \note Video calls not implemented due to lack of support in QXmpp
    /// \param callType bitfield containing flags defined in CallExtension::CallType enum
    /// \param peerResource Resource the call is connected to (must have voice-v1 capability)
    /// \param peerJid remote party's JabberID
    /// \return bool true on succesful call request
    bool callUser(QString peerJid, QString peerResource, int callType);

    /// Script friendly overload
    /// \note Video calls not implemented due to lack of support in QXmpp
    /// \param peerJid remote party's JabberID
    /// \param peerResource Resource the call is connected to (must have voice-v1 capability)
    /// \param callType QStringList with 1-2 elements:
    ///         "Voice" for voice capability
    ///         "Video" for video capability
    ///         Empty list defaults to voice call
    /// \return bool true on succesful call request
    bool callUser(QString peerJid, QString peerResource, QStringList callType = QStringList());

    //! Accept incoming call
    //! \param peerJid JabberID the call is associated with
    //! \return bool true on call found
    bool acceptCall(QString peerJid);

    //! Disconnect call
    //! \param peerJid JabberID the call is associated with
    //! \return bool true on call found and disconnected
    bool disconnectCall(QString peerJid);

    /// Sets active call on hold
    bool suspendCall(QString peerJid);

    /// Returns list of suspended calls
    /// \note Doesn't contain currently active call
    /// \return QStringList containing JabberIDs associated with the calls
    QStringList getCalls() const;

    //! Returns currently active call
    //! \return QString JabberID associated with the active call, empty for call not found
    QString getActiveCall();

    //! Change active call
    //! \note Only one call can be active at a time,
    //!       previously active call will be suspended
    //! \return true on success
    bool setActiveCall(QString peerJid);

private slots:
    void handleCallReceived(QXmppCall* qxmppCall);
    void handleCallStateChanged(Call::State state);
    void handleCallDisconnected(Call *call);

private:
    static QString extension_name_;
    QXmppCallManager *qxmpp_call_manager_;
    Framework *framework_;
    Client *client_;
    QMap<QString, Call*> calls_;
    QMap<QString, Call*> incoming_calls_;

signals:
    void callSuspended(QString peerJid);
    void callDisconnected(QString peerJid);
    void callIncoming(QString peerJid);
    void callActive(QString peerJid);
};

} // end of namespace: XMPP

#endif // incl_XMPP_CallExtension_h
