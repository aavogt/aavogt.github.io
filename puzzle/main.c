#ifndef RAYLIBD
#include "raygui.h"
#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#endif
#include <stdbool.h>
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

// 1=x 2=o, 0 empty or draw
Texture img;
#define nw 6
#define nh 3
Rectangle boards[nw][nh];
Rectangle boardd[nw][nh];
typedef struct {
  int i, j;
} IJ;
IJ perm[nw][nh];

#define drag_max 10
typedef struct {
  bool used;
  int i, j;
  Vector2 pos, pos0;
} Drag;
Drag drags[drag_max + 1];

RenderTexture2D board_cache;
bool board_dirty = true;

typedef struct {
  double start, end;
} TileAnimation;
TileAnimation tile_animations[nw][nh];

void pulse_correct(int i, int j) {
  double start = GetTime();
  tile_animations[i][j] = (TileAnimation){start, start + 0.5};
  board_dirty = true;
}

bool correct(int i, int j) {
  IJ d = perm[i][j];
  return d.i == i && d.j == j;
}

void place1(int i, int j) {
  Rectangle dest = boardd[i][j];
  IJ d = perm[i][j];
  bool done = correct(i, j);
  if (!done) {
    dest.x += 2;
    dest.y += 2;
    dest.width -= 4;
    dest.height -= 4;
  } else {
    TileAnimation animation = tile_animations[i][j];
    if (animation.end > animation.start) {
      double now = GetTime();
      if (now < animation.end) {
        float t = (float)((now - animation.start) /
                          (animation.end - animation.start));
        dest.x += 12.0f * sinf(12.5663706f * t);
      }
    }
  }
  DrawTexturePro(img, boards[d.i][d.j], dest, Vector2Zero(), 0, WHITE);
}

void place_drags() {
  Drag *d = drags;
  while (d->used) {
    Rectangle dest = boardd[d->i][d->j];
    dest.x += d->pos.x - d->pos0.x;
    dest.y += d->pos.y - d->pos0.y;
    IJ p = perm[d->i][d->j];
    DrawTexturePro(img, boards[p.i][p.j], dest, Vector2Zero(), 0, WHITE);
    d++;
  }
}

bool drag_has(int i, int j) {
  Drag *d = drags;
  while (d->used) {
    if (d->i == i && d->j == j)
      return true;
    d++;
  }
  return false;
}

int drag_find(int i, int j) {
  int n = 0;
  Drag *d = drags;
  while (d[n].used) {
    if (d[n].i == i && d[n].j == j)
      return n;
    n++;
  }
  return -1;
}

int drag_count() {
  int n = 0;
  Drag *d = drags;
  while (d++->used) {
    n++;
  }
  return n;
}

void drag_mouse() {
  static bool down = false;
  static int k0 = -1;
  Vector2 p = GetMousePosition();
  int i = p.x * nw / GetScreenWidth();
  int j = p.y * nh / GetScreenHeight();
  int k = drag_find(i, j);
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { // press
    if (correct(i, j)) {
      pulse_correct(i, j);
    } else if (k < 0) { // not found
      int kend = drag_count();
      if (kend < drag_max) {
        drags[kend] = (Drag){true, i, j, p, p};
        k0 = kend;
        board_dirty = true;
        down = true;
      }
    }
  } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && down) { // release
    Drag src = drags[k0];
    if (src.i != i || src.j != j) {
      if (correct(i, j)) {
        pulse_correct(i, j);
      } else {
        IJ *d = &perm[i][j];
        IJ *s = &perm[src.i][src.j];
        IJ tmp = *d;
        *d = *s;
        *s = tmp;
      }
    }

    int kend = drag_count();
    drags[k0].used = false;
    if (kend > k0) {
      drags[k0] = drags[kend];
      drags[kend].used = false;
    }
    board_dirty = true;
    down = false;
  }

  if (down)
    drags[k0].pos = GetMousePosition();
}

