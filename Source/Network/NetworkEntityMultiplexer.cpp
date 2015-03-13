#include "NetworkEntityMultiplexer.h"
#include "Message.h"
#include "Session.h"
#include "NetworkEntity.h"

#include <stdlib.h>
#include <string.h>

#define DEBUG

using namespace Networking;

NetworkEntityMultiplexer* NetworkEntityMultiplexer::instance = 0;

/**
 * returns the singleton instance of the NetworkEntityMultiplexer, and
 *   instantiates it if needed.
 *
 * @function   NetworkEntityMultiplexer::getInstance
 *
 * @date       2015-03-12
 *
 * @revision   none
 *
 * @designer   Networking Team
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  NetworkEntityMultiplexer* NetworkEntityMultiplexer::getInstance()
 *
 * @return     the singleton instance of the NetworkEntityMultiplexer
 */
NetworkEntityMultiplexer* NetworkEntityMultiplexer::getInstance()
{
    if(instance == 0)
    {
        instance = new NetworkEntityMultiplexer();
    }
    return instance;
}
/**
 * instantiates a NetworkEntityMultiplexer.
 *
 * @function   NetworkEntityMultiplexer::NetworkEntityMultiplexer
 *
 * @date       2015-03-12
 *
 * @revision   none
 *
 * @designer   EricTsang
 *
 * @programmer EricTsang
 *
 * @note       none
 *
 * @signature  NetworkEntityMultiplexer::NetworkEntityMultiplexer()
 */
NetworkEntityMultiplexer::NetworkEntityMultiplexer()
{
}
/**
 * destructs a NetworkEntityMultiplexer.
 *
 * @function   NetworkEntityMultiplexer::~NetworkEntityMultiplexer
 *
 * @date       2015-03-12
 *
 * @revision   none
 *
 * @designer   EricTsang
 *
 * @programmer EricTsang
 *
 * @note       none
 *
 * @signature  NetworkEntityMultiplexer::~NetworkEntityMultiplexer()
 */
NetworkEntityMultiplexer::~NetworkEntityMultiplexer()
{
}
/**
 * method with the same signature as the Session::onMessage. this
 *   function should be invoked within the session's onMessage method
 *   and forwarded the parameters if the message received by the session
 *   was sent from another {NetworkEntityMultiplexer}.
 *
 * @function   NetworkEntityMultiplexer::onMessage
 *
 * @date       2015-02-28
 *
 * @revision   none
 *
 * @designer   Networking Team
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  int onMessage(Session* session, Message msg);
 *
 * @param      session session that received the message
 * @param      msg message received from a session object.
 *
 * @return integer indicating the outcome of the operation
 */
void NetworkEntityMultiplexer::onMessage(Session* session, Message msg)
{
    int* intPtr = (int*) msg.data;
    Message logicMsg = msg;
    switch(msg.type)
    {
    case MSG_TYPE_UPDATE:
        logicMsg.data = ((int*)logicMsg.data)+1;
        logicMsg.len -= 4;
        entities[intPtr[0]]->onUpdate(logicMsg);

        // print debug message
        #ifdef DEBUG
        printf("NetworkEntity#%d::onUpdate(\"",intPtr[0]);
        for(int i = 0; i < logicMsg.len; ++i)
        {
            printf("%c",((char*)logicMsg.data)[i]);
        }
        printf("\")\n");
        #endif
        break;
    case MSG_TYPE_REGISTER:
        logicMsg.data = ((int*)logicMsg.data)+2;
        logicMsg.len -= 8;
        entities[intPtr[0]] = onRegister(intPtr[0],intPtr[1],session,logicMsg);
        entities[intPtr[0]]->silentRegister(session);

        // print debug message
        #ifdef DEBUG
        printf("NetworkEntity#%d::onRegisterSession(%d,Session%p,\"",intPtr[0],intPtr[1],session);
        for(int i = 0; i < logicMsg.len; ++i)
        {
            printf("%c",((char*)logicMsg.data)[i]);
        }
        printf("\")\n");
        #endif
        break;
    case MSG_TYPE_UNREGISTER:
        logicMsg.data = ((int*)logicMsg.data)+1;
        logicMsg.len -= 4;
        entities[intPtr[0]]->onUnregister(session, logicMsg);
        entities[intPtr[0]]->silentUnregister(session);
        entities.erase(*intPtr);

        // print debug message
        #ifdef DEBUG
        printf("NetworkEntity#%d::onUnregisterSession(Session%p,\"",intPtr[0],session);
        for(int i = 0; i < logicMsg.len; ++i)
        {
            printf("%c",((char*)logicMsg.data)[i]);
        }
        printf("\")\n");
        #endif
        break;
    case MSG_TYPE_WARNING:
        printf("REMOTE %s\n",(char*)msg.data);
        break;
    }
}
/**
 * should only be called by {NetworkEntity} objects only. it
 *   encapsulates the passed data into a packet, to be sent to all
 *   session objects registered with the {NetworkEntity} associated with
 *   {id}.
 *
 * @function   NetworkEntityMultiplexer::update
 *
 * @date       2015-02-28
 *
 * @revision   none
 *
 * @designer   Networking Team
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  int update(int id, std::set<Session*>& sessions, Message msg);
 *
 * @param      id identifier associated with a {NetworkEntity} instance
 * @param      sessions set of sessions associated with the network entity that need to be informed of the update
 * @param      msg describes the message to send over the wire
 *
 * @return     integer indicating the result of the operation
 */
