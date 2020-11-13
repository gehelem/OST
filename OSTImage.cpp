/*
 * Thanks to
 * original code can be found here :
 * https://www.lost-infinity.com/night-sky-image-processing-part-1-noise-reduction-using-anisotropic-diffusion-with-c/
 * https://github.com/carsten0x51h/astro_tools
 *
 * we might have to give a try with this one, same author :
 * https://github.com/carsten0x51h/focus_finder
 *
 *
 */
#include <basedevice.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <cmath>
#include <fitsio.h>
#include <tuple>
#include <functional>
#include <list>
#include <set>
#include <array>
#include <vector>

#include "OSTImage.h"



OSTImage::OSTImage() {
    ResetData();
}
OSTImage::~OSTImage() {
    ResetData();
}
void OSTImage::ResetData(void) {

}


bool OSTImage::LoadFromBlob(IBLOB *bp)
{
    IDLog("readblob %s %s %i \n",bp->label,bp->name,bp->size);
    ResetData();

    fitsfile *fptr;  // FITS file pointer
    int status = 0, anynullptr = 0;
    int hdutype, naxis;
    int nhdus=0;
    long fits_size[2];
    long fpixel[3] = {1,1,1};
    long naxes[3];
    size_t bsize = static_cast<size_t>(bp->bloblen);
    // load blob to CFITSIO
    if (fits_open_memfile(&fptr,"",READONLY,&(bp->blob),&bsize,0,NULL,&status) )
    {
        IDLog("Unsupported type or read error loading FITS blob\n");
        return false;
    }
    if (fits_get_hdu_type(fptr, &hdutype, &status) || hdutype != IMAGE_HDU) {
        IDLog("FITS file is not of an image\n");
        fits_close_file(fptr,&status);
        return false;
    }
    // Get HDUs and size
    fits_get_img_dim(fptr, &naxis, &status);
    fits_get_img_size(fptr, 2, fits_size, &status);
    stats.width = (int) fits_size[0];
    stats.height= (int) fits_size[1];
    stats.samples_per_channel = stats.width * stats.height;
    fits_get_num_hdus(fptr,&nhdus,&status);
    if ((nhdus != 1) || (naxis != 2)) {
        IDLog("Unsupported type or read error loading FITS file\n");
        fits_close_file(fptr,&status);
        return false;
    }
    int fitsBitPix = 0;
    if (fits_get_img_param(fptr, 3, &fitsBitPix, &(stats.ndim), naxes, &status))
    {
        IDLog("FITS file open error (fits_get_img_param)\n");
        fits_close_file(fptr, &status);
        return false;
    }

    if (stats.ndim < 2)
    {
        IDLog("1D FITS images are not supported.\n");
        fits_close_file(fptr, &status);
        return false;
    }
    switch (fitsBitPix)
    {
        case BYTE_IMG:
            stats.dataType      = TBYTE;
            stats.bytesPerPixel = sizeof(uint8_t);
            break;
        case SHORT_IMG:
            // Read SHORT image as USHORT
            stats.dataType      = TUSHORT;
            stats.bytesPerPixel = sizeof(int16_t);
            break;
        case USHORT_IMG:
            stats.dataType      = TUSHORT;
            stats.bytesPerPixel = sizeof(uint16_t);
            break;
        case LONG_IMG:
            // Read LONG image as ULONG
            stats.dataType      = TULONG;
            stats.bytesPerPixel = sizeof(int32_t);
            break;
        case ULONG_IMG:
            stats.dataType      = TULONG;
            stats.bytesPerPixel = sizeof(uint32_t);
            break;
        case FLOAT_IMG:
            stats.dataType      = TFLOAT;
            stats.bytesPerPixel = sizeof(float);
            break;
        case LONGLONG_IMG:
            stats.dataType      = TLONGLONG;
            stats.bytesPerPixel = sizeof(int64_t);
            break;
        case DOUBLE_IMG:
            stats.dataType      = TDOUBLE;
            stats.bytesPerPixel = sizeof(double);
            break;
        default:
            IDLog("Bit depth %i is not supported.\n",fitsBitPix);
            fits_close_file(fptr,&status);
            return false;
    }


    //clearImageBuffers();
    delete[] m_ImageBuffer;
    m_ImageBuffer = nullptr;
    //m_BayerBuffer = nullptr;

    //m_Channels = static_cast<uint8_t>(naxes[2]);
    m_Channels = 16;
    m_ImageBufferSize = stats.samples_per_channel * m_Channels * static_cast<uint16_t>(stats.bytesPerPixel);
    m_ImageBuffer = new uint8_t[m_ImageBufferSize];
    if (m_ImageBuffer == nullptr)
    {
        IDLog("FITSData: Not enough memory for image_buffer channel. Requested: %i bytes\n",m_ImageBufferSize);
        //clearImageBuffers();
        delete[] m_ImageBuffer;
        m_ImageBuffer = nullptr;
        //m_BayerBuffer = nullptr;
        fits_close_file(fptr, &status);
        return false;
    }

    //long nelements = stats.samples_per_channel * m_Channels;

    /*rawdata = new unsigned short[stats.width *stats.height ];
    if (fits_read_pix(fptr, TUSHORT                              , fpixel, stats.width*stats.height , NULL, rawdata, NULL, &status) )
    if (fits_read_img(fptr, static_cast<uint16_t>(stats.dataType), 1,      stats.width*stats.height, nullptr, m_ImageBuffer, &anynullptr, &status))
    {
        IDLog("Error reading data\n");
        fits_close_file(fptr,&status);
        return false;
    }*/

    //if (fits_read_img(fptr,TUSHORT,1,nelements,nullptr,m_ImageBuffer,&anynullptr,&status))
    if (fits_read_pix(fptr, static_cast<uint16_t>(stats.dataType), fpixel, stats.width*stats.height, nullptr, m_ImageBuffer, &anynullptr, &status))
    {
        IDLog("Error reading imag. %i\n",status);
        //IDLog("Error reading imag.\n");
        fits_close_file(fptr, &status);
        return false;
    }


    fits_close_file(fptr,&status);
    CalcStats();
    FindStars();
    IDLog("readblob done %ix%i\n",stats.width ,stats.height );
    return true;
}

