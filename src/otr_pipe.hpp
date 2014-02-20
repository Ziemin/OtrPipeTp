#include <QtDBus>
#include "pipe.hpp"
#include "connections.hpp"

class OtrPipe : public Pipe {

    public:
        OtrPipe();
        void registerObject();

    public Q_SLOTS:
        virtual QDBusObjectPath createPipeChannel(QDBusObjectPath channelObject);

    private:
        OtrConnectionManager cm;

};
