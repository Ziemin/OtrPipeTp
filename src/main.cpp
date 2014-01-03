#include <QtCore>
#include <TelepathyQt/Debug>
#include <TelepathyQt/Constants>
#include <TelepathyQt/Types>

#include "otr_pipe.hpp"
#include "pipe_adaptor.h"

int main(int argc, char **argv) {

    QCoreApplication app(argc, argv);

    Tp::registerTypes();
    Tp::enableDebug(true);
    Tp::enableWarnings(true);

    OtrPipe otr_pipe;
    otr_pipe.registerObject();

    return app.exec();
}
