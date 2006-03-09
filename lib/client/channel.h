/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CHANNEL_H
#define EQ_CHANNEL_H

#include "commands.h"
#include "pixelViewport.h"
#include "window.h"

#include <eq/net/base.h>
#include <eq/net/object.h>

namespace eq
{
    class Channel;
    class Node;
    class RenderContext;

    class Channel : public eqNet::Base, public eqNet::Object
    {
    public:
        /** 
         * Constructs a new channel.
         */
        Channel();

        /**
         * Destructs the channel.
         */
        virtual ~Channel();

        /**
         * @name Data Access
         */
        //*{
        Pipe*   getPipe()   const
            { return ( _window ? _window->getPipe() : NULL );}
        Config* getConfig() const 
            { return ( _window ? _window->getConfig() : NULL );}

        /** 
         * Set the near and far planes for this channel.
         * 
         * The near and far planes are set during initialisation and are
         * inherited by source channels contributing to the rendering of this
         * channel. Dynamic near and far planes can be applied using
         * applyNearFar.
         *
         * @param near the near plane.
         * @param far the far plane.
         */
        void setNearFar( const float Nnear, const float Ffar )
            { _near = Nnear; _far = Ffar; }

        /** 
         * Returns the current near and far planes for this channel.
         *
         * The current near and far plane depends on the context from which this
         * function is called.
         * 
         * @param near a pointer to store the near plane.
         * @param far a pointer to store the far plane.
         */
        void getNearFar( float *near, float *far );
        //*}

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //@{

        /** 
         * Initialises this channel.
         * 
         * @param initID the init identifier.
         */
        virtual bool init( const uint32_t initID ){ return true; }

        /** 
         * Exit this channel.
         */
        virtual void exit(){}

        /** 
         * Clear the frame buffer.
         * 
         * @param frameID the per-frame identifier.
         */
        virtual void clear( const uint32_t frameID );

        /** 
         * Draw the scene.
         * 
         * @param frameID the per-frame identifier.
         */
        virtual void draw( const uint32_t frameID );
        //@}

        /**
         * @name Operations
         *
         * Operations are only available from within certain callbacks.
         */
        //*{

        /** 
         * Apply the current rendering buffer.
         */
        void applyBuffer();

        /** 
         * Apply the OpenGL viewport for the current rendering task.
         */
        void applyViewport();

        /**
         * Apply the frustum matrix for the current rendering task.
         */
        void applyFrustum();

        /** 
         * Apply the modeling transformation to position and orient the view
         * frustum.
         */
        void applyHeadTransform();

        //*}
    private:
        /** The parent node. */
        friend class   Window;
        Window*        _window;

        /** Static near plane. */
        float          _near;
        /** Static far plane. */
        float          _far;

        /** server-supplied rendering data. */
        RenderContext *_context;

        /** The pixel viewport for the current task. */
        PixelViewport  _pvp;

        /* The command handler functions. */
        eqNet::CommandResult _pushRequest( eqNet::Node* node,
                                           const eqNet::Packet* packet );
        eqNet::CommandResult _reqInit( eqNet::Node* node,
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _reqExit( eqNet::Node* node,
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _reqClear( eqNet::Node* node,
                                        const eqNet::Packet* packet );
        eqNet::CommandResult _reqDraw( eqNet::Node* node,
                                       const eqNet::Packet* packet );
    };
}

#endif // EQ_CHANNEL_H

