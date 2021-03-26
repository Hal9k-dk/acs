// LCD library: https://github.com/prenticedavid/MCUFRIEND_kbv

#include <SPI.h>          // f.k. for Arduino-1.5.2
#include "Adafruit_GFX.h"// Hardware-specific library
#include <MCUFRIEND_kbv.h>

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>

#define SMALL_FONT  FreeSans9pt7b
#define LARGE_FONT  FreeSansBold18pt7b

MCUFRIEND_kbv tft;

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifdef USE_ANIMATER
#  include "animater.h"
#endif

const char* version = "1.1.2";

const int SW_PIN = A5;

const int TFT_BRIGHTNESS = 200;

const int screen_height = 240;
const int screen_width = 400;
   
#ifdef USE_ANIMATER
const int lcd_top = 38;
#else
const int lcd_top = 0;
#endif
const int lcd_line_height_large = 40;
const int lcd_baseline_offset_large = 8;
const int lcd_line_height_small = 22;
const int lcd_baseline_offset_small = 4;
const int lcd_last_large_line = (screen_height - lcd_top)/lcd_line_height_large - 1;
const int lcd_last_small_line = (screen_height - lcd_top)/lcd_line_height_small - 1;

#ifdef USE_ANIMATER
Animater anim(tft);
#endif

void setup()
{
  pinMode(SW_PIN, INPUT);

  Serial.begin(115200);

  uint16_t ID = tft.readID();
  tft.begin(ID);

  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setFont(&LARGE_FONT);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);

  randomSeed(analogRead(0));
}

const int BUF_SIZE = 50;
char buf[BUF_SIZE+1];
int buf_index = 0;

const int colours[] =
{
    static_cast<int>(TFT_WHITE),
    static_cast<int>(TFT_BLUE),
    static_cast<int>(TFT_GREEN),
    static_cast<int>(TFT_RED),
    static_cast<int>(TFT_NAVY),
    static_cast<int>(TFT_DARKGREEN),
    static_cast<int>(TFT_DARKCYAN),
    static_cast<int>(TFT_CYAN),
    static_cast<int>(TFT_MAROON),
    static_cast<int>(TFT_OLIVE),
    static_cast<int>(TFT_LIGHTGREY),
    static_cast<int>(TFT_DARKGREY),
    static_cast<int>(TFT_MAGENTA),
    static_cast<int>(TFT_ORANGE),
    static_cast<int>(TFT_YELLOW)
};

bool drawn_logo = true;//false;

static bool key_pressed[3];

void erase_small(int line)
{
    tft.fillRect(0, lcd_top+line*lcd_line_height_small+1,
                 screen_width, lcd_line_height_small,
                 TFT_BLACK);
}

void erase_large(int line)
{
    tft.fillRect(0, lcd_top+line*lcd_line_height_large+1,
                 screen_width, lcd_line_height_large,
                 TFT_BLACK);
}

int get_2digit_number(const char* buf)
{
    return 10*(buf[1] - '0')+buf[2] - '0';
}

