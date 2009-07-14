
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
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
#include "pluginRegistry.h"
#include "compressor.h" 
#include "global.h"


#include <eq/base/fileSearch.h> 
#include <eq/base/dso.h> 
#include <vector>

namespace eq
{


void PluginRegistry::init()
{
    EQASSERT( _compressors.empty( ));

    // add self
    _initCompressor( "" );
    EQASSERT( _compressors.size() == 1 );

    // search all plugin directories for compressor DSOs
    const StringVector& directories = Global::getPluginDirectories();

    // for each directory
    for( StringVector::const_iterator i = directories.begin();
         i != directories.end(); ++i )
    {
        const std::string& directory = *i;
        EQINFO << "Searching compressors in " << directory << std::endl;

        // search the number of files in the director
#ifdef WIN32
        StringVector files = base::fileSearch( directory, 
                                                "EqualizerCompressor*.dll" );
        const char DIRSEP = '\\';
#elif defined (Darwin)
        StringVector files = base::fileSearch( directory, 
                                                "libeqCompressor*dylib" );
        const char DIRSEP = '/';
#else
        StringVector files = base::fileSearch( directory,
                                                "libeqCompressor*so" );
        const char DIRSEP = '/';
#endif
        
        // for each file in the directory
        for( StringVector::const_iterator j = files.begin();
             j != files.end(); ++j )
        {
            // build path + name of library
            const std::string libraryName = 
                directory.empty() ? *j : directory + DIRSEP + *j;
            _initCompressor( libraryName );
        }
    }
}

void PluginRegistry::_initCompressor( const std::string& filename )
{
    Compressor* compressor = new Compressor(); 
    bool add = compressor->init( filename );

    const CompressorInfoVector& infos = compressor->getInfos();
    if( infos.empty( ))
        add = false;
            
    for( CompressorVector::const_iterator i = _compressors.begin();
         add && i != _compressors.end(); ++i )
    {
        const CompressorInfoVector& infos2 = (*i)->getInfos();
        // Simple test to avoid using the same dll twice
        if( infos.front().name == infos2.front().name )
            add = false;
    }

    if( add )
    {
        _compressors.push_back( compressor );
        EQINFO << "Found compressor " << filename << " @" << (void*)compressor
               << " providing " << compressor->getInfos().size() << " engines"
               << std::endl;
    }
    else
        delete compressor;
}

void PluginRegistry::exit()
{
    for( CompressorVector::const_iterator i = _compressors.begin(); 
         i != _compressors.end(); ++i )
    {
        Compressor* compressor = *i;
        compressor->exit();
        delete compressor;
    }

    _compressors.clear();
}

Compressor* PluginRegistry::findCompressor( const uint32_t name )
{

    for( CompressorVector::const_iterator i = _compressors.begin(); 
         i != _compressors.end(); ++i )
    {
        Compressor* compressor = *i;
        if ( compressor->implementsType( name ))
            return compressor;
    }

    return 0;
}

const CompressorVector& PluginRegistry::getCompressors() const
{
    return _compressors;
}


}
