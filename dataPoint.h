#ifndef DATAPOINT_H
#define DATAPOINT_H

#include "mqtt_cfg.h"
#include <string.h>

class dataPoint
{
    public:
        dataPoint();
        dataPoint(unsigned int id, char * name);// {SetId(id); SetcValue(name)};
        dataPoint(unsigned int id, char * name, char * value);
        virtual ~dataPoint();

        int ID = 0;
        char name[MAX_MQTT_TAG_LENGTH];
        char tag[MAX_MQTT_TAG_LENGTH];

        void SetName(char * name) {strcpy(m_name,name);}

        void SetID(unsigned int val) { m_ID = val;}
        void SetdValue(double val) { m_dValue = val; }
        void SetiValue(int val) { m_iValue = val; }
        void SetuiValue(unsigned int val) { m_uiValue = val; }
        void SetcValue(char * val) { strcpy(m_cValue, val); }


        unsigned int GetID() { return m_ID; }
        double GetdValue() { return m_dValue; }
        int GetiValue();
        unsigned int GetuiValue();
        char * GetcValue() { return m_cValue; }



    protected:
    private:
        unsigned int m_ID = 0;
        char m_name[MAX_MQTT_STRING_LENGTH+1];
        char m_tag[MAX_MQTT_TAG_LENGTH];
        double m_dValue;
        int m_iValue;
        unsigned int m_uiValue;
        char m_cValue[MAX_MQTT_STRING_LENGTH+1];
};

#endif // DATAPOINT_H
