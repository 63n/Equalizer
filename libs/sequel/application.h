
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQSEQUEL_APPLICATION_H
#define EQSEQUEL_APPLICATION_H

#include <eq/sequel/types.h>

#include <eq/client.h>      // base class

namespace seq
{
    /** The main application object. */
    class Application : public eq::Client
    {
    public:
        /** Construct a new application instance. @version 1.0 */
        SEQ_API Application();

        /** Destruct this application instance. @version 1.0 */
        SEQ_API virtual ~Application();

        /** 
         * Initialize the application instance.
         *
         * @param argc the command line argument count.
         * @param argv the command line arguments.
         * @return true on success, false otherwise.
         * @version 1.0
         */
        SEQ_API virtual bool init( const int argc, char** argv );
        
        /**
         * Run the application main loop.
         *
         * @return true on success, false otherwise.
         * @version 1.0
         */
        SEQ_API virtual bool run();

        /**
         * Exit this application instance.
         *
         * @return true on success, false otherwise.
         * @version 1.0
         */
        SEQ_API virtual bool exit();

    private:
        detail::Application* _impl;
    };
}
#endif // EQSEQUEL_APPLICATION_H
