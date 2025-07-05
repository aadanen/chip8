#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <iniparser.h>

#include <chip8.h>

#define SINGLE_STEP_MODE 0
#define GAPS 1

const char* config_name = "chip8_config.ini";

// lets try this
// default: try a database lookup, then switch to the config 
// option: force config quirks
// option: try database and exit on fail
uint8_t quirks[CHIP8_NUM_QUIRKS] = {0};

typedef struct chip8_settings {
  uint32_t screen_width;
  uint32_t screen_height;
  uint32_t target_fps;
  uint32_t chip8_clock_speed;

} chip8_settings;

bool read_config(chip8_settings* settings) {
  const char* basepath = SDL_GetBasePath();
  uint32_t len = SDL_strlen(basepath) + SDL_strlen(config_name);
  char* confpath = (char*)SDL_malloc(len);
  SDL_memcpy(confpath, basepath, (SDL_strlen(basepath)-6));
  SDL_strlcat(confpath, config_name, len);

  dictionary* ini = iniparser_load(confpath);
  settings->screen_width = iniparser_getint(ini, "sdl:screen_width", -1);
  settings->screen_height = iniparser_getint(ini, "sdl:screen_height", -1);
  settings->target_fps = iniparser_getint(ini, "sdl:target_fps", -1);
  settings->chip8_clock_speed = iniparser_getint(ini, 
      "chip8:chip8_clock_speed", -1);

  // parse quirks
  enum CHIP8_QUIRK_INDEX index;
  index = SHIFT;
  quirks[index] = iniparser_getint(ini, "chip8:shift", -1);

  index = MEM_INCREMENT_X;
  quirks[index] = iniparser_getint(ini, "chip8:mem_increment_x:", -1);

  index = MEM_I_UNCHANGED;
  quirks[index] = iniparser_getint(ini, "chip8:mem_i_unchanged", -1);

  index = WRAP;
  quirks[index] = iniparser_getint(ini, "chip8:wrap", -1);

  index = JUMP;
  quirks[index] = iniparser_getint(ini, "chip8:jump", -1);

  index = VBLANK;
  quirks[index] = iniparser_getint(ini, "chip8:vblank", -1);

  index = VF_RESET;
  quirks[index] = iniparser_getint(ini, "chip8:vf_reset", -1);
  
  iniparser_freedict(ini);
  SDL_free(confpath);
  return true;
}

void audio_linear_fade_in(float* samples, uint32_t nsamples, 
    float fade_length) {
  if (fade_length > nsamples) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Fade in length > num samples\n");
  }
  for (uint32_t i = 0; i < fade_length; i++) {
    samples[i] *= (i/fade_length);
  }
}
void audio_linear_fade_out(float* samples, uint32_t nsamples, 
    float fade_length) {
  if (fade_length > nsamples) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Fade out length > num samples\n");
  }
  for (uint32_t i = nsamples - fade_length - 1; i < nsamples; i++) {
    samples[i] *= (nsamples - i - 1)/fade_length;
  }
}

