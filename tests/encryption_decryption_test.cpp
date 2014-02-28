#define BOOST_TEST_MODULE encryption_decryption_test

#include <boost/test/unit_test.hpp>
#include <QtCore>
#include <list>
#include <iostream>
extern "C" {
#include <libotr/privkey.h>
#include <libotr/instag.h>
#include <libotr/proto.h>
#include <libotr/message.h>
#include <libotr/userstate.h>
}
#include <sys/types.h>
#include <sys/stat.h>

#include "../src/otr_encryption.hpp"


OtrlPolicy policy(void *opdata, ConnContext *context);
void create_privkey(void *opdata, const char *accountname, const char *protocol);
void inject_message(void *opdata, const char *accountname, const char *protocol, const char *recipient, const char *message);
void gone_secure(void *opdata, ConnContext *context);
void gone_insecure(void *opdata, ConnContext *context);
void still_secure(void *opdata, ConnContext *context, int is_reply);
const char * otr_error_message(void *opdata, ConnContext *context, OtrlErrorCode err_code);
void handle_msg_event(void *opdata, OtrlMessageEvent msg_event, ConnContext *context, const char *message, gcry_error_t err);
const char *account_name(void *opdata, const char *account, const char *protocol);

struct OtrFixture {

    OtrFixture() {
        OTRL_INIT; // initialize library
        otrAppOps = {
            policy,             // policy
            create_privkey,     // create_privkey
            nullptr,            // is_logged_in
            inject_message,     // inject_message
            nullptr,            // update_context_list
            nullptr,            // new_fingerprint
            nullptr,            //  write_fingerprints
            gone_secure,        // gone_secure
            gone_insecure,      // gone_insecure
            still_secure,       // still_secure
            nullptr,            // max_message_size
            account_name,       // account_name
            nullptr,            // account_name_free
            nullptr,            // received_symkey
            otr_error_message,  // otr_erro_message
            nullptr,            // otr_erro_message_free
            nullptr,            // resent_ms_prefix
            nullptr,            // resent_msg_prefix_free
            nullptr,            // handle_smp_event 
            handle_msg_event,   // handle_msg_event
            nullptr,            // crate_instag 
            nullptr,            // conver_msg
            nullptr,            // convert_free
            nullptr             // timer_control
        };
    }

    ~OtrFixture() {
    }

    OtrlMessageAppOps otrAppOps;
};

struct OtrUser {

    OtrUser(std::string userName, std::string recipient, std::string protocol, OtrlMessageAppOps otrAppOps) 
        : accountId(std::move(userName))
    { 
        gcry_error_t error;
        userState = otrl_userstate_create();

        struct stat buffer;
        std::string key_file = accountId + "_privkey";
        std::string instag_file = accountId + "_instag";
        std::string finger_file = accountId + "_finger";
        if(stat(key_file.c_str(), &buffer) == 0) {
            error = otrl_privkey_read(userState, key_file.c_str());
        } else {
            error = otrl_privkey_generate(userState, key_file.c_str(), accountId.c_str(), protocol.c_str());
        }
        if(error) {
            otrl_userstate_free(userState);
            throw otr::OtrGcryException("Privkey", error);
        }

        error = otrl_instag_read(userState, instag_file.c_str());
        if(error) {
            otrl_userstate_free(userState);
            throw otr::OtrGcryException("Instag", error);
        }

        error = otrl_instag_read(userState, finger_file.c_str());
        if(error) {
            otrl_userstate_free(userState);
            throw otr::OtrGcryException("Fingerprints", error);
        }

        userC.reset(new otr::OtrUserContext(userName, recipient, protocol, userState, otrAppOps));
        userC->setOpdata(reinterpret_cast<void*>(this)); // unsafe!
    }


    ~OtrUser() {
        if(userState != nullptr) otrl_userstate_free(userState);
    }

    void processMessage(const QString& message) {
        std::cout << "User: " << accountId << " processing -> " << std::endl
            << message.toStdString() << std::endl;

        std::cout << "Decrypting..." << std::endl;
        auto decrypted = userC->decryptMessage(message);
        std::cout << "Decrypted message type: " << (int)decrypted.second << std::endl
            << "message -> " << decrypted.first.toStdString() << std::endl;
    }

    void sendMessage(const QString& message) {
        std::cout << "User: " << accountId << " sends a message -> " << message.toStdString() << std::endl << std::endl;
        QString result = userC->encryptMessage(message);
        outMessages.push_back(result);
    }

    void begin(const QString& message) {
        QString initMes("?OTRv23?");
        std::cout << "User: " << accountId << " starts conversation with message -> " << initMes.toStdString() << std::endl;
        mesToSend = message;
        sendMessage(initMes);
    }

