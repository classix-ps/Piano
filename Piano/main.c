/**
  * CSFML SETUP:
  * 1. Go to project properties and set active configuration to 'All Configurations'
  * 2. Under C/C++ >> General, set Additional Include Directories to CSFML/include
  * 3. Under Linker >> General, set Additional Include Libraries to CSFML/lib/msvc
  * 4. Under Linker >> Input, set Additional dependencies to csfml-graphics.lib;csfml-window.lib;csfml-system.lib
  * 5. Add the csfml-graphics-2 dll from CSFML/bin to the project folder, where main is located
 */

#include <SFML/Graphics.h>
#include <SFML/Audio.h>

#include <stdbool.h>

#include "../dirent.h"

#define WHITEKEYCOUNT 21
#define BLACKKEYCOUNT 15
#define OCTAVE 12

void drawPiano(sfRenderWindow* window, sfRectangleShape* keys[]) {
  for (size_t i = 0; i < WHITEKEYCOUNT + BLACKKEYCOUNT; i++) {
    if (!((i % 12 < 4 && i % 2 == 1) || (i % 12 > 5 && i % 2 == 0))) {
      sfRenderWindow_drawRectangleShape(window, keys[i], NULL);
    }
  }
  for (size_t i = 0; i < WHITEKEYCOUNT + BLACKKEYCOUNT; i++) {
    if ((i % 12 < 4 && i % 2 == 1) || (i % 12 > 5 && i % 2 == 0)) {
      sfRenderWindow_drawRectangleShape(window, keys[i], NULL);
    }
  }
}

struct Map {
  sfKeyCode key;
  size_t value;
};

int getMapValue(struct Map map[], sfKeyCode key) {
  for (size_t i = 0; i < OCTAVE; i++) {
    if (map[i].key == key) {
      return map[i].value;
    }
  }

  return -1;
}

