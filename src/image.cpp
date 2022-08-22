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
#include <QtConcurrent>

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
#include "image.h"
#include "stretch.h"

double square(double value){ return value*value;}

Image::Image() {
    ResetData();
}

Image::~Image() {
    ResetData();
}
void Image::ResetData(void) {
    delete[] m_ImageBuffer;
}
bool Image::saveStretchedToJpeg(QString filename,int compress)
{
    //CImg<uint16_t> imgtmp = img_stretched;
    //imgtmp.resize_halfXY();
    //imgtmp.resize_halfXY();
    //imgtmp.save_jpeg(filename.toStdString().c_str(),compress);
    return true;
}
bool Image::saveToJpeg(QString filename,int compress)
{

    //computeHistogram();
    //CImg<uint16_t> imgtmp = img;
    //imgtmp.resize_halfXY();
    //imgtmp.resize_halfXY();
    //imgtmp.save_jpeg(filename.toStdString().c_str(),compress);
    //int buf_size = sizeof(img.data());
    //jpegmem.load(filename.toStdString().c_str());

    return true;
}
//This method was copied and pasted and modified from the method privateLoad in fitsdata in KStars
//It loads a FITS file, reads the FITS Headers, and loads the data from the image
bool Image::LoadFromBlob(IBLOB *bp)
{
    BOOST_LOG_TRIVIAL(debug) << "Image load from blob " << bp->label << "-" << bp->name << "-" << bp->size;

    int status = 0, anynullptr = 0;
    long naxes[3];
    size_t bsize = static_cast<size_t>(bp->bloblen);
    fitsfile *fptr;  // FITS file pointer


    // Use open diskfile as it does not use extended file names which has problems opening
    // files with [ ] or ( ) in their names.
    if (fits_open_memfile(&fptr,"",READONLY,&bp->blob,&bsize,0,NULL,&status) )

    {
         BOOST_LOG_TRIVIAL(debug) << "IMG Unsupported type or read error loading FITS blob";
        return false;
    }
    else
        stats.size = bsize;

    if (fits_movabs_hdu(fptr, 1, IMAGE_HDU, &status))
    {
        BOOST_LOG_TRIVIAL(debug) <<  "Could not locate image HDU.";
        fits_close_file(fptr, &status);
        return false;
    }

    int fitsBitPix = 0;
    if (fits_get_img_param(fptr, 3, &fitsBitPix, &(stats.ndim), naxes, &status))
    {
        BOOST_LOG_TRIVIAL(debug) <<  "FITS file open error (fits_get_img_param).";
        fits_close_file(fptr, &status);
        return false;
    }

    if (stats.ndim < 2)
    {
        BOOST_LOG_TRIVIAL(debug) <<  "1D FITS images are not supported.";
        fits_close_file(fptr, &status);
        return false;
    }

    switch (fitsBitPix)
    {
        case BYTE_IMG:
            stats.dataType      = 11; //SEP_TBYTE;
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
            BOOST_LOG_TRIVIAL(debug) <<  "Bit depth " << fitsBitPix << " is not supported.";
            fits_close_file(fptr, &status);
            return false;
    }

    if (stats.ndim < 3)
        naxes[2] = 1;

    if (naxes[0] == 0 || naxes[1] == 0)
    {
        BOOST_LOG_TRIVIAL(debug) <<  "Image has invalid dimensions " << naxes[0] << "-" << naxes[1];

    }

    stats.width               = static_cast<uint16_t>(naxes[0]);
    stats.height              = static_cast<uint16_t>(naxes[1]);
    stats.channels            = static_cast<uint8_t>(naxes[2]);
    stats.samples_per_channel = stats.width * stats.height;

    m_ImageBufferSize = stats.samples_per_channel * stats.channels * static_cast<uint16_t>(stats.bytesPerPixel);
    deleteImageBuffer();
    m_ImageBuffer = new uint8_t[m_ImageBufferSize];
    if (m_ImageBuffer == nullptr)
    {
        BOOST_LOG_TRIVIAL(debug) <<  "FITSData: Not enough memory for image_buffer channel. Requested:" << m_ImageBufferSize << " bytes";
        fits_close_file(fptr, &status);
        return false;
    }

    long nelements = stats.samples_per_channel * stats.channels;

    if (fits_read_img(fptr, static_cast<uint16_t>(stats.dataType), 1, nelements, nullptr, m_ImageBuffer, &anynullptr, &status))
    {
        BOOST_LOG_TRIVIAL(debug) <<  "Error reading image.";
        fits_close_file(fptr, &status);
        return false;
    }

    /*if( !justLoadBuffer )
    {
        if(checkDebayer())
            debayer();

        getSolverOptionsFromFITS();

        parseHeader();
    }*/

    fits_close_file(fptr, &status);
    generateQImage();
    return true;
}

