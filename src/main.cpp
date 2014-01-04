#include <QtCore>
#include <QtDBus>
#include <TelepathyQt/Debug>
#include <TelepathyQt/Constants>
#include <TelepathyQt/Types>

#include "otr_pipe.hpp"
#include "pipeadaptor.h"

void registerOtrTypes() {
    typedef Tp::RequestableChannelClassList RequestableChannelClassList;
    qRegisterMetaType<RequestableChannelClassList>("RequestableChannelClassList");
    qDBusRegisterMetaType<RequestableChannelClassList>();
}

int main(int argc, char **argv) {

    QCoreApplication app(argc, argv);

    Tp::registerTypes();
    Tp::enableDebug(true);
    Tp::enableWarnings(true);

    registerOtrTypes();

    OtrPipe otr_pipe;
    otr_pipe.registerObject();

    return app.exec();
}
