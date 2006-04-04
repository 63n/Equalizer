
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"
#include "frameData.h"
#include "initData.h"

using namespace std;
using namespace eqBase;
using namespace eqNet;

bool Channel::init( const uint32_t initID )
{
    EQINFO << "Init channel initID " << initID << " ptr " << this << endl;
    eq::Config* config = getConfig();
    _initData = RefPtr_static_cast< InitData, Mobject >( 
        config->getMobject( initID ));
    _frameData = _initData->getFrameData();

    EQASSERT( _frameData );

    return true;
}

bool Channel::exit()
{
    _initData  = NULL;
    _frameData = NULL;
    EQINFO << "Exit " << this << endl;
    return eq::Channel::exit();
}

void Channel::draw( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();
            
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    applyFrustum();
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    applyHeadTransform();
    
    _frameData->sync( frameID );
    
    glTranslatef( 0, 0, -3 );
    glRotatef( _frameData->spin, 0, 0, 1. );
    
    glColor3f( 1, 1, 0 );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f( -.25, -.25, -.25 );
    glVertex3f( -.25,  .25, -.25 );
    glVertex3f(  .25, -.25, -.25 );
    glVertex3f(  .25,  .25, -.25 );
    glEnd();
    glFinish();
}