//This method was copied and pasted from Fitsview in KStars
//It sets up the image that will be displayed on the screen
void Image::generateQImage()
{
    int sampling = 2;
    // Account for leftover when sampling. Thus a 5-wide image sampled by 2
    // would result in a width of 3 (samples 0, 2 and 4).

    int w = (stats.width + sampling - 1) / sampling;
    int h = (stats.height + sampling - 1) / sampling;

    if (stats.channels == 1)
    {
        rawImage = QImage(w, h, QImage::Format_Indexed8);

        rawImage.setColorCount(256);
        for (int i = 0; i < 256; i++)
            rawImage.setColor(i, qRgb(i, i, i));
    }
    else
    {
        rawImage = QImage(w, h, QImage::Format_RGB32);
    }

    Stretch stretch(static_cast<int>(stats.width),
                    static_cast<int>(stats.height),
                    stats.channels, static_cast<uint16_t>(stats.dataType));

    // Compute new auto-stretch params.
    StretchParams stretchParams = stretch.computeParams(m_ImageBuffer);

    stretch.setParams(stretchParams);
    stretch.run(m_ImageBuffer, &rawImage, sampling);

}
void Image::deleteImageBuffer()
{
    if(m_ImageBuffer)
    {
        delete[] m_ImageBuffer;
        m_ImageBuffer = nullptr;
    }
}

