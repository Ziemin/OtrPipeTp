#ifndef PIPE_HPP
#define PIPE_HPP

#include <QObject>
#include <TelepathyQt/Channel>

class Pipe : public QObject {

    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "ziemin.telepathy.Pipe");
    Q_PROPERTY(QString name READ name);
    Q_PROPERTY(QStringList supportedChannelTypes READ supportedChannelTypes);
      
    public:
        Pipe(QString name, QStringList channelTypes);
        virtual ~Pipe();

        QString name() const;
        QStringList supportedChannelTypes() const;

    public Q_SLOTS:
        virtual QString createPipeChannel(QString channelObject) = 0;

    private:
        QString channelName;
        QStringList channelTypes;

};

#endif
