#ifndef PIPE_OTR_CONNECTION_HPP
#define PIPE_OTR_CONNECTION_HPP

#include <TelepathyQt/BaseConnectionManager>
#include <TelepathyQt/BaseProtocol>
#include <TelepathyQt/BaseConnection>
#include <TelepathyQt/SharedPtr>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ChannelFactory>
#include <TelepathyQt/ContactFactory>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/ConnectionInterfaceContactsInterface>
#include <tuple>

#include "otr_channel.hpp"
#include "otr_encryption.hpp"

using namespace Tp;

namespace consts {

    const QString TP_QT_PIPE_OTR_CONNECTION_MANAGER_NAME = "OtrPipeCM";
    const QString TP_QT_PIPE_OTR_PROTOCOL_NAME = "OtrDoNotUse";
    const QString TP_QT_PIPE_NAME_OTR = "Otr";
    const QString TP_QT_PIPE_OTR_SERVICE = "org.freedesktop.Telepathy.Pipe." + TP_QT_PIPE_NAME_OTR;
    const QString TP_QT_PIPE_OTR_OBJECT = "/org/freedesktop/Telepathy/Pipe/" + TP_QT_PIPE_NAME_OTR;
}

namespace data {

    const RequestableChannelClassList REQUESTABLE_CHANNELS = 
        RequestableChannelClassList() << RequestableChannelClassSpec::textChat().bareClass();
}

namespace utils {

    std::tuple<std::string, std::string, std::string> getConversationIdsFor(ConnectionPtr connection, uint targetHandle);
}

class OtrConnection : public BaseConnection {

    public:
        OtrConnection(const QDBusConnection &dbusConnection, otr::OtrApp &otrApp) 
            : BaseConnection(
                    dbusConnection, 
                    consts::TP_QT_PIPE_OTR_CONNECTION_MANAGER_NAME,
                    consts::TP_QT_PIPE_OTR_PROTOCOL_NAME,
                    QVariantMap()),
            otrApp(otrApp)
        {
            setSelfHandle(1);

            setConnectCallback(memFun(this, &OtrConnection::connectCb));
            setCreateChannelCallback(memFun(this, &OtrConnection::createChannelCb));
            setRequestHandlesCallback(memFun(this, &OtrConnection::requestHandlesCb));
            setInspectHandlesCallback(memFun(this, &OtrConnection::inspectHandlesCb));
        }

    private:
        // as channelType is passed object path to channel to pipe, the rest of the arguments is ignored
        // ugly for now, TODO create different version of pipable channel on dbus not using standard
        // cm/connection/protocol approach
        BaseChannelPtr createChannelCb(
                const QString &channelType, uint /* targetHandleType */, uint targetHandle, Tp::DBusError *error) 
        {
            QString chanObjectPath = channelType;
            uint lastSlash = chanObjectPath.lastIndexOf('/');
            QString conObjectPath = chanObjectPath.left(lastSlash);
            QString conBusName = conObjectPath.right(conObjectPath.length()-1);
            conBusName.replace('/', '.');
            qDebug() << "Creating otr channel for channel at: " << chanObjectPath
                << " from connection: (" << conBusName << ", " << conObjectPath << ")";

            ConnectionPtr pipedCon = Connection::create(
                    QDBusConnection::sessionBus(), 
                    conBusName, 
                    conObjectPath,
                    ChannelFactory::create(QDBusConnection::sessionBus()),
                    ContactFactory::create());

            ChannelPtr underChan = Channel::create(pipedCon, chanObjectPath, QVariantMap());

            if(!underChan || !pipedCon) {
                qWarning() << "Returning empty channel from dummy connection in otr pipe";
                error->set(TP_QT_ERROR_INVALID_ARGUMENT, "Cannot create channel to pipe with otr");
                return BaseChannelPtr();
            }

            PendingReady *pendingReady = underChan->becomeReady(Channel::FeatureCore); 
            {
                QEventLoop loop;
                QObject::connect(pendingReady, &Tp::PendingOperation::finished,
                        &loop, &QEventLoop::quit);
                loop.exec();
            }
            try {
                auto convIds = utils::getConversationIdsFor(pipedCon, targetHandle);
                otr::OtrUserContext userContext = 
                    otrApp.getUserContext(
                        std::get<0>(convIds),
                        std::get<1>(convIds),
                        std::get<2>(convIds));

                return BaseChannelPtr(new OtrChannel(QDBusConnection::sessionBus(), this, underChan, std::move(userContext)));
            } catch(const std::runtime_error &re) { //  TODO special exception
                error->set(TP_QT_ERROR_NOT_AVAILABLE, re.what());
                return BaseChannelPtr();
            }
        }