bool Image::LoadFromFile(QString filename)
{
    BOOST_LOG_TRIVIAL(debug) << "Image load from file " << filename.toStdString();
    ResetData();
    int status = 0, anynullptr = 0;
    long naxes[3];

    fitsfile *fptr;  // FITS file pointer
    // load blob to CFITSIO

    if (fits_open_diskfile(&fptr,filename.toStdString().c_str(),READONLY,&status))
    {
        IDLog("IMG Unsupported type or read error loading FITS blob\n");
        return false;
    } else {
        stats.size =QFile(filename).size();
    }

    // Use open diskfile as it does not use extended file names which has problems opening
    // files with [ ] or ( ) in their names.
    /*QString fileToProcess;
    fileToProcess = "/home/gilles/ekos6/Light/lum/M_33_Light_lum_60_secs_2020-11-06T22-25-13_139.fits";
    if (fits_open_diskfile(&fptr,fileToProcess.toLatin1() , READONLY, &status))
    {
        IDLog("Unsupported type or read error loading FITS blob\n");
        return false;
    }
    else
        stats.size = QFile(fileToProcess).size();*/

    if (fits_movabs_hdu(fptr, 1, IMAGE_HDU, &status))
    {
        IDLog("IMG Could not locate image HDU.\n");
        fits_close_file(fptr, &status);
        return false;
    }

    int fitsBitPix = 0;
    if (fits_get_img_param(fptr, 3, &fitsBitPix, &(stats.ndim), naxes, &status))
    {
        IDLog("IMG FITS file open error (fits_get_img_param).\n");
        fits_close_file(fptr, &status);
        return false;
    }

    if (stats.ndim < 2)
    {
        IDLog("IMG 1D FITS images are not supported.\n");
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
            IDLog("IMG Bit depth %i is not supported.\n",fitsBitPix);
            fits_close_file(fptr, &status);
            return false;
    }

    if (stats.ndim < 3)
        naxes[2] = 1;

    if (naxes[0] == 0 || naxes[1] == 0)
    {
        IDLog("IMG Image has invalid dimensions %lix %li\n",naxes[0],naxes[1]);
        return false;
    }

    stats.width               = static_cast<uint16_t>(naxes[0]);
    stats.height              = static_cast<uint16_t>(naxes[1]);
    stats.channels            = static_cast<uint8_t>(naxes[2]);
    stats.samples_per_channel = stats.width * stats.height;
    delete[] m_ImageBuffer;
    m_ImageBufferSize = stats.samples_per_channel * stats.channels * static_cast<uint16_t>(stats.bytesPerPixel);
    m_ImageBuffer = new uint8_t[m_ImageBufferSize];
/*    IDLog("--------------------------\n");
    IDLog("m_ImageBufferSize %i bytes\n"        ,m_ImageBufferSize);
    IDLog("stats.samples_per_channel %i bytes\n",stats.samples_per_channel);
    IDLog("stats.channels %i bytes\n"           ,stats.channels);
    IDLog("stats.bytesPerPixel %i bytes\n"      ,stats.bytesPerPixel);
    IDLog("--------------------------\n");
*/

    if (m_ImageBuffer == nullptr)
    {
        IDLog("IMG FITSData: Not enough memory for image_buffer channel. Requested: %i bytes\n",m_ImageBufferSize);
        delete[] m_ImageBuffer;
        m_ImageBuffer = nullptr;
        fits_close_file(fptr, &status);
        return false;
    }

    long nelements = stats.samples_per_channel * stats.channels;

    //long int firstpixel =1;
    //if (fits_read_img(fptr,TUSHORT,1,nelements,nullptr,m_ImageBuffer,&anynullptr,&status))
    /*if (fits_read_pix(fptr, static_cast<uint16_t>(stats.dataType), &firstpixel, nelements, nullptr, m_ImageBuffer, &anynullptr, &status))
    {
        IDLog("Error reading imag. %i\n",status);
        fits_close_file(fptr, &status);
        return false;
    }*/
    //if (fits_read_pix(fptr, TUSHORT                              , fpixel, xsize*ysize, nullptr, ImageData, nullptr, &status))
    if (fits_read_img(fptr, static_cast<uint16_t>(stats.dataType), 1, nelements, nullptr, m_ImageBuffer, &anynullptr, &status))
    {
        IDLog("IMG Error reading imag. %i\n",status);
        fits_close_file(fptr, &status);
        return false;
    }

    fits_close_file(fptr,&status);
    // Load image into Cimg object
    //img=nullptr;
    //img.clear();
    //img.resize(stats.width ,stats.height,1,1);
    //cimg_forXY(img, x, y)
    //    {
    //        img(x, img.height() - y - 1) = (reinterpret_cast<uint16_t *>(m_ImageBuffer))[img.offset(x, y)]; // FIXME ???
    //    }

    CalcStats();
    //saveToJpeg("/home/gilles/OST/toto.jpeg");
    //FindStars();
    //IDLog("IMG readblob done %ix%i\n",stats.width ,stats.height );
    return true;
}
void Image::CalcStats(void)
{
    //stats.min[0] =img.min();
    //stats.max[0] =img.max();
    //stats.mean[0]=img.mean();
    //stats.median[0]=img.median();
    //stats.stddev[0]=sqrt(img.variance(1));
    //IDLog("IMG Min=%f Max=%f Avg=%.2f Med=%f StdDev=%.2f\n",stats.min[0],stats.max[0],stats.mean[0],stats.median[0],stats.stddev[0]);
}



