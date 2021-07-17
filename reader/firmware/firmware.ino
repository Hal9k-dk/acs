#include <ctype.h>

#include <RDM6300.h>
#include <SoftwareSerial.h>

const int PIN_RX = 2;
const int PIN_TX = A5; // Not connected
const int PIN_GREEN = 5; // Needs PWM support
const int PIN_RED = 3; // Needs PWM support
const int PIN_BUZZER = 6; // Needs PWM support

SoftwareSerial swSerial(PIN_RX, PIN_TX);

void setup()
{
    Serial.begin(115200);
    swSerial.begin(9600);

    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
}

RDM6300 decoder;
int n = 0;

const int MAX_SEQ_SIZE = 500;
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
int sequence_period = 1;
int delay_counter = 0;
int sequence_repeats = 1; // to force idle sequence on startup
int sequence_iteration = 0;
int pwm_max = 255;
int buzzer_on = 255;

bool parse_int(const char* line, int& index, int& value)
{
    if (!isdigit(line[index]))
    {
        Serial.print("Expected number, got ");
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
            Serial.println("Sequence too long");
            return false;
        }
        seq[index++] = (char) elem;
    }
    return true;
}

String current_card;

void decode_line(const char* line, bool send_reply = true)
{
    int i = 0;
    switch (tolower(line[i]))
    {
    case 'v':
        // Show version
        Serial.println("ACS cardreader v 0.8 (active buzzer)");
        return;

    case 'c':
        // Read card ID
        Serial.print("ID");
        Serial.println(current_card);
        current_card = "";
        return;

    case 'i':
        // Set LED intensity
        {
            int inten = 0;
            ++i;
            if (!parse_int(line, i, inten))
            {
                Serial.print("Value must follow I: ");
                Serial.println(line);
                return;
            }
            if ((inten < 1) || (inten > 255))
            {
                Serial.print("Intensity must be between 1 and 255: ");
                Serial.println(line);
                return;
            }
            pwm_max = inten;
            Serial.println("OK");
        }
        return;
        
    case 'b':
        // Set buzzer intensity
        {
            int inten = 0;
            ++i;
            if (!parse_int(line, i, inten))
            {
                Serial.print("Value must follow B: ");
                Serial.println(line);
                return;
            }
            if ((inten < 1) || (inten > 255))
            {
                Serial.print("Intensity must be between 1 and 255: ");
                Serial.println(line);
                return;
            }
            buzzer_on = inten;
            Serial.println("OK");
        }
        return;
        
    case 'p':
        break;

    default:
        Serial.print("Line must begin with P: ");
        Serial.println(line);
        return;
    }
    ++i;
    int period = 0;
    if (!parse_int(line, i, period))
    {
        Serial.print("Period must follow P: ");
        Serial.println(line);
        return;
    }
    if (period <= 0)
    {
        Serial.print("Period cannot be zero: ");
        Serial.println(line);
        return;
    }
    if (tolower(line[i]) != 'r')
    {
        Serial.print("Period must be followed by R, got ");
        Serial.print(line[i]);
        Serial.print(": ");
        Serial.println(line);
        return;
    }
    ++i;
    int repeats = 0;
    if (!parse_int(line, i, repeats))
    {
        Serial.print("Repeats must follow R: ");
        Serial.println(line);
        return;
    }
    if (tolower(line[i]) != 's')
    {
        Serial.print("Repeats must be followed by S, got ");
        Serial.print(line[i]);
        Serial.print(": ");
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
            Serial.print("Sequence too long: ");
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
                    Serial.print("X must be followed by repeats");
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
                    Serial.print("Unexpected character after X: ");
                    Serial.print(line[i]);
                    Serial.print(": ");
                    Serial.println(line);
                    return;
                }
            }
            break;
        default:
            Serial.print("Unexpected sequence character: ");
            Serial.print(line[i]);
            Serial.print(": ");
            Serial.println(line);
            return;
        }
        ++i;
    }
    sequence_index = 0;
    sequence_period = period;
    sequence_repeats = repeats;
    sequence_iteration = 0;
    for (int i = 0; i < seq_len; ++i)
        sequence[i] = seq[i];
    sequence_len = seq_len;
    if (send_reply)
        Serial.println("OK");
}

const int MAX_LINE_LENGTH = 80;
char line[MAX_LINE_LENGTH+1];
int line_len = 0;

unsigned long card_flash_start = 0;
bool card_flash_active = false;
bool card_flash_state = false;

void loop()
{
    delay(1);
    
    const auto c = swSerial.read();
    if (c > 0)
        if (decoder.add_byte(c))
        {
            current_card = decoder.get_id();
            if (!card_flash_active)
            {
                card_flash_active = true;
                const auto now = millis();
                card_flash_start = now;
                analogWrite(PIN_BUZZER, buzzer_on);
            }
        }

    if (card_flash_active)
    {
        analogWrite(PIN_RED, card_flash_state ? 0 : pwm_max);
        analogWrite(PIN_GREEN, card_flash_state ? pwm_max : 0);
        card_flash_state = !card_flash_state;
        delay(100);
        analogWrite(PIN_BUZZER, card_flash_state ? buzzer_on : 0);
        const auto now = millis();
        if (now - card_flash_start > 500)
        {
            card_flash_active = false;
            analogWrite(PIN_BUZZER, 0);
        }
        return;
    }

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
        }
        else if (line_len < MAX_LINE_LENGTH)
            line[line_len++] = c;
        else
        {
            Serial.print("Line too long: ");
            Serial.println(line);
            line_len = 0;
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
                decode_line("P5R0SGX199N", false);
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
