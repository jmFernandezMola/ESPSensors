#include "dataPoint.h"
#include <string.h>

dataPoint::dataPoint(void)
{
    //ctor
}

dataPoint::dataPoint(unsigned int id, char * name)
{
    m_ID = id;
    strcpy(m_name, name);
    //ctor
}

dataPoint::dataPoint(unsigned int id, char * name, char * value)
{
    m_ID = id;
    strcpy(m_name, name);
    strcpy(m_cValue, value);
}

dataPoint::~dataPoint()
{
    //dtor
}
