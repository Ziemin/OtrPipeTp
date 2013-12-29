#ifndef PIPE_HPP
#define PIPE_HPP

#include <QObject>

class Pipe : public QObject {

    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "ziemin.telepathy.Pipe")

};

#endif
