#include <ctype.h>

//#define PROTOCOL_DEBUG

#include <RDM6300.h>
#include <SoftwareSerial.h>

static constexpr const int PIN_RX = 3;
static constexpr const int PIN_TX = A5; // Not connected
static constexpr const int PIN_GREEN = 5;
static constexpr const int PIN_RED = 6;
static constexpr const int PIN_BUZZER1 = 7;
static constexpr const int PIN_BUZZER2 = 8;

static constexpr const int MIN_BEEP_INTERVAL_MS = 1500;

// 200/500 = boo
// 900/200 = yay

SoftwareSerial swSerial(PIN_RX, PIN_TX);

int beep_duration = 0;
int beep_interval = 0;
int beep_duration_left = 0;
int beep_interval_left = 0;
int beep_last_tick = 0;
bool beep_flag = false;

void update_leds();

void setup()
{
    Serial.begin(9600);
    swSerial.begin(9600);

    pinMode(7, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_BUZZER1, OUTPUT);
    pinMode(PIN_BUZZER2, OUTPUT);

    // set timer2 interrupt at 8kHz
    TCCR2A = 0;// set entire TCCR2A register to 0
    TCCR2B = 0;// same for TCCR2B
    TCNT2  = 0;//initialize counter value to 0
    // set compare match register for 8khz increments
    OCR2A = 249;// = (16*10^6) / (8000*8) - 1 (must be <256)
    // turn on CTC mode
    TCCR2A |= (1 << WGM21);
    // Set CS21 bit for 8 prescaler
    TCCR2B |= (1 << CS21);   
    // enable timer compare interrupt
    TIMSK2 |= (1 << OCIE2A);
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
int sequence_period = 80;
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
    beep_interval = 4000/freq;
    beep_interval_left = beep_interval;
    beep_duration = duration*8;
    beep_duration_left = beep_duration;
}

char current_card[RDM6300::ID_SIZE * 2 + 1] = { 0 };

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
                Serial.print(F("Value must follow I"));
                return;
            }
            if ((inten < 1) || (inten > 255))
            {
                Serial.print(F("Intensity must be between 1 and 255"));
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
            int freq = 0;
            ++i;
            if (!parse_int(line, i, freq))
            {
                Serial.print(F("Value must follow S: "));
                Serial.println(line);
                return;
            }
            if ((freq < 100) || (freq > 8000))
            {
                Serial.print(F("Frequency must be between 100 and 8000: "));
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
    sequence_period = period*8; // 8 kHz
    sequence_repeats = repeats;
    sequence_iteration = 0;
    for (int i = 0; i < seq_len; ++i)
        sequence[i] = seq[i];
    sequence_len = seq_len;
    if (send_reply)
        Serial.println(F("OK"));
}

void update_leds()
{
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
                // Idle LED pattern
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

SIGNAL(TIMER2_COMPA_vect) 
{
    if (beep_duration_left >= 0)
    {
        --beep_duration_left;
        --beep_interval_left;
        if (beep_interval_left <= 0)
        {
            beep_interval_left = beep_interval;
            if (beep_flag)
            {
                digitalWrite(PIN_BUZZER1, 0);
                digitalWrite(PIN_BUZZER2, 1);
            }
            else
            {
                digitalWrite(PIN_BUZZER1, 1);
                digitalWrite(PIN_BUZZER2, 0);
            }
            beep_flag = !beep_flag;
        }
    }
    static int delay_counter = 0;
    if (++delay_counter < sequence_period)
        return;
    delay_counter = 0;
    update_leds();
}

const int MAX_LINE_LENGTH = 80;
char line[MAX_LINE_LENGTH+1];
int line_len = 0;

unsigned long last_beep_tick = 0;

void loop()
{
    delay(100);
    
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
            const auto now = millis();
            if ((last_beep_tick == 0) ||
                (now - last_beep_tick > MIN_BEEP_INTERVAL_MS))
            {
                last_beep_tick = now;
                beep(1200, 100);
            }
            make_swserial_work();
        }
    }

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

}