void OSTImage::CalcStats(void)
{
    /*stats.min =65535;
    stats.max =0;
    stats.mean=0;
    stats.median=0;
    long n;
    for (int i=0;i<stats.width ;i++) {
        for (int j=0;j<stats.height ;j++) {
            n=j*stats.width  +i;
            stats.mean=(n*stats.mean+rawdata[n]);
            stats.mean=stats.mean/(n+1);
            //if (rawdata[j*stats.width  +i] > 1000 )IDLog("%i %i %i\n",i,j,rawdata[j*stats.width  +i]);
            if (rawdata[n] > stats.max ) stats.max = rawdata[n];
            if (rawdata[n] < stats.min ) stats.min = rawdata[n];
        }
    }*/
    //IDLog("Min=%i Max=%i Avg=%.2f Med=%i\n",stats.min,stats.max,stats.mean,stats.median);
}

void OSTImage::FindStars(void)
{

    //IDLog("before\n");
    stellarSolver.reset(new StellarSolver(stats, m_ImageBuffer));
    //stellarSolver = new StellarSolver(stats, m_ImageBuffer);
    //stellarSolver->setParent(this->parent());
    //stellarSolver->moveToThread(this->thread());
    //IDLog("after\n");
    connect(stellarSolver.get(),&StellarSolver::logOutput,this,&OSTImage::sslogOutput);
    connect(stellarSolver.get(),&StellarSolver::ready,this,&OSTImage::ssReady);
    //stellarSolver->setParent(this->parent());
    stellarSolver->setLogLevel(LOG_ALL);
    stellarSolver->setSSLogLevel(LOG_VERBOSE);
    printf("SS version = %s\n",stellarSolver->getVersion().toUtf8().data());
    //stellarSolver.m_ExtractorType
    /*typedef enum { EXTRACT,            //This just sextracts the sources
                   EXTRACT_WITH_HFR,   //This sextracts the sources and finds the HFR
                   SOLVE                //This solves the image
                 } ProcessType;

    typedef enum { EXTRACTOR_INTERNAL, //This uses internal SEP to Sextract Sources
                   EXTRACTOR_EXTERNAL,  //This uses the external sextractor to Sextract Sources.
                   EXTRACTOR_BUILTIN  //This uses whatever default sextraction method the selected solver uses
                 } ExtractorType;

    typedef enum { SOLVER_STELLARSOLVER,    //This uses the internal build of astrometry.net
                   SOLVER_LOCALASTROMETRY,  //This uses an astrometry.net or ANSVR locally on this computer
                   SOLVER_ASTAP,            //This uses a local installation of ASTAP
                   SOLVER_ONLINEASTROMETRY  //This uses the online astrometry.net or ASTAP
                 } SolverType;    */
    stellarSolver->setProperty("ProcessType",EXTRACT);
    stellarSolver->setProperty("ExtractorType",EXTRACTOR_INTERNAL);
    stellarSolver->setProperty("SolverType",SOLVER_STELLARSOLVER);
    stellarSolver->setParameterProfile(SSolver::Parameters::ALL_STARS);
    stellarSolver->clearSubFrame();

    stellarSolver->start();
    IDLog("stellarSolver Start\n");
/*    while(!stellarSolver.isNull() && !stellarSolver->finished();)
    {
        usleep(1000000);
        stars = stellarSolver->getStarList();
        IDLog("got %i stars\n",stellarSolver->getStarList().size());
        //IDLog(".");

    }
    IDLog("\n");
    usleep(5000000);
    stars = stellarSolver->getStarList();
    IDLog("got %i stars\n",stellarSolver->getStarList().size());*/

}

void OSTImage::sslogOutput(QString text)
{
    printf("logoutpout\n");
    IDLog("%s\n",text.toUtf8().data());
}
void OSTImage::ssReady(void)
{
    printf("ssready\n");
    IDLog("stellarSolver ready\n");
    stars = stellarSolver->getStarList();
    //IDLog("got %i stars\n",stellarSolver->getStarList().size());
    IDLog("ssready\n");

}

