//
//  matchprofile.h
//  common
//
//  Created by Albrecht Eckardt on 2018-07-29.
//  Copyright © 2018 Albrecht Eckardt. All rights reserved.
//

#ifndef matchprofile_h
#define matchprofile_h

#include <stdio.h>
#include <iostream>
#include <vector>

#include "image.h"

/////////////////////////////////////////////////////////////////////////////
//
// MatchProfile class declaration
//
/////////////////////////////////////////////////////////////////////////////

class MatchProfile
{
public:
    struct MatchPoint
    {
        short x;
        short y;
        ColorType color;
        uint32_t diff;
    };

    // create
    // a match profile is created which includes all points in img
    // that have a max_diff color difference to pixels at the same
    // (x, y) position at any given image in cmpArr
    void create(const Image& img, const ImageArray& cmpArr, uint32_t max_diff);
    
    // this function does the same as the function above, except
    // that all pixels which have a max_diff color difference in
    // BOTH image arrays are excluded from being added as match points
    // it is used to distinguish between similar images to tell which type it is
    // the result is: you get pixels in img that are in all images in incl
    // with max_diff color difference, but which are at the same time not in excl
    void createExclude(const Image& img, const ImageArray& incl, const ImageArray& excl, uint32_t max_diff);
    
    // this function refers to the below functions createMatchImage and createDiffImage,
    // which creates an image that represents a match profile in relation to the template image
    void createFromImage(const Image& templ, const Image& img);
    
    bool match(const Image& img, double min_val, double& max_val) const;
    bool match(const Image& img, const cv::Point& at, double min_val, double& max_val) const;
    
    // check if position exists
    bool posExists(const cv::Point& pt) const;
    
    // basic access functions
    inline MatchPoint& add() { pts.push_back(MatchPoint()); return *pts.rbegin(); }
    inline cv::Point& addPos() { pos.push_back(cv::Point()); return *pos.rbegin(); }
    inline void erase(int index) { pts.erase(pts.begin() + index); }
    
    void makeSparse(size_t max_elem);
    
    inline size_t size() const { return pts.size(); }
    inline size_t positions() const { return pos.size(); }
    
    inline const MatchPoint& operator[](size_t index) const { return pts[index]; }
    inline const cv::Point& getPos(size_t index) const { return pos[index]; }
    
    MatchProfile& operator=(const MatchProfile& src);
    
private:
    std::vector<MatchPoint> pts;
    std::vector<cv::Point> pos;

    void matchPos(const Image& img, const cv::Point& at, size_t _pos, double min_val, double& val) const;
};

/////////////////////////////////////////////////////////////////////////////
//
// MatchProfileArray class declaration
//
/////////////////////////////////////////////////////////////////////////////

class MatchProfileArray
{
public:
    void clear() { mps.clear(); }

    // basic access methods
    inline MatchProfile& add() { mps.push_back(MatchProfile()); return *mps.rbegin(); }
    inline size_t size() const { return mps.size(); }
    inline const MatchProfile& operator[](size_t index) const { return mps[index]; }
    
private:
    std::vector<MatchProfile> mps;
};

/////////////////////////////////////////////////////////////////////////////
//
// tool functions
//
/////////////////////////////////////////////////////////////////////////////

void createHeatMap(const Image& img, const ImageArray& cmpArr, uint32_t max_diff, Image& output);
void createMatchImage(const Image& templ, const ImageArray& cmpArr, uint32_t max_diff, Image& output);
void createDiffImage(const Image& templ, const ImageArray& cmpArr, const ImageArray& exclArr, uint32_t max_diff, Image& output, std::vector<cv::Point>& pos);

#endif /* matchprofile_h */
