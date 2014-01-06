#include <QtCore>
#include <QtDBus>
#include <TelepathyQt/Constants>

#include "otr_pipe.hpp"
#include "pipeadaptor.h"

using namespace Tp;

namespace consts {

    const QString TP_QT_PIPE_NAME_OTR = "Otr";
    const QString TP_QT_PIPE_OTR_SERVICE = "org.freedesktop.Telepathy.Pipe." + TP_QT_PIPE_NAME_OTR;
    const QString TP_QT_PIPE_OTR_OBJECT = "/org/freedesktop/Telepathy/Pipe/" + TP_QT_PIPE_NAME_OTR;
}


OtrPipe::OtrPipe() 
: Pipe(consts::TP_QT_PIPE_NAME_OTR,
        RequestableChannelClassList() << RequestableChannelClassSpec::textChat().bareClass()) 
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

}
