//
//  matchprofile.cpp
//  common
//
//  Created by Albrecht Eckardt on 2018-07-29.
//  Copyright © 2018 Albrecht Eckardt. All rights reserved.
//

#include <algorithm>

#include "matchprofile.h"

uint32_t L1Norm(const std::vector<cv::Mat>& vec, int x, int y);
uint32_t MaxNorm(const std::vector<cv::Mat>& vec, int x, int y);
uint32_t MinDiff(const std::vector<cv::Mat>& vec, int x, int y);

/////////////////////////////////////////////////////////////////////////////
//
// MatchProfile implementation
//
/////////////////////////////////////////////////////////////////////////////

bool MatchProfile::posExists(const cv::Point& pt) const
{
    size_t k;
    for (k = 0; k < pos.size(); k++)
    {
        if (pos[k] == pt)
        {
            return true;
        }
    }
    
    return false;
}

void MatchProfile::create(const Image& templ, const ImageArray& cmpArr, uint32_t max_diff)
{
    pts.clear();
    pos.clear();

    int width = templ.width();
    int height = templ.height();

    std::vector<cv::Mat> diff_mat;
    
    diff_mat.resize(cmpArr.size());
    
    cv::Point pt;
    double val;
    
    size_t k;
    short int x, y;
    for (k = 0; k < cmpArr.size(); k++)
    {
        if (templ.width() != cmpArr[k].width() && 
            templ.height() != cmpArr[k].height())
        {
            cmpArr[k].matchTemplate(templ, pt, val);
        }
        else
        {
            pt = cv::Point(0, 0);
        }
        
        if (!posExists(pt))
        {
            pos.push_back(pt);
        }
        
        diff_mat[k].create(width, height, CV_32SC1);
        
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                diff_mat[k].at<uint32_t>(x, y) =
                    templ(x, y) - cmpArr[k](pt.x + x, pt.y + y);
            }
        }
    }
                    
    uint32_t max_norm;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            max_norm = MaxNorm(diff_mat, x, y);
            if (max_norm <= max_diff)
            {
                pts.push_back(MatchPoint());
                
                MatchPoint& mp = *pts.rbegin();
                mp.x = x;
                mp.y = y;
                mp.color = templ(x, y);
                mp.diff = max_norm;
            }
        }
    }
}

void MatchProfile::createExclude(const Image& templ, const ImageArray& cmpArr, const ImageArray& exclArr, uint32_t max_diff)
{
    pts.clear();
    pos.clear();

    int width = templ.width();
    int height = templ.height();

    std::vector<cv::Mat> diff_mat;
    std::vector<cv::Mat> diff_mat_excl;
    
    diff_mat.resize(cmpArr.size());
    diff_mat_excl.resize(exclArr.size());
    
    cv::Point pt;
    double val;
    
    size_t k;
    short int x, y;
    for (k = 0; k < cmpArr.size(); k++)
    {
        if (templ.width() != cmpArr[k].width() ||
            templ.height() != cmpArr[k].height())
        {
            cmpArr[k].matchTemplate(templ, pt, val);
        }
        else
        {
            pt = cv::Point(0, 0);
        }
        
        if (!posExists(pt))
        {
            pos.push_back(pt);
        }
        
        diff_mat[k].create(width, height, CV_32SC1);
        
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                diff_mat[k].at<uint32_t>(x, y) =
                    templ(x, y) - cmpArr[k](pt.x + x, pt.y + y);
            }
        }
    }
                    
    for (k = 0; k < exclArr.size(); k++)
    {
        if (templ.width() != exclArr[k].width() ||
            templ.height() != exclArr[k].height())
        {
            exclArr[k].matchTemplate(templ, pt, val);
        }
        else
        {
            pt = cv::Point(0, 0);
        }

        // add positions only for incl
        
        diff_mat_excl[k].create(width, height, CV_32SC1);
        
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                diff_mat_excl[k].at<uint32_t>(x, y) =
                    templ(x, y) - exclArr[k](pt.x + x, pt.y + y);
            }
        }
    }
                    
    uint32_t max_norm;
    uint32_t min_excl;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            max_norm = MaxNorm(diff_mat, x, y);
            min_excl = MinDiff(diff_mat_excl, x, y);
            if (max_norm <= max_diff && min_excl > max_diff)
            {
                pts.push_back(MatchPoint());
                
                MatchPoint& mp = *pts.rbegin();
                mp.x = x;
                mp.y = y;
                mp.color = templ(x, y);
                mp.diff = max_norm;
            }
        }
    }
}

