
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include <eq/client/init.h>
#include <eq/client/nodeFactory.h>
#include <eq/plugins/compressor.h>

#include <eq/client/init.h>
#include <eq/client/nodeFactory.h>
#include <eq/base/buffer.h>
#include <eq/base/clock.h>
#include <eq/base/compressor.h>
#include <eq/base/compressorDataCPU.h>
#include <eq/base/file.h>
#include <eq/base/global.h>
#include <eq/base/pluginRegistry.h>
#include <eq/base/types.h>

#include <iostream>  // for std::cerr
#include <numeric>
#include <fstream>


void testCompressByte( uint32_t nameCompressor,
                       char* data, uint64_t size,
                       std::ofstream* logFile );

void testCompressorFile(  std::ofstream* logFile );

bool compare( const char *dst, const char *src, uint32_t nbytes );

std::string getCompressorName( const uint32_t name );

std::vector< std::string > getFiles( std::string path, 
                                    std::vector< std::string >& files, 
                                    const std::string& ext );

eq::base::Bufferb* readDataFile( const std::string& filename );

int main( int argc, char **argv )
{
    eq::NodeFactory nodeFactory;
    eq::init( argc, argv, &nodeFactory );
    
    std::ofstream* logFile = new std::ofstream( "result.html" );
    *logFile << "<html><head><title>Result of Compression TEST</title>" 
             << "</head><center><H1>Result of Compression TEST<H1></center>"
             << "<body>";
    
    testCompressorFile( logFile );

    *logFile << "</TABLE>";
    *logFile << "</body></html>";
    logFile->close();
    
    eq::exit();
}

void testCompressByte( uint32_t nameCompressor,
                       char* data, uint64_t size,
                       std::ofstream* logFile )
{
    eq::base::Clock clock;
    
    eq::base::Bufferb destfile;

    // find compressor in the corresponding plugin
    eq::base::PluginRegistry& registry = eq::base::Global::getPluginRegistry();
    eq::base::Compressor* compressor = registry.findCompressor( nameCompressor );
    void* instanceComp = compressor->newCompressor( nameCompressor );
    void* instanceUncomp = compressor->newDecompressor( nameCompressor );
    
    uint64_t flags = EQ_COMPRESSOR_DATA_1D;
    const char* dataorigine = reinterpret_cast<const char*>( data );
    uint64_t inDims[2]  = { 0, size }; 
    clock.reset();
    compressor->compress( instanceComp, nameCompressor, 
                          data, inDims, flags );

    uint64_t time = clock.getTime64();
    const size_t numResults = compressor->getNumResults( 
                                                instanceComp, nameCompressor );

    uint64_t totalSize = 0;

    std::vector< void * > vectorVoid;
    vectorVoid.resize(numResults);
    std::vector< uint64_t > vectorSize;
    vectorSize.resize(numResults);
    for( size_t i = 0; i < numResults ; i++ )
    {
        compressor->getResult( instanceComp, nameCompressor,
                               i, 
                               &vectorVoid[i], 
                               &vectorSize[i] );
        totalSize += vectorSize[i];
    }
    
    std::cout   << totalSize << ", " << std::setw(10)
                << time << ", " << std::setw(10);

    *logFile << "<TD> " << totalSize <<"</TD>";
    *logFile << "<TD> " << time <<"</TD>";
    eq::base::Bufferb result;
    result.resize( size );
    void* outData = reinterpret_cast< uint8_t* >( result.getData() );
    clock.reset();
    
    compressor->decompress( instanceUncomp,
                            nameCompressor,
                            &vectorVoid.front(),
                            &totalSize,
                            numResults, 
                            outData, 
                            inDims, 
                            flags );
    
    time = clock.getTime64();
    std::cout  << time << std::endl;
    *logFile << "<TD> " << time <<"</TD>";
    char* outData8 = reinterpret_cast< char* >( outData );

    if ( compare( outData8, dataorigine, size ) )
        *logFile << "<TD> " << "OK" <<"</TD>";
    else
        *logFile << "<TD> " << "KO" <<"</TD>";
        

}

