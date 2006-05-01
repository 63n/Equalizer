
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

#include "config.h"
#include "configParams.h"
#include "global.h"
#include "node.h"
#include "nodeFactory.h"
#include "openParams.h"
#include "packets.h"

#include <eq/net/connection.h>

using namespace eq;
using namespace eqBase;
using namespace std;

Server::Server()
        : Node( CMD_SERVER_ALL ),
          _state( STATE_STOPPED )
{
    registerCommand( CMD_SERVER_CHOOSE_CONFIG_REPLY, this,
                     reinterpret_cast<CommandFcn>( 
                         &eq::Server::_cmdChooseConfigReply ));

    EQINFO << "New server at " << (void*)this << endl;
}

bool Server::open( const OpenParams& params )
{
    if( _state != STATE_STOPPED )
        return false;

    RefPtr<eqNet::Connection> connection =
        eqNet::Connection::create( eqNet::Connection::TYPE_TCPIP );

    RefPtr<eqNet::ConnectionDescription> connDesc = 
        new eqNet::ConnectionDescription;

    const char* envServer = getenv( "EQ_SERVER" );
    const string address  = params.address.size() > 0 ? params.address :
                            envServer ? envServer : "localhost";
    const size_t colonPos = address.rfind( ':' );

    if( colonPos == string::npos )
        connDesc->hostname = address;
    else
    {
        connDesc->hostname   = address.substr( 0, colonPos );
        string port          = address.substr( colonPos+1 );
        connDesc->TCPIP.port = atoi( port.c_str( ));
    }

    if( !connDesc->TCPIP.port )
        connDesc->TCPIP.port = 4242;

    if( !connection->connect( connDesc ))
        return false;

    eqNet::Node* localNode = Node::getLocalNode();
    if( !localNode->connect( this, connection ))
        return false;

    // TODO: send open packet (appName)

    _state = STATE_OPENED;
    return true;
}

bool Server::close()
{
    if( _state != STATE_OPENED )
        return false;

    Node* localNode = eq::Node::getLocalNode();
    if( !localNode->disconnect( this ))
        return false;

    _state = STATE_STOPPED;
    return true;
}

Config* Server::chooseConfig( const ConfigParams& parameters )
{
    if( _state != STATE_OPENED )
        return NULL;

    ServerChooseConfigPacket packet;

    packet.requestID          = _requestHandler.registerRequest();
    packet.compoundModes      = parameters.compoundModes;
    const std::string rendererInfo = parameters.workDir + ":" + 
        parameters.renderClient;

    send( packet, rendererInfo );

    Config* config = (Config*)_requestHandler.waitRequest( packet.requestID );
    if( !config )
        return NULL;

    return config;
}

void Server::releaseConfig( Config* config )
{
    if( _state != STATE_OPENED )
        return;

    ServerReleaseConfigPacket packet;
    packet.configID = config->getID();
    send( packet );
    delete config;
}

void Server::addConfig( Config* config )
{
    EQASSERT( config->getID() != EQ_INVALID_ID );
    config->_server = this;
    _configs[config->getID()] = config;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Server::_cmdChooseConfigReply( eqNet::Node* node, 
                                    const eqNet::Packet* pkg )
{
    ServerChooseConfigReplyPacket* packet = (ServerChooseConfigReplyPacket*)pkg;
    EQINFO << "Handle choose config reply " << packet << endl;

    if( packet->configID == EQ_INVALID_ID )
    {
        _requestHandler.serveRequest( packet->requestID, NULL );
        return eqNet::COMMAND_HANDLED;
    }

    Config* config    = Global::getNodeFactory()->createConfig();
    Node*   localNode = Node::getLocalNode();

    localNode->addSession( config, node, packet->configID, packet->sessionName);
    addConfig( config );

    _requestHandler.serveRequest( packet->requestID, config );
    return eqNet::COMMAND_HANDLED;
}
