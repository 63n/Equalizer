
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

#ifndef GPUSD_DNS_SD_MODULE_H
#define GPUSD_DNS_SD_MODULE_H

#include <gpusd1/module.h> // base class

namespace gpusd
{
namespace dns_sd
{
    /** The DNS_SD implementation for remote GPU discovery. */
    class Module : public gpusd::Module
    {
    public:
        Module() : gpusd::Module() {}
        virtual ~Module() {}

    protected:
        virtual GPUInfos discoverGPUs_() const;
    };
}
}
#endif // GPUSD_DNS_SD_MODULE_H

