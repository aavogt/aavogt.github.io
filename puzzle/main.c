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
bool rotate_board;
typedef struct {
  int i, j;
} IJ;
IJ perm[nw][nh];

#define drag_max 10
typedef struct {
  bool used;
  int i, j;
  Vector2 pos, pos0;
  double time;
} Drag;
Drag drags[drag_max + 1];

RenderTexture2D board_cache;
bool board_dirty = true;

typedef enum { PickUp, PutDown, Throw, Locked } TileAnimationType;
typedef struct {
  TileAnimationType type;
  double start, end;
  int to_i, to_j;
  IJ image;
} TileAnimation;
TileAnimation tile_animations[nw][nh];

void start_tile_animation(int i, int j, TileAnimationType type,
                          double duration) {
  double start = GetTime();
  tile_animations[i][j] =
      (TileAnimation){.type = type, .start = start, .end = start + duration};
  board_dirty = true;
}

void pulse_correct(int i, int j) { start_tile_animation(i, j, Locked, 0.5); }

void animate_pickup(int i, int j) { start_tile_animation(i, j, PickUp, 0.3); }

void animate_putdown(int i, int j) { start_tile_animation(i, j, PutDown, 0.3); }

void animate_throw(Drag src, int i, int j) {
  double now = GetTime();
  double duration = now - src.time;
  if (duration < 0.1)
    duration = 0.1;
  if (duration > 0.5)
    duration = 0.5;

  IJ image = perm[src.i][src.j];
  IJ tmp = perm[src.i][src.j];
  perm[src.i][src.j] = perm[i][j];
  perm[i][j] = tmp;
  tile_animations[src.i][src.j] = (TileAnimation){
      .type = Throw,
      .start = now,
      .end = now + duration,
      .to_i = i,
      .to_j = j,
      .image = image,
  };
  board_dirty = true;
}

float animation_progress(TileAnimation animation, double now) {
  if (now <= animation.start)
    return 0;
  if (now >= animation.end)
    return 1;
  return (float)((now - animation.start) / (animation.end - animation.start));
}

bool correct(int i, int j) {
  IJ d = perm[i][j];
  return d.i == i && d.j == j;
}

IJ board_index(Vector2 p) {
  int w = GetScreenWidth(), h = GetScreenHeight();
  if (rotate_board)
    return (IJ){p.y * nw / h, p.x * nh / w};
  return (IJ){p.x * nw / w, p.y * nh / h};
}

bool screen_contains(Vector2 p) {
  return p.x >= 0 && p.x < GetScreenWidth() && p.y >= 0 &&
         p.y < GetScreenHeight();
}

void draw_tile(IJ source, Rectangle dest) {
  Vector2 origin = Vector2Zero();
  float rotation = 0;
  if (rotate_board) {
    Rectangle rotated = {dest.x + dest.width, dest.y, dest.height, dest.width};
    dest = rotated;
    origin = Vector2Zero();
    rotation = 90;
  }
  Rectangle source_rect =
      boards[source.i][rotate_board ? nh - 1 - source.j : source.j];
  DrawTexturePro(img, source_rect, dest, origin, rotation, WHITE);
}

void place1(int i, int j) {
  Rectangle base = boardd[i][j];
  Rectangle dest = base;
  IJ d = perm[i][j];
  bool done = correct(i, j);
  if (!done) {
    dest.x += 2;
    dest.y += 2;
    dest.width -= 4;
    dest.height -= 4;
  }

  TileAnimation animation = tile_animations[i][j];
  double now = GetTime();
  if (animation.end > animation.start && now < animation.end) {
    float t = animation_progress(animation, now);
    if (animation.type == Locked) {
      dest.x += 12.0f * sinf(12.5663706f * t);
    } else if (animation.type == PutDown) {
      Rectangle picked = base;
      picked.x += 10;
      picked.y += 10;
      picked.width -= 20;
      picked.height -= 20;
      dest.x = picked.x + (dest.x - picked.x) * t;
      dest.y = picked.y + (dest.y - picked.y) * t;
      dest.width = picked.width + (dest.width - picked.width) * t;
      dest.height = picked.height + (dest.height - picked.height) * t;
    } else if (animation.type == PickUp) {
      float padding = 10 * t;
      dest.x += padding;
      dest.y += padding;
      dest.width -= 2 * padding;
      dest.height -= 2 * padding;
    }
  }
  draw_tile(d, dest);
}

void place_drags() {
  double now = GetTime();
  Drag *d = drags;
  while (d->used) {
    Rectangle dest = boardd[d->i][d->j];
    dest.x += d->pos.x - d->pos0.x;
    dest.y += d->pos.y - d->pos0.y;
    TileAnimation animation = tile_animations[d->i][d->j];
    if (animation.type == PickUp && now < animation.end) {
      float padding = 10 * animation_progress(animation, now);
      dest.x += padding;
      dest.y += padding;
      dest.width -= 2 * padding;
      dest.height -= 2 * padding;
    } else {
      float padding = 10;
      dest.x += padding;
      dest.y += padding;
      dest.width -= 2 * padding;
      dest.height -= 2 * padding;
    }
    IJ p = perm[d->i][d->j];
    draw_tile(p, dest);
    d++;
  }
}

