#include <QtDBus>
#include "pipe.hpp"

class OtrPipe : public Pipe {

    public:
        OtrPipe();
        void registerObject();

    public Q_SLOTS:
        virtual QDBusObjectPath createPipeChannel(QDBusObjectPath channelObject);

};
