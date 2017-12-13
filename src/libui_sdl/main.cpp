/*
    Copyright 2016-2017 StapleButter

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <SDL/SDL.h>

#include "../types.h"
#include "../version.h"
#include "../Config.h"

#include "../NDS.h"
#include "../GPU.h"
#include "../SPU.h"
#include "../Wifi.h"
#include "../Platform.h"

unsigned char exit_game = 0;

int EmuRunning;
volatile int EmuStatus;

bool RunningSomething;
char ROMPath[1024];
char homepath[128];
char bios7_path[128];
char bios9_path[128];
char firmware_path[128];

bool ScreenDrawInited = false;
u32 KeyInputMask;

SDL_Surface* game_screen;

void AudioCallback(void* data, Uint8* stream, int len)
{
    // resampling:
    // buffer length is 1024 samples
    // which is 710 samples at the original sample rate

    //SPU::ReadOutput((s16*)stream, len>>2);
    s16 buf_in[710*2];
    s16* buf_out = (s16*)stream;

    int num_in = SPU::ReadOutput(buf_in, 710);
    int num_out = 1024;

    int margin = 6;
    if (num_in < 710-margin)
    {
        int last = num_in-1;
        if (last < 0) last = 0;

        for (int i = num_in; i < 710-margin; i++)
            ((u32*)buf_in)[i] = ((u32*)buf_in)[last];

        num_in = 710-margin;
    }

    float res_incr = num_in / (float)num_out;
    float res_timer = 0;
    int res_pos = 0;

    for (int i = 0; i < 1024; i++)
    {
        // TODO: interp!!
        buf_out[i*2  ] = buf_in[res_pos*2  ];
        buf_out[i*2+1] = buf_in[res_pos*2+1];

        res_timer += res_incr;
        while (res_timer >= 1.0)
        {
            res_timer -= 1.0;
            res_pos++;
        }
    }
}

//int EmuThreadFunc(void* burp)
void Games()
{
	bool pressed;
    NDS::Init();

    SDL_AudioSpec whatIwant, whatIget;
    memset(&whatIwant, 0, sizeof(SDL_AudioSpec));
    whatIwant.freq = 47340;
    whatIwant.format = AUDIO_S16LSB;
    whatIwant.channels = 2;
    whatIwant.samples = 1024;
    whatIwant.callback = AudioCallback;

    if (SDL_OpenAudio(&whatIwant, &whatIget) < 0)
    {
        printf("Audio init failed: %s\n", SDL_GetError());
    }
    else
    {
        SDL_PauseAudio(0);
    }

    KeyInputMask = 0xFFF;
    
	NDS::LoadROM(ROMPath, 1);
    EmuRunning = 1;
    RunningSomething = true;

    while (EmuRunning != 0)
    {
        if (EmuRunning == 1)
        {
            EmuStatus = 1;
            // poll input
            u32 keymask = 0xFFF;
			const Uint8 *state = SDL_GetKeyState(NULL);
			
			if (state[SDLK_UP]) keymask -= 64;
			else if (state[SDLK_DOWN])  keymask -= 128;
			
			if (state[SDLK_LEFT]) keymask -= 32;
			else if (state[SDLK_RIGHT])  keymask -= 16;
		
			if (state[SDLK_LCTRL]) keymask -= 1;
			if (state[SDLK_LALT])  keymask -= 2;
			if (state[SDLK_LSHIFT])  keymask -= 1024;
			if (state[SDLK_SPACE])  keymask -= 2048;
			
			if (state[SDLK_TAB])  keymask -= 512;
			if (state[SDLK_BACKSPACE])  keymask -= 256;
			
			if (state[SDLK_RETURN])  keymask -= 8;
			if (state[SDLK_ESCAPE])  keymask -= 4;
			
			if (keymask==4083) 
			{
				EmuRunning = 0;
			}
			
			SDL_Event event;
			SDL_PollEvent(&event);
			
            NDS::SetKeyMask(keymask);

            // emulate
            u32 nlines = NDS::RunFrame();
            if (EmuRunning == 0) break;

			SDL_LockSurface(game_screen);
			memcpy(game_screen->pixels, GPU::Framebuffer, (256 * 384)*4);
			SDL_UnlockSurface(game_screen);
			SDL_Flip(game_screen);
			
        }
        else
        {
            EmuStatus = 2;
        }
    }
    
    EmuStatus = 0;
	SDL_PauseAudio(1);
    NDS::DeInit();
    
    exit_game = 1;
    //return 44203;
}


void Run()
{
    EmuRunning = 1;
    RunningSomething = true;
}

void Stop(bool internal)
{
    EmuRunning = 2;
    if (!internal) // if shutting down from the UI thread, wait till the emu thread has stopped
        while (EmuStatus != 2);
    RunningSomething = false;
}

void TryLoadROM(char* file, int prevstatus)
{
    char oldpath[1024];
    strncpy(oldpath, ROMPath, 1024);

    strncpy(ROMPath, file, 1023);
    ROMPath[1023] = '\0';

    if (NDS::LoadROM(ROMPath, Config::DirectBoot))
        Run();
    else
    {
        strncpy(ROMPath, oldpath, 1024);
        EmuRunning = prevstatus;
    }
}

void ApplyNewSettings()
{
    if (!RunningSomething) return;

    int prevstatus = EmuRunning;
    EmuRunning = 2;
    while (EmuStatus != 2);

    GPU3D::SoftRenderer::SetupRenderThread();

    if (Wifi::MPInited)
    {
        Platform::MP_DeInit();
        Platform::MP_Init();
    }

    EmuRunning = prevstatus;
}


bool _fileexists(char* name)
{
	char checkname[128];
	snprintf(checkname, sizeof(checkname), "%s/%s", homepath, name);
	
    FILE* f = fopen(checkname, "rb");
    if (!f) return false;
    fclose(f);
    return true;
}


int main(int argc, char** argv)
{
	char configpath[128];
    srand(time(NULL));

	snprintf(configpath, sizeof(configpath), "%s/.config", getenv("HOME"));
	if(access( configpath, F_OK ) == -1) 
	{
		mkdir(configpath, 0755);	
	}
	snprintf(homepath, sizeof(homepath), "%s/.config/melonds", getenv("HOME"));
	if(access( homepath, F_OK ) == -1) 
	{
		mkdir(homepath, 0755);	
	}
	snprintf(bios7_path, sizeof(bios7_path), "%s/%s", homepath, "bios7.bin");
	snprintf(bios9_path, sizeof(bios9_path), "%s/%s", homepath, "bios9.bin");
	snprintf(firmware_path, sizeof(firmware_path), "%s/%s", homepath, "firmware.bin");
	
    printf("melonDS " MELONDS_VERSION "\n");
    printf(MELONDS_URL "\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("SDL shat itself :(\n");
        return 1;
    }

    if (!_fileexists("bios7.bin") || !_fileexists("bios9.bin") || !_fileexists("firmware.bin"))
    {
        printf("No ROMS found\n");
        SDL_Quit();
        return 0;
    }

	game_screen = SDL_SetVideoMode(256, 384, 32, SDL_SWSURFACE);
	SDL_ShowCursor(0);
    
    EmuRunning = 2;
    RunningSomething = false;
    
	snprintf(ROMPath, sizeof(ROMPath), "%s", argv[1]);

    while (exit_game != 1)
    {
		Games();
	}

    EmuRunning = 0;
	if (game_screen != NULL) SDL_FreeSurface(game_screen);
    //SDL_Quit();
    
    return 0;
}