void loop()
{
    const auto sw = analogRead(SW_PIN);
    
    // R1/R4: 512
    // R2/R4: 704
    // R3/R4: 856
    if (!key_pressed[0] && !key_pressed[1] && !key_pressed[2])
    {
        if (sw < 522 && sw > 502)
            key_pressed[0] = true;
        else if (sw < 714 && sw > 694)
            key_pressed[1] = true;
        else if (sw < 866 && sw > 846)
            key_pressed[2] = true;
    }    
    if (Serial.available())
    {
        // Command
        char c = Serial.read();
        if ((c == '\r') || (c == '\n'))
        {
            buf[buf_index] = 0;
            buf_index = 0;
            switch (buf[0])
            {
            case 'V':
                // Version
                Serial.print(F("ACS UI v "));
                Serial.println(version);
                break;

            case 'C':
                // Clear screen
#ifdef USE_ANIMATER
                if (!drawn_logo)
                {
                    drawn_logo = true;
                    anim.reset();
                }
                else
#endif
                  tft.fillRect(0, lcd_top, screen_width, screen_height, TFT_BLACK);
                Serial.println(F("OK C"));
                break;
            case 'E':
                {
                    // Erase large line
                    // E<line>
                    const int line = get_2digit_number(buf);
                    if ((line < 0) || (line > lcd_last_large_line))
                    {
                        Serial.print(F("Bad line number: "));
                        Serial.println(buf);
                        break;
                    }
                    erase_large(line);
                    Serial.println(F("OK E"));
                }
                break;
            case 'e':
                {
                    // Erase small line
                    // e<line>
                    const int line = get_2digit_number(buf);
                    if ((line < 0) || (line > lcd_last_small_line))
                    {
                        Serial.print(F("Bad line number: "));
                        Serial.println(buf);
                        break;
                    }
                    erase_small(line);
                    Serial.println(F("OK e"));
                }
                break;
            case 'T':
                {
                    // Large text
                    // T<line><colour><erase><text>
                    const int line = get_2digit_number(buf);
                    if ((line < 0) || (line > lcd_last_large_line))
                    {
                        Serial.print(F("Bad line number: "));
                        Serial.println(buf);
                        break;
                    }
                    const int col = get_2digit_number(buf + 2);
                    if ((col < 0) || (col > static_cast<int>(sizeof(colours)/sizeof(colours[0]))))
                    {
                        Serial.print(F("Bad colour: "));
                        Serial.println(buf);
                        break;
                    }
                    tft.setFont(&LARGE_FONT);
                    if (buf[5] != '0')
                        erase_large(line);
                    String s(buf+6);
                    int16_t dummy1;
                    uint16_t dummy2, w;
                    tft.getTextBounds(s, 0, 0, &dummy1, &dummy1, &w, &dummy2);
                    tft.setCursor((screen_width - w)/2, lcd_top+(line+1)*lcd_line_height_large-lcd_baseline_offset_large);
                    tft.setTextColor(colours[col]);
                    tft.print(s);
                    Serial.println(F("OK T"));
                }
                break;
            case 't':
                {
                    // Small text
                    // t<line><colour><erase><text>
                    const int line = get_2digit_number(buf);
                    if ((line < 0) || (line > lcd_last_small_line))
                    {
                        Serial.print(F("Bad line number: "));
                        Serial.println(buf);
                        break;
                    }
                    const int col = get_2digit_number(buf + 2);
                    if ((col < 0) || (col > static_cast<int>(sizeof(colours)/sizeof(colours[0]))))
                    {
                        Serial.print(F("Bad colour: "));
                        Serial.println(buf);
                        break;
                    }
                    tft.setFont(&SMALL_FONT);
                    if (buf[5] != '0')
                        erase_small(line);
                    String s(buf+6);
                    int16_t dummy1;
                    uint16_t dummy2, w;
                    tft.getTextBounds(s, 0, 0, &dummy1, &dummy1, &w, &dummy2);
                    tft.setCursor((screen_width - w)/2, lcd_top+(line+1)*lcd_line_height_small-lcd_baseline_offset_small);
                    tft.setTextColor(colours[col]);
                    tft.print(s);
                    Serial.println(F("OK t"));
                }
                break;
            case 'c':
                // Clock
                tft.fillRect(screen_width/2 - 50, screen_height - 2*lcd_line_height_small + 2,
                             100, lcd_line_height_small,
                             TFT_BLACK);
                tft.setFont(&SMALL_FONT);
                tft.setCursor(screen_width/2 - 20, screen_height - lcd_line_height_small);
                tft.setTextColor(TFT_GREEN);
                tft.print(buf + 1);
                Serial.println(F("OK c"));
                break;

            case 'S':
                Serial.print(F("S"));
                for (size_t i = 0; i < sizeof(key_pressed)/sizeof(key_pressed[0]); ++i)
                    Serial.print(key_pressed[i]);
                Serial.println();
                for (size_t i = 0; i < sizeof(key_pressed)/sizeof(key_pressed[0]); ++i)
                    key_pressed[i] = false;
                break;
                
            default:
                Serial.print(F("Unknown command: "));
                Serial.println(buf);
                break;
            }
        }
        else
        {
            if (buf_index >= BUF_SIZE)
            {
                Serial.println(F("Error: Line too long"));
                buf_index = 0;
                return;
            }
            buf[buf_index++] = c;
        }
    }
#ifdef USE_ANIMATER
    anim.update();
#endif
}
