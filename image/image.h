#ifndef image_h
#define image_h

#include <string>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"

#include <opencv2/opencv.hpp>

#pragma clang diagnostic pop

#include "xmlres.h"

#define MATRIX_TYPE CV_8UC4

// CV_8UC3 is slower at taking screenshots, because the internal format from
// the Mac functions is always 32bit and thus has to be converted.
// However, template matching is much more time consuming with CV_8UC4.
// Saving an image takes about the same amount of time, even with the
// conversion from BGRA to BGR before saving.
//
// Here is a comparison from farm king time measurements:
// With CV_8UC3 (average based on 100 iterations)
//     take screenshot: 100 ms
//     template matching on screen: 450 ms
//     find identifier with hashimage: 1.25 ms
//     save screen: 2550 ms
//
// With CV_8UC4 (average based on 100 iterations)
//     take screenshot: 60 ms
//     template matching on screen: 600 ms
//     find identifier with hashimage: 0.95 ms
//     save screen: 2550 ms
    
// #define MATCHTEMPLATE_DISPLAY_RESULT

typedef char __disable_pragma_warning__;

#pragma pack(push, 1)

struct BGRAColor
{
    char blue;
    char green;
    char red;
    char alpha;

    inline BGRAColor() {}
    inline BGRAColor(uint32_t clr32) { *this = clr32; }

    inline BGRAColor& operator=(uint32_t clr32);
    inline bool operator==(const BGRAColor clr) const;
    inline bool operator!=(const BGRAColor clr) const;

    inline operator uint32_t() const;
};

struct BGRColor
{
    uchar blue;
    uchar green;
    uchar red;
    
    inline BGRColor() {}
    inline BGRColor(uint32_t clr32) { *this = clr32; }
    
    inline BGRColor& operator=(uint32_t clr32);
    inline bool operator==(const BGRColor clr) const;
    inline bool operator!=(const BGRColor clr) const;
    
    inline operator uint32_t() const;
};

#pragma pack(pop)

#if MATRIX_TYPE == CV_8UC3
    #define MATRIX_CHANNELS 3
    #define ColorType BGRColor 
#elif MATRIX_TYPE == CV_8UC4
    #define MATRIX_CHANNELS 4
    #define ColorType BGRAColor
#endif

/////////////////////////////////////////////////////////////////////////////
//
// Image class declaration
//
/////////////////////////////////////////////////////////////////////////////

class Image
{
public:
	Image();
    Image(const Image& src); // copy constructor (used for push_back(img))
    Image(const char* filename);
	~Image();
    
    // screenshot (using ApplicationServices)
    bool capture();
    bool captureRegion(const cv::Rect &rect);
    bool captureRegion(int left, int top, int width, int height);

    // match template (using openCV methods)
    void matchTemplate(const Image& templ, cv::Point& pt, double& max_val) const;
    
    // difference between images
    double match(const Image& img, double min_val, uint32_t max_diff) const;
    double matchAt(int x, int y, const Image& img, double min_val, uint32_t max_diff) const;
    
    // display methods
    static void initWindow(std::string imgwnd);
    static void destroyWindow(std::string imgwnd);
    
    void display(std::string imgwnd, int x, int y, int milliseconds) const;
    
    // load from and save to png
    void load(const char* filename);
    void save(const char* filename) const;
    
    // basic access methods
    inline bool empty() const { return width() == 0 || height() == 0; }
    inline const cv::Mat& matrix() const { return mat; }
    
    inline int width() const { return mat.cols; }
    inline int height() const { return mat.rows; }
    
    void resize(int width, int height);
    
    // copy operator
    Image& operator =(const Image& src);
    
    // copy rectangular region to another image
    void copyRect(Image& dest, int left, int top, int dest_left, int dest_top, int width, int height) const;
    void extract(Image& dest, int left, int top, int width, int height) const;
    
    // pixel manipulation
    inline ColorType& operator()(int x, int y) { return *reinterpret_cast<ColorType*>(mat.ptr(y) + x); }
    inline const ColorType& operator()(int x, int y) const { return *reinterpret_cast<const ColorType*>(mat.ptr(y) + x); }
    
private:
    cv::Mat mat;

    void writeBuf(const uchar* buf, int bpp, int bpr);
};

bool operator==(const Image& img1, const Image& img2);

/////////////////////////////////////////////////////////////////////////////
//
// ImageArray class declaration
//
/////////////////////////////////////////////////////////////////////////////

class ImageArray
{
public:
	ImageArray();
    ImageArray(const char* filename, int width);
    
    // bestMatch finds the image in the array that
    // matches best somewhere in the given image
    int bestMatch(const Image& img, double min_threshold) const;
    
    void load(const char* filename, int width);
    void save(const char* filename) const;

    // basic access methods
    inline bool empty() const { return images.empty(); }
    inline size_t size() const { return images.size(); }
    
    inline int width() const { return empty() ? 0 : images[0].width(); }
    inline int height() const { return empty() ? 0 : images[0].height(); }
    
    // vector manipulation methods
    inline void clear() { images.clear(); }
    inline void erase(int index) { images.erase(images.begin() + index); }
    inline void pushBack(const Image& img);
    inline const Image& operator[](size_t index) const { return images[index]; }
    
