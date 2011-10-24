#ifndef incl_XMPP_UserItem_h
#define incl_XMPP_UserItem_h

#include "Framework.h"

#include <qxmpp/QXmppRosterIq.h>

#include <QObject>
#include <QMap>
#include <QByteArray>

class QXmppVCardIq;
class QXmppPresence;

namespace XMPP
{

//! Represents single entry in user's roster
//!
//!
//! This class is contained within XMPP:Client don't use directly
class UserItem : public QObject
{
    Q_OBJECT

public:
    struct ResourceItem
    {
        bool available;
        QStringList capabilities; // QStringList for script friendliness (scripts don't mix well with enums)
    };

    UserItem(const QString &bareJid);
    ~UserItem();

    void updateRosterItem(const QXmppRosterIq::Item &item);
    void updatePresence(const QString &resource, const QXmppPresence &presence);
    void updateVCard(const QXmppVCardIq &vcard);

public slots:
    //! Get user's Jabber ID
    QString getJid() { return bare_jid_; }

    //! Get all the capabilities for user/resource
    //! \param resource capabilities of the resource, if empty
    //!        all the capabilities the user has will be returned
    //! \return QStringList containing resource/user capabilities,
    //!         empty if resource not found.
    QStringList getCapabilities(QString resource = QString());

    //! Get user's resources
    QStringList getResources();

    //! Get status of the user's vCard
    bool hasVCard() { return has_vcard_; }

    //! Number of clients connected to user's Jabber ID
    int resourceCount() { return resources_.size(); }

    //! Returns users availability status
    //! \return true if atleast one of the user's resources is available
    bool isAvailable() const { return available_; }

    QString getBirthday() { return birthday_; }
    QString getEmail() { return email_; }
    QString getFullName() { return full_name_; }
    QByteArray getPhoto() { return photo_; }
    QString getPhotoType() { return photo_type_; }
    QString getUrl() { return url_; }

private:
    //! Check if any of the user's resources are in available state
    void checkAvailability();

    QMap<QString, ResourceItem> resources_;
    QString bare_jid_;

    QString birthday_;
    QString email_;
    QString full_name_;
    QByteArray photo_;
    QString photo_type_;
    QString url_;

    bool available_;
    bool has_vcard_;

signals:
    void availabilityChanged(bool availability);
};

} // end of namespace: XMPP

#endif // incl_XMPP_UserItem_h
