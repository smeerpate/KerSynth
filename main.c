#include "OLED.h"
#include "userInput.h"
#include <alsa/asoundlib.h>
#include <fluidsynth.h>

int openFirstAvailableMidi(snd_rawmidi_t **input, snd_rawmidi_t **output) {
    int card = -1;
    int device = -1;
    int subdevice = 0;
    int err;
    char name[32];

    // Iterate over all sound cards
    if (snd_card_next(&card) < 0 || card < 0) {
        fprintf(stderr, "No sound cards found.\n");
        return -1;
    }

    while (card >= 0) {
        device = -1;

        // Iterate over all devices on this card
        while (snd_ctl_rawmidi_next_device(card, &device) >= 0 && device >= 0) {
            snprintf(name, sizeof(name), "hw:%d,%d,%d", card, device, subdevice);

            // Try to open the device
            err = snd_rawmidi_open(input, output, name, 0);
            if (err == 0) {
                printf("Opened MIDI device: %s\n", name);
                return 0;
            } else {
                fprintf(stderr, "Failed to open MIDI device %s: %s\n", name, snd_strerror(err));
            }
        }

        // Move to the next card
        if (snd_card_next(&card) < 0) {
            break;
        }
    }

    fprintf(stderr, "No available MIDI devices found.\n");
    return -1;
}

void parseMidiMessage(fluid_synth_t* synth, unsigned char* buffer)
{
    unsigned char status = buffer[0];
    unsigned char data1 = buffer[1];
    unsigned char data2 = buffer[2];

    unsigned char midiChannel = (status & 0x0F) + 1;

    switch (status & 0xF0)
    {
        case 0x80: // Note Off
            fluid_synth_noteoff(synth, status & 0x0F, data1);
            printf("Note Off: Channel %d, Note %d\n", midiChannel, data1);
            OLED_writeLine(5, 4, ":-|");
            break;

        case 0x90: // Note On
            if (data2 == 0)
            {
                fluid_synth_noteoff(synth, status & 0x0F, data1); // Note On with velocity 0 is Note Off
                printf("Note Off: Channel %d, Note %d\n", midiChannel, data1);
                OLED_writeLine(5, 4, ":-|");
            }
            else
            {
                fluid_synth_noteon(synth, status & 0x0F, data1, data2);
                printf("Note On: Channel %d, Note %d, Velocity %d\n", midiChannel, data1, data2);
                OLED_writeLine(5, 4, ":-o");
            }
            break;

        case 0xC0: // Program Change
            fluid_synth_program_change(synth, status & 0x0F, data1);
            printf("Program Change: Channel %d, Program %d\n", midiChannel, data1);
            break;

        case 0xE0: // Pitch Bend
            fluid_synth_pitch_bend(synth, status & 0x0F, (data2 << 7) | data1);
            printf("Pitch Bend: Channel %d, Value %d\n", midiChannel, (data2 << 7) | data1);
            break;

        default:
            break;
    }
}

void processMidiBytes(fluid_synth_t* synth, unsigned char *buffer, int nBytesToProcess)
{
    int eventStartIdx[128] = {0};
    int eventIdxPointer = 0;
    int verbose = 0;

    if (verbose > 0)
        printf("====\n");

    for (int i = 0; i < nBytesToProcess; i++)
    {
        if (verbose > 1)
            printf("    read 0x%02x\n",buffer[i]);
        if (buffer[i] & 0x80)
        {
            // we hebben een status byte vast...
            eventStartIdx[eventIdxPointer] = i;
            if (verbose > 0)
                printf("    Added 0x%02x MIDI event at buffer idx %d copied to event idx %d\n", buffer[i], i, eventIdxPointer);
            eventIdxPointer++;
        }
    }

    if (verbose > 1)
    {
        printf("Events at indexes:\n");
        for (int i = 0; i < eventIdxPointer; i++)
            printf("%d ", eventStartIdx[i]);
        printf("\n");
    }

    for (int i = 0; i < eventIdxPointer; i++)
    {
        if (verbose > 0)
                printf("    Parsing 0x%02x MIDI event at event idx %d\n", buffer[eventStartIdx[i]], i);
        parseMidiMessage(synth, &buffer[eventStartIdx[i]]);
    }
}

