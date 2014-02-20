#ifndef PIPE_OTR_CHANNEL_HPP
#define PIPE_OTR_CHANNEL_HPP

#include <TelepathyQt/BaseChannel>
#include <TelepathyQt/Channel>
#include <TelepathyQt/ChannelInterface>
#include <TelepathyQt/PendingVariant>
#include <TelepathyQt/PendingVariantMap>

using namespace Tp;

class OtrChannel : public Tp::BaseChannel {

    public:
        OtrChannel(const QDBusConnection &dbusCon, BaseConnection *connection, ChannelPtr underChan) 
            : BaseChannel(dbusCon, connection, underChan->channelType(), underChan->targetHandle(), underChan->targetHandleType()),
            underChan(underChan),
            chanIface(QDBusConnection::sessionBus(), underChan->busName(), underChan->objectPath())
        {

            pipedTextIface = underChan->interface<Client::ChannelTypeTextInterface>();
            pipedMesIface = underChan->interface<Client::ChannelInterfaceMessagesInterface>();

            // attaching textType iface
            textType = BaseChannelTextType::create(this);
            textType->setMessageAcknowledgedCallback(Tp::memFun(this, &OtrChannel::messageAcknowledgedCb));
            Tp::PendingVariant *pendingRep = pipedMesIface->requestPropertyPendingMessages();
            connect(pendingRep, &Tp::PendingVariant::finished,
                    this, [this, pendingRep](Tp::PendingOperation* op) {
                        if(op->isValid()) {
                            // get already collected messages
                            QDBusArgument dbusArg = pendingRep->result().value<QDBusArgument>();
                            Tp::MessagePartListList messages;
                            dbusArg >> messages;
                            for(const Tp::MessagePartList &mes: messages) 
                                this->messageReceivedCb(mes);

                            // connect to new ones
                            this->connect(this->pipedMesIface, &Tp::Client::ChannelInterfaceMessagesInterface::MessageReceived,
                                this, &OtrChannel::messageReceivedCb);
                        } else {
                            qDebug() << "Error happened when getting list of pending messages: "
                                    << op->errorName() << " -> " << op->errorMessage();
                        }
                    });

            plugInterface(textType);

            // attaching messages iface
            //BaseChannelMessagesInterface messagesIfacePtr = BaseChannelMessagesInterface::create();
            Tp::PendingVariantMap *pendingMapRep = pipedMesIface->requestAllProperties();
            { // wait for operation to finish
                QEventLoop loop;
                QObject::connect(pendingMapRep, &Tp::PendingOperation::finished,
                        &loop, &QEventLoop::quit);
                loop.exec();
            }
            if(pendingRep->isValid()) {

                QVariantMap resMap = pendingMapRep->result();
                QStringList supportedContentTypes = resMap["SupportedContentTypes"].toStringList();
                Tp::UIntList messageTypes = resMap["MessageTypes"].value<Tp::UIntList>();
                uint supportedFlags = resMap["MessagePartSupportFlags"].toUInt();
                uint deliveryReportingSupport = resMap["DeliveryReportingSupport"].toUInt();

                messagesIface = BaseChannelMessagesInterface::create(
                        textType.data(),
                        supportedContentTypes,
                        messageTypes,
                        supportedFlags,
                        deliveryReportingSupport);

                messagesIface->setSendMessageCallback(Tp::memFun(this, &OtrChannel::sendMessageCb));
                plugInterface(messagesIface);
            } else {
                qWarning() << "Could not get all properties of messages interface to pipe";
            }

            connect(&chanIface, &Client::ChannelInterface::Closed, this, [this]{ emit this->closed(); });
        }

    private:
        void messageAcknowledgedCb(QString token) {

            // copied from proxy channel
            auto it = pendingTokenMap.find(token);
            if(it != pendingTokenMap.end()) {
                Tp::UIntList ids;
                ids.append(*it);
                pendingTokenMap.erase(it); // remove mapping
                pipedTextIface->AcknowledgePendingMessages(ids);
            } else {
                qWarning() << "Cannot acknowledge message with token: " << token;
            }
        }

        void messageReceivedCb(const Tp::MessagePartList &newMessage) {

            // copied from proxy channel
            if(!newMessage.empty()) {
                const Tp::MessagePart &header = newMessage.front();
                auto itToken = header.find(QLatin1String("message-token"));
                auto itId = header.find(QLatin1String("pending-message-id"));
                if(itToken != header.end() && itId != header.end()) {
                    // add new mapping
                    pendingTokenMap[itToken->variant().toString()] = itId->variant().toUInt();
                    textType->addReceivedMessage(newMessage);
                } else {
                    qWarning() << "Received message has no message-token or pending-message-id";
                }
            } else {
                qDebug() << "Received an empty message";
            }
        }

        QString sendMessageCb(const Tp::MessagePartList &messages, uint flags, Tp::DBusError* error) {

            // copied from proxy channel
            QDBusPendingReply<QString> pendingToken = pipedMesIface->SendMessage(messages, flags);
            pendingToken.waitForFinished();
            if(pendingToken.isValid()) {
                return pendingToken.value();
            } else {
                error->set(pendingToken.error().name(), pendingToken.error().message());
                return QString();
            }
        }

    private:

        ChannelPtr underChan;
        Client::ChannelInterface chanIface;
        Client::ChannelTypeTextInterface *pipedTextIface;
        Client::ChannelInterfaceMessagesInterface *pipedMesIface;
        BaseChannelTextTypePtr textType;
        BaseChannelMessagesInterfacePtr messagesIface;

        QMap<QString, uint> pendingTokenMap;
};

#endif
