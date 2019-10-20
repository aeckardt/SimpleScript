#include "image.h"

#include <algorithm>

// this include is used to enable screenshots
#include <ApplicationServices/ApplicationServices.h>

/////////////////////////////////////////////////////////////////////////////
//
// Image implementation
//
/////////////////////////////////////////////////////////////////////////////

Image::Image()
{
}

Image::Image(const Image& src)
{
    *this = src;
}

Image::Image(const char* filename)
{
    load(filename);
}

Image::~Image()
{
}

bool Image::capture()
{
    CGDirectDisplayID main_display = CGMainDisplayID();
    
    CGImageRef image_ref = CGDisplayCreateImage(main_display);
    CGDataProviderRef provider = CGImageGetDataProvider(image_ref);
    CFDataRef dataref = CGDataProviderCopyData(provider);
    
    int width = static_cast<int>(CGImageGetWidth(image_ref));
    int height = static_cast<int>(CGImageGetHeight(image_ref));
    
    if (width == 0 || height == 0)
    {
        return false;
    }
    
    int bpp = static_cast<int>(CGImageGetBitsPerPixel(image_ref) >> 3);
    int bpr = static_cast<int>(CGImageGetBytesPerRow(image_ref));
    
    resize(width, height);

    writeBuf(CFDataGetBytePtr(dataref), bpp, bpr);
    
    CFRelease(dataref);
    CGImageRelease(image_ref);
    
    return true;
}