void MatchProfile::createFromImage(const Image& templ, const Image& img)
{
    pts.clear();
    pos.clear();
    
    pos.push_back(cv::Point(0, 0));

    int width = templ.width();
    int height = templ.height();
    
    if (width != img.width() || height != img.height())
    {
        //error: dimensions don't match!
        return;
    }

    short int x, y;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            if (img(x, y) != ColorType(0))
            {
                MatchPoint& mpt = add();
                mpt.x = x;
                mpt.y = y;
                mpt.color = templ(x, y);
                mpt.diff = templ(x, y) - img(x, y);
            }
        }
    }
}

void MatchProfile::matchPos(const Image& img, const cv::Point& at, size_t pos_i, double min_val, double& val) const
{
    size_t pt_i;
    size_t nonmatch = 0;
    size_t max_nonmatch = static_cast<size_t>(static_cast<double>(pts.size()) * (1 - min_val));
    
    for (pt_i = 0; pt_i < size(); pt_i++)
    {
        cv::Point pt = pos[pos_i] + cv::Point(pts[pt_i].x, pts[pt_i].y);
        
        if (img(at.x + pt.x, at.y + pt.y) - pts[pt_i].color > pts[pt_i].diff)
        {
            nonmatch++;
            if (nonmatch > max_nonmatch)
            {
                val = 0;
                return;
            }
        }
    }

    val = static_cast<double>(size() - nonmatch) / static_cast<double>(size());
}

bool MatchProfile::match(const Image& img, double min_val, double& max_val) const
{
    double val;
    max_val = 0;
    
    cv::Point at = cv::Point(0, 0);

    size_t j;
    for (j = 0; j < positions(); j++)
    {
        matchPos(img, at, j, min_val, val);
        if (val > max_val)
        {
            max_val = val;
        }
    }
    
    return max_val >= min_val;
}

bool MatchProfile::match(const Image& img, const cv::Point& at, double min_val, double& max_val) const
{
    double val;
    max_val = 0;    
    
    size_t j;
    for (j = 0; j < positions(); j++)
    {
        matchPos(img, at, j, min_val, val);
        if (val > max_val)
        {
            max_val = val;
        }
    }
    
    return max_val >= min_val;
}

void MatchProfile::makeSparse(size_t max_elem)
{
    size_t size = pts.size();
    if (size <= max_elem)
    {
        return;
    }
    
    MatchProfile copy = *this;
    
    pts.clear();

    size_t i;
    size_t min_i;
    uint32_t min_diff;
    while (pts.size() < max_elem)
    {
        min_diff = 0xffffff;
        min_i = 0xfffffffful;
        for (i = 0; i < copy.size(); ++i)
        {
            if (copy.pts[i].diff < min_diff)
            {
                min_diff = copy.pts[i].diff;
                min_i = i;
            }
        }
        pts.push_back(copy.pts[min_i]);
        copy.erase(static_cast<int>(min_i));
    }
}

MatchProfile& MatchProfile::operator=(const MatchProfile& src)
{
    pts.clear();
    pos.clear();
        
    size_t i;
    for (i = 0; i < src.pts.size(); i++)
    {
        pts.push_back(src.pts[i]);
    }
    
    for (i = 0; i < src.pos.size(); i++)
    {
        pos.push_back(src.pos[i]);
    }
    
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
//
// tool functions
//
/////////////////////////////////////////////////////////////////////////////

uint32_t L1Norm(const std::vector<cv::Mat>& vec, int x, int y)
{
    size_t k;

    uint32_t sum = 0;
    for (k = 0; k < vec.size(); k++)
    {
        sum += vec[k].at<uint32_t>(x, y);
    }    
    return sum;
}

uint32_t MaxNorm(const std::vector<cv::Mat>& vec, int x, int y)
{
    size_t k;

    uint32_t max = 0;
    for (k = 0; k < vec.size(); k++)
    {
        if (vec[k].at<uint32_t>(x, y) > max)
        {
            max = vec[k].at<uint32_t>(x, y);
        }
    }    
    return max;
}

uint32_t MaxElem(const std::vector<cv::Mat>& vec, int x, int y)
{
    uint32_t k;

    uint32_t max = 0;
    uint32_t max_k = 0xfffffffful;
    for (k = 0; k < vec.size(); k++)
    {
        if (vec[k].at<uint32_t>(x, y) > max)
        {
            max = vec[k].at<uint32_t>(x, y);
            max_k = k;
        }
    }    
    return max_k;
}

uint32_t MinDiff(const std::vector<cv::Mat>& vec, int x, int y)
{
    uint32_t k;

    uint32_t min = 0x030000;
    for (k = 0; k < vec.size(); k++)
    {
        if (vec[k].at<uint32_t>(x, y) < min)
        {
            min = vec[k].at<uint32_t>(x, y);
        }
    }    
    return min;
}

void createHeatMap(const Image& img, const ImageArray& cmpArr, uint32_t max_diff, Image& output)
{
    int width = img.width();
    int height = img.height();

    std::vector<cv::Mat> diff_mat;
    
    diff_mat.resize(cmpArr.size());
    
    cv::Point pt;
    double val;
    
    size_t k;
    short int x, y;
    for (k = 0; k < cmpArr.size(); k++)
    {
        if (img.width() != cmpArr[k].width() && 
            img.height() != cmpArr[k].height())
        {
            cmpArr[k].matchTemplate(img, pt, val);
        }
        else
        {
            pt = cv::Point(0, 0);
        }
        
        diff_mat[k].create(width, height, CV_32SC1);
        
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                diff_mat[k].at<uint32_t>(x, y) =
                    img(x, y) - cmpArr[k](pt.x + x, pt.y + y);
            }
        }
    }
                    
    output.resize(width, height);

    uint32_t norm;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            norm = MaxNorm(diff_mat, x, y);
            uint32_t ch = (norm * 0xff) / (max_diff);
            if (ch > 0xff) { ch = 0xff; }
            output(x, y) = (0xff - ch) + (ch << 16);
        }
    }
}

