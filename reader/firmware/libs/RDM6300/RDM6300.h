#pragma once

class RDM6300
{
public:
    /// Size of ID in bytes
    static const int ID_SIZE = 6;
    
    RDM6300()
    {
    }
    
    bool add_byte(uint8_t input)
    {
#ifdef PROTOCOL_DEBUG
        Serial.print(F("Add "));Serial.println((int) input);
#endif
        if (input == STX)
        {
            m_state = 1;
            m_checksum = 0;
            m_index = 0;
#ifdef PROTOCOL_DEBUG
            Serial.println(F("State 1 (got STX)"));
#endif
        }
        else if (m_state == 1)
        { 
            m_temp = input - '0';
            if (m_temp > 9)
                m_temp -= 7;
            m_state = 2;
#ifdef PROTOCOL_DEBUG
            Serial.print(F("State 2: ")); Serial.println(m_temp);
#endif
        }
        else if (m_state == 2)
        {
            input -= '0';
            if (input > 9)
                input -= 7;
            m_temp = (m_temp << 4) | input;
#ifdef PROTOCOL_DEBUG
            Serial.print(F("Store ")); Serial.print(m_temp); Serial.print(F(" at ")); Serial.println(m_index);
#endif
            m_buf[m_index++] = m_temp;
            if (m_index >= ID_SIZE)
            {
                m_state = 3; 
#ifdef PROTOCOL_DEBUG
                Serial.println(F("State 3"));
#endif
            }
            else
            { 
                m_state = 1; 
#ifdef PROTOCOL_DEBUG
                Serial.print(F("State 1: ")); Serial.println(m_temp);
#endif
                m_checksum ^= m_temp;
            }
        }
        else if (m_state == 3)
        {
            if (input == ETX)
            { 
                m_state = 4; 
#ifdef PROTOCOL_DEBUG
                Serial.println(F("State 4"));
#endif
            }
            else
            { 
                m_state = 0; 
#ifdef PROTOCOL_DEBUG
                Serial.println(F("Error: Expected ETX"));
#endif
            }
        }
        else
            m_state = 0;
        if (m_state == 4)
        {
            m_state = 0;
            return true;
        }
        return false;
    }

    const char* get_id() const
    {
        for (int i = 0; i < ID_SIZE; ++i)
        {
            sprintf(m_id + i * 2, "%02X", m_buf[i]);
        }
        m_id[ID_SIZE * 2] = 0;
        return m_id;
    }
    
private:
    static const char STX = 2;
    static const char ETX = 3;
    int m_state = 0;
    int m_checksum = 0;
    int m_temp = 0;
    int m_index = 0;
    unsigned char m_buf[ID_SIZE];
    mutable char m_id[ID_SIZE * 2 + 1 + 20]; //!!
};
