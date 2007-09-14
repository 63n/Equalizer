
#include "rawConverter.h"
#include "rawVolModel.h"
#include <iostream>
#include <fstream>
#include "hlp.h"

namespace eVolve
{
    
using hlpFuncs::clip;


void getHeaderParameters( const string& fileName, uint32_t &w, uint32_t &h, uint32_t &d, vector<uint8_t> &TF );
void CreateTransferFunc( int t, uint8_t *transfer );

void lFailed( char* msg ) { EQERROR << msg << endl; }

void RawConverter::ConvertRawToRawPlusDerivatives( const string& src, const string& dst )
{
    uint32_t w, h, d;
//read header
    {
        string configFileName = src;
        hFile info( fopen( configFileName.append( ".vhf" ).c_str(), "rb" ) );
        FILE* file = info.f;

        if( file==NULL ) return lFailed( "Can't open header file" );

        fscanf( file, "w=%u\n", &w );
        fscanf( file, "h=%u\n", &h );
        fscanf( file, "d=%u\n", &d );
    }
    EQWARN << "Creating derivatives for raw model: " << src << " " << w << " x " << h << " x " << d << endl;

//read model    
    vector<uint8_t> volume( w*h*d, 0 );

    EQWARN << "Reading model" << endl;
    {
        ifstream file( src.c_str(), ifstream::in | ifstream::binary | ifstream::ate );

        if( !file.is_open() )
            return lFailed( "Can't open volume file" );

        ifstream::pos_type size;

        size = min( (int)file.tellg(), (int)volume.size() );

        file.seekg( 0, ios::beg );
        file.read( (char*)( &volume[0] ), size );    

        file.close();
    }
    
//calculate and save derivatives
    {
        EQWARN << "Calculating derivatives" << endl;
        ofstream file ( dst.c_str(), ifstream::out | ifstream::binary | ifstream::trunc );
   
        if( !file.is_open() )
            return lFailed( "Can't open destination volume file" );

        int wh = w*h;

        vector<uint8_t> GxGyGzA( wh*d*4, 0 );
    
        for( uint32_t z=1; z<d-1; z++ )
        {
            int zwh = z*wh;

            const uint8_t *curPz = &volume[0] + zwh;

            for( uint32_t y=1; y<h-1; y++ )
            {
                int zwh_y = zwh + y*w;
                const uint8_t * curPy = curPz + y*w ;
                for( uint32_t x=1; x<w-1; x++ )
                {
                    const uint8_t * curP = curPy +  x;
                    const uint8_t * prvP = curP  - wh;
                    const uint8_t * nxtP = curP  + wh;
                    int32_t gx = 
                          nxtP[  1+w ]+ 3*curP[  1+w ]+   prvP[  1+w ]+
                        3*nxtP[  1   ]+ 6*curP[  1   ]+ 3*prvP[  1   ]+
                          nxtP[  1-w ]+ 3*curP[  1-w ]+   prvP[  1-w ]-

                          nxtP[ -1+w ]- 3*curP[ -1+w ]-   prvP[ -1+w ]-
                        3*nxtP[ -1   ]- 6*curP[ -1   ]- 3*prvP[ -1   ]-
                          nxtP[ -1-w ]- 3*curP[ -1-w ]-   prvP[ -1-w ];

                    int32_t gy = 
                          nxtP[  1+w ]+ 3*curP[  1+w ]+   prvP[  1+w ]+
                        3*nxtP[    w ]+ 6*curP[    w ]+ 3*prvP[    w ]+
                          nxtP[ -1+w ]+ 3*curP[ -1+w ]+   prvP[ -1+w ]-

                           nxtP[  1-w ]- 3*curP[  1-w ]-   prvP[  1-w ]-
                        3*nxtP[   -w ]- 6*curP[   -w ]- 3*prvP[   -w ]-
                          nxtP[ -1-w ]- 3*curP[ -1-w ]-   prvP[ -1-w ];

                    int32_t gz = 
                          prvP[  1+w ]+ 3*prvP[  1   ]+   prvP[  1-w ]+
                        3*prvP[    w ]+ 6*prvP[  0   ]+ 3*prvP[   -w ]+
                          prvP[ -1+w ]+ 3*prvP[ -1   ]+   prvP[ -1-w ]-

                          nxtP[  1+w ]- 3*nxtP[  1   ]-   nxtP[  1-w ]-
                        3*nxtP[   +w ]- 6*nxtP[  0   ]- 3*nxtP[   -w ]-
                          nxtP[ -1+w ]- 3*nxtP[ -1   ]-   nxtP[ -1-w ];


                    int32_t length = static_cast<int32_t>(sqrt( (gx*gx+gy*gy+gz*gz) )+1);

                    gx = ( gx*255/length + 255 )/2; 
                    gy = ( gy*255/length + 255 )/2;
                    gz = ( gz*255/length + 255 )/2;

                    GxGyGzA[(zwh_y + x)*4   ] = static_cast<uint8_t>( gx );
                    GxGyGzA[(zwh_y + x)*4 +1] = static_cast<uint8_t>( gy );
                    GxGyGzA[(zwh_y + x)*4 +2] = static_cast<uint8_t>( gz );
                    GxGyGzA[(zwh_y + x)*4 +3] = curP[0];
                }
            }
        }
        
        EQWARN << "Writing derivatives: " << dst.c_str() << " " << GxGyGzA.size() << " bytes" <<endl;
        file.write( (char*)( &GxGyGzA[0] ), GxGyGzA.size() );
        
        file.close();
    }
}

void RawConverter::SavToVhfConverter( const string& src, const string& dst )
{
    //read original header
    uint32_t w=1;
    uint32_t h=1;
    uint32_t d=1;
    float wScale=1.0;
    float hScale=1.0;
    float dScale=1.0;
    {
        hFile info( fopen( dst.c_str(), "rb" ) );
        FILE* file = info.f;
    
        if( file!=NULL )
        {
            fscanf( file, "w=%u\n", &w );
            fscanf( file, "h=%u\n", &h );
            fscanf( file, "d=%u\n", &d );

            fscanf( file, "wScale=%g\n", &wScale );
            fscanf( file, "hScale=%g\n", &hScale );
            fscanf( file, "dScale=%g\n", &dScale );
        }
    }
    
    //read sav
    int TFSize = 0;
    vector< uint8_t > TF( 256*4, 0 );
    
    if( true )
    {    
        hFile info( fopen( src.c_str(), "rb" ) );
        FILE* file = info.f;
    
        if( file==NULL ) return lFailed( "Can't open source sav file" );
    
        float t;
        int   ti;
        float tra;
        float tga;
        float tba;
        fscanf( file, "2DTF:\n"          );
        fscanf( file, "num=%d\n"    , &ti);
        fscanf( file, "mode=%d\n"   , &ti);
        fscanf( file, "rescale=%f\n", &t );
        fscanf( file, "gescale=%f\n", &t );
        fscanf( file, "bescale=%f\n", &t );
        fscanf( file, "rascale=%f\n", &t );
        fscanf( file, "gascale=%f\n", &t );
        fscanf( file, "bascale=%f\n", &t );
        fscanf( file, "TF:\n"            );
        fscanf( file, "res=%d\n",&TFSize );
        fscanf( file, "rescale=%f\n", &t );
        fscanf( file, "gescale=%f\n", &t );
        fscanf( file, "bescale=%f\n", &t );
        fscanf( file, "rascale=%f\n", &t );
        fscanf( file, "gascale=%f\n", &t );
        if( fscanf( file, "bascale=%f\n", &t ) != 1)
            return lFailed( "failed to read header of sav file" );

        if( TFSize!=256  )  return lFailed( "Wrong size of transfer function, should be 256" );
        TF.resize( TFSize*4 );
        
        for( int i=0; i<TFSize; i++ )
        {
            fscanf( file, "re=%f\n", &t   ); TF[4*i  ] = clip( static_cast<int32_t>( t*255.0 ), 0, 255 );
            fscanf( file, "ge=%f\n", &t   ); TF[4*i+1] = clip( static_cast<int32_t>( t*255.0 ), 0, 255 );
            fscanf( file, "be=%f\n", &t   ); TF[4*i+2] = clip( static_cast<int32_t>( t*255.0 ), 0, 255 );
            fscanf( file, "ra=%f\n", &tra );    
            fscanf( file, "ga=%f\n", &tba );
            if( fscanf( file, "ba=%f\n", &tga ) !=1 )
            {
                EQERROR << "Failed to read entity #" << i << " of sav file" << endl;
                return;
            }
            TF[4*i+3] = clip( static_cast<int32_t>( (tra+tga+tba)*255.0/3.0 ), 0, 255 );
        }        
    }else
    {   //predefined transfer functions and parameters
         TFSize = 256;
        TF.resize( TFSize*4 );
        getHeaderParameters( src, w, h, d, TF );
    }
    
    //write vhf
    {
        hFile info( fopen( dst.c_str(), "wb" ) );
        FILE* file = info.f;
    
        if( file==NULL ) return lFailed( "Can't open destination header file" );
    
        fprintf( file, "w=%u\n", w );
        fprintf( file, "h=%u\n", h );
        fprintf( file, "d=%u\n", d );

        fprintf( file, "wScale=%g\n", wScale );
        fprintf( file, "hScale=%g\n", hScale );
        fprintf( file, "dScale=%g\n", dScale );
    
        fprintf( file,"TF:\n" );
    
        fprintf( file, "size=%d\n", TFSize );
    
        int tmp;
        for( int i=0; i<TFSize; i++ )
        {
            tmp = TF[4*i  ]; fprintf( file, "r=%d\n", tmp );
            tmp = TF[4*i+1]; fprintf( file, "g=%d\n", tmp );
            tmp = TF[4*i+2]; fprintf( file, "b=%d\n", tmp );
            tmp = TF[4*i+3]; fprintf( file, "a=%d\n", tmp );
        }
    }
    EQWARN << "file " << src.c_str() << " > " << dst.c_str() << " converted" << endl;
}

void RawConverter::DscToVhfConverter( const string& src, const string& dst )
{
    EQWARN << "converting " << src.c_str() << " > " << dst.c_str() << " .. ";
//Read Description file
    uint32_t w=1;
    uint32_t h=1;
    uint32_t d=1;
    float wScale=1.0;
    float hScale=1.0;
    float dScale=1.0;
    {
        hFile info( fopen( src.c_str(), "rb" ) );
        FILE* file = info.f;
    
        if( file==NULL ) return lFailed( "Can't open source Dsc file" );
    
        if( fscanf( file, "reading PVM file\n" ) != 0 ) 
            return lFailed( "Not a proper file format, first line should be:\nreading PVM file" );
            
        uint32_t c=0;
        if( fscanf( file, "found volume with width=%u height=%u depth=%u components=%u\n", &w, &h, &d, &c ) != 4 )
            return lFailed( "Not a proper file format, second line should be:\nfound volume with width=<num> height=<num> depth=<num> components=<num>" );
        if( c!=1 )
            return lFailed( "'components' should be equal to '1', only 8 bit volumes supported so far" );
        
        fscanf( file, "and edge length %g/%g/%g\n", &wScale, &hScale, &dScale ); 
    }
//Write Vhf file
    {
        hFile info( fopen( dst.c_str(), "wb" ) );
        FILE* file = info.f;
    
        if( file==NULL ) return lFailed( "Can't open destination header file" );
    
        fprintf( file, "w=%u\n", w );
        fprintf( file, "h=%u\n", h );
        fprintf( file, "d=%u\n", d );

        fprintf( file, "wScale=%g\n", wScale );
        fprintf( file, "hScale=%g\n", hScale );
        fprintf( file, "dScale=%g\n", dScale );
    }
    EQWARN << "succeed" << endl;
}

void getHeaderParameters( const string& fileName, uint32_t &w, uint32_t &h, uint32_t &d, vector<uint8_t> &TF )
{
    int t=w=h=d=0;

    if( fileName.find( "spheres128x128x128"    , 0 ) != string::npos )
        t=0, w=h=d=128;
    
    if( fileName.find( "fuel"                , 0 ) != string::npos )
        t=1, w=h=d=64;
        
    if( fileName.find( "neghip"                , 0 ) != string::npos )
        t=2, w=h=d=64;
        
    if( fileName.find( "Bucky32x32x32"        , 0 ) != string::npos )
        t=3, w=h=d=32;
        
    if( fileName.find( "hydrogen"            , 0 ) != string::npos )
        t=4, w=h=d=128;
        
    if( fileName.find( "Engine256x256x256"    , 0 ) != string::npos )
        t=5, w=h=d=256;
        
    if( fileName.find( "skull"                , 0 ) != string::npos )
        t=6, w=h=d=256;
        
    if( fileName.find( "vertebra8"            , 0 ) != string::npos )
        t=7, w=h=512, d=256;
    
    if( w!=0 )
        CreateTransferFunc( t, &TF[0] );
}


void CreateTransferFunc( int t, uint8_t *transfer )
{
    memset( transfer, 0, 256*4 );

    int i;
    switch(t)
    {
        case 0: 
//spheres    
        EQWARN << "transfer: spheres" << endl;
            for (i=40; i<255; i++) {
                transfer[(i*4)]   = 115;
                transfer[(i*4)+1] = 186;
                transfer[(i*4)+2] = 108;
                transfer[(i*4)+3] = 255;
            }
            break;
    
        case 1:
// fuel    
            EQWARN << "transfer: fuel" << endl;
            for (i=0; i<65; i++) {
                transfer[(i*4)] = 255;
                transfer[(i*4)+1] = 255;
                transfer[(i*4)+2] = 255;
            }
            for (i=65; i<193; i++) {
                transfer[(i*4)]  =  0;
                transfer[(i*4)+1] = 160;
                transfer[(i*4)+2] = transfer[(192-i)*4];
            }
            for (i=193; i<255; i++) {
                transfer[(i*4)]  =  0;
                transfer[(i*4)+1] = 0;
                transfer[(i*4)+2] = 255;
            }
    
            for (i=2; i<80; i++) {
                transfer[(i*4)+3] = (unsigned char)((i-2)*56/(80-2));
            }    
            for (i=80; i<255; i++) {
                transfer[(i*4)+3] = 128;
            }
            for (i=200; i<255; i++) {
                transfer[(i*4)+3] = 255;
            }
            break;
            
        case 2: 
//neghip    
        EQWARN << "transfer: neghip" << endl;
            for (i=0; i<65; i++) {
                transfer[(i*4)] = 255;
                transfer[(i*4)+1] = 0;
                transfer[(i*4)+2] = 0;
            }
            for (i=65; i<193; i++) {
                transfer[(i*4)]  =  0;
                transfer[(i*4)+1] = 160;
                transfer[(i*4)+2] = transfer[(192-i)*4];
            }
            for (i=193; i<255; i++) {
                transfer[(i*4)]  =  0;
                transfer[(i*4)+1] = 0;
                transfer[(i*4)+2] = 255;
            }
    
            for (i=2; i<80; i++) {
                transfer[(i*4)+3] = (unsigned char)((i-2)*36/(80-2));
            }    
            for (i=80; i<255; i++) {
                transfer[(i*4)+3] = 128;
            }
            break;
    
        case 3: 
//bucky
        EQWARN << "transfer: bucky" << endl;
            
            for (i=20; i<99; i++) {
                transfer[(i*4)]  =  200;
                transfer[(i*4)+1] = 200;
                transfer[(i*4)+2] = 200;
            }
            for (i=20; i<49; i++) {
                transfer[(i*4)+3] = 10;
            }
            for (i=50; i<99; i++) {
                transfer[(i*4)+3] = 20;
            }
    
            for (i=100; i<255; i++) {
                transfer[(i*4)]  =  93;
                transfer[(i*4)+1] = 163;
                transfer[(i*4)+2] = 80;
            }

            for (i=100; i<255; i++) {
                transfer[(i*4)+3] = 255;
            }
            break;
    
        case 4: 
//hydrogen    
        EQWARN << "transfer: hydrogen" << endl;
            for (i=4; i<20; i++) {
                transfer[(i*4)] = 137;
                transfer[(i*4)+1] = 187;
                transfer[(i*4)+2] = 188;
                transfer[(i*4)+3] = 5;
            }

            for (i=20; i<255; i++) {
                transfer[(i*4)]  =  115;
                transfer[(i*4)+1] = 186;
                transfer[(i*4)+2] = 108;
                transfer[(i*4)+3] = 250;
            }
            break;
    
        case 5: 
//engine    
        EQWARN << "transfer: engine" << endl;
            for (i=100; i<200; i++) {
                transfer[(i*4)] =     44;
                transfer[(i*4)+1] = 44;
                transfer[(i*4)+2] = 44;
            }
            for (i=200; i<255; i++) {
                transfer[(i*4)]  =  173;
                transfer[(i*4)+1] = 22;
                transfer[(i*4)+2] = 22;
            }
            // opacity

            for (i=100; i<200; i++) {
                transfer[(i*4)+3] = 8;
            }
            for (i=200; i<255; i++) {
                transfer[(i*4)+3] = 255;
            }
            break;
    
        case 6: 
//skull
        EQWARN << "transfer: skull" << endl;
            for (i=40; i<255; i++) {
                transfer[(i*4)]  =  128;
                transfer[(i*4)+1] = 128;
                transfer[(i*4)+2] = 128;
                transfer[(i*4)+3] = 128;
            }
            break;
    
        case 7: 
//vertebra
        EQWARN << "transfer: vertebra" << endl;
            for (i=40; i<255; i++) {
                int k = i*2;
                if (k > 255) k = 255;
                transfer[(i*4)]  =  k;
                transfer[(i*4)+1] = k;
                transfer[(i*4)+2] = k;
                transfer[(i*4)+3] = k;
            }    
            break;
    }
}


}


