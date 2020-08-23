#include <ctype.h>

//#define PROTOCOL_DEBUG

#include <RDM6300.h>
#include <SoftwareSerial.h>

const int PIN_RX = 3;
const int PIN_TX = A5; // Not connected
const int PIN_GREEN = 5;
const int PIN_RED = 6;
const int PIN_BUZZER1 = 7;
const int PIN_BUZZER2 = 8;



// 200/500 = boo
// 900/200 = yay

SoftwareSerial swSerial(PIN_RX, PIN_TX);

void setup()
{
    Serial.begin(115200);
    swSerial.begin(9600);

    pinMode(7, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_BUZZER1, OUTPUT);
    pinMode(PIN_BUZZER2, OUTPUT);
}

RDM6300 decoder;
int n = 0;

const int MAX_SEQ_SIZE = 250;
enum class Sequence
{
    Red,
    Green,
    Both,
    None
};
char sequence[MAX_SEQ_SIZE];
int sequence_len = 0;
int sequence_index = 0;
int sequence_period = 10;
int delay_counter = 0;
int sequence_repeats = 1; // to force idle sequence on startup
int sequence_iteration = 0;
int pwm_max = 64;
int buzzer_on = 255;

bool parse_int(const char* line, int& index, int& value)
{
    if (!isdigit(line[index]))
    {
        Serial.print(F("Expected number, got "));
        Serial.println(line[index]);
        return false;
    }
    value = 0;
    while (isdigit(line[index]))
    {
        value = value*10 + line[index] - '0';
        ++index;
    }
    return true;
}

bool fill_seq(char* seq, int& index, int reps, Sequence elem)
{
    while (reps--)
    {
        if (index >= MAX_SEQ_SIZE)
        {
            Serial.println(F("Sequence too long"));
            return false;
        }
        seq[index++] = (char) elem;
    }
    return true;
}

bool swserial_active = false;

void make_swserial_work()
{
    if (!swserial_active)
    {
        swSerial.end();
        swSerial.begin(9600);
    }
}

void beep(int freq, int duration)
{
    int count = ((long) duration)*freq/2000;
    int del = 500000/freq;
    for (int i = 0; i < count; ++i)
    {
        digitalWrite(PIN_BUZZER1, 0);
        digitalWrite(PIN_BUZZER2, 1);
        delayMicroseconds(del);
        digitalWrite(PIN_BUZZER1, 1);
        digitalWrite(PIN_BUZZER2, 0);
        delayMicroseconds(del);
    }
}

char current_card[RDM6300::ID_SIZE * 2 + 1] = { 0 };
bool card_sound_active = false;

