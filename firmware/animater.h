#pragma once

#include "TFT_22_ILI9225.h"

#include "logo.h"

class Animater
{
public:
    Animater(TFT_22_ILI9225& _tft)
        : tft(_tft)
    {
    }
    
    void reset()
    {
        count = 0;
        large_logo = false;
        height = 36;
        tft.clear();
        drawSmallLogo(0, 0);
    }

    void next()
    {
        int old = mode;
        mode = random(7);
        if (mode == old)
            ++mode;
        count = 0;
    }

    void drawLogo(int x_offset, int y_offset)
    {
        if (large_logo)
            drawLargeLogo(x_offset, y_offset);
        else
            drawSmallLogo(x_offset, y_offset);
    }

    void drawLargeLogo(int x_offset, int y_offset)
    {
        if ((x_offset == 0) && (y_offset == 0))
        {
            tft.drawBitmap(0, 0, logo_large_a, 220, 62, COLOR_WHITE, COLOR_BLACK);
            tft.drawBitmap(0, 66, logo_large_b, 220, 60, COLOR_RED, COLOR_BLACK);
            return;
        }
        if (x_offset > 0)
            tft.fillRectangle(x_offset-1, 0, x_offset-1, 62, COLOR_BLACK);
        if (y_offset > 0)
            tft.fillRectangle(0, y_offset-1, ILI9225_LCD_HEIGHT-1, y_offset-1, COLOR_BLACK);
        int h = 62;
        if (y_offset+h >= height)
            h = height - y_offset;
        tft.drawBitmap(x_offset, y_offset, logo_large_a, 220, h, COLOR_WHITE, COLOR_BLACK);
        h = 60;
        if (y_offset+66+h >= height)
            h = height - 66 - y_offset;
        if (y_offset < height-60)
            tft.drawBitmap(x_offset, y_offset+66, logo_large_b, 220, h, COLOR_RED, COLOR_BLACK);
    }

    void drawSmallLogo(int x_offset, int y_offset)
    {
        if ((x_offset == 0) && (y_offset == 0))
        {
            tft.drawBitmap(0, 0, logo_small_a, 132, 36, COLOR_WHITE);
            tft.drawBitmap(220-86, 0, logo_small_b, 86, 36, COLOR_RED);
            return;
        }
        tft.fillRectangle(x_offset-1, 0, x_offset-1, 36, COLOR_BLACK);
        tft.drawBitmap(x_offset, 0, logo_small_a, 132, 36, COLOR_WHITE, COLOR_BLACK);
        if (x_offset < 220-86)
            tft.drawBitmap(x_offset+220-86, 0, logo_small_b, 86, 36, COLOR_RED, COLOR_BLACK);
    }

    
    void update()
    {
        const auto now = millis();
        if (now - last_tick < 800)
            return;
        last_tick = now;
        switch (mode)
        {
        case 0:
            // Scroll to the right
            if (count > 0)
                tft.fillRectangle(count, 0, count, height-1, COLOR_BLACK);
            drawLogo(count, 0);
            ++count;
            if (count >= ILI9225_LCD_HEIGHT)
                next();
            break;

        case 1:
            // Scroll to the left
            if (count == 0)
            {
                tft.fillRectangle(0, 0, ILI9225_LCD_HEIGHT-1, height, COLOR_BLACK);
                ++count;
                return;
            }
            drawLogo(ILI9225_LCD_HEIGHT-count-1, 0);
            ++count;
            if (count >= ILI9225_LCD_HEIGHT)
                next();
            break;

        case 2:
            // Erase every fourth line
            if (count == 0)
            {
                drawLogo(0, 0);
                ++count;
                count2 = -1;
                return;
            }
            else
            {
                int h = ((height+3)/4)*4;
                int row = (count % (h/4)) * 4;
                if (row < 4)
                    ++count2;
                row += count2;
                tft.fillRectangle(0, row, ILI9225_LCD_HEIGHT-1, row, COLOR_BLACK);
                ++count;
                if (row+2 >= h)
                    next();
            }
            break;

        case 3:
            // Erase lines from top
            if (count == 0)
            {
                drawLogo(0, 0);
                ++count;
                return;
            }
            tft.fillRectangle(0, count, ILI9225_LCD_HEIGHT-1, count, COLOR_BLACK);
            ++count;
            if (count >= height)
                next();
            break;

        case 4:
            // Scroll down
            if (count == 0)
            {
                drawLogo(0, 0);
                ++count;
                return;
            }
            drawLogo(0, count);
            ++count;
            if (count >= height)
                next();
            break;

        case 5:
            // Scroll up
            if (count == 0)
            {
                tft.fillRectangle(0, 0, ILI9225_LCD_HEIGHT-1, height, COLOR_BLACK);
                ++count;
                return;
            }
            drawLogo(0, height-count);
            ++count;
            if (count >= height)
                next();
            break;

        case 6:
            // Erase from the outside
            if (count == 0)
            {
                drawLogo(0, 0);
                ++count;
                return;
            }
            else
            {
                int n = count-1;
                tft.fillRectangle(n, n, ILI9225_LCD_HEIGHT-1-n, n, COLOR_BLACK);
                tft.fillRectangle(n, height-1-n,
                                  ILI9225_LCD_HEIGHT-1-n, height-1-n,
                                  COLOR_BLACK);
                if (count <= height)
                {
                    tft.fillRectangle(n, n, n, height-1-n, COLOR_BLACK);
                    tft.fillRectangle(ILI9225_LCD_HEIGHT-1-n, n,
                                      ILI9225_LCD_HEIGHT-1-n, height-1-n,
                                      COLOR_BLACK);
                }
                ++count;
                if (count >= height/2)
                    next();
            }
            break;

        case 7:
            // Random dissolve
            if (count == 0)
            {
                drawLogo(0, 0);
                ++count;
                return;
            }
            for (int i = 0; i < 6*(count/ILI9225_LCD_HEIGHT+1); ++i)
            {
                int x = random(ILI9225_LCD_HEIGHT);
                int y = random(height-1);
                int dx = 1+random(16);
                if (x >= dx)
                    x -= dx;
                tft.fillRectangle(x, y, x+dx, y+1, COLOR_BLACK);
            }
            ++count;
            if (count > ILI9225_LCD_HEIGHT*2) // arbitrary factor
                next();
            break;
            
        default:
            mode = 0;
        }
    }

private:
    TFT_22_ILI9225& tft;
    int height = 126;
    int mode = 2;
    bool large_logo = true;
    int count = 0;
    int count2 = 0;
    unsigned long last_tick = 0;
};
