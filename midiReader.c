// Compileren met gcc -o midiReader midiReader.c -lasound
#include <stdio.h>
#include <alsa/asoundlib.h>

void parseMidiMessage(unsigned char *buffer) {
    unsigned char status = buffer[0];
    unsigned char data1 = buffer[1];
    unsigned char data2 = buffer[2];

    switch (status & 0xF0)
    {
        case 0x80: // Note Off
            printf("Note Off: %d, Velocity: %d\n", data1, data2);
            break;
        case 0x90: // Note On
            printf("Note On: %d, Velocity: %d\n", data1, data2);
            break;
        case 0xA0: // Aftertouch
            printf("Aftertouch: %d, Pressure: %d\n", data1, data2);
            break;
        case 0xB0: // Control Change
            printf("Control Change: %d, Value: %d\n", data1, data2);
            break;
        case 0xC0: // Program Change
            printf("Program Change: %d\n", data1);
            break;
        case 0xD0: // Channel Pressure
            printf("Channel Pressure: %d\n", data1);
            break;
        case 0xE0: // Pitch Bend
            printf("Pitch Bend: %d\n", (data2 << 7) | data1);
            break;
        default:
            printf("Unknown MIDI message\n");
            break;
    }
}

int main() {
    snd_rawmidi_t *midiin = NULL;
    snd_rawmidi_open(&midiin, NULL, "hw:1,0,0", SND_RAWMIDI_NONBLOCK);

    if (!midiin)
    {
        fprintf(stderr, "Error opening MIDI input\n");
        return 1;
    }

    unsigned char buffer[3];
    int status;
    while (1)
    {
        status = snd_rawmidi_read(midiin, buffer, sizeof(buffer));
        if (status > 0)
        {
            printf("[MIDI] Bytes ontvangen: %02x %02x %02x\n       ", buffer[0], buffer[1], buffer[2]);
            parseMidiMessage(buffer);
        }
        else
            if (status < 0 && status != -EAGAIN)
            {
                fprintf(stderr, "Error reading MIDI input\n");
                break;
            }
    }

    snd_rawmidi_close(midiin);
    return 0;
}
