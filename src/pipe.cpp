#include "pipe.hpp"

using namespace Tp;

Pipe::Pipe(QString name, const Tp::RequestableChannelClassList &reqChans) 
    : channelName(name),
    reqChans(reqChans) 
{

}

QString Pipe::name() const {
    return channelName;
}

RequestableChannelClassList Pipe::RequestableChannelClasses() const {

    return reqChans;
}