bool Image::captureRegion(int left, int top, int width, int height)
{
    CGDirectDisplayID main_display = CGMainDisplayID();
    CGImageRef image_ref = CGDisplayCreateImageForRect(main_display, CGRectMake(left, top, width, height));
    
    int _width = static_cast<int>(CGImageGetWidth(image_ref));
    int _height = static_cast<int>(CGImageGetHeight(image_ref));

    if (_width == 0 || _height == 0)
    {
        return false;
    }
    
    CGDataProviderRef provider = CGImageGetDataProvider(image_ref);
    CFDataRef dataref = CGDataProviderCopyData(provider);
    
    int bpp = static_cast<int>(CGImageGetBitsPerPixel(image_ref) >> 3);
    int bpr = static_cast<int>(CGImageGetBytesPerRow(image_ref));
    
    resize(_width, _height);
    
    writeBuf(CFDataGetBytePtr(dataref), bpp, bpr);
    
    CFRelease(dataref);
    CGImageRelease(image_ref);
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////
//
// matchTemplate
//
/////////////////////////////////////////////////////////////////////////////
//
// Parameters
//
//   templ: template image
//   pt: location of template in image
//   max_val: accuracy of best match (range 0.0 - 1.0)
//
// Description
//
//   searches through the image and matches template (using openCV)
//   best match location and best match accuracy are returned
//
/////////////////////////////////////////////////////////////////////////////

void Image::matchTemplate(const Image& templ, cv::Point& pt, double& max_val) const
{
    // Create the result matrix
    int result_cols = width() - templ.width() + 1;
    int result_rows = height() - templ.height() + 1;
    
    if (result_cols < 1 || result_rows < 1)
    {
        max_val = 0;
        return;
    }
    
    if (mat.empty() || templ.mat.empty())
    {
        std::cout << "Error: No image loaded" << std::endl;
        return;
    }
    
    cv::Mat conv;
    conv.create(result_rows, result_cols, CV_8UC1);
    
    // Do the Matching and Normalize
    cv::matchTemplate(mat, templ.mat, conv, CV_TM_CCOEFF_NORMED);
    
    // Localizing the best match with minMaxLoc
    double min_val;
    cv::Point min_loc; cv::Point max_loc;
    
    cv::minMaxLoc(conv, &min_val, &max_val, &min_loc, &max_loc, cv::Mat());
    
#ifdef MATCHTEMPLATE_DISPLAY_RESULT
    
    std::cout << "minVal = " << minVal << ", maxVal = " << maxVal << std::endl;
    
    std::string image_window = "Image Window";
    std::string result_window = "Result Window";
    
    cv::namedWindow( image_window, CV_WINDOW_NORMAL );
    cv::namedWindow( result_window, CV_WINDOW_NORMAL );
    
    /// Show me what you got
    cv::rectangle( mat, maxLoc, cv::Point( maxLoc.x + templ.width() , maxLoc.y + templ.height() ), cv::Scalar::all(0), 2, 8, 0 );
    
    cv::imshow( image_window, mat );
    cv::imshow( result_window, conv );
    
    cv::waitKey(0);
    
#endif
    
    pt = max_loc;
    
#if defined DEBUG && defined PRINT_LOG
    std::cout << "max_loc: (" << pt.x << ", " << pt.y << ") with " << max_val << " parity" << std::endl;
#endif
}

double Image::match(const Image& img, double min_val, uint32_t max_diff) const
{
    if (img.width() != width() || img.height() != height())
    {
        return 0;
    }
    
    return matchAt(0, 0, img, min_val, max_diff);
}

double Image::matchAt(int x, int y, const Image& templ, double min_val, uint32_t max_diff) const
{
    int pixels = templ.width() * templ.height();
    
    if (pixels == 0)
    {
        return 1;
    }

    int nonmatch = 0;
    int max_nonmatch = static_cast<int>(static_cast<double>(pixels) * (1 - min_val));
    
    int i, j;
    
    for (j = 0; j < templ.height(); j++)
    {
        for (i = 0; i < templ.width(); i++)
        {
            if (operator()(x + i, y + j) - templ(i, j) > max_diff)
            {
                nonmatch++;
                if (nonmatch > max_nonmatch)
                {
                    return 0;
                }
            }
        }
    }
    
    return static_cast<double>(pixels - nonmatch) / pixels;
}

void Image::initWindow(std::string imgwnd)
{
    cv::namedWindow(imgwnd, CV_WINDOW_NORMAL);
}

void Image::destroyWindow(std::string imgwnd)
{
    cv::destroyWindow(imgwnd);
}

void Image::display(std::string imgwnd, int x, int y, int milliseconds) const
{
    cv::imshow(imgwnd, mat);
    cv::resizeWindow(imgwnd, width(), height() + 22);
    cv::moveWindow(imgwnd, x, y);

    cv::waitKey(milliseconds); 
}

void Image::load(const char* filename)
{
#if MATRIX_TYPE == CV_8UC3
    mat = cv::imread(filename, CV_LOAD_IMAGE_COLOR);
#elif MATRIX_TYPE == CV_8UC4
    cv::Mat temp = cv::imread(filename, CV_LOAD_IMAGE_ANYCOLOR);
    if (temp.channels() == MATRIX_CHANNELS)
    {
        mat = temp;
    }
    else
    {
        cv::cvtColor(temp, mat, CV_BGR2BGRA);
    }
#endif
}

void Image::save(const char* filename) const
{
#if MATRIX_TYPE == CV_8UC3
    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
    
    imwrite(filename, mat, compression_params);
#elif MATRIX_TYPE == CV_8UC4
    cv::Mat temp;
    cv::cvtColor(mat, temp, CV_BGRA2BGR);
    
    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
    
    imwrite(filename, temp, compression_params);
#endif
}

void Image::resize(int width, int height)
{
    if (this->width() == width && this->height() == height)
    {
        // nothing to do
        return;
    }
    
    mat.create(height, width, MATRIX_TYPE);
}

void Image::copyRect(Image& dest, int left, int top, int dest_left, int dest_top, int width, int height) const
{
    for (int line = 0; line < height; line++)
    {
        const uchar* psrc  = mat.ptr(top + line)           + left      * MATRIX_CHANNELS;
        uchar*       pdest = dest.mat.ptr(dest_top + line) + dest_left * MATRIX_CHANNELS;
        memcpy(pdest, psrc, static_cast<size_t>(width) * MATRIX_CHANNELS);
    }
}

void Image::extract(Image& dest, int left, int top, int width, int height) const
{
    dest.resize(width, height);
    copyRect(dest, left, top, 0, 0, width, height);
}

/////////////////////////////////////////////////////////////////////////////
//
// writeBuf
//
/////////////////////////////////////////////////////////////////////////////
//
// Parameters
//
//   buf: Buffer
//   bpp: Bytes per Pixel
//   bpr: Bytes per Row
//
// Description
//
//   copies buffer into local variable mat
//   bpp can differ from previously created matrix channels
//   in that case all bytes have to be written "by hand"
//
/////////////////////////////////////////////////////////////////////////////

void Image::writeBuf(const uchar* buf, int bpp, int bpr)
{
    if (bpp == MATRIX_CHANNELS)
    {
        int y;
        for (y = 0; y < height(); y++)
        {
            // copy buffer linewise
            memcpy(mat.ptr(y), buf + y * bpr, static_cast<size_t>(width()) * MATRIX_CHANNELS);
        }
    }
    else
    {
        int x, y;
        uint8_t const* addr;
        for (y = 0; y < height(); y++)
        {
            addr = buf + bpr * y;
            for (x = 0; x < width(); x++)
            {
                operator()(x, y) = *reinterpret_cast<const ColorType*>(addr);
                addr += bpp;
            }
        }
    }
}

bool operator==(const Image& img1, const Image& img2)
{
    if (img1.width() != img2.width() || img1.height() != img2.height())
    {
        return false;
    }
    
    int i, j;
    for (j = 0; j < img1.height(); j++)
    {
        for (i = 0; i < img1.width(); i++)
        {
            if (img1(i, j) != img2(i, j))
            {
                return false;
            }
        }
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////
//
// ImageArray implementation
//
/////////////////////////////////////////////////////////////////////////////

ImageArray::ImageArray()
{
}

ImageArray::ImageArray(const char* filename, int width)
{
    load(filename, width);
}

int ImageArray::bestMatch(const Image& img, double min_threshold) const
{
    if (empty())
    {
        return -1;
    }
    
    double max_val = 0;
    int best_match = -1;
    
    cv::Point pt;
    double val;
    
    size_t k;
    for (k = 0; k < size(); k++)
    {
        img.matchTemplate(images[k], pt, val);

#if defined DEBUG && defined PRINT_LOG
        std::cout << "element " << k << " matches with " << val << " parity" << std::endl;
#endif
    
        if (val > max_val)
        {
            max_val = val;
            best_match = static_cast<int>(k);
        }
    }
    
    if (min_threshold > max_val)
    {
        return -1;
    }
    
    return best_match;
}

void ImageArray::load(const char *filename, int width)
{
    clear();
    
    if (width == 0)
    {
        // error: division by zero
        return;
    }
    
    Image composite;
    composite.load(filename);
    
    int frames = composite.width() / width;
    
    int i;
    for (i = 0; i < frames; i++)
    {
        images.push_back(Image());
        composite.extract(*images.rbegin(), i * width, 0, width, composite.height());
    }
}

void ImageArray::save(const char* filename) const
{
    if (empty())
    {
        return;
    }
    
    Image composite;
    composite.resize(static_cast<int>(size()) * width(), height());
    
    size_t i;
    for (i = 0; i < size(); i++)
    {
        images[i].copyRect(composite, 0, 0, static_cast<int>(i) * width(), 0, width(), height());
    }
    
    composite.save(filename);
}

bool operator!=(const Image& img, const ImageArray& imgArr)
{
    if (imgArr.size() == 0)
    {
        return true;
    }
    
    if ((imgArr[0].width() != img.width()) || (imgArr[0].height() != img.height()))
    {
        return true;
    }
    
    size_t k;
    for (k = 0; k < imgArr.size(); k++)
    {
        if (img == imgArr[k])
        {
            return false;
        }
    }
    
    return true;
}
