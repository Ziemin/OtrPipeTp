#include "connections.hpp"

namespace utils {

    /**
     * @return tuple of (accountId, recipientId, protocolId)
     * @throws runtime_exception when failed getting data fo ids
     */
    std::tuple<std::string, std::string, std::string> getConversationIdsFor(ConnectionPtr connection, uint targetHandle) {

        std::tuple<std::string, std::string, std::string> convIds;
        std::get<2>(convIds) = connection->protocolName().toStdString();

        // getting IDs for both sides
        Tp::UIntList handles;
        handles << connection->selfHandle();
        handles << targetHandle;

        Client::ConnectionInterfaceContactsInterface* contactsIface = 
            connection->interface<Client::ConnectionInterfaceContactsInterface>();
        QDBusPendingReply<ContactAttributesMap> pendingRep = contactsIface->GetContactAttributes(handles, QStringList(), false);
        pendingRep.waitForFinished();

        if(pendingRep.isValid()) {
            ContactAttributesMap attrs = pendingRep.value();
            auto it = attrs.find(connection->selfHandle());
            if(it != attrs.end()) {
                std::get<0>(convIds) = it.value()["org.freedesktop.Telepathy.Connection/contact-id"].toString().toStdString();
            } else {
                throw std::runtime_error("No identifier for self handle");
            }
            it = attrs.find(targetHandle);
            if(it != attrs.end()) {
                std::get<1>(convIds) = it.value()["org.freedesktop.Telepathy.Connection/contact-id"].toString().toStdString();
            } else {
                throw std::runtime_error("No identifier for target handle");
            }

            return convIds;
        } else {
            throw std::runtime_error(pendingRep.error().message().toStdString());
        }
    }
}
