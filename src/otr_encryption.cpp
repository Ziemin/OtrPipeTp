#include "otr_encryption.hpp"

#include <functional>
extern "C" {
#include <libotr/privkey.h>
#include <libotr/instag.h>
#include <libotr/proto.h>
#include <gpg-error.h>
}

namespace otr {

    struct OtrAppOpsHandler {

        static OtrlMessageAppOps otrAppOps;
        static OtrApp *otrApp;

        // OtrlMessageAppOps related
        static void create_privkey(void * /* opdata */, const char * /* accountname */, const char * /* protocol */) {

        }

        static void new_fingerprint(void * /* opdata */, OtrlUserState  /* us */,
                const char * /* accountname */, const char * /* protocol */,
                const char * /* username */, unsigned char * /* fingerprint[20] */) 
        {

        }

        static void write_fingerprints(void * /* opdata */) {

        }

        static const char *account_name(void * /* opdata */, const char * /* account */, const char * /* protocol */) {

            return "";
        }

        static void account_name_free(void * /* opdata */, const char * /* account_name */) {

        }

        static void create_instag(void * /* opdata */, const char * /* accountname */, const char * /* protocol */) {

        }
    };

    OtrlMessageAppOps OtrAppOpsHandler::otrAppOps = []{

        OtrlMessageAppOps appOps;

        appOps.create_privkey = &OtrAppOpsHandler::create_privkey;
        appOps.new_fingerprint = &OtrAppOpsHandler::new_fingerprint;
        appOps.write_fingerprints = &OtrAppOpsHandler::write_fingerprints;
        appOps.account_name = &OtrAppOpsHandler::account_name;
        appOps.account_name_free = &OtrAppOpsHandler::account_name_free;
        appOps.create_instag = &OtrAppOpsHandler::create_instag;

        return appOps;
    }();

    OtrApp *OtrAppOpsHandler::otrApp = nullptr;

    // --------- Otr Exceptions ----------------------------------------------------------------------------
    OtrException::OtrException(const std::string &message) : std::runtime_error(message)
    {
    }
            
    OtrGcryException::OtrGcryException(const std::string &message, gcry_error_t error) 
        : OtrException(message + " : " + std::string(gpg_strerror(error))), 
        error(error)
    {

    }

    // --------- OtrUserContext -----------------------------------------------------------------------------

    OtrUserContext::OtrUserContext(
            std::string accountId,
            std::string recipientId,
            std::string protocolId,
            OtrlUserState userState,
            OtrlMessageAppOps otrAppOps)
        : 
            accountId(std::move(accountId)),
            recipientId(std::move(recipientId)),
            protocolId(std::move(protocolId)),
            userState(userState),
            otrAppOps(otrAppOps)
    { 
    }

    OtrUserContext::OtrUserContext(OtrUserContext &&other) 
        : accountId(std::move(other.accountId)),
        recipientId(std::move(other.recipientId)),
        protocolId(std::move(other.protocolId)),
        userState(other.userState)
    {
        other.userState = nullptr;
    }

    void OtrUserContext::setOpdata(void *opdata) {
        this->opdata = opdata;
    }

    QString OtrUserContext::encryptMessage(const QString &message) {

        char *newMessage = nullptr;
        std::string stdMessage = message.toStdString();

        // opdata is NULL - meanwhile I don't know what it is
        gcry_error_t error = otrl_message_sending(
                userState, &otrAppOps, opdata, accountId.c_str(), protocolId.c_str(), 
                recipientId.c_str(), OTRL_INSTAG_BEST, stdMessage.c_str(), 
                nullptr, &newMessage, OtrlFragmentPolicy::OTRL_FRAGMENT_SEND_SKIP, nullptr, nullptr, nullptr);

        if(error) {
            otrl_message_free(newMessage);
            throw OtrGcryException("Cannot encrypt message", error);
        }

        QString result(newMessage);
        otrl_message_free(newMessage);
        return result;
    }

