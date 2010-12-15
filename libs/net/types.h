
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQNET_TYPES_H
#define EQNET_TYPES_H

#include <eq/base/hash.h>
#include <eq/base/refPtr.h>
#include <eq/base/uuid.h>

#include <deque>
#include <list>
#include <vector>

namespace eq
{
namespace net
{

#define EQNET_SEPARATOR '#'

class Node;
class LocalNode;
class Object;
class Barrier;
class Command;
class CommandQueue;
class Connection;
class ConnectionDescription;
class ObjectDataIStream;
struct ObjectVersion;

/** A unique identifier for nodes. */
typedef base::UUID NodeID;

typedef base::uint128_t uint128_t;

/** A reference pointer for Node pointers. */
typedef base::RefPtr< Node >                  NodePtr;
/** A reference pointer for LocalNode pointers. */
typedef base::RefPtr< LocalNode >             LocalNodePtr;
/** A reference pointer for Connection pointers. */
typedef base::RefPtr< Connection >            ConnectionPtr;
/** A reference pointer for ConnectionDescription pointers. */
typedef base::RefPtr< ConnectionDescription > ConnectionDescriptionPtr;

/** A vector of NodePtr's. */
typedef std::vector< NodePtr >                   Nodes;
/** A vector of Objects. */
typedef std::vector< Object* >                   Objects;
/** A vector of Barriers. */
typedef std::vector< Barrier* >                  Barriers;
/** A vector of ConnectionPtr's. */
typedef std::vector< ConnectionPtr >             Connections;
/** A vector of ConnectionDescriptionPtr's. */
typedef std::vector< ConnectionDescriptionPtr >  ConnectionDescriptions;
/** An iterator for a vector of ConnectionDescriptionPtr's. */
typedef std::vector< ConnectionDescriptionPtr >::iterator
                                                 ConnectionDescriptionsIter;
/** A const iterator for a vector of ConnectionDescriptionPtr's. */
typedef std::vector< ConnectionDescriptionPtr >::const_iterator
                                                 ConnectionDescriptionsCIter;

/** @cond IGNORE */
typedef std::vector< Command* > Commands;
typedef std::deque< Command* > CommandDeque;
typedef std::list< Command* > CommandList;
typedef stde::hash_map< base::uint128_t, Objects > ObjectsHash;
typedef std::vector< ObjectVersion > ObjectVersions;
typedef std::deque< ObjectDataIStream* > ObjectDataIStreamDeque;
typedef std::vector< ObjectDataIStream* > ObjectDataIStreams;
/** @endcond */

#ifdef EQ_USE_DEPRECATED
typedef Nodes NodeVector;
typedef Objects ObjectVector;
typedef Barriers BarrierVector;
typedef Connections ConnectionVector;
typedef ConnectionDescriptions ConnectionDescriptionVector;
typedef ObjectVersions ObjectVersionVector;
#endif
}
}

#endif // EQNET_TYPES_H