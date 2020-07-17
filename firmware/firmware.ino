// Note: Use https://github.com/MicroBahner/TFT_22_ILI9225

#include "SPI.h"
#include "TFT_22_ILI9225.h"

#include "animater.h"

const char* version = "0.0.4";

const int RED_SW_PIN = 5;
const int GREEN_SW_PIN = 6;

const int RELAY_PIN = 12;

const int TFT_RST = 8;
const int TFT_RS = 9;
const int TFT_CS = 10;   // SS
const int TFT_SDI = 11;  // MOSI
const int TFT_CLK = 13;  // SCK
const int TFT_LED = 7;

const int TFT_BRIGHTNESS = 200;

const int LOCK_OPEN_TIME_MS = 5000;

TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);

const int lcd_top = 38;
const int lcd_line_height_large = 17;
const int lcd_line_height_small = 10;
const int lcd_last_large_line = (ILI9225_LCD_WIDTH - lcd_top)/lcd_line_height_large - 1;
const int lcd_last_small_line = (ILI9225_LCD_WIDTH - lcd_top)/lcd_line_height_small - 1;

Animater anim(tft);

void setup()
{
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, 0);
  
  tft.begin();

  Serial.begin(115200);

  tft.setOrientation(1);
  tft.setFont(Terminal12x16);
  tft.clear();
  tft.setBackgroundColor(COLOR_BLACK);

  randomSeed(analogRead(0));
}

const int BUF_SIZE = 50;
char buf[BUF_SIZE+1];
int buf_index = 0;

const int colours[] =
{
    static_cast<int>(COLOR_WHITE),
    static_cast<int>(COLOR_BLUE),
    static_cast<int>(COLOR_GREEN),
    static_cast<int>(COLOR_RED),
    static_cast<int>(COLOR_NAVY),
    static_cast<int>(COLOR_DARKBLUE),
    static_cast<int>(COLOR_DARKGREEN),
    static_cast<int>(COLOR_DARKCYAN),
    static_cast<int>(COLOR_CYAN),
    static_cast<int>(COLOR_TURQUOISE),
    static_cast<int>(COLOR_INDIGO),
    static_cast<int>(COLOR_DARKRED),
    static_cast<int>(COLOR_OLIVE),
    static_cast<int>(COLOR_GRAY),
    static_cast<int>(COLOR_GREY),
    static_cast<int>(COLOR_SKYBLUE),
    static_cast<int>(COLOR_BLUEVIOLET),
    static_cast<int>(COLOR_LIGHTGREEN),
    static_cast<int>(COLOR_DARKVIOLET),
    static_cast<int>(COLOR_YELLOWGREEN),
    static_cast<int>(COLOR_BROWN),
    static_cast<int>(COLOR_DARKGRAY),
    static_cast<int>(COLOR_DARKGREY),
    static_cast<int>(COLOR_SIENNA),
    static_cast<int>(COLOR_LIGHTBLUE),
    static_cast<int>(COLOR_GREENYELLOW),
    static_cast<int>(COLOR_SILVER),
    static_cast<int>(COLOR_LIGHTGRAY),
    static_cast<int>(COLOR_LIGHTGREY),
    static_cast<int>(COLOR_LIGHTCYAN),
    static_cast<int>(COLOR_VIOLET),
    static_cast<int>(COLOR_AZUR),
    static_cast<int>(COLOR_BEIGE),
    static_cast<int>(COLOR_MAGENTA),
    static_cast<int>(COLOR_TOMATO),
    static_cast<int>(COLOR_GOLD),
    static_cast<int>(COLOR_ORANGE),
    static_cast<int>(COLOR_SNOW),
    static_cast<int>(COLOR_YELLOW)
};

bool drawn_logo = false;

bool red_key_pressed = false;
bool green_key_pressed = false;

void erase_small(int line)
{
    tft.fillRectangle(0, lcd_top+line*lcd_line_height_small,
                      ILI9225_LCD_HEIGHT-1, lcd_top+(line+1)*lcd_line_height_small,
                      COLOR_BLACK);
}

void erase_large(int line)
{
    tft.fillRectangle(0, lcd_top+line*lcd_line_height_large,
                      ILI9225_LCD_HEIGHT-1, lcd_top+(line+1)*lcd_line_height_large,
                      COLOR_BLACK);
}

enum LockState
{
    LOCK_OPEN,
    LOCK_CLOSED,
    LOCK_TIMED
};

LockState lock_state = LOCK_CLOSED;
unsigned long lock_open_tick = 0;

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
            lock_state = LOCK_CLOSED;
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
                    Serial.println("ERROR");
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
                    tft.fillRectangle(0, lcd_top, ILI9225_LCD_HEIGHT-1, ILI9225_LCD_WIDTH-1, COLOR_BLACK);
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
                    const int line = 10*(buf[1] - '0')+buf[2] - '0';
                    if ((line < 0) || (line > lcd_last_large_line))
                    {
                        Serial.println("Bad line number");
                        break;
                    }
                    const int col = 10*(buf[3] - '0')+buf[4] - '0';
                    if ((col < 0) || (col > static_cast<int>(sizeof(colours)/sizeof(colours[0]))))
                    {
                        Serial.println("Bad colour");
                        break;
                    }
                    tft.setFont(Terminal12x16);
                    if (buf[5] != '0')
                        erase_large(line);
                    String s(buf+6);
                    const auto w = tft.getTextWidth(s);
                    tft.drawText((tft.maxX() - w)/2, lcd_top+line*lcd_line_height_large, s, colours[col]);
                    Serial.println("OK T");
                }
                break;
            case 't':
                {
                    // Small text
                    // t<line><colour><erase><text>
                    const int line = 10*(buf[1] - '0')+buf[2] - '0';
                    if ((line < 0) || (line > lcd_last_small_line))
                    {
                        Serial.println("Bad line number");
                        break;
                    }
                    const int col = 10*(buf[3] - '0')+buf[4] - '0';
                    if ((col < 0) || (col > static_cast<int>(sizeof(colours)/sizeof(colours[0]))))
                    {
                        Serial.println("Bad colour");
                        break;
                    }
                    tft.setFont(Terminal6x8);
                    if (buf[5] != '0')
                        erase_small(line);
                    String s(buf+6);
                    const auto w = tft.getTextWidth(s);
                    tft.drawText((tft.maxX() - w)/2, lcd_top+line*lcd_line_height_small, s, colours[col]);
                    Serial.println("OK t");
                }
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
