// LCD library: https://github.com/prenticedavid/MCUFRIEND_kbv

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#include <SPI.h>          // f.k. for Arduino-1.5.2
#include "Adafruit_GFX.h"// Hardware-specific library
#include <OPENSMART_kbv.h>

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

OPENSMART_kbv tft;

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#include "animater.h"

const char* version = "0.0.4";

const int RED_SW_PIN = 3;
const int GREEN_SW_PIN = 2;

const int RELAY_PIN = 12;

const int TFT_BRIGHTNESS = 200;

const int LOCK_OPEN_TIME_MS = 5000;

const int screen_height = 240;
const int screen_width = 400;
   
const int lcd_top = 38;
const int lcd_line_height_large = 21;
const int lcd_line_height_small = 16;
const int lcd_last_large_line = (screen_height - lcd_top)/lcd_line_height_large - 1;
const int lcd_last_small_line = (screen_height - lcd_top)/lcd_line_height_small - 1;
  
Animater anim(tft);

void setup()
{
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, 0);

  Serial.begin(115200);

  uint16_t ID = tft.readID();
  Serial.print("ID = 0x");
  Serial.println(ID, HEX);
  tft.begin(ID);

  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setFont(&FreeSans12pt7b);
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

bool red_key_pressed = false;
bool green_key_pressed = false;

void erase_small(int line)
{
    tft.fillRect(0, lcd_top+line*lcd_line_height_small,
                 screen_width, lcd_line_height_small,
                 TFT_BLACK);
}

void erase_large(int line)
{
    tft.fillRect(0, lcd_top+line*lcd_line_height_large,
                 screen_width, lcd_line_height_large,
                 TFT_BLACK);
}

enum LockState
{
    LOCK_OPEN,
    LOCK_CLOSED,
    LOCK_TIMED
};

LockState lock_state = LOCK_CLOSED;
unsigned long lock_open_tick = 0;

int get_2digit_number(const char* buf)
{
    return 10*(buf[1] - '0')+buf[2] - '0';
}

void loop()
{
    if (!digitalRead(RED_SW_PIN))
        red_key_pressed = true;
    if (!digitalRead(GREEN_SW_PIN))
        green_key_pressed = true;

    switch (lock_state)
    {
    case LOCK_OPEN:
    case LOCK_TIMED:
        digitalWrite(RELAY_PIN, 1);
        break;
    case LOCK_CLOSED:
        digitalWrite(RELAY_PIN, 0);
        break;
    }

    if (lock_state == LOCK_TIMED)
    {
        const auto elapsed = millis() - lock_open_tick;
        if (elapsed > LOCK_OPEN_TIME_MS)
            lock_open_tick = LOCK_CLOSED;
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
                Serial.print("ACS UI v ");
                Serial.println(version);
                break;

            case 'L':
                // Control lock
                // L<on>
                switch (buf[1])
                {
                case '0':
                    lock_state = LOCK_CLOSED;
                    break;
                case '1':
                    lock_state = LOCK_OPEN;
                    break;
                case 'T':
                    lock_state = LOCK_TIMED;
                    lock_open_tick = millis();
                    break;
                default:
                    break;
                }
                Serial.println("OK L");
                break;
            case 'C':
                // Clear screen
                if (!drawn_logo)
                {
                    drawn_logo = true;
                    anim.reset();
                }
                else
                  tft.fillRect(0, lcd_top, screen_width, screen_height, TFT_BLACK);
                Serial.println("OK C");
                break;
            case 'E':
                {
                    // Erase large line
                    // E<line>
                    const int line = 10*(buf[1] - '0')+buf[2] - '0';
                    if ((line < 0) || (line > lcd_last_large_line))
                    {
                        Serial.print("Bad line number: ");
                        Serial.println(line);
                        break;
                    }
                    erase_large(line);
                    Serial.println("OK E");
                }
                break;
            case 'e':
                {
                    // Erase small line
                    // e<line>
                    const int line = 10*(buf[1] - '0')+buf[2] - '0';
                    if ((line < 0) || (line > lcd_last_small_line))
                    {
                        Serial.print("Bad line number: ");
                        Serial.println(line);
                        break;
                    }
                    erase_small(line);
                    Serial.println("OK e");
                }
                break;
            case 'T':
                {
                    // Large text
                    // T<line><colour><erase><text>
                    const int line = get_2digit_number(buf);
                    if ((line < 0) || (line > lcd_last_large_line))
                    {
                        Serial.println("Bad line number");
                        break;
                    }
                    const int col = get_2digit_number(buf + 2);
                    if ((col < 0) || (col > static_cast<int>(sizeof(colours)/sizeof(colours[0]))))
                    {
                        Serial.println("Bad colour");
                        break;
                    }
                    tft.setFont(&FreeSans12pt7b);
                    if (buf[5] != '0')
                        erase_large(line);
                    String s(buf+6);
                    int16_t dummy1;
                    uint16_t dummy2, w;
                    tft.getTextBounds(s, 0, 0, &dummy1, &dummy1, &w, &dummy2);
                    tft.setCursor((screen_width - w)/2, lcd_top+(line+1)*lcd_line_height_large);
                    tft.setTextColor(colours[col]);
                    tft.print(s);
                    Serial.println("OK T");
                }
                break;
            case 't':
                {
                    // Small text
                    // t<line><colour><erase><text>
                    const int line = get_2digit_number(buf);
                    if ((line < 0) || (line > lcd_last_small_line))
                    {
                        Serial.println("Bad line number");
                        break;
                    }
                    const int col = get_2digit_number(buf + 2);
                    if ((col < 0) || (col > static_cast<int>(sizeof(colours)/sizeof(colours[0]))))
                    {
                        Serial.println("Bad colour");
                        break;
                    }
                    tft.setFont(&FreeSans9pt7b);
                    if (buf[5] != '0')
                        erase_small(line);
                    String s(buf+6);
                    int16_t dummy1;
                    uint16_t dummy2, w;
                    tft.getTextBounds(s, 0, 0, &dummy1, &dummy1, &w, &dummy2);
                    tft.setCursor((screen_width - w)/2, lcd_top+(line+1)*lcd_line_height_small);
                    tft.setTextColor(colours[col]);
                    tft.print(s);
                    Serial.println("OK t");
                }
                break;
            case 'c':
                // Clock
                tft.setFont(&FreeSans9pt7b);
                tft.setCursor(screen_width/2 - 20, screen_height - lcd_line_height_small);
                tft.setTextColor(TFT_GREEN);
                tft.print(buf + 1);
                Serial.println("OK c");
                break;

            case 'S':
                Serial.print("S");
                Serial.print(red_key_pressed);
                Serial.println(green_key_pressed);
                red_key_pressed = green_key_pressed = false;
                break;
                
            default:
                Serial.print("Unknown command: ");
                Serial.println(buf);
                break;
            }
        }
        else
        {
            if (buf_index >= BUF_SIZE)
            {
                Serial.println("Error: Line too long");
                buf_index = 0;
                return;
            }
            buf[buf_index++] = c;
        }
    }

    anim.update();
}
