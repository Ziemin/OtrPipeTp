#ifndef PIPE_HPP
#define PIPE_HPP

#include <QObject>
#include <QtDBus>
#include <TelepathyQt/Channel>
#include <TelepathyQt/RequestableChannelClassSpecList>

#define TP_QT_IFACE_PIPE "org.freedesktop.Telepathy.Pipe"

using namespace Tp;

class Pipe : public QObject {

    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", TP_QT_IFACE_PIPE);
    Q_PROPERTY(QString name READ name);
    Q_PROPERTY(RequestableChannelClassList RequestableChannelClasses READ RequestableChannelClasses);
      
    public:
        Pipe(QString name, const RequestableChannelClassList &reqChans);
        virtual ~Pipe() = default;

        QString name() const;
        RequestableChannelClassList RequestableChannelClasses() const;

    public Q_SLOTS:
        virtual QDBusObjectPath createPipeChannel(QDBusObjectPath channelObject) = 0;

    private:
        QString channelName;
        RequestableChannelClassList reqChans;
};

#endif