void Image::computeHistogram(void)
{

/* https://github.com/HYPJUDY/histogram-equalization-on-grayscale-and-color-image  */
    //int L = 256*256; // number of grey levels used
    //int w = img._width;
    //int h = img._height;
    //int number_of_pixels = w * h;
    //histogram.clear();
    //histogram.resize(256*256, 1, 1, 1, 0);
    //cimg_forXY(img, x, y) ++histogram[img(x, y)];
    //int count = 0;
    //double cdf[256*256] = { 0 };
    //unsigned int equalized[256*256] = { 0 };
    //
    //cimg_forX(histogram, pos) { // calculate cdf and equalized transform
    //    count += histogram[pos];
    //    cdf[pos] = 1.0 * count /number_of_pixels  ;
    //    equalized[pos] = round(cdf[pos] * (L - 1));
    //    //qDebug() << "-----" << pos << "----" << equalized[pos];
    //}
    //img_stretched.clear();
    //img_stretched.resize(stats.width ,stats.height,1,1);
    //cimg_forXY(img_stretched, x, y) // calculate histogram equalization result
    //        img_stretched(x, y, 0) = equalized[img(x, y)];
    //histogram256.clear();
    //histogram256.resize(256, 1, 1, 1, 0);
    //cimg_forXY(img, x, y) histogram256[img(x, y)/255] = histogram256[img(x, y)/255] +1;

}

