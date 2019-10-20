//
//  xmlres.h
//  common
//
//  Created by Albrecht Eckardt on 2018-07-16.
//  Copyright © 2018 Albrecht Eckardt. All rights reserved.
//

#ifndef xmlres_h
#define xmlres_h

#include <string>
#include <map>
#include <vector>

#include <tinyxml/tinyxml.h>

/////////////////////////////////////////////////////////////////////////////
//
// XMLResource class declaration
//
/////////////////////////////////////////////////////////////////////////////

class XMLResource
{
public:
    enum Mode
    {
        eIn,
        eOut
    };

    virtual ~XMLResource();
    
    void load(const char* filename);
    void load(TiXmlNode* node);
    
    void save(const char* filename);
    void save(TiXmlNode* node);
    
    virtual void members(Mode mode) = 0;
    
    template<typename T>
    void member(T& mem, const std::string& name, Mode mode) 
    {
        if (mode == eIn)
        {
            TiXmlNode* node = root->FirstChild(name.c_str());
            if (node != nullptr)
            {
                readResource(node, mem);
            }
        }
        else
        {
            TiXmlElement* elem = new TiXmlElement(name.c_str());
            writeResource(elem, mem);
            root->LinkEndChild(elem);
        }
    }
    
protected:
    TiXmlNode* root;
};

/////////////////////////////////////////////////////////////////////////////
//
// XMLResource standard I/O functions
//
/////////////////////////////////////////////////////////////////////////////

#define readStructInit() \
    memset(&val, 0, sizeof(val)); \
    TiXmlNode const* entry;

#define readClassInit() \
    TiXmlNode const* entry;

#define readStructMember(mem, str) \
    entry = node->FirstChild(str); \
    if (entry != NULL) \
    { \
        readResource(entry, val.mem); \
    }

#define writeStructInit() \
    TiXmlElement* elem;

#define writeStructMember(mem, str) \
    elem = new TiXmlElement(str); \
    writeResource(elem, val.mem); \
    node->LinkEndChild(elem);
    
inline void readResource(const TiXmlNode* node, short& val)
{
    const TiXmlNode* text = node->FirstChild();
    if (text != nullptr)
    {
        sscanf(text->Value(), "%hd", &val);
    }
    else
    {
        // error: no text found
        val = 0;
    }
}

inline void readResource(const TiXmlNode* node, int& val)
{
    const TiXmlNode* text = node->FirstChild();
    if (text != nullptr)
    {
        sscanf(text->Value(), "%d", &val);
    }
    else
    {
        // error: no text found
        val = 0;
    }
}

inline void readResource(const TiXmlNode* node, uint32_t& val)
{
    const TiXmlNode* text = node->FirstChild();
    if (text != nullptr)
    {
        sscanf(text->Value(), "%u", &val);
    }
    else
    {
        // error: no text found
        val = 0;
    }
}

inline void readResource(const TiXmlNode* node, size_t& val)
{
    const TiXmlNode* text = node->FirstChild();
    if (text != nullptr)
    {
        sscanf(text->Value(), "%ld", &val);
    }
    else
    {
        // error: no text found
        val = 0;
    }
}

inline void readResource(const TiXmlNode* node, double& val)
{
    const TiXmlNode* text = node->FirstChild();
    if (text != nullptr)
    {
        val = atof(text->Value());
    }
    else
    {
        // error: no text found
        val = 0.0;
    }
}

inline void readResource(const TiXmlNode* node, std::string& val)
{
    const TiXmlNode* text = node->FirstChild();
    if (text != nullptr)
    {
        val = text->Value();
    }
    else
    {
        // error: no text found
        val.clear();
    }
}

template<typename T1, typename T2>
inline void readResource(const TiXmlNode *node, std::pair<T1, T2> &val)
{
    const TiXmlNode *container = node->FirstChild("first");
    readResource(container, val.first);
    
    container = node->FirstChild("second");
    readResource(container, val.second);
}

template<typename T1, typename T2>
inline void readResource(const TiXmlNode *node, std::map<T1, T2> &val)
{
    val.clear();
    
    const TiXmlNode *container = node->FirstChild("map_entry");
    while (container != nullptr)
    {
        std::pair<T1, T2> entry;
        readResource(container, entry);
        
        val[entry.first] = entry.second;
        
        container = container->NextSiblingElement("map_entry");
    }
}

template<typename T>
inline void readResource(const TiXmlNode* node, std::vector<T>& val)
{
    val.clear();
    
    const TiXmlNode* container = node->FirstChild("entry");
    while (container != nullptr)
    {
        val.push_back(T());
        T& entry = *val.rbegin();
        
        readResource(container, entry);
        
        container = container->NextSiblingElement("entry");
    }
}

inline void writeResource(TiXmlNode* node, const short& val)
{
    char num[6];
    sprintf(num, "%hd", val);
    
    node->LinkEndChild(new TiXmlText(num));
}

inline void writeResource(TiXmlNode* node, const int& val)
{
    char num[12];
    sprintf(num, "%d", val);
    
    node->LinkEndChild(new TiXmlText(num));
}

inline void writeResource(TiXmlNode* node, const uint32_t& val)
{
    char num[12];
    sprintf(num, "%u", val);
    
    node->LinkEndChild(new TiXmlText(num));
}

inline void writeResource(TiXmlNode* node, const size_t& val)
{
    char num[12];
    sprintf(num, "%ld", val);

    node->LinkEndChild(new TiXmlText(num));
}

inline void writeResource(TiXmlNode* node, const double& val)
{
    char numf[22];
    sprintf(numf, "%f", val);
    if (strlen(numf) > 7)
    {
        sprintf(numf, "%.6f", val);
    }
    
    node->LinkEndChild(new TiXmlText(numf));
}

inline void writeResource(TiXmlNode* node, const std::string& val)
{
    node->LinkEndChild(new TiXmlText(val.c_str()));
}

template<typename T1, typename T2>
inline void writeResource(TiXmlNode* node, const std::pair<T1, T2>& val)
{
    TiXmlElement* entry = new TiXmlElement("first");
    writeResource(entry, val.first);
    node->LinkEndChild(entry);

    entry = new TiXmlElement("second");
    writeResource(entry, val.second);
    node->LinkEndChild(entry);
}

template<typename T1, typename T2>
inline void writeResource(TiXmlNode *node, const std::map<T1, T2> &val)
{
    for (auto const& item : val)
    {
        TiXmlElement* entry = new TiXmlElement("map_entry");
        writeResource(entry, item);
        node->LinkEndChild(entry);
    }
}

template<typename T>
inline void writeResource(TiXmlNode* node, const std::vector<T>& val)
{
    for (auto const &item : val)
    {
        TiXmlElement* entry = new TiXmlElement("entry");
        writeResource(entry, item);
        node->LinkEndChild(entry);
    }
}

#endif /* xmlres_h */