void place_throws(double now) {
  for (int i = 0; i < nw; i++)
    for (int j = 0; j < nh; j++) {
      TileAnimation animation = tile_animations[i][j];
      if (animation.type != Throw || now >= animation.end)
        continue;

      Rectangle from = boardd[i][j];
      Rectangle to = boardd[animation.to_i][animation.to_j];
      float t = animation_progress(animation, now);
      float cx0 = from.x + from.width / 2;
      float cy0 = from.y + from.height / 2;
      float cx1 = to.x + to.width / 2;
      float cy1 = to.y + to.height / 2;
      Rectangle dest = {cx0 + (cx1 - cx0) * t - from.width / 2,
                        cy0 + (cy1 - cy0) * t - from.height / 2, from.width,
                        from.height};
      draw_tile(animation.image, dest);
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
  bool inside = screen_contains(p);
  IJ cell = inside ? board_index(p) : (IJ){-1, -1};
  int i = cell.i, j = cell.j;
  int k = inside ? drag_find(i, j) : -1;
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && inside) { // press
    if (correct(i, j)) {
      pulse_correct(i, j);
    } else if (k < 0) { // not found
      int kend = drag_count();
      if (kend < drag_max) {
        drags[kend] = (Drag){true, i, j, p, p, GetTime()};
        k0 = kend;
        animate_pickup(i, j);
        down = true;
      }
    }
  } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && down) { // release
    Drag src = drags[k0];
    if (inside && (src.i != i || src.j != j)) {
      if (correct(i, j)) {
        pulse_correct(i, j);
        animate_putdown(src.i, src.j);
      } else {
        IJ *d = &perm[i][j];
        IJ *s = &perm[src.i][src.j];
        IJ tmp = *d;
        *d = *s;
        *s = tmp;
        animate_putdown(src.i, src.j);
        animate_putdown(i, j);
      }
    } else {
      animate_putdown(src.i, src.j);
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
    drags[k0].pos = p;
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
      if (screen_contains(p)) {
        IJ cell = board_index(p);
        int i = cell.i, j = cell.j;
        if (correct(i, j)) {
          pulse_correct(i, j);
        } else {
          int kend = drag_count();
          if (drag_find(i, j) < 0 && kend < drag_max) {
            drags[kend] = (Drag){true, i, j, p, p, GetTime()};
            k0[t] = kend;
            animate_pickup(i, j);
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
      if (screen_contains(src.pos)) {
        IJ cell = board_index(src.pos);
        int i = cell.i, j = cell.j;
        if (i != src.i || j != src.j) {
          if (correct(i, j)) {
            pulse_correct(i, j);
            animate_putdown(src.i, src.j);
          } else {
            IJ tmp = perm[src.i][src.j];
            perm[src.i][src.j] = perm[i][j];
            perm[i][j] = tmp;
            animate_putdown(src.i, src.j);
            animate_putdown(i, j);
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
            animate_putdown(src.i, src.j);
          } else {
            animate_throw(src, i, j);
          }
        } else {
          animate_putdown(src.i, src.j);
        }
      } else {
        animate_putdown(src.i, src.j);
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
  bool animating_background = false;
  bool throw_target[nw][nh] = {0};
  for (int i = 0; i < nw; i++)
    for (int j = 0; j < nh; j++) {
      TileAnimation *animation = &tile_animations[i][j];
      if (animation->end <= animation->start)
        continue;
      if (now >= animation->end) {
        *animation = (TileAnimation){0};
        board_dirty = true;
      } else if (animation->type == Throw) {
        throw_target[animation->to_i][animation->to_j] = true;
      } else if (animation->type == PutDown || animation->type == Locked) {
        animating_background = true;
      }
    }
  if (animating_background)
    board_dirty = true;

  if (board_dirty) {
    BeginTextureMode(board_cache);
    ClearBackground(BLACK);
    for (int i = 0; i < nw; i++)
      for (int j = 0; j < nh; j++) {
        if (!drag_has(i, j) && !throw_target[i][j])
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
  place_throws(now);
}

void boardd_reset() {
  static bool inited = false;
  if (!inited || IsWindowResized()) {
    int w = GetScreenWidth(), h = GetScreenHeight();
    rotate_board = h > w;
    for (int i = 0; i < nw; i++)
      for (int j = 0; j < nh; j++) {
        if (rotate_board)
          boardd[i][j] =
              (Rectangle){j * w / nh, i * h / nw, (j + 1) * w / nh - j * w / nh,
                          (i + 1) * h / nw - i * h / nw};
        else
          boardd[i][j] =
              (Rectangle){i * w / nw, j * h / nh, (i + 1) * w / nw - i * w / nw,
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
      boards[i][j] =
          (Rectangle){i * w / nw, j * h / nh, (i + 1) * w / nw - i * w / nw,
                      (j + 1) * h / nh - j * h / nh};
    }
}

bool replace_image(const char *path) {
  Texture next = LoadTexture(path);
  if (!IsTextureValid(next)) {
    TraceLog(LOG_WARNING, "Unable to load image: %s", path);
    return false;
  }
  if (IsTextureValid(img))
    UnloadTexture(img);
  img = next;
  boards_reset();
  board_dirty = true;
  return true;
}

#if defined(PLATFORM_WEB)
EMSCRIPTEN_KEEPALIVE bool load_image_from_browser(const char *path) {
  return replace_image(path);
}
#endif

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
#if defined(PLATFORM_WEB)
  replace_image(FileExists("/user-image.png") ? "/user-image.png"
                                              : "imgs/lake.jpg");
#else
  replace_image("imgs/lake.jpg");
#endif
  perm_reset();
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