bool Image::saveMapToJpeg(QString filename,int compress,QList<FITSImage::Star> stars)
{

    //computeHistogram();
    int stretch=stats.width/500;
    //CImg<uint16_t> imgtmp = img;
    float fact=1/stats.max[0];

    //CImg<unsigned char> imgtmp(stats.width/stretch+1,stats.height/stretch+1,1,3,0);
    //cimg_forXY(img, x, y) {
    //    //if (img(x,y)>stats.median[0]*10) {
    //            //BOOST_LOG_TRIVIAL(debug) << x << "/" << y << "/" << img(x,y);
    //            imgtmp(x/stretch, y/stretch,0)=255*fact*img(x,y);
    //            imgtmp(x/stretch, y/stretch,1)=255*fact*img(x,y);
    //            imgtmp(x/stretch, y/stretch,2)=255*fact*img(x,y);
    //    //}
    //    //imgtmp(x/stretch, y/stretch,0)=imgtmp(x/stretch, y/stretch,0)+img(x,y)/(255*1);
    //    //imgtmp(x/stretch, y/stretch,1)=imgtmp(x/stretch, y/stretch,1)+img(x,y)/(255*1);
    //    //imgtmp(x/stretch, y/stretch,2)=imgtmp(x/stretch, y/stretch,2)+img(x,y)/(255*1);
    //}
    const unsigned char
        white[] = { 255,255,255 },
        red[]   = { 255,0,0 },
        blue [] = { 0,0,255 },
        black[] = { 0,0,0 },
        green[] = { 0,255,0 };
    //for (int i=0;i<stars.size();i++)
    //{
    //
    //    imgtmp.draw_text  (stars[i].x/stretch, imgtmp.height()-stars[i].y/stretch,
    //                    QString::number(stars[i].HFR , 'G', 3).toStdString().c_str(),
    //                    red,black,
    //                    1, 5);
    //    //imgtmp.draw_circle(stars[i].x/stretch, imgtmp.height()-stars[i].y/stretch, sqrt(stars[i].numPixels*1), blue, 1,TRUE);
    //    imgtmp.draw_ellipse(stars[i].x/stretch, imgtmp.height()-stars[i].y/stretch,
    //                       square(square(stars[i].a/stars[i].b)),
    //                       square(square(stars[i].b/stars[i].a)),
    //                       stars[i].theta,
    //                       green, 1,TRUE);
    //}
    double HFR0=0;
    double HFR1=0;
    double HFR2=0;
    double HFR3=0;
    double HFR4=0;
    int16_t n1=0;
    int16_t n2=0;
    int16_t n3=0;
    int16_t n4=0;
    for (int i=0;i<stars.size();i++)
    {
        HFR0=HFR0+stars[i].HFR;
        if ((stars[i].x < stats.width/2) && (stars[i].y < stats.height/2)) {HFR1=HFR1+stars[i].HFR;n1++;};
        if ((stars[i].x > stats.width/2) && (stars[i].y < stats.height/2)) {HFR2=HFR2+stars[i].HFR;n2++;};
        if ((stars[i].x < stats.width/2) && (stars[i].y > stats.height/2)) {HFR3=HFR3+stars[i].HFR;n3++;};
        if ((stars[i].x > stats.width/2) && (stars[i].y > stats.height/2)) {HFR4=HFR4+stars[i].HFR;n4++;};
    }
    HFR0=HFR0/stars.size();
    HFR1=HFR1/n1;
    HFR2=HFR2/n2;
    HFR3=HFR3/n3;
    HFR4=HFR4/n4;
    int mul=100;
    //int16_t x0=imgtmp.width()/2;
    //int16_t y0=imgtmp.height()/2;
    //
    //int16_t x1=1*imgtmp.width() /4-mul*(HFR1-HFR0);
    //int16_t y1=3*imgtmp.height()/4+mul*(HFR1-HFR0);
    //
    //int16_t x2=3*imgtmp.width() /4+mul*(HFR2-HFR0);
    //int16_t y2=3*imgtmp.height()/4+mul*(HFR2-HFR0);
    //
    //int16_t x3=1*imgtmp.width() /4-mul*(HFR3-HFR0);
    //int16_t y3=1*imgtmp.height()/4-mul*(HFR3-HFR0);
    //
    //int16_t x4=3*imgtmp.width() /4+mul*(HFR4-HFR0);
    //int16_t y4=1*imgtmp.height()/4-mul*(HFR4-HFR0);
    //
    //imgtmp.draw_text  (x0, y0, QString::number(HFR0 , 'G', 4).toStdString().c_str(), white,black,1, 50);
    //imgtmp.draw_text  (x1, y1, QString::number(HFR1 , 'G', 4).toStdString().c_str(), white,black,1, 50);
    //imgtmp.draw_text  (x2, y2, QString::number(HFR2 , 'G', 4).toStdString().c_str(), white,black,1, 50);
    //imgtmp.draw_text  (x3, y3, QString::number(HFR3 , 'G', 4).toStdString().c_str(), white,black,1, 50);
    //imgtmp.draw_text  (x4, y4, QString::number(HFR4 , 'G', 4).toStdString().c_str(), white,black,1, 50);
    //
    //imgtmp.draw_line(x1,y1,x2,y2,white);
    //imgtmp.draw_line(x1,y1,x3,y3,white);
    //imgtmp.draw_line(x1,y1,x4,y4,white);
    //imgtmp.draw_line(x2,y2,x3,y3,white);
    //imgtmp.draw_line(x2,y2,x4,y4,white);
    //imgtmp.draw_line(x3,y3,x4,y4,white);
    //
    //imgtmp.save_jpeg(filename.toStdString().c_str(),compress);

    return true;
}


void Image::appendStarsFound(QList<FITSImage::Star> stars)
{
    const unsigned char
        white[] = { 255,255,255 },
        red[]   = { 255,0,0 },
        blue [] = { 0,0,255 },
        black[] = { 0,0,0 },
        green[] = { 0,255,0 };
    //for (int i=0;i<stars.size();i++)
    //{
    //    img.draw_text  (stars[i].x, img.height()-stars[i].y,
    //                    QString::number(stars[i].HFR , 'G', 3).toStdString().c_str(),
    //                    white,
    //                    1, 20);
    //    img.draw_circle(stars[i].x, img.height()-stars[i].y, stars[i].numPixels*10, white, 1,TRUE);
    //}
}
