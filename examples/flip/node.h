
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FLIP_NODE_H
#define EQ_FLIP_NODE_H

#include <eq/eq.h>

class Node : public eq::Node
{
protected:
    bool init( const uint32_t initID );
};

#endif // EQ_FLIP_NODE_H
