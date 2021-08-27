#ifdef UNUSED_FEAT
#include "sound.h"
#include "moon/mod-engine/hooks/hook.h"

#include "moon/utils/moon-env.h"
#include "pc/configfile.h"
#include "moon/libs/audeo/audeo/audeo.hpp"

#include <SDL2/SDL.h>

#include <iostream>
#include <string>
using namespace std;

namespace MoonInternal {
    void setupSoundModule(string status){
        if(status == "PreStartup"){
            audeo::SoundSource source1;
            audeo::SoundSource source2;
            audeo::SoundSource source3;

            Moon::registerHookListener({.hookName = WINDOW_API_INIT, .callback = [&](HookCall call){
                audeo::init();
                source1 = audeo::load_source("/home/alex/Videos/15.wav", audeo::AudioType::Effect);
                source2 = audeo::load_source("/home/alex/Videos/15.wav", audeo::AudioType::Effect);
                source3 = audeo::load_source("/home/alex/Videos/15.wav", audeo::AudioType::Effect);
            }});

            Moon::registerHookListener({.hookName = WINDOW_API_HANDLE_EVENTS, .callback = [&](HookCall call){
                SDL_Event* ev = (SDL_Event*) call.baseArgs["event"];
                switch (ev->type){
                    case SDL_KEYDOWN:
                        if(ev->key.keysym.sym == SDLK_i){
                            cout << "Playing owo" << endl;
                            audeo::Sound effect = audeo::play_sound(source1);
                        }
                        if(ev->key.keysym.sym == SDLK_o){
                            cout << "Playing owo" << endl;
                            audeo::Sound effect = audeo::play_sound(source2);
                        }
                        if(ev->key.keysym.sym == SDLK_p){
                            cout << "Playing owo" << endl;
                            audeo::Sound effect = audeo::play_sound(source3);
                        }
                    break;
                }
            }});
        }
    }
}
#endif