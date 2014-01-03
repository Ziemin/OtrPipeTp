#ifndef PIPE_HPP
#define PIPE_HPP

#include <QObject>
#include <QtDBus>
#include <TelepathyQt/Channel>

#define TP_QT_IFACE_PIPE "org.freedesktop.Telepathy.Pipe"

class Pipe : public QObject {

    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", TP_QT_IFACE_PIPE);
    Q_PROPERTY(QString name READ name);
    Q_PROPERTY(QStringList supportedChannelTypes READ supportedChannelTypes);
      
    public:
        Pipe(QString name, QStringList channelTypes);
        virtual ~Pipe() = default;

        QString name() const;
        QStringList supportedChannelTypes() const;

    public Q_SLOTS:
        virtual QDBusObjectPath createPipeChannel(QDBusObjectPath channelObject) = 0;

    private:
        QString channelName;
        QStringList channelTypes;

};

#endif