void drag_touch() {
  static bool down[drag_max];
  static int touch_id[drag_max];
  static int k0[drag_max];
  static int count = 0;
  int point_count = GetTouchPointCount();

  for (int n = 0; n < point_count; n++) {
    int id = GetTouchPointId(n);
    int t = 0;
    while (t < count && (!down[t] || touch_id[t] != id))
      t++;

    if (t == count) {
      if (count == drag_max)
        continue;
      t = count++;
      down[t] = true;
      touch_id[t] = id;
      k0[t] = -1;

      Vector2 p = GetTouchPosition(n);
      if (p.x >= 0 && p.x < GetScreenWidth() && p.y >= 0 &&
          p.y < GetScreenHeight()) {
        int i = p.x * nw / GetScreenWidth();
        int j = p.y * nh / GetScreenHeight();
        if (correct(i, j)) {
          pulse_correct(i, j);
        } else {
          int kend = drag_count();
          if (drag_find(i, j) < 0 && kend < drag_max) {
            drags[kend] = (Drag){true, i, j, p, p};
            k0[t] = kend;
            board_dirty = true;
          }
        }
      }
    }

    if (k0[t] >= 0)
      drags[k0[t]].pos = GetTouchPosition(n);
  }

  for (int t = 0; t < count;) {
    bool found = false;
    for (int n = 0; n < point_count; n++) {
      if (GetTouchPointId(n) == touch_id[t]) {
        found = true;
        break;
      }
    }
    if (found) {
      t++;
      continue;
    }

    if (k0[t] >= 0) {
      Drag src = drags[k0[t]];
      float dx = src.pos.x - src.pos0.x;
      float dy = src.pos.y - src.pos0.y;
      if (src.pos.x >= 0 && src.pos.x < GetScreenWidth() && src.pos.y >= 0 &&
          src.pos.y < GetScreenHeight()) {
        int i = src.pos.x * nw / GetScreenWidth();
        int j = src.pos.y * nh / GetScreenHeight();
        if (i != src.i || j != src.j) {
          if (correct(i, j)) {
            pulse_correct(i, j);
          } else {
            IJ tmp = perm[src.i][src.j];
            perm[src.i][src.j] = perm[i][j];
            perm[i][j] = tmp;
          }
        } else if (dx * dx + dy * dy > 20 * 20) {
          float ax = dx < 0 ? -dx : dx;
          float ay = dy < 0 ? -dy : dy;
          int di = 0, dj = 0;
          if (ax > ay)
            di = dx > 0 ? 1 : -1;
          else
            dj = dy > 0 ? 1 : -1;
          int i = (src.i + nw + di) % nw;
          int j = (src.j + nh + dj) % nh;
          if (correct(i, j)) {
            pulse_correct(i, j);
          } else {
            IJ tmp = perm[src.i][src.j];
            perm[src.i][src.j] = perm[i][j];
            perm[i][j] = tmp;
          }
        }
      }

      int last_drag = drag_count() - 1;
      drags[k0[t]].used = false;
      if (last_drag > k0[t]) {
        drags[k0[t]] = drags[last_drag];
        drags[last_drag].used = false;
        for (int other = 0; other < count; other++) {
          if (other != t && k0[other] == last_drag)
            k0[other] = k0[t];
        }
      }
      board_dirty = true;
    }

    int last_touch = count - 1;
    if (t < last_touch) {
      down[t] = down[last_touch];
      touch_id[t] = touch_id[last_touch];
      k0[t] = k0[last_touch];
    }
    count--;
  }
}

void drag() {
  drag_touch();
  drag_mouse();
}

void place() {
  double now = GetTime();
  bool animating = false;
  for (int i = 0; i < nw; i++)
    for (int j = 0; j < nh; j++) {
      TileAnimation *animation = &tile_animations[i][j];
      if (animation->end > animation->start) {
        if (now < animation->end) {
          animating = true;
        } else {
          *animation = (TileAnimation){0};
          board_dirty = true;
        }
      }
    }
  if (animating)
    board_dirty = true;

  if (board_dirty) {
    BeginTextureMode(board_cache);
    ClearBackground(BLACK);
    for (int i = 0; i < nw; i++)
      for (int j = 0; j < nh; j++) {
        if (!drag_has(i, j))
          place1(i, j);
      }
    EndTextureMode();
    board_dirty = false;
  }

  DrawTextureRec(
      board_cache.texture,
      (Rectangle){0, 0, board_cache.texture.width, -board_cache.texture.height},
      Vector2Zero(), WHITE);
  place_drags();
}

void boardd_reset() {
  static bool inited = false;
  if (!inited || IsWindowResized()) {
    int w = GetScreenWidth(), h = GetScreenHeight();
    for (int i = 0; i < nw; i++)
      for (int j = 0; j < nh; j++) {
        boardd[i][j] = (Rectangle){i * w / nw, j * h / nh,
                                   (i + 1) * w / nw - i * w / nw,
                                   (j + 1) * h / nh - j * h / nh};
      }
    if (inited)
      UnloadRenderTexture(board_cache);
    board_cache = LoadRenderTexture(w, h);
    board_dirty = true;
    inited = true;
  }
}

void boards_reset() {
  int w = img.width, h = img.height;
  for (int i = 0; i < nw; i++)
    for (int j = 0; j < nh; j++) {
      boards[i][j] = (Rectangle){i * w / nw, j * h / nh,
                                 (i + 1) * w / nw - i * w / nw,
                                 (j + 1) * h / nh - j * h / nh};
    }
}

// nh*nw swaps
// compare https://okmij.org/ftp/Haskell/perfect-shuffle.txt or TAOCP
void perm_reset() {
  for (int i = 0; i < nw; i++)
    for (int j = 0; j < nh; j++) {
      perm[i][j] = (IJ){i, j};
    }
  for (int i = 0; i < nw; i++)
    for (int j = 0; j < nh; j++) {
      int dij = rand() % (nw * nh);
      IJ *src = &perm[i][j], *dst = perm[0] + dij, tmp = *src;
      *src = *dst;
      *dst = tmp;
    }
  board_dirty = true;
}

void UpdateDrawFrame(void) {
  boardd_reset();
  drag();
  BeginDrawing();
  ClearBackground(BLACK);
  place();
  EndDrawing();
}

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

  InitWindow(800, 600, "main.c");
  img = LoadTexture("imgs/lake.jpg");
  perm_reset();
  boards_reset();
#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
  SetTargetFPS(20);
  while (!WindowShouldClose() && !IsKeyPressed(KEY_ESCAPE)) {
    UpdateDrawFrame();
  }
  UnloadRenderTexture(board_cache);
  UnloadTexture(img);
  CloseWindow();
#endif
  return 0;
}
