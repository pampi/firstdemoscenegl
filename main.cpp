//For ubuntu or debian, try to run: #apt-get install libmikmod3 libglfw3
//in order to run demo

#include <cstdio>
#include <cstdlib>

#include <thread>
#include <atomic>

#include <unistd.h>
#include <mikmod.h>     //libmikmod

#include "music.inl"
#include "demo.hpp"

MODULE *module;
void play_music();

std::atomic_bool shutdown;

int main()
{
    shutdown = false;

    MikMod_RegisterAllDrivers();
    MikMod_RegisterLoader(&load_xm);

    md_mode |= DMODE_FLOAT;

    if (MikMod_Init("")) {
        printf("Could not initialize sound, reason: %s\n", MikMod_strerror(MikMod_errno));
        return EXIT_FAILURE;
    }

    Demo demo;

    std::thread music_thread(play_music);

    demo.run();


    shutdown = true;
    music_thread.join();

    MikMod_Exit();

    return 0;
}

void play_music()
{
    module = Player_LoadMem((const char*)my_tracker_music, sizeof(my_tracker_music), 32, 0);
    if (module)
    {
        Player_Start(module);

        while(Player_Active() && !shutdown)
        {
            usleep(10000);
            MikMod_Update();

            if (!Player_Active())
            {
                //replay
                Player_SetPosition(0);
                Player_Start(module);
            }
        }

        Player_Stop();
        Player_Free(module);
    }
}
