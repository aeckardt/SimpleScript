//
//  xmlfile.cpp
//  fkhint
//
//  Created by Albrecht Eckardt on 2018-07-16.
//  Copyright © 2018 Albrecht Eckardt. All rights reserved.
//

#include "xmlres.h"

/////////////////////////////////////////////////////////////////////////////
//
// XMLResource implementation
//
/////////////////////////////////////////////////////////////////////////////

XMLResource::~XMLResource()
{
}

void XMLResource::load(const char* filename)
{
    TiXmlDocument doc;
    
    doc.LoadFile(filename);
    
    load(&doc);
}

void XMLResource::load(TiXmlNode* node)
{
    root = node;
    
    members(eIn);
}

void XMLResource::save(const char* filename)
{
    TiXmlDocument doc;
    
    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "utf-8", "true" );
    doc.LinkEndChild(decl);
    
    save(&doc);
    
    doc.SaveFile(filename);
}

void XMLResource::save(TiXmlNode* node)
{
    root = node;
    
    members(eOut);
}
