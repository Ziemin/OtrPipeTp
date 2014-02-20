#include <QtCore>
#include <QtDBus>
#include <TelepathyQt/Constants>

#include "otr_pipe.hpp"
#include "pipeadaptor.h"

using namespace Tp;


OtrPipe::OtrPipe() 
: 
    Pipe(consts::TP_QT_PIPE_NAME_OTR, data::REQUESTABLE_CHANNELS),
    cm(QDBusConnection::sessionBus())
{
    new PipeAdaptor(this);
}

void OtrPipe::registerObject() {

    QDBusConnection dbusConnection = QDBusConnection::sessionBus();
    if(!dbusConnection.registerService(consts::TP_QT_PIPE_OTR_SERVICE) ||
            !dbusConnection.registerObject(consts::TP_QT_PIPE_OTR_OBJECT, this)) {

        qFatal("Otr Pipe could not be registered");
        QCoreApplication::exit(1);
    }
}

QDBusObjectPath OtrPipe::createPipeChannel(QDBusObjectPath channelObject) {

    SharedPtr<OtrConnection> connection = cm.getConnection();
    Tp::DBusError error;
    BaseChannelPtr chanPtr = connection->createChannel(channelObject.path(), 0, 0, 0, false, &error);

    if(chanPtr) return QDBusObjectPath(chanPtr->objectPath());
    else return QDBusObjectPath();
}
