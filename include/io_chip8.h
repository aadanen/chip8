#ifndef IO_CHIP8_H
#define IO_CHIP8_H

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <cJSON.h>

const char* config_name = "chip8_config.ini";
const char* sha1_name = "database/sha1-hashes.json";

uint8_t quirks[CHIP8_NUM_QUIRKS] = {0};

// https://stackoverflow.com/questions/14002954/c-how-to-read-an-entire-file-into-a-buffer/44894946#44894946
/* Size of each input chunk to be
   read and allocate for. */
#ifndef  READALL_CHUNK
#define  READALL_CHUNK  262144
#endif

#define  READALL_OK          0  /* Success */
#define  READALL_INVALID    -1  /* Invalid parameters */
#define  READALL_ERROR      -2  /* Stream error */
#define  READALL_TOOMUCH    -3  /* Too much input */
#define  READALL_NOMEM      -4  /* Out of memory */

/* This function returns one of the READALL_ constants above.
   If the return value is zero == READALL_OK, then:
     (*dataptr) points to a dynamically allocated buffer, with
     (*sizeptr) chars read from the file.
     The buffer is allocated for one extra char, which is NUL,
     and automatically appended after the data.
   Initial values of (*dataptr) and (*sizeptr) are ignored.
*/
int readall(FILE *in, char **dataptr, size_t *sizeptr) {
    char  *data = NULL, *temp;
    size_t size = 0;
    size_t used = 0;
    size_t n;

    /* None of the parameters can be NULL. */
    if (in == NULL || dataptr == NULL || sizeptr == NULL)
        return READALL_INVALID;

    /* A read error already occurred? */
    if (ferror(in))
        return READALL_ERROR;

    while (1) {

        if (used + READALL_CHUNK + 1 > size) {
            size = used + READALL_CHUNK + 1;

            /* Overflow check. Some ANSI C compilers
               may optimize this away, though. */
            if (size <= used) {
                free(data);
                return READALL_TOOMUCH;
            }

            temp = realloc(data, size);
            if (temp == NULL) {
                free(data);
                return READALL_NOMEM;
            }
            data = temp;
        }

        n = fread(data + used, 1, READALL_CHUNK, in);
        if (n == 0)
            break;

        used += n;
    }

    if (ferror(in)) {
        free(data);
        return READALL_ERROR;
    }

    temp = realloc(data, used + 1);
    if (temp == NULL) {
        free(data);
        return READALL_NOMEM;
    }
    data = temp;
    data[used] = '\0';

    *dataptr = data;
    *sizeptr = used;

    return READALL_OK;
}


typedef struct chip8_settings {
  uint32_t screen_width;
  uint32_t screen_height;
  uint32_t target_fps;
  uint32_t chip8_clock_speed;

} chip8_settings;

char* create_abs_path(const char* relative_path) {
  const char* basepath = SDL_GetBasePath();
  uint32_t len = SDL_strlen(basepath) + SDL_strlen(relative_path);

  char* p = (char*)SDL_malloc(len);
  SDL_memset(p, 0, len);
  SDL_memcpy(p, basepath, (SDL_strlen(basepath)-6));
  SDL_strlcat(p, relative_path, len);
  return p;
}

char* calc_sha1(char* rom_path) {
  FILE *f;
  int i, j;
  char* output = (char*)malloc(41);
  sha1_context ctx;
  unsigned char buf[1000];
  unsigned char sha1sum[20];

  if (!(f = fopen(rom_path, "rb"))) {
    return NULL;
  }
  sha1_starts(&ctx);
  while((i = fread(buf, 1, sizeof(buf), f)) > 0) {
    sha1_update(&ctx, buf, i);
  }
  sha1_finish(&ctx, sha1sum);
  
  for (j = 0; j < 20; j++) {
    sprintf(output+j*2, "%02x", sha1sum[j]);
  }
  return output;
}

bool read_config(chip8_settings* settings) {
  char* confpath = create_abs_path(config_name);
  dictionary* ini = iniparser_load(confpath);
  settings->screen_width = iniparser_getint(ini, "sdl:screen_width", -1);
  settings->screen_height = iniparser_getint(ini, "sdl:screen_height", -1);
  settings->target_fps = iniparser_getint(ini, "sdl:target_fps", -1);
  settings->chip8_clock_speed = iniparser_getint(ini, 
      "chip8:chip8_clock_speed", -1);

  // parse quirks
  quirks[CHIP8_SHIFT] = iniparser_getint(ini, "chip8:shift", -1);

  quirks[CHIP8_MEM_INCREMENT_X] = iniparser_getint(ini, "chip8:mem_increment_x:", -1);

  quirks[CHIP8_MEM_I_UNCHANGED] = iniparser_getint(ini, "chip8:mem_i_unchanged", -1);

  quirks[CHIP8_WRAP] = iniparser_getint(ini, "chip8:wrap", -1);

  quirks[CHIP8_JUMP] = iniparser_getint(ini, "chip8:jump", -1);

  quirks[CHIP8_VBLANK] = iniparser_getint(ini, "chip8:vblank", -1);

  quirks[CHIP8_VF_RESET] = iniparser_getint(ini, "chip8:vf_reset", -1);
  
  iniparser_freedict(ini);
  SDL_free(confpath);
  return true;
}


bool query_database(char* rom_path) {
  // Calculate the SHA1 hash of a ROM file
  char* sha1 = calc_sha1(rom_path);

  // Look up the SHA1 hash in sha1-hashes.json, which gives you an index

  // the longest filename in database is 16 characters
  // it also just happens to be the first file i need so ill reuse this buffer
  char* buffer;
  char* dbpath = create_abs_path(sha1_name);
  size_t fsize;
  int err;

  FILE* fptr = fopen(dbpath, "r");
  if (!fptr) {
    printf("failed to open %s\n", dbpath);
    return false;
  }
  err = readall(fptr, &buffer, &fsize);
  if (err != 0) {
    printf("failed to read %s\n", dbpath);
    return false;
  }

  const cJSON* hashes = cJSON_Parse(buffer);
  const cJSON* program;
  int32_t program_index = -1;
  program = cJSON_GetObjectItemCaseSensitive(hashes, sha1);
  if (program == NULL) {
    printf("failed to locate hash %s\n", sha1);
    return false;
  }
  printf("%d\n", program->valueint);

  // Use the index to find the program metadata in the programs.json file
  // Find the ROM metadata in the roms list of the program metadata
  // Configure your interpreter to run the ROM using platforms.json and quirks.json
  free(dbpath);
  free(buffer);
  return true;
}

#endif /*IO_CHIP8_H*/
