#include <QtCore>
#include <TelepathyQt/Debug>
#include <TelepathyQt/Constants>
#include <TelepathyQt/Types>

int main(int argc, char **argv) {

    QCoreApplication app(argc, argv);

    Tp::registerTypes();
    Tp::enableDebug(true);
    Tp::enableWarnings(true);

    return app.exec();
}