    void printState() {
        std::cout << "--- State of user: " << accountId << " -> " << std::endl;
        std::cout << "Message state: " << (userState->context_root ? userState->context_root->msgstate : -1) << std::endl;
        std::cout << "Auth state: " << (userState->context_root ? userState->context_root->auth.authstate : -1) << std::endl;
        std::cout << "------------------------" << std::endl;
    }

    std::list<QString> outMessages;
    std::string accountId;
    OtrlUserState userState;
    bool isActive = true;
    std::unique_ptr<otr::OtrUserContext> userC;
    private:
        QString mesToSend;
};

struct Conversation {
    
    Conversation(OtrUser &userA, OtrUser &userB) : userA(userA), userB(userB)
    {
    }

    void startConversation(const QString &message) {

        std::cout << "Starting conversation between: " << userA.accountId << " and " << userB.accountId << std::endl
            << "with message: " << message.toStdString() << std::endl;
        userA.isActive = userB.isActive = true;

        userA.begin(message);
        while(userB.isActive || userA.isActive) {
            if(userB.isActive) {
                while(!userA.outMessages.empty()) {
                    std::cout << "Printing state before" << std::endl;
                    userB.printState();
                    userB.processMessage(userA.outMessages.front());
                    userA.outMessages.pop_front();
                    std::cout << std::endl;
                }
            }
            if(userA.isActive) {
                while(!userB.outMessages.empty()) {
                    std::cout << "Printing state before" << std::endl;
                    userA.printState();
                    userA.processMessage(userB.outMessages.front());
                    userB.outMessages.pop_front();
                    std::cout << std::endl;
                }
            }
        }

        std::cout << "Ending conversation between: " << userA.accountId << " and " << userB.accountId << std::endl
            << "with message: " << message.toStdString() << std::endl;
    }

    OtrUser &userA, &userB;
};

OtrlPolicy policy(void *opdata, ConnContext *context) {
    std::cout << "->~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    std::cout << "Policy" << std::endl;
    std::cout << "<-~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    return OTRL_POLICY_DEFAULT;   
}

void create_privkey(void *opdata, const char *accountname, const char *protocol) {
    std::cout << "->~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    std::cout << "create privkey" << std::endl;
    std::cout << "<-~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
}

void inject_message(void *opdata, const char *accountname, const char *protocol, const char *recipient, const char *message) {
    std::cout << "->~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    std::cout << "Injecting message for account: " << accountname << " to: " << recipient << std::endl;
    OtrUser* user = reinterpret_cast<OtrUser*>(opdata);
    user->sendMessage(message);
    std::cout << "<-~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
}

void gone_secure(void *opdata, ConnContext *context) {
    std::cout << "->~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    std::cout << "Gone secure: " << context->accountname << std::endl;
    std::cout << "<-~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
}

void gone_insecure(void *opdata, ConnContext *context) {
    std::cout << "->~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    std::cout << "Gone insecure: " << context->accountname << std::endl;
    std::cout << "<-~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
}

void still_secure(void *opdata, ConnContext *context, int is_reply) {
    std::cout << "->~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    std::cout << "Still secure: " << context->accountname << " is reply: " << is_reply << std::endl;
    std::cout << "<-~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
}

const char * otr_error_message(void *opdata, ConnContext *context, OtrlErrorCode err_code) {
    std::cout << "->~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    std::cout << "User: " << context->accountname << " got error with code: " << err_code << std::endl;
    std::cout << "<-~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    return "";
}

void handle_msg_event(void *opdata, OtrlMessageEvent msg_event, ConnContext *context, const char *message, gcry_error_t err) {
    std::cout << "->~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    std::cout << "Message event of type: " << msg_event << " at user: " << context->accountname << std::endl
        << "with message: " << message << std::endl
        << "of error: " << err << std::endl;
    std::cout << "<-~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
}     

const char *account_name(void *opdata, const char *account, const char *protocol) {
    std::cout << "->~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    std::cout << "Account name for account: " << account << std::endl;
    std::cout << "<-~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    return account; // the same we will not deallocate
}

BOOST_FIXTURE_TEST_SUITE(encryption_decryption_test, OtrFixture)

    BOOST_AUTO_TEST_CASE(simple_conversation_test) {
        OtrUser a("Alice", "Bob", "xmpp", otrAppOps);
        OtrUser b("Bob", "Alice", "xmpp", otrAppOps);

        Conversation conv(a, b);
        conv.startConversation("Hello Bob!");
    }

BOOST_AUTO_TEST_SUITE_END()