        // return nonsense handles
        UIntList requestHandlesCb(uint /* handleType */, const QStringList &identifiers, Tp::DBusError * /*error */) {

            UIntList res;
            for(const QString& id: identifiers) res << id.toUInt();
            return res;
        }

        void connectCb(Tp::DBusError * /* error */) { }

        // return nonsense ids
        QStringList inspectHandlesCb(uint /* handleType */, const UIntList &handles, Tp::DBusError * /* error */) {
            QStringList res;
            for(uint handle: handles) res << QString(handle);
            return res;
        }

    private:
        otr::OtrApp &otrApp;
};

class OtrProtocol : public BaseProtocol {

    public:
        OtrProtocol(const QDBusConnection &dbusConnection, otr::OtrApp &otrApp)
            : BaseProtocol(dbusConnection, consts::TP_QT_PIPE_OTR_PROTOCOL_NAME), otrApp(otrApp)
        {

            setEnglishName(QLatin1String(name().toStdString().c_str()));
            setRequestableChannelClasses(data::REQUESTABLE_CHANNELS);
            setCreateConnectionCallback(memFun(this, &OtrProtocol::createConnectionCb));
        }

    private:
        BaseConnectionPtr createConnectionCb(
                const QVariantMap & /* parameters */, Tp::DBusError * /* error */) 
        {
            qDebug() << "Creating connection";
            if(!dumbConnectionPtr) {

                qDebug() << "Building new connection";
                BaseConnectionPtr newCon(new OtrConnection(QDBusConnection::sessionBus(), otrApp));
                dumbConnectionPtr = newCon;
                return newCon;
            } else {

                qDebug() << "Returning old connection";
                return dumbConnectionPtr.toStrongRef();
            }
        }

    private:
        WeakPtr<BaseConnection> dumbConnectionPtr;
        otr::OtrApp &otrApp;
};

class OtrConnectionManager : public BaseConnectionManager {

    public:
        OtrConnectionManager(const QDBusConnection &dbusConnection, otr::OtrApp &otrApp)
            : BaseConnectionManager(dbusConnection, consts::TP_QT_PIPE_OTR_CONNECTION_MANAGER_NAME) 
        {
            otrProtocol = BaseProtocolPtr(new OtrProtocol(QDBusConnection::sessionBus(), otrApp));
            addProtocol(otrProtocol);

            if(!registerObject()) {
                qCritical() << "Could not register pipe connection manager";
                QCoreApplication::exit(1);
            }
            Tp::DBusError error;
            connectionPtr = otrProtocol->createConnection(QVariantMap(), &error);
            if(error.isValid()) {
                qWarning() << "Cannot create connection: " << error.name() << " -> " << error.message();
                return;
            }
            // have to register it myself, since new connection is created from inside this program
            connectionPtr->registerObject(&error);
            if(error.isValid()) {
                qWarning() << "Cannot create connection: " << error.name() << " -> " << error.message();
            }
        }

        SharedPtr<OtrConnection> getConnection() {
            if(!connectionPtr) {
                Tp::DBusError error;
                connectionPtr = otrProtocol->createConnection(QVariantMap(), &error);
                if(error.isValid()) {
                    qWarning() << "Cannot create connection: " << error.name() << " -> " << error.message();
                    return SharedPtr<OtrConnection>();
                }
                // have to register it myself, since new connection is created from inside this program
                connectionPtr->registerObject(&error);
                if(error.isValid()) {
                    qWarning() << "Cannot create connection: " << error.name() << " -> " << error.message();
                    return SharedPtr<OtrConnection>();
                }
            }
            return SharedPtr<OtrConnection>::dynamicCast(connectionPtr);
        }

    private:
        BaseProtocolPtr otrProtocol;
        BaseConnectionPtr connectionPtr;
};


#endif