void NetworkEntityMultiplexer::update(int id, std::set<Session*>& sessions, Message msg)
{
    // allocate enough memory to hold message header, and payload
    int datalen = msg.len+sizeof(int);
    char* data = (char*)malloc(datalen);

    // inject header information
    int* intPtr = (int*) data;
    intPtr[0] = id;

    // inject payload information
    memcpy(&data[datalen-msg.len],msg.data,msg.len);

    // create message structure
    Message wireMsg;
    wireMsg.type = MSG_TYPE_UPDATE;
    wireMsg.data = data;
    wireMsg.len  = datalen;

    // send the message to all the sessions
    for(auto it = sessions.begin(); it != sessions.end(); ++it)
    {
        (*it)->send(&wireMsg);
    }

    // free allocated data
    free(data);
}
/**
 * should only be called by the {NetworkEntity} class. it registers the
 *   passed {Session} object with the {NetworkEntity} associated with
 *   {id}, and sends the {msg} to the {session}.
 *
 * @function   NetworkEntityMultiplexer::registerSession
 *
 * @date       2015-02-28
 *
 * @revision   none
 *
 * @designer   Networking Team
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  int registerSession(int id, int type, Session* session,
 *   Message msg)
 *
 * @param      id identifier associated with a {NetworkEntity} instance
 * @param      type type of entity that's being registered
 * @param      session {Session} to be registered with the
 *   {NetworkEntity} instance
 * @param      msg describes the message to send over the wire. this
 *   message is only sent to the {session}.
 *
 * @return     integer indicating the result of the operation
 */
void NetworkEntityMultiplexer::registerSession(int id, int type, Session* session, Message msg)
{
    // allocate enough memory to hold message header, and payload
    int datalen = msg.len+sizeof(int)*2;
    char* data = (char*)malloc(datalen);

    // inject header information
    int* intPtr = (int*) data;
    intPtr[0] = id;
    intPtr[1] = type;

    // inject payload information
    memcpy(&data[datalen-msg.len],msg.data,msg.len);

    // create message structure
    Message wireMsg;
    wireMsg.type = MSG_TYPE_REGISTER;
    wireMsg.data = data;
    wireMsg.len  = datalen;

    // send the message to the sessions
    session->send(&wireMsg);

    // free allocated data
    free(data);
}
/**
 * should only be invoked by the {NetworkEntity} class. it unregisters
 *   the {session} from the {NetworkEntity} instance associated with
 *   {id}, and sends the {msg} to the {session}.
 *
 * @function   NetworkEntityMultiplexer::unregisterSession
 *
 * @date       2015-02-28
 *
 * @revision   none
 *
 * @designer   Networking Team
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  int unregisterSession(int id, Session* session, Message
 *   msg)
 *
 * @param      id identifier associated with a {NetworkEntity} instance
 * @param      session {Session} to be unregistered with the
 *   {NetworkEntity} instance
 * @param      msg describes the message to send over the wire. this
 *   message is only sent to the {session}.
 *
 * @return     integer indicating the result of the operation
 */
void NetworkEntityMultiplexer::unregisterSession(int id, Session* session, Message msg)
{
    // allocate enough memory to hold message header, and payload
    int datalen = msg.len+sizeof(int);
    char* data = (char*)malloc(datalen);

    // inject header information
    int* intptr = (int*) data;
    intptr[0] = id;

    // inject payload information
    memcpy(&data[datalen-msg.len],msg.data,msg.len);

    // create message structure
    Message wireMsg;
    wireMsg.type = MSG_TYPE_UNREGISTER;
    wireMsg.data = data;
    wireMsg.len  = datalen;

    // send the message to the sessions
    session->send(&wireMsg);

    // free allocated data
    free(data);
}
/**
 * should only be called from within the Networking library. it calls
 *   the update method of the {NetworkEntity} instance associated with
 *   {id}.
 *
 * @function   NetworkEntityMultiplexer::onUpdate
 *
 * @date       2015-02-28
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  void onUpdate(int id, Message msg)
 *
 * @param      id identifier associated with a {NetworkEntity} instance.
 * @param      msg describes the message to send over the wire. this
 *   message is only sent to the {session}.
 */
void NetworkEntityMultiplexer::onUpdate(int id, Message msg)
{
    entities[id]->onUpdate(msg);
}
NetworkEntity* NetworkEntityMultiplexer::onRegister(int id, int entityType, Session* session, Message msg)
{
    return new NetworkEntity(id,entityType);
}
/**
 * should only be called from within the Networking library. it calls
 *   the onUnregister method of the {NetworkEntity} instance associated
 *   with {id}.
 *
 * @function   NetworkEntityMultiplexer::onUnregister
 *
 * @date       2015-02-28
 *
 * @revision   none
 *
 * @designer   Networking Team
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  void onUnregister(int id, Session* session, Message msg)
 *
 * @param      id identifier associated with a {NetworkEntity} instance.
 * @param      session session being registered with the NetworkEntity.
 * @param      msg describes the message to send over the wire. this message
 *   is only sent to the {session}.
 */
void NetworkEntityMultiplexer::onUnregister(int id, Session* session, Message msg)
{
    entities[id]->onUnregister(session,msg);
}