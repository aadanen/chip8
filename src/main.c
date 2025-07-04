#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <chip8.h>

#define SINGLE_STEP_MODE 0
#define GAPS 1
const char* CONFIG_NAME = "chip8.conf";


void read_config() {
  const char* basepath = SDL_GetBasePath();
  uint32_t baselen = SDL_strlen(basepath);
  char* confpath = (char*)SDL_malloc(baselen*sizeof(char));
  SDL_memcpy(confpath, basepath, (baselen-6)*sizeof(char));
  printf("%s\n", confpath);
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

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Bad arguments\n");
    return 1;
  }
  //read_config();

  // Initialization
  uint32_t screenWidth = 1000;
  uint32_t screenHeight = 500;
  uint32_t pixelHeight = screenHeight/CHIP8_SCREEN_HEIGHT;
  uint32_t pixelWidth = screenWidth/CHIP8_SCREEN_WIDTH;

  // for the chip8
  uint16_t keyboard = 0;
  uint32_t target_fps = 60;
  uint32_t target_ticks_per_frame = 1000/target_fps;
  #if !(SINGLE_STEP_MODE)
  const uint32_t cycles_per_frame = CHIP8_CLOCK_SPEED/target_fps;
  #endif
  CHIP8_initialize();
  CHIP8_load(argv[1]);

  // for sdl
  SDL_SetAppMetadata("chip8", "1.0", "");
  SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  int *numkeys = 0;

  SDL_Window *window;
  window = SDL_CreateWindow(
      "chip8",
      screenWidth,
      screenHeight,
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
  bkgd.w = screenWidth;
  bkgd.h = screenHeight;
  SDL_FRect chip8_pixel;
  chip8_pixel.x = 0;
  chip8_pixel.y = 0;
  chip8_pixel.w = pixelWidth - 2*GAPS;
  chip8_pixel.h = pixelHeight - 2*GAPS;


  SDL_AudioSpec spec;
  
  // beep is: 
  // mono
  // constant sample rate
  // float32 data
  spec.channels = 1;
  spec.freq = 16000;
  spec.format = SDL_AUDIO_F32;
  //const float silence_value = SDL_GetSilenceValueForFormat(SDL_AUDIO_F32);
  SDL_AudioStream *stream = NULL;
  static float samples[40000] = {0};

  /*
  stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, 
      &spec, NULL, NULL);
      */
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

    #if !(SINGLE_STEP_MODE)
    for (uint32_t i = 0; i < cycles_per_frame; i++) {
      CHIP8_cycle(keyboard);
    }
    #else
    if (1) {
      CHIP8_dump();
      CHIP8_cycle(keyboard);
    }
    #endif

    // audio
    if (chip8_sound > 0 && !playing_audio) {
      uint32_t needed_samples = (chip8_sound/(float)target_fps)*spec.freq;
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