void testCompressorFile(  std::ofstream* logFile )
{
    std::vector< uint32_t >compressorNames = 
        eq::base::CompressorDataCPU::getCompressorNames( 
                                        EQ_COMPRESSOR_DATATYPE_BYTE );

    std::vector< std::string > files;
    
    getFiles( "", files, "*.dll" );
    getFiles( "text", files, "*.txt" );
    getFiles( "", files, "*.exe" );
    getFiles( "", files, "*.so" );
    getFiles( "text", files, "*.a" );
    getFiles( "text", files, "*.dylib" );
    
 
    *logFile << "<TABLE BORDER=""1"">";
    *logFile << "<CAPTION> Statistic for compressor bytes </CAPTION>";
    *logFile << "<TR>";
    *logFile << "<TH> File </TH>";
    *logFile << "<TH> Name </TH>";
    *logFile << "<TH> Size </TH>";
    *logFile << "<TH> Compressed size </TH>";
    *logFile << "<TH> T compress [mS]</TH>";
    *logFile << "<TH> T uncompress [mS]</TH>";
    *logFile << "<TH> State</TH>";
    *logFile << "</TR>";
    std::cout.setf( std::ios::right, std::ios::adjustfield );
    std::cout.precision( 5 );
    std::cout << "File,              IMAGE,    SIZE, "
              << " COMPRESSED,     t_comp,   t_decomp" << std::endl;
    for ( std::vector< std::string >::const_iterator i = files.begin();
          i != files.end(); i++ )
    {
        eq::base::Bufferb* datas = readDataFile( *i );
        for ( std::vector< uint32_t >::const_iterator j = compressorNames.begin();
          j != compressorNames.end(); j++ )
        {
            *logFile << "<TR>";
            *logFile << "<TH> " << *i  <<"</TH>";
            *logFile << "<TD> " << getCompressorName( *j ) <<"</TD>";
            *logFile << "<TD> " << datas->getSize() <<"</TD>";

            std::cout  << std::setw(2) << *i << ", " << std::setw( 5 )
                       << getCompressorName( *j ) << ", " << std::setw(5) 
                       << datas->getSize() << ", ";
            testCompressByte( *j, reinterpret_cast<char*>(datas->getData()), 
                              datas->getSize(), logFile );
            *logFile << "</TR>";
            
        }
    }
    *logFile << "</TABLE>";
}


bool compare( const char *dst, const char *src, uint32_t nbytes )
{
    for ( uint64_t i = 0; i < nbytes; i++ )
    {
        if (*dst != *src)
        {
            std::cerr << "error data in: " << i <<std::endl;
            return false;
        }
		dst++;
		src++;
    }
    return true;
}

std::string getCompressorName( const uint32_t name )
{
    switch ( name )
    {
    case  EQ_COMPRESSOR_RLE_UNSIGNED:
        return "RLE_UNSIGNED";
    case  EQ_COMPRESSOR_RLE_BYTE:
        return "RLE_BYTE";
    case  EQ_COMPRESSOR_RLE_3_BYTE:
        return "RLE_3_BYTE";
    case  EQ_COMPRESSOR_RLE_4_BYTE:
        return "RLE_4_BYTE";
    case  EQ_COMPRESSOR_RLE_4_FLOAT:
        return "RLE_4_FLOAT";
    case  EQ_COMPRESSOR_RLE_4_HALF_FLOAT:
        return "RLE_4_HALF_FLOAT";
    case  EQ_COMPRESSOR_DIFF_RLE_3_BYTE:
        return "RLE_3_BYTE";
    case  EQ_COMPRESSOR_DIFF_RLE_4_BYTE:
        return "RLE_4_BYTE";
    case  EQ_COMPRESSOR_RLE_4_BYTE_UNSIGNED:
        return "RLE_4_BYTE_UNSIGNED";
    case  0xf0000004u:
        return "WKDM";
    case  0xf0000005u:
        return "QLZ14";
    case  0xf0000001u:
        return "QLZ15A";
    case  0xf0000002u:
        return "QLZ15B";
    case  0xf0000003u:
        return "QLZ15C";
    case EQ_COMPRESSOR_DIFF_RLE_10A2:
        return "DIFF_RLE_10A2";
    default:
        return "COMPRESSOR_NONE";
    }
}

std::vector< std::string > getFiles( std::string path, 
                                    std::vector< std::string >& files, 
                                    const std::string& ext )
{
    const eq::base::Strings& paths = eq::base::Global::getPluginDirectories();
    for ( uint64_t j = 0; j < paths.size(); j++)
    {
        eq::base::Strings candidats = eq::base::searchDirectory( paths[j], ext.c_str() );
        for( eq::base::Strings::const_iterator i = candidats.begin();
                i != candidats.end(); ++i )
        {
            const std::string& filename = *i;
            files.push_back( paths[j] + "/" + filename );
        }    
    }
    return files;
}

eq::base::Bufferb* readDataFile( const std::string& filename )
{
    eq::base::Bufferb* datas = new eq::base::Bufferb();
    std::ifstream file( filename.c_str(),  std::ios::in | std::ios::binary );
    char temp;
    
    while( !file.eof() )
    {
        file.read ((char *)&temp, sizeof(char));
        datas->append( temp );
    }

    file.close();
    return datas;
}