uint16_t get_keypad(const bool* sdl_keyboard) {
    uint16_t keyboard = 0;
    if (sdl_keyboard[SDL_SCANCODE_1]) {
      keyboard |= 0x2;
    }
    if (sdl_keyboard[SDL_SCANCODE_2]) {
      keyboard |= 0x4;
    }
    if (sdl_keyboard[SDL_SCANCODE_3]) {
      keyboard |= 0x8;
    }
    if (sdl_keyboard[SDL_SCANCODE_4]) {
      keyboard |= 0x1000;
    }


    if (sdl_keyboard[SDL_SCANCODE_Q]) {
      keyboard |= 0x10;
    }
    if (sdl_keyboard[SDL_SCANCODE_W]) {
      keyboard |= 0x20;
    }
    if (sdl_keyboard[SDL_SCANCODE_E]) {
      keyboard |= 0x40;
    }
    if (sdl_keyboard[SDL_SCANCODE_R]) {
      keyboard |= 0x2000;
    }

    if (sdl_keyboard[SDL_SCANCODE_A]) {
      keyboard |= 0x80;
    }
    if (sdl_keyboard[SDL_SCANCODE_S]) {
      keyboard |= 0x100;
    }
    if (sdl_keyboard[SDL_SCANCODE_D]) {
      keyboard |= 0x200;
    }
    if (sdl_keyboard[SDL_SCANCODE_F]) {
      keyboard |= 0x4000;
    }

    if (sdl_keyboard[SDL_SCANCODE_Z]) {
      keyboard |= 0x400;
    }
    if (sdl_keyboard[SDL_SCANCODE_X]) {
      keyboard |= 0x1;
    }
    if (sdl_keyboard[SDL_SCANCODE_C]) {
      keyboard |= 0x800;
    }
    if (sdl_keyboard[SDL_SCANCODE_V]) {
      keyboard |= 0x8000;
    }
    return keyboard;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Bad arguments\n");
    return 1;
  }
  chip8_settings* settings = (chip8_settings*)SDL_malloc(sizeof(*settings));
  if (!read_config(settings)) {
    return 1;
  }

  uint32_t pixelHeight = settings->screen_height/CHIP8_SCREEN_HEIGHT;
  uint32_t pixelWidth = settings->screen_width/CHIP8_SCREEN_WIDTH;

  // Initialization

  // for the chip8
  uint16_t keyboard = 0;
  uint32_t target_ticks_per_frame = 1000/settings->target_fps;
  #if !(SINGLE_STEP_MODE)
  const uint32_t cycles_per_frame = 
    settings->chip8_clock_speed/settings->target_fps;
  #endif
  CHIP8_initialize(quirks);
  CHIP8_load(argv[1]);

  // for sdl
  SDL_SetAppMetadata("chip8", "1.0", "");
  SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  int *numkeys = 0;

  SDL_Window *window;
  window = SDL_CreateWindow(
      "chip8",
      settings->screen_width,
      settings->screen_height,
      SDL_WINDOW_INPUT_FOCUS 
      );
  if (window == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", 
         SDL_GetError());
    return 1;
  }


  SDL_Renderer *renderer;
  renderer = SDL_CreateRenderer(window, NULL);
  if (renderer == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create renderer: %s\n", 
         SDL_GetError());
    return 1;
  }
  SDL_FRect bkgd;
  bkgd.x = 0;
  bkgd.y = 0;
  bkgd.w = settings->screen_width;
  bkgd.h = settings->screen_height;
  SDL_FRect chip8_pixel;
  chip8_pixel.x = 0;
  chip8_pixel.y = 0;
  chip8_pixel.w = pixelWidth - 2*GAPS;
  chip8_pixel.h = pixelHeight - 2*GAPS;


  SDL_AudioSpec spec;
  
  spec.channels = 1;
  spec.freq = 16000;
  spec.format = SDL_AUDIO_F32;
  SDL_AudioStream *stream = NULL;
  static float samples[40000] = {0};

  SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(
      SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);

  SDL_AudioSpec device_spec;
  SDL_GetAudioDeviceFormat(audio_device, &device_spec, NULL);
  stream = SDL_CreateAudioStream(&spec, &device_spec);
  SDL_BindAudioStream(audio_device, stream);
  SDL_ResumeAudioStreamDevice(stream);


  // Main game loop
  bool done = false;
  bool playing_audio = false;
  uint64_t frame_start_ticks;
  uint64_t elapsed_ticks;

  while (!done) {
    frame_start_ticks = SDL_GetTicks();
    keyboard = 0;

    // handle system events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT || 
          (event.type == SDL_EVENT_KEY_DOWN && 
           event.key.scancode == SDL_SCANCODE_ESCAPE)) {
        done = true;
      }
    }
    const bool *sdl_keyboard = SDL_GetKeyboardState(numkeys);
    keyboard = get_keypad(sdl_keyboard);

    #if !(SINGLE_STEP_MODE)
    for (uint32_t i = 0; i < cycles_per_frame; i++) {
      CHIP8_cycle(keyboard, i);
    }
    #else
    if (1) {
      CHIP8_dump();
      CHIP8_cycle(keyboard);
    }
    #endif

    // audio
    if (chip8_sound > 0 && !playing_audio) {
      uint32_t needed_samples = (chip8_sound/(float)settings->target_fps)*spec.freq;
      uint32_t cur_sample = 0;

      for (uint64_t i = 0; i < needed_samples; i++) {
          const int freq = 260;
          const float phase = cur_sample * freq / 16000.0f;
          samples[i] = SDL_sinf(phase * 2 * SDL_PI_F);
          cur_sample++;
          cur_sample %= 16000;
      }
      if (needed_samples >= 800) {

        audio_linear_fade_in(samples, needed_samples, 100.0f);
        audio_linear_fade_out(samples, needed_samples, 700.0f);
      } else {
        audio_linear_fade_in(samples, needed_samples, needed_samples/8.0f);
        audio_linear_fade_out(samples, needed_samples, needed_samples * (7.0f/8.0f));
      }

      SDL_PutAudioStreamData(stream, samples, (needed_samples)*sizeof(float));
      playing_audio = true;
    } 
    if (chip8_sound == 0) {
      playing_audio = false;
    }


    
    // graphics
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &bkgd);


    SDL_SetRenderDrawColor(renderer, 0, 255, 51, SDL_ALPHA_OPAQUE);
    for (uint8_t i = 0; i < CHIP8_SCREEN_HEIGHT; i++) {
      for (uint8_t j = 0; j < CHIP8_SCREEN_WIDTH; j++) {
        chip8_pixel.y = i*(pixelHeight)+GAPS;
        chip8_pixel.x = j*(pixelWidth)+GAPS;

        //SDL_SetRenderDrawColor(renderer, R, G, B, SDL_ALPHA_OPAQUE);
        if (chip8_screen[i][j]) {
          SDL_RenderFillRect(renderer, &chip8_pixel);
        }
      }
    }


    SDL_RenderPresent(renderer);


    // sound
    if (chip8_sound > 0) {
      chip8_sound--;
    }
    if (chip8_delay > 0) {
      chip8_delay--;
    }
    elapsed_ticks = SDL_GetTicks() - frame_start_ticks;
    if (elapsed_ticks < target_ticks_per_frame) {
      SDL_Delay(target_ticks_per_frame - elapsed_ticks);
    }
  }
  SDL_Quit();
  return 0;
}