void decode_line(const char* line, bool send_reply = true)
{
    int i = 0;
    switch (tolower(line[i]))
    {
    case 'v':
        // Show version
        Serial.println(F("ACS cardreader v 0.99"));
        return;

    case 'c':
        // Read card ID
        Serial.print(F("ID"));
        Serial.println(current_card);
        current_card[0] = '\0';
        return;

    case 'i':
        // Set LED intensity
        {
            int inten = 0;
            ++i;
            if (!parse_int(line, i, inten))
            {
                Serial.print(F("Value must follow I: "));
                Serial.println(line);
                return;
            }
            if ((inten < 1) || (inten > 255))
            {
                Serial.print(F("Intensity must be between 1 and 255: "));
                Serial.println(line);
                return;
            }
            pwm_max = inten;
            Serial.println(F("OK"));
        }
        return;
        
    case 'b':
        // Set buzzer intensity
        {
            int inten = 0;
            ++i;
            if (!parse_int(line, i, inten))
            {
                Serial.print(F("Value must follow B: "));
                Serial.println(line);
                return;
            }
            if ((inten < 1) || (inten > 255))
            {
                Serial.print(F("Intensity must be between 1 and 255: "));
                Serial.println(line);
                return;
            }
            buzzer_on = inten;
            Serial.println(F("OK"));
        }
        return;

    case 's':
        // Make sound
        {
            card_sound_active = false;
            int freq = 0;
            ++i;
            if (!parse_int(line, i, freq))
            {
                Serial.print(F("Value must follow S: "));
                Serial.println(line);
                return;
            }
            if ((freq < 100) || (freq > 10000))
            {
                Serial.print(F("Frequency must be between 100 and 10000: "));
                Serial.println(line);
                return;
            }
            int duration = 0;
            ++i;
            if (!parse_int(line, i, duration))
            {
                Serial.print(F("Duration must follow frequency: "));
                Serial.println(line);
                return;
            }
            if ((duration < 10) || (duration > 1000))
            {
                Serial.print(F("Duration must be between 10 and 1000: "));
                Serial.println(line);
                return;
            }
            beep(freq, duration);
            Serial.println(F("OK"));
        }
        return;
        
    case 'p':
        break;

    default:
        Serial.print(F("Line must begin with P: "));
        Serial.println(line);
        return;
    }
    ++i;
    // P<period>R<repeats>S<sequence>
    int period = 0;
    if (!parse_int(line, i, period))
    {
        Serial.print(F("Period must follow P: "));
        Serial.println(line);
        return;
    }
    if (period <= 0)
    {
        Serial.print(F("Period cannot be zero: "));
        Serial.println(line);
        return;
    }
    if (tolower(line[i]) != 'r')
    {
        Serial.print(F("Period must be followed by R, got "));
        Serial.print(line[i]);
        Serial.print(F(": "));
        Serial.println(line);
        return;
    }
    ++i;
    int repeats = 0;
    if (!parse_int(line, i, repeats))
    {
        Serial.print(F("Repeats must follow R: "));
        Serial.println(line);
        return;
    }
    if (tolower(line[i]) != 's')
    {
        Serial.print(F("Repeats must be followed by S, got "));
        Serial.print(line[i]);
        Serial.print(F(": "));
        Serial.println(line);
        return;
    }
    ++i;
    char seq[MAX_SEQ_SIZE];
    int seq_len = 0;
    while (line[i])
    {
        if (seq_len == MAX_SEQ_SIZE)
        {
            Serial.print(F("Sequence too long: "));
            Serial.println(line);
            return;
        }
        switch (tolower(line[i]))
        {
        case 'r':
            seq[seq_len++] = (char) Sequence::Red;
            break;
        case 'g':
            seq[seq_len++] = (char) Sequence::Green;
            break;
        case 'b':
            seq[seq_len++] = (char) Sequence::Both;
            break;
        case 'n':
            seq[seq_len++] = (char) Sequence::None;
            break;
        case 'x':
            {
                int reps = 0;
                ++i;
                if (!parse_int(line, i, reps))
                {
                    Serial.print(F("X must be followed by repeats"));
                    Serial.println(line);
                    return;
                }
                switch (tolower(line[i]))
                {
                case 'r':
                    if (!fill_seq(seq, seq_len, reps, Sequence::Red))
                        return;
                    break;
                case 'g':
                    if (!fill_seq(seq, seq_len, reps, Sequence::Green))
                        return;
                    break;
                case 'b':
                    if (!fill_seq(seq, seq_len, reps, Sequence::Both))
                        return;
                    break;
                case 'n':
                    if (!fill_seq(seq, seq_len, reps, Sequence::None))
                        return;
                    break;
                default:
                    Serial.print(F("Unexpected character after X: "));
                    Serial.print(line[i]);
                    Serial.print(F(": "));
                    Serial.println(line);
                    return;
                }
            }
            break;
        default:
            Serial.print(F("Unexpected sequence character: "));
            Serial.print(line[i]);
            Serial.print(F(": "));
            Serial.println(line);
            return;
        }
        ++i;
    }
    sequence_index = 0;
    sequence_period = 10*period;
    sequence_repeats = repeats;
    sequence_iteration = 0;
    for (int i = 0; i < seq_len; ++i)
        sequence[i] = seq[i];
    sequence_len = seq_len;
    if (send_reply)
        Serial.println(F("OK"));
}

const int MAX_LINE_LENGTH = 80;
char line[MAX_LINE_LENGTH+1];
int line_len = 0;

void loop()
{
    const int c = swSerial.read();
    if (c > 0)
    {
        //Serial.println(c);
        swserial_active = true;
        if (decoder.add_byte(c))
        {
            swserial_active = false;
            strcpy(current_card, decoder.get_id());
            //Serial.print(F("ID.size: ")); Serial.println(strlen(current_card));
            beep(1200, 100);
            make_swserial_work();
        }
    }
    if (card_sound_active)
    {
        //delay(100);
        //analogWrite(PIN_BUZZER, card_sound_state ? buzzer_on : 0);
    }

    delayMicroseconds(100);
    if (++delay_counter < sequence_period)
        return;
    delay_counter = 0;

    if (Serial.available())
    {
        const char c = Serial.read();
        if ((c == '\r') || (c == '\n'))
        {
            line[line_len] = 0;
            line_len = 0;
            decode_line(line);
            make_swserial_work();
        }
        else if (line_len < MAX_LINE_LENGTH)
            line[line_len++] = c;
        else
        {
            Serial.print(F("Line too long: "));
            Serial.println(line);
            line_len = 0;
            make_swserial_work();
        }
    }

    if (sequence_index >= sequence_len)
    {
        sequence_index = 0;
        if (sequence_repeats > 0)
        {
            if (sequence_iteration >= sequence_repeats)
            {
                // Done
                analogWrite(PIN_GREEN, 0);
                analogWrite(PIN_RED, 0);
                sequence_len = 0;
                decode_line("P10R0SGX99N", false);
                return;
            }
            ++sequence_iteration;
        }
    }
    if (sequence_index < sequence_len)
    {
        switch ((Sequence) sequence[sequence_index++])
        {
        case Sequence::Red:
            analogWrite(PIN_GREEN, 0);
            analogWrite(PIN_RED, pwm_max);
            break;
        case Sequence::Green:
            analogWrite(PIN_GREEN, pwm_max);
            analogWrite(PIN_RED, 0);
            break;
        case Sequence::Both:
            analogWrite(PIN_GREEN, pwm_max);
            analogWrite(PIN_RED, pwm_max);
            break;
        case Sequence::None:
            analogWrite(PIN_GREEN, 0);
            analogWrite(PIN_RED, 0);
            break;
        }
    }
}