bool posExists(const std::vector<cv::Point>& pos, cv::Point& _p)
{
    size_t k;
    for (k = 0; k < pos.size(); k++)
    {
        if (pos[k] == _p)
        {
            return true;
        }
    }
    
    return false;
}

void createMatchImage(const Image& templ, const ImageArray& cmpArr, uint32_t max_diff, Image& output)
{
    int width = templ.width();
    int height = templ.height();

    std::vector<cv::Mat> diff_mat;
    
    diff_mat.resize(cmpArr.size());
    
    size_t k;
    short int x, y;
    for (k = 0; k < cmpArr.size(); k++)
    {
        diff_mat[k].create(width, height, CV_32SC1);
        
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                diff_mat[k].at<uint32_t>(x, y) =
                    templ(x, y) - cmpArr[k](x, y);
            }
        }
    }
                    
    output.resize(width, height);
    
    uint32_t norm;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            norm = MaxNorm(diff_mat, x, y);
            k = MaxElem(diff_mat, x, y);

            if (norm <= max_diff)
            {
                output(x, y) = cmpArr[k](x, y);
            }
            else
            {
                output(x, y) = 0;
            }
        }
    }
}

void createDiffImage(const Image& templ, const ImageArray& cmpArr, const ImageArray& exclArr, uint32_t max_diff, Image& output, std::vector<cv::Point>& pos)
{
    int width = templ.width();
    int height = templ.height();

    std::vector<cv::Mat> diff_mat;
    std::vector<cv::Mat> diff_mat_excl;
    std::vector<cv::Point> pos_vec;
    
    diff_mat.resize(cmpArr.size());
    diff_mat_excl.resize(exclArr.size());
    pos.clear();
    
    cv::Point pt;
    double val;
    
    size_t k;
    short int x, y;
    for (k = 0; k < cmpArr.size(); k++)
    {
        if (templ.width() != cmpArr[k].width() ||
            templ.height() != cmpArr[k].height())
        {
            cmpArr[k].matchTemplate(templ, pt, val);
        }
        else
        {
            pt = cv::Point(0, 0);
        }
        
        if (!posExists(pos, pt))
        {
            pos.push_back(pt);
        }
        
        diff_mat[k].create(width, height, CV_32SC1);
        pos_vec.push_back(pt);
        
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                diff_mat[k].at<uint32_t>(x, y) =
                    templ(x, y) - cmpArr[k](pt.x + x, pt.y + y);
            }
        }
    }
                    
    for (k = 0; k < exclArr.size(); k++)
    {
        if (templ.width() != exclArr[k].width() ||
            templ.height() != exclArr[k].height())
        {
            exclArr[k].matchTemplate(templ, pt, val);
        }
        else
        {
            pt = cv::Point(0, 0);
        }

        // add positions only for incl
        
        diff_mat_excl[k].create(width, height, CV_32SC1);
        
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                diff_mat_excl[k].at<uint32_t>(x, y) =
                    templ(x, y) - exclArr[k](pt.x + x, pt.y + y);
            }
        }
    }
                    
    output.resize(width, height);

    uint32_t max_norm;
    uint32_t min_excl;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            max_norm = MaxNorm(diff_mat, x, y);
            k = MaxElem(diff_mat, x, y);
            min_excl = MinDiff(diff_mat_excl, x, y);
            if (max_norm <= max_diff && min_excl > max_diff)
            {
                output(x, y) = cmpArr[k](pos_vec[k].x + x, pos_vec[k].y + y);
            }
            else
            {
                output(x, y) = 0;
            }
        }
    }
}
