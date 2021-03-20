#pragma once

#include <MCUFRIEND_kbv.h>

#include "logo.h"

extern const int screen_height;
extern const int screen_width;

const int logo_height = 36;
const int logo_width = 220;

class Animater
{
public:
    Animater(MCUFRIEND_kbv& _tft)
        : tft(_tft)
    {
    }
    
    void reset()
    {
        count = 0;
        tft.fillScreen(TFT_BLACK);
        drawSmallLogo((screen_width - logo_width) / 2, 0);
    }

    void drawLogo(int x_offset, int y_offset)
    {
        drawSmallLogo(x_offset, y_offset);
    }

    void drawSmallLogo(int x_offset, int y_offset)
    {
        if ((x_offset == 0) && (y_offset == 0))
        {
            tft.drawBitmap(0, 0, logo_small_a, 132, 36, TFT_WHITE);
            tft.drawBitmap(220-86, 0, logo_small_b, 86, 36, TFT_RED);
            return;
        }
        tft.fillRect(x_offset-1, 0, x_offset-1, 36, TFT_BLACK);
        tft.drawBitmap(x_offset, 0, logo_small_a, 132, 36, TFT_WHITE, TFT_BLACK);
        if (x_offset < 220-86)
            tft.drawBitmap(x_offset+220-86, 0, logo_small_b, 86, 36, TFT_RED, TFT_BLACK);
    }

    
    void update()
    {
        const auto now = millis();
        if (now - last_tick < 300)
            return;
        last_tick = now;
        if (count == 0)
            drawSmallLogo((screen_width - logo_width) / 2, 0);
        else
        {
            const auto offset = (screen_width - logo_width) / 2 + count - 1;
            tft.fillRect(offset, 0, 1, logo_height, TFT_BLACK);
            tft.fillRect(screen_width - offset, 0, 1, logo_height, TFT_BLACK);
        }
        ++count;
        if (count >= logo_width/2)
            count = 0;
    }

private:
    MCUFRIEND_kbv& tft;
    int count = 0;
    unsigned long last_tick = 0;
};
