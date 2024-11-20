#include "OLED.h"
#include <alsa/asoundlib.h>
#include <fluidsynth.h>

void parseMidiMessage(fluid_synth_t* synth, unsigned char* buffer)
{
    unsigned char status = buffer[0];
    unsigned char data1 = buffer[1];
    unsigned char data2 = buffer[2];

    switch (status & 0xF0)
    {
        case 0x80: // Note Off
            fluid_synth_noteoff(synth, status & 0x0F, data1);
            printf("Note Off: Channel %d, Note %d\n", status & 0x0F, data1);
            OLED_writeLine(5, 4, ":-|");
            break;

        case 0x90: // Note On
            if (data2 == 0)
            {
                fluid_synth_noteoff(synth, status & 0x0F, data1); // Note On with velocity 0 is Note Off
                printf("Note Off: Channel %d, Note %d\n", status & 0x0F, data1);
                OLED_writeLine(5, 4, ":-|");
            }
            else
            {
                fluid_synth_noteon(synth, status & 0x0F, data1, data2);
                printf("Note On: Channel %d, Note %d, Velocity %d\n", status & 0x0F, data1, data2);
                OLED_writeLine(5, 4, ":-o");
            }
            break;

        case 0xC0: // Program Change
            fluid_synth_program_change(synth, status & 0x0F, data1);
            printf("Program Change: Channel %d, Program %d\n", status & 0x0F, data1);
            break;

        case 0xE0: // Pitch Bend
            fluid_synth_pitch_bend(synth, status & 0x0F, (data2 << 7) | data1);
            printf("Pitch Bend: Channel %d, Value %d\n", status & 0x0F, (data2 << 7) | data1);
            break;

        default:
            break;
    }
}

int main()
{
    OLED_init();
    OLED_clear();
    OLED_drawText6x8(5, 10, "Hallo Freddy!");
    OLED_drawText6x8(5, 10+8, "We gaan dat hier");
    OLED_drawText6x8(5, 10+8+8, "ne keer starten e...");

    // Create settings
    fluid_settings_t* settings = new_fluid_settings();
    if (!settings)
    {
        OLED_clear();
        OLED_drawText6x8(5, 10, "Settings init Error");
        return 1;
    }

    // Create synthesizer
    fluid_synth_t* synth = new_fluid_synth(settings);
    if (!synth)
    {
        OLED_clear();
        OLED_drawText6x8(5, 10, "Synth init Error");
        delete_fluid_settings(settings);
        return 1;
    }

    // Load a SoundFont
    int sfont_id = fluid_synth_sfload(synth, "/home/kerman/FluidSynth/fluidsynth/sf2/VintageDreamsWaves-v2.sf2", 1);
    if (sfont_id == FLUID_FAILED)
    {
        OLED_clear();
        OLED_drawText6x8(5, 10, "SFont Error");
        delete_fluid_synth(synth);
        delete_fluid_settings(settings);
        return 1;
    }

    // Create audio driver
    fluid_audio_driver_t* adriver = new_fluid_audio_driver(settings, synth);
    if (!adriver)
    {
        OLED_clear();
        OLED_drawText6x8(5, 10, "Audio init Error");
        delete_fluid_synth(synth);
        delete_fluid_settings(settings);
        return 1;
    }

    // Open MIDI input port
    snd_rawmidi_t *midiin = NULL;
    snd_rawmidi_open(&midiin, NULL, "hw:1,0,0", SND_RAWMIDI_NONBLOCK);
    if (!midiin)
    {
        OLED_clear();
        OLED_drawText6x8(5, 10, "MIDI init Error");
        delete_fluid_audio_driver(adriver);
        delete_fluid_synth(synth);
        delete_fluid_settings(settings);
        return 1;
    }

    // Read and process MIDI events
    unsigned char buffer[3];
    int status;
    while (1)
    {
        status = snd_rawmidi_read(midiin, buffer, sizeof(buffer));
        if (status > 0)
        {
            parseMidiMessage(synth, buffer);
        }
        else if (status < 0 && status != -EAGAIN)
        {
            OLED_clear();
            OLED_drawText6x8(5, 10, "MIDI read Error");
            break;
        }
    }

    // Clean up
    snd_rawmidi_close(midiin);
    delete_fluid_audio_driver(adriver);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    OLED_dispose();

    return 0;
}
