#include "pipe.hpp"

Pipe::Pipe(QString name, QStringList channelTypes) 
    : channelName(name),
    channelTypes(channelTypes) 
{

}

QString Pipe::name() const {
    return channelName;
}

QStringList Pipe::supportedChannelTypes() const {
    return channelTypes;
}