int main()
{
	snd_rawmidi_t *midiin = NULL;
    snd_rawmidi_t *midiout = NULL;
	
    OLED_init();
    OLED_clear();
    OLED_drawText6x8(5, 10, "Hallo Freddy!");
    OLED_drawText6x8(5, 10+8, "We gaan dat hier");
	UI_init();
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

    // Get the SoundFont object
    fluid_sfont_t* sfont = fluid_synth_get_sfont_by_id(synth, sfont_id);
    if (!sfont) {
        fprintf(stderr, "Failed to get SoundFont\n");
        return 1;
    }

    // Iterate through presets and print their names
    int sfIndex = 0;
    fluid_preset_t* preset = NULL;
    fluid_sfont_iteration_start(sfont);
    while ((preset = fluid_sfont_iteration_next(sfont)) != NULL)
    {
        int bank_num = fluid_preset_get_banknum(preset);
        const char* preset_name = fluid_preset_get_name(preset);
        printf("Preset %d: %s (bank %d)\n", sfIndex, preset_name, bank_num);
        sfIndex++;
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

    // explicitly assign presets to MIDI channels
    //              channels      1   ,2   ,3   ,4   ,5   ,6   ,7   ,8   ,9   ,10  ,11  ,12  ,13  ,14  ,15  ,16
    int midiChannelPresets[16] = {1   ,17  ,53  ,4   ,5   ,6   ,7   ,8   ,9   ,135-128 ,11  ,12  ,13  ,14  ,15  ,16};
    int midiChannelBanks[16] =   {0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,128 ,0   ,0   ,0   ,0   ,0   ,0};
    for (int chan = 0; chan < 16; chan++)
    {
        fluid_synth_program_select(synth, chan, sfont_id, midiChannelBanks[chan], midiChannelPresets[chan]);
    }

    // Retrieve and print the selected programs for each MIDI channel
    for (int chan = 0; chan < 16; chan++)
    {
        int sfont_id, bank_num, preset_num;
        fluid_synth_get_program(synth, chan, &sfont_id, &bank_num, &preset_num);
        printf("Channel %d: SoundFont ID %d, Bank %d, Preset %d\n", chan+1, sfont_id, bank_num, preset_num);
    }


    // Open MIDI input port
    //snd_rawmidi_t *midiin = NULL;
    snd_rawmidi_open(&midiin, NULL, "hw:1,0,0", SND_RAWMIDI_NONBLOCK);
    if (!midiin)
	//if (openFirstAvailableMidi(&midiin, &midiout) != 0)
    {
        OLED_clear();
        OLED_drawText6x8(5, 10, "MIDI init Error");
        delete_fluid_audio_driver(adriver);
        delete_fluid_synth(synth);
        delete_fluid_settings(settings);
        return 1;
    }
	


    // Application loop: Read and process MIDI events
    unsigned char buffer[1024];
    int status;
    while (1)
    {
        // read midi bytes from ring buffer and parse
        status = snd_rawmidi_read(midiin, buffer, sizeof(buffer));
        if (status > 0) // status geeft nu het aantal gelezen bytes weer
        {
            processMidiBytes(synth, buffer, status);
            //parseMidiMessage(synth, buffer);
        }
        else if (status < 0 && status != -EAGAIN)
        {
            OLED_clear();
            OLED_drawText6x8(5, 10, "MIDI read Error");
            break;
        }

		UI_checkRotary();
		
        usleep(500);
    }

    // Clean up
	if (midiin) snd_rawmidi_close(midiin);
    if (midiout) snd_rawmidi_close(midiout);
    delete_fluid_audio_driver(adriver);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    OLED_dispose();

    return 0;
}
