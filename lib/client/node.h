
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODE_H
#define EQ_NODE_H

#include "commands.h"

#include <eq/base/gate.h>
#include <eq/net/node.h>
#include <eq/net/object.h>
#include <eq/net/requestQueue.h>

namespace eq
{
    class Config;
    class Pipe;
    class Server;

    class Node : public eqNet::Node, public eqNet::Object
    {
    public:
        /** 
         * Constructs a new node.
         */
        Node();

        /**
         * Destructs the node.
         */
        virtual ~Node();

        /** 
         * Returns the config of this node.
         * 
         * @return the config of this node. 
         */
        Config* getConfig() const { return _config; }

    protected:
        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //@{

        /** 
         * Initialises this node.
         * 
         * @param initID the init identifier.
         */
        virtual bool init( const uint32_t initID ){ return true; }

        /** 
         * Exit this node.
         */
        virtual void exit(){}
        //@}

        /** @sa eqNet::Node::clientLoop */
        virtual void clientLoop();

        /** @sa eqNet::Node::handlePacket */
        virtual eqNet::CommandResult handlePacket( eqNet::Node* node, 
                                                   const eqNet::Packet* packet);

        /** @sa eqNet::Node::createNode */
        virtual eqBase::RefPtr<eqNet::Node> createNode();
        
        /** @sa eqNet::Node::createSession */
        virtual eqNet::Session* createSession();

    private:
        eqBase::RefPtr<Server> _server;
        Config*                _config;
        std::vector<Pipe*>     _pipes;

        /** The receiver->node thread request queue. */
        eqNet::RequestQueue    _requestQueue;
        bool                   _clientLoopRunning;

        /** The state of the node thread. */
        eqBase::Gate           _initialized;

        void _addPipe( Pipe* pipe );
        void _removePipe( Pipe* pipe );

        /** 
         * Push a request to the node thread to be handled asynchronously.
         * 
         * @param node the node sending the packet.
         * @param packet the command packet.
         */
        void _pushRequest( eqNet::Node* node, const eqNet::Packet* packet )
            { _requestQueue.push( node, packet ); }

        /** The command functions. */
        eqNet::CommandResult _cmdCreateConfig( eqNet::Node* node,
                                        const eqNet::Packet* packet );
        eqNet::CommandResult _cmdCreatePipe( eqNet::Node* node,
                                      const eqNet::Packet* packet );
        eqNet::CommandResult _cmdDestroyPipe( eqNet::Node* node,
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _cmdInit( eqNet::Node* node, 
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _reqInit( eqNet::Node* node,
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _reqExit( eqNet::Node* node,
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _reqStop( eqNet::Node* node,
                                       const eqNet::Packet* packet );
    };
}

#endif // EQ_NODE_H

