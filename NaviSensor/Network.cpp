#include "Network.h"
#include "../NaviSensorUI/tools.h"

Comm::DataNode::DataNode (const unsigned int port, MsgReadCb readCb, void *param, void *param2) :
    Socket (), worker (workerProcInternal, readCb, param, param2, this)
{
    this->active = true;
    this->port   = port;

    create (port);
}

Comm::DataNode::~DataNode ()
{
    active = false;

    worker.join ();

    close ();
}

void Comm::DataNode::sendMessage (MsgType msgType, byte *data, const int dataSize, const unsigned int port, const char *destAddr)
{
    sendMessage (this, msgType, data, dataSize, port, destAddr);
    /*#pragma pack(1)
    union
    {
        GenericMsg msg;
        byte       buffer [2000];
    };
    #pragma pack()

    msg.msgType = msgType;
    msg.size    = sizeof (GenericMsg) + dataSize;

    memcpy (buffer + sizeof (GenericMsg), data, dataSize);

    sendTo ((const char *) buffer, msg.size, port, destAddr);*/
}

void Comm::DataNode::sendMessage (Socket *transmitter, MsgType msgType, byte *data, const int dataSize, const unsigned int port, const char *destAddr)
{
    #pragma pack(1)
    union
    {
        GenericMsg msg;
        byte       buffer[2000];
    };
    #pragma pack()

    msg.msgType = msgType;
    msg.size    = sizeof (GenericMsg) + dataSize;

    memcpy (buffer + sizeof (GenericMsg), data, dataSize);

    if (transmitter)
        transmitter->sendTo ((const char *) buffer, msg.size, port, destAddr);
}

unsigned int Comm::DataNode::sendCommand (CmdType cmd, const unsigned char arg1, const unsigned char arg2, const unsigned short arg3, const unsigned int port, const char *destAddr)
{
    /*unsigned int arg = (((unsigned int) arg1) << 24) + (((unsigned int) arg2) << 16) + (unsigned int) arg3;

    return sendCommand (cmd, arg, port, destAddr);*/

    return sendCommand (this, cmd, arg1, arg2, arg3, port, destAddr);
}

unsigned int Comm::DataNode::sendCommand (Socket *transmitter, CmdType cmd, const unsigned char arg1, const unsigned char arg2, const unsigned short arg3, const unsigned int port, const char *destAddr)
{
    unsigned int arg = (((unsigned int)arg1) << 24) + (((unsigned int)arg2) << 16) + (unsigned int)arg3;

    return sendCommand (transmitter, cmd, arg, port, destAddr);
}

unsigned int Comm::DataNode::sendCommand (CmdType cmd, const unsigned short arg1, const unsigned short arg2, const unsigned int port, const char *destAddr)
{
    /*unsigned int arg = (((unsigned int) arg1) << 16) + (unsigned int) arg2;

    return sendCommand (cmd, arg, port, destAddr);*/

    return sendCommand (this, cmd, arg1, arg2, port, destAddr);
}

unsigned int Comm::DataNode::sendCommand (Socket *transmitter, CmdType cmd, const unsigned short arg1, const unsigned short arg2, const unsigned int port, const char *destAddr)
{
    unsigned int arg = (((unsigned int) arg1) << 16) + (unsigned int) arg2;

    return sendCommand (transmitter, cmd, arg, port, destAddr);
}

unsigned int Comm::DataNode::sendCommand (CmdType cmd, const unsigned int arg, const unsigned int port, const char *destAddr)
{
    /*static unsigned int seqNumber = 1;

    #pragma pack(1)
    union
    {
        Command msg;
        byte    buffer [2000];
    };
    #pragma pack()

    msg.command   = cmd;
    msg.seqNumber = seqNumber;
    msg.argument  = arg;

    sendMessage (MsgType::Cmd, buffer + sizeof (GenericMsg), sizeof (Command) - sizeof (GenericMsg), port, destAddr);

    return seqNumber ++;*/

    return sendCommand (this, cmd, arg, port, destAddr);
}

unsigned int Comm::DataNode::sendCommand (Socket *transmitter, CmdType cmd, const unsigned int arg, const unsigned int port, const char *destAddr)
{
    static unsigned int seqNumber = 1;

    #pragma pack(1)
    union
    {
        Command msg;
        byte    buffer[2000];
    };
    #pragma pack()

    msg.command = cmd;
    msg.seqNumber = seqNumber;
    msg.argument = arg;

    sendMessage (transmitter, MsgType::Cmd, buffer + sizeof(GenericMsg), sizeof(Command) - sizeof(GenericMsg), port, destAddr);

    return seqNumber++;
}

void Comm::DataNode::workerProc (MsgReadCb readCb, void *param, void *param2)
{
    char    buffer [2000];
    int     bytesReceived;
    in_addr sender;

    while (active)
    {
        bytesReceived = receiveFrom (buffer, sizeof (buffer), sender);

        if (bytesReceived > 0)
        {
            GenericMsg *msg = (GenericMsg *) buffer;

            readCb ((MsgType) msg->msgType, buffer + sizeof (GenericMsg), msg->size, param, param2);
        }

        Tools::sleepFor (5);
    }
}

void Comm::DataNode::workerProcInternal (MsgReadCb readCb, void *param, void *param2, DataNode *self)
{
    if (self)
        self->workerProc (readCb, param, param2);
}
