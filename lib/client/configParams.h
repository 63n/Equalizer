
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CONFIG_PARAMS_H
#define EQ_CONFIG_PARAMS_H

#include <eq/base/base.h>
#include <string>

namespace eq
{
    class ConfigParams
    {
    public:
        ConfigParams();
        virtual ~ConfigParams(){}

        ConfigParams& operator = ( const ConfigParams& rhs );

        std::string renderClient;
        std::string workDir;
        uint32_t    compoundModes;
    private:
    };
}

#endif // EQ_CONFIG_PARAMS_H