    inline ImageArray& operator=(const ImageArray& src);
    inline ImageArray& operator+=(const Image& src);
    inline ImageArray& operator+=(const ImageArray& src);
    
private:
    std::vector<Image> images;
};

bool operator!=(const Image& img, const ImageArray& imgArr);

/////////////////////////////////////////////////////////////////////////////
//
// BGRAColor and BGRColor inline methods
//
/////////////////////////////////////////////////////////////////////////////

inline BGRAColor& BGRAColor::operator=(uint32_t clr32) 
{ 
    memcpy(&blue, &clr32, sizeof(BGRAColor));
    return *this;
}

inline bool BGRAColor::operator==(const BGRAColor clr) const
{
    return memcmp(this, &clr, sizeof(BGRAColor)) == 0;
}

inline bool BGRAColor::operator!=(const BGRAColor clr) const
{
    return !(*this == clr);
}

inline BGRAColor::operator uint32_t() const
{
    return *reinterpret_cast<const uint32_t*>(&blue);
}

inline uint32_t operator-(const BGRAColor color1, const BGRAColor color2)
{
    return static_cast<uint32_t>(
          (color1.blue - color2.blue) * (color1.blue - color2.blue)
        + (color1.green - color2.green) * (color1.green - color2.green)
        + (color1.red - color2.red) * (color1.red - color2.red) +
        + (color1.alpha - color2.alpha) * (color1.alpha - color2.alpha));
}

inline BGRColor& BGRColor::operator=(uint32_t clr32) 
{ 
    blue = clr32 & 0xff; 
    green = (clr32 & 0xff00) >> 8; 
    red = static_cast<uchar>((clr32 & 0xff00000) >> 16);
    return *this;
}

inline bool BGRColor::operator==(const BGRColor clr) const
{
    return memcmp(this, &clr, sizeof(BGRColor)) == 0;
}

inline bool BGRColor::operator!=(const BGRColor clr) const
{
    return !(*this == clr);
}

inline BGRColor::operator uint32_t() const
{
    return static_cast<uint32_t>(blue) +
        (static_cast<uint32_t>(green) << 8) +
        (static_cast<uint32_t>(red) << 16);
}

inline uint32_t operator-(const BGRColor color1, const BGRColor color2)
{
    return (color1.blue - color2.blue) * (color1.blue - color2.blue)
        + (color1.green - color2.green) * (color1.green - color2.green)
        + (color1.red - color2.red) * (color1.red - color2.red);
}

/////////////////////////////////////////////////////////////////////////////
//
// cv::Point inline methods
//
/////////////////////////////////////////////////////////////////////////////

inline cv::Point operator %(const cv::Point& pt, int n)
{ 
    return {pt.x % n, pt.y % n};
}

/////////////////////////////////////////////////////////////////////////////
//
// Image inline methods
//
/////////////////////////////////////////////////////////////////////////////

inline bool Image::captureRegion(const cv::Rect &rect)
{
    return captureRegion(rect.x, rect.y, rect.width, rect.height);
}

inline Image &Image::operator =(const Image& src)
{
    src.extract(*this, 0, 0, src.width(), src.height());
    
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
//
// ImageArray inline methods
//
/////////////////////////////////////////////////////////////////////////////

inline void ImageArray::pushBack(const Image& img)
{ 
    if (size() != 0)
    {
        if (width() != img.width() || height() != img.height())
        {
            // error: dimensions don't match
            return;
        }
    }
    
    images.push_back(img);
}

inline ImageArray& ImageArray::operator=(const ImageArray& src)
{
    images.clear();
    
    size_t k;
    for (k = 0; k < src.size(); k++)
    {
        images.push_back(src[k]);
    }
    
    return *this;
}

inline ImageArray& ImageArray::operator+=(const Image& src)
{
    pushBack(src);
    return *this;
}
    
inline ImageArray& ImageArray::operator+=(const ImageArray& src)
{
    if (size() > 0 && src.size() > 0)
    {
        if (width() != src.width() || height() != src.height())
        {
            // error: dimensions don't match
            return *this;
        }
    }

    size_t k;
    for (k = 0; k < src.size(); k++)
    {
        images.push_back(src[k]);
    }

    return *this;
}
    
/////////////////////////////////////////////////////////////////////////////
//
// XMLResource inline methods
//
/////////////////////////////////////////////////////////////////////////////

inline void readResource(const TiXmlNode* node, cv::Point& val)
{
    readStructInit();
    readStructMember(x, "x_pos");
    readStructMember(y, "y_pos");
}

inline void readResource(const TiXmlNode* node, cv::Rect& val)
{
    readStructInit();
    readStructMember(x, "x_pos");
    readStructMember(y, "y_pos");
    readStructMember(width, "width");
    readStructMember(height, "height");
}

inline void writeResource(TiXmlNode* node, const cv::Point& val)
{
    writeStructInit();
    writeStructMember(x, "x_pos");
    writeStructMember(y, "y_pos");
}

inline void writeResource(TiXmlNode* node, const cv::Rect& val)
{
    writeStructInit();
    writeStructMember(x, "x_pos");
    writeStructMember(y, "y_pos");
    writeStructMember(width, "width");
    writeStructMember(height, "height");
}

#endif // image_h