int main() {
  /* WINDOW */
  sfVector2u windowSize = { 1920, 540 };

  sfVideoMode video; video.width = windowSize.x; video.height = windowSize.y; video.bitsPerPixel = 4;
  sfRenderWindow* window = sfRenderWindow_create(video, "Piano", sfDefaultStyle, NULL);

  /* KEYS */
  bool pressedKeys[WHITEKEYCOUNT + BLACKKEYCOUNT];
  memset(pressedKeys, false, (WHITEKEYCOUNT + BLACKKEYCOUNT) * sizeof(bool));

  sfRectangleShape* keys[WHITEKEYCOUNT + BLACKKEYCOUNT];

  float keyBorder = 3.f;

  sfVector2f whiteKeySize = { ((float)windowSize.x - 2 * keyBorder) / WHITEKEYCOUNT, windowSize.y - 2 * keyBorder };
  sfVector2f blackKeySize = { 0.5f * whiteKeySize.x, 0.65f * windowSize.y };

  sfRectangleShape* whiteKey = sfRectangleShape_create();
  sfRectangleShape_setFillColor(whiteKey, sfWhite);
  sfRectangleShape_setOutlineColor(whiteKey, sfBlack);
  sfRectangleShape_setOutlineThickness(whiteKey, 3.f);
  sfRectangleShape_setSize(whiteKey, whiteKeySize);

  sfRectangleShape* blackKey = sfRectangleShape_create();
  sfRectangleShape_setFillColor(blackKey, sfBlack);
  sfRectangleShape_setOutlineColor(blackKey, sfBlack);
  sfRectangleShape_setOutlineThickness(blackKey, 3.f);
  sfRectangleShape_setSize(blackKey, blackKeySize);

  for (size_t i = 0, w = 0; i < WHITEKEYCOUNT + BLACKKEYCOUNT; i++) {
    if ((i % 12 < 4 && i % 2 == 1) || (i % 12 > 5 && i % 2 == 0)) {
      keys[i] = sfRectangleShape_copy(blackKey);
      sfVector2f currentBlackKeyPos = { w * whiteKeySize.x + keyBorder / 2 - blackKeySize.x / 2, keyBorder };
      sfRectangleShape_setPosition(keys[i], currentBlackKeyPos);
    }
    else {
      keys[i] = sfRectangleShape_copy(whiteKey);
      sfVector2f currentWhiteKeyPos = { w * whiteKeySize.x + keyBorder, keyBorder };
      sfRectangleShape_setPosition(keys[i], currentWhiteKeyPos);

      w++;
    }
  }

  /* SOUNDS */
  // https://theremin.music.uiowa.edu/MISpiano.html
  sfSoundBuffer* soundBuffers[WHITEKEYCOUNT + BLACKKEYCOUNT];
  sfSound* sounds[WHITEKEYCOUNT + BLACKKEYCOUNT];

  const char* dirname = "../resources/";
  DIR* dir = opendir(dirname);

  if (!dir) {
    fprintf(stderr, "Cannot open %s (%s)\n", dirname, strerror(errno));
    return EXIT_FAILURE;
  }

  dirent* ent;
  while ((ent = readdir(dir)) != NULL) {
    if (ent->d_type == DT_REG) {
      size_t index;
      sscanf(ent->d_name, "%u _", &index);

      char* filename = malloc((strlen(dirname) + strlen(ent->d_name) + 1) * sizeof(char));
      strcpy(filename, dirname);
      strcat(filename, ent->d_name);
      //printf("%s\n", filename);
      soundBuffers[index-1] = sfSoundBuffer_createFromFile(filename);
      sounds[index-1] = sfSound_create();
      sfSound_setBuffer(sounds[index-1], soundBuffers[index-1]);
      sfSound_setVolume(sounds[index-1], 25.f);
    }
  }
  closedir(dir);

  struct Map keyMap[OCTAVE] = {
    [0] = {sfKeyY, 12},
    [1] = {sfKeyS, 13},
    [2] = {sfKeyX, 14},
    [3] = {sfKeyD, 15},
    [4] = {sfKeyC, 16},
    [5] = {sfKeyV, 17},
    [6] = {sfKeyG, 18},
    [7] = {sfKeyB, 19},
    [8] = {sfKeyH, 20},
    [9] = {sfKeyN, 21},
    [10] = {sfKeyJ, 22},
    [11] = {sfKeyM, 23},
  };
  
  drawPiano(window, keys);
  sfRenderWindow_display(window);
  
  /* MAIN LOOP */
  while (sfRenderWindow_isOpen(window)) {
    sfEvent e;

    while (sfRenderWindow_waitEvent(window, &e)) {
      if (e.type == sfEvtClosed) {
        sfRenderWindow_close(window);
      }
      else if (e.type == sfEvtMouseButtonPressed) {
        bool keyFound = false;

        for (size_t i = 0; i < WHITEKEYCOUNT + BLACKKEYCOUNT && !keyFound; i++) {
          if (((i % 12 < 4 && i % 2 == 1) || (i % 12 > 5 && i % 2 == 0))) {
            sfFloatRect bounds = sfRectangleShape_getGlobalBounds(keys[i]);
            if (sfFloatRect_contains(&bounds, (float)e.mouseButton.x, (float)e.mouseButton.y)) {
              sfSound_play(sounds[i]);
              sfRectangleShape_setFillColor(keys[i], sfColor_fromRGB(50, 50, 50));
              pressedKeys[i] = true;
              keyFound = true;
            }
          }
        }
        for (size_t i = 0; i < WHITEKEYCOUNT + BLACKKEYCOUNT && !keyFound; i++) {
          if (!(i % 12 < 4 && i % 2 == 1) || (i % 12 > 5 && i % 2 == 0)) {
            sfFloatRect bounds = sfRectangleShape_getGlobalBounds(keys[i]);
            if (sfFloatRect_contains(&bounds, (float)e.mouseButton.x, (float)e.mouseButton.y)) {
              sfSound_play(sounds[i]);
              sfRectangleShape_setFillColor(keys[i], sfColor_fromRGB(200, 200, 200));
              pressedKeys[i] = true;
              keyFound = true;
            }
          }
        }
      }
      else if (e.type == sfEvtMouseButtonReleased) {
        for (size_t i = 0; i < WHITEKEYCOUNT + BLACKKEYCOUNT; i++) {
          if ((i % 12 < 4 && i % 2 == 1) || (i % 12 > 5 && i % 2 == 0)) {
            sfRectangleShape_setFillColor(keys[i], sfBlack);
            pressedKeys[i] = false;
          }
          else {
            sfRectangleShape_setFillColor(keys[i], sfWhite);
            pressedKeys[i] = false;
          }
        }
      }
      else if (e.type == sfEvtKeyPressed) {
        if (e.key.code == sfKeyEscape) {
          sfRenderWindow_close(window);
          continue;
        }
        else if (e.key.code == sfKeyAdd) {
          float newVolume = min(sfSound_getVolume(sounds[0]) + 5.f, 100.f);
          for (size_t i = 0; i < WHITEKEYCOUNT + BLACKKEYCOUNT; i++) {
            sfSound_setVolume(sounds[i], newVolume);
          }
        }
        else if (e.key.code == sfKeySubtract) {
          float newVolume = max(sfSound_getVolume(sounds[0]) - 5.f, 0.f);
          for (size_t i = 0; i < WHITEKEYCOUNT + BLACKKEYCOUNT; i++) {
            sfSound_setVolume(sounds[i], newVolume);
          }
        }

        int val = getMapValue(keyMap, e.key.code);
        if (val == -1) {
          continue;
        }

        int delta = 0;
        if (sfKeyboard_isKeyPressed(sfKeyLControl)) {
          delta -= 12;
        }
        if (sfKeyboard_isKeyPressed(sfKeyLShift)) {
          delta += 12;
        }

        int pressedKey = val + delta;
        if (pressedKeys[pressedKey] == false) {
          sfSound_play(sounds[pressedKey]);
          sfColor setColor = (val % 12 < 4 && val % 2 == 1) || (val % 12 > 5 && val % 2 == 0) ? sfColor_fromRGB(50, 50, 50) : sfColor_fromRGB(200, 200, 200);
          sfRectangleShape_setFillColor(keys[pressedKey], setColor);
          pressedKeys[pressedKey] = true;
        }
      }
      else if (e.type == sfEvtKeyReleased) {
        int val = getMapValue(keyMap, e.key.code);
        if (val == -1) {
          continue;
        }

        sfColor resetColor = (val % 12 < 4 && val % 2 == 1) || (val % 12 > 5 && val % 2 == 0) ? sfBlack : sfWhite;

        sfRectangleShape_setFillColor(keys[val - 12], resetColor);
        pressedKeys[val - 12] = false;

        sfRectangleShape_setFillColor(keys[val], resetColor);
        pressedKeys[val] = false;

        sfRectangleShape_setFillColor(keys[val + 12], resetColor);
        pressedKeys[val + 12] = false;
      }

      sfRenderWindow_clear(window, sfWhite);
      drawPiano(window, keys);
      sfRenderWindow_display(window);
    }
  }

  /* CLEANUP */
  sfRenderWindow_destroy(window);
  sfRectangleShape_destroy(whiteKey);
  sfRectangleShape_destroy(blackKey);
  for (size_t i = 0; i < WHITEKEYCOUNT + BLACKKEYCOUNT; i++) {
    sfRectangleShape_destroy(keys[i]);
    sfSoundBuffer_destroy(soundBuffers[i]);
    sfSound_destroy(sounds[i]);
  }

  return EXIT_SUCCESS;
}