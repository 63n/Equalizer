
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WALL_H
#define EQ_WALL_H

#include <eq/base/log.h>

#include <iostream>

namespace eq
{
    // Definitions for common display systems in meters
#   define WALL_30INCH_16x10 {                            \
        { -.32, -.20, -1, },                        \
        {  .32, -.20, -1, },                        \
        { -.32,  .20, -1, }}

#   define WALL_20INCH_16x10 {                            \
        { -.21672, -.13545, -1, },                        \
        {  .21672, -.13545, -1, },                        \
        { -.21672,  .13545, -1, }}

#   define WALL_12INCH_4x3 {                              \
        { -.12294, -.09220, -1, },                        \
        {  .12294, -.09220, -1, },                        \
        { -.12294,  .09220, -1, }}

    /**
     * A wall defining a view frustum.
     * 
     * The three points describe the bottom left, bottom right and top left
     * coordinate of the wall in real-world coordinates.
     */
    class Wall
    {
    public:
        void translate( float x, float y, float z );

        float bottomLeft[3];
        float bottomRight[3];
        float topLeft[3];
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Wall& wall );
}

#endif // EQ_WALL_H

