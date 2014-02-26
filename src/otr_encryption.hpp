#ifndef PIPE_OTR_ENCRYPTION_HPP
#define PIPE_OTR_ENCRYPTION_HPP

#include <string>
#include <map>
#include <utility>
#include <memory>
#include <QString>

extern "C" {
#include <libotr/proto.h>
#include <libotr/message.h>
#include <libotr/userstate.h>
}

namespace otr {

    class OtrException : public std::runtime_error {

        public:
            OtrException(const std::string &message);
    };

    class OtrGcryException : public OtrException {

        public:
            OtrGcryException(const std::string &message, gcry_error_t error);

            const gcry_error_t error;
    };

    enum class MessageType { USER, PROTOCOL, NON_OTR };

    class OtrUserContext {

        public:
            OtrUserContext(
                    std::string accountId,
                    std::string recipientId,
                    std::string protocolId,
                    OtrlUserState userState,
                    OtrlMessageAppOps otrAppOps);

            OtrUserContext(const OtrUserContext &other) = default;
            OtrUserContext(OtrUserContext &&other);

            QString encryptMessage(const QString &message);
            std::pair<QString, MessageType>  decryptMessage(const QString &message);

        private:
            const std::string accountId;
            const std::string recipientId;
            const std::string protocolId;
            OtrlUserState userState;
            OtrlMessageAppOps otrAppOps;
    };

    class Account {

        public:
            /**
             * Assumes all given files exists
             */
            Account(std::string accountId,
                    std::string protocolId,
                    const std::string &privKeyFile,
                    const std::string &instagFile,
                    const std::string &fingerPrintsFile);

            ~Account();

            std::string accountId() const;
            std::string protocolId() const;
            OtrlUserState userState() const;
            bool operator==(const Account &other) const;
            bool operator!=(const Account &other) const;

        private:
            const std::string _accountId;
            const std::string _protocolId;
            OtrlUserState _userState;
    };
    
    typedef std::shared_ptr<Account> AccountPtr;

    class OtrApp {

        friend struct OtrAppOpsHandler;

        public:
            explicit OtrApp(std::string otrDir);

            /**
             * @throw OtrException when could get files with specific data for OtrlUserState
             * @throw OtrGcryException when OtrlUserState could not be properly initialized
             */
            OtrUserContext getUserContext(
                    const std::string &accountId, const std::string &recipientId, const std::string &protocolId);

        private:
            std::string getPrivKeyFileFor(const std::string &accountId, const std::string &protocolId) const;
            std::string getInstanceTagsFileFor(const std::string &accountId, const std::string &protocolId) const;
            std::string getFingerprintsFileFOr(const std::string &accountId, const std::string &protocolId) const;

        private:
            const std::string otrDir;
            // simple implementation with priv_key per account
            std::map<std::pair<std::string, std::string>, AccountPtr> accounts;
    };
}

#endif