    std::pair<QString, MessageType>  OtrUserContext::decryptMessage(const QString &message) {

        char *newMessage = nullptr;
        std::string stdMessage = message.toStdString();

        int recResult = otrl_message_receiving(
                userState, &otrAppOps, opdata, accountId.c_str(), protocolId.c_str(), 
                recipientId.c_str(), stdMessage.c_str(), 
                &newMessage, nullptr, nullptr, nullptr, nullptr);

        MessageType mesType;
        QString resultMessage;
        if(recResult) { // internal message
            mesType = MessageType::PROTOCOL;
            resultMessage = message;
        } else if(newMessage != nullptr) {
            mesType = MessageType::USER;
            resultMessage = QString(newMessage);
        } else {
            mesType = MessageType::NON_OTR;
            resultMessage = message;
        }

        std::pair<QString, MessageType> result = std::make_pair(resultMessage, mesType);
        otrl_message_free(newMessage);
        return result;
    }

    // --------- Account ---------------------------------------------------------------------------------

    Account::Account(std::string accountId,
                    std::string protocolId,
                    const std::string &privKeyFile,
                    const std::string &instagFile,
                    const std::string &fingerprintsFile) 
        : _accountId(std::move(accountId)),
        _protocolId(std::move(protocolId))
    {
        _userState = otrl_userstate_create();
        gcry_error_t error = otrl_privkey_read(_userState, privKeyFile.c_str());
        if(error) {
            otrl_userstate_free(_userState);
            throw OtrGcryException("Cannot read private key to user state", error);
        }
        error = otrl_instag_read(_userState, instagFile.c_str());
        if(error) {
            otrl_userstate_free(_userState);
            throw OtrGcryException("Cannot read instance tags to user state", error);
        }
        error = otrl_privkey_read_fingerprints(_userState, fingerprintsFile.c_str(), NULL, NULL);
        if(error) {
            otrl_userstate_free(_userState);
            throw OtrGcryException("Cannot read fingerprints to user state", error);
        }
    }

    Account::~Account() {
        if(_userState != nullptr) {
            otrl_userstate_free(_userState);
        }
    }

    bool Account::operator==(const Account &other) const {
        return _accountId == other._accountId && _protocolId == other._protocolId;
    }

    bool Account::operator!=(const Account &other) const {
        return !(*this == other);
    }
    std::string Account::accountId() const {
        return _accountId;
    }

    std::string Account::protocolId() const {
        return _protocolId;
    }

    OtrlUserState Account::userState() const {
        return _userState;
    }

    // --------- OtrApp ----------------------------------------------------------------------------------
    
    OtrApp::OtrApp(std::string otrDir) : otrDir(std::move(otrDir))
    {
    }

    OtrUserContext OtrApp::getUserContext(
            const std::string &accountId, const std::string &recipientId, const std::string &protocolId) 
    {
        OtrAppOpsHandler::otrApp = this;

        auto acIt = accounts.find(std::make_pair(accountId, recipientId));
        if(acIt != end(accounts)) {
            return { accountId, recipientId, protocolId, acIt->second->userState(),  OtrAppOpsHandler::otrAppOps };
        } else {
            std::string privKeyFile = getPrivKeyFileFor(accountId, protocolId);
            std::string instanceTagsFile = getInstanceTagsFileFor(accountId, protocolId);
            std::string fingerprintsFile = getFingerprintsFileFOr(accountId, protocolId);

            AccountPtr newAccount = std::make_shared<Account>(
                    accountId, protocolId, privKeyFile, instanceTagsFile, fingerprintsFile);

            accounts[std::make_pair(accountId, recipientId)] = newAccount;

            return { accountId, recipientId, protocolId, newAccount->userState(), OtrAppOpsHandler::otrAppOps };
        }
    }

    std::string OtrApp::getPrivKeyFileFor(const std::string &accountId, const std::string &protocolId) const {
        // TODO
    }

    std::string OtrApp::getInstanceTagsFileFor(const std::string &accountId, const std::string &protocolId) const {
        // TODO
    }

    std::string OtrApp::getFingerprintsFileFOr(const std::string &accountId, const std::string &protocolId) const {
        // TODO
    }

}
