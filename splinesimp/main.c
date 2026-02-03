#include <gsl/gsl_interp.h>
#include <gsl/gsl_multimin.h>

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

const double radius = 30;
const double marginx = 30;
const double marginy = 120;

typedef double spline_t[5];
static spline_t px, py, pw;
float regularize = 0;

gsl_interp *crs5, *crs4, *warp;

typedef enum { ABDE, ABCDE, WARP } spline_eval_which;
#define MAX(a, b)                                                              \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a > _b ? _a : _b;                                                         \
  })
#define MIN(a, b)                                                              \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a < _b ? _a : _b;                                                         \
  })
#define CLAMP(x, a, b) MIN(MAX(x, a), b)

/// swapc(m, n, xs) puts xs[m] at xs[n-1],
/// and shifts m+1 .. n-1 elements over 1
void swapc(int m, int n, double xs[]) {
  double temp = xs[m];
  for (int i = m; i < n - 1; i++) {
    xs[i] = xs[i + 1];
  }
  xs[n - 1] = temp;
}

/// unswapc(m, n, xs) reverses swapc,
/// putting xs[n-1] back at xs[m] and shifting m .. n-2 over 1
void unswapc(int m, int n, double xs[]) {
  double temp = xs[n - 1];
  for (int i = n - 1; i > m; i--) {
    xs[i] = xs[i - 1];
  }
  xs[m] = temp;
}

Vector2 spline_eval(spline_eval_which which, double t) {
  static double ts[5];
  t = CLAMP(t, 1e-6, 1 - 1e-6);
  gsl_interp_accel *acc = NULL;
  Vector2 result;
  switch (which) {
  case ABDE: {
    // generalize taking an array len of indices to delete instead of hardcoding
    // 5 and 4?
    int n = 4;
    swapc(2, 5, px);
    swapc(2, 5, py);
    for (int i = 0; i < n; i++)
      ts[i] = (double)i / (n - 1);
    gsl_interp_init(crs4, ts, px, n);
    result.x = gsl_interp_eval(crs4, ts, px, t, acc);
    gsl_interp_init(crs4, ts, py, n);
    result.y = gsl_interp_eval(crs4, ts, py, t, acc);
    unswapc(2, 5, px);
    unswapc(2, 5, py);
    break;
  }
  case ABCDE: {
    int n = 5;
    for (int i = 0; i < n; i++)
      ts[i] = (double)i / (n - 1);

    gsl_interp_init(warp, ts, pw, n);
    double s = gsl_interp_eval(warp, ts, pw, t, acc);
    s = CLAMP(s, 0, 1);
    gsl_interp_init(crs5, ts, px, n);
    result.x = gsl_interp_eval(crs5, ts, px, s, acc);
    gsl_interp_init(crs5, ts, py, n);
    result.y = gsl_interp_eval(crs5, ts, py, s, acc);
    break;
  }
  case WARP: {
    int n = 5;
    for (int i = 0; i < n; i++)
      ts[i] = (double)i / (n - 1);
    gsl_interp_init(warp, ts, pw, n);
    result.x = t;
    result.y = gsl_interp_eval(warp, ts, pw, t, acc);

    break;
  }
  }
  gsl_interp_accel_free(acc);
  return result;
}

// Srivastava 2016 who surveys a regularization option, gamma'' eq 4.16
// this doesn't seem to be necessary to add to result in f
// because my warping minimizes the maximum distance, and their
// survey seems to minimize the total distance
double spline_warp_roughness() {
  double result, ts[5];
  int n = 5;
  gsl_interp_accel *acc = NULL;
  for (int i = 0; i < n; i++)
    ts[i] = (double)i / (n - 1);

  gsl_interp_init(warp, ts, pw, n);

  const int imax = 100;
  for (int i = 0; i <= imax; i++) {
    double t = (double)i / imax;
    double s = gsl_interp_eval_deriv2(warp, ts, pw, t, acc);
    result += s * s;
  }
  gsl_interp_accel_free(acc);
  return result / (imax + 1);
}
double tstar = -1;
double last_result = 0;
double last_r = 0;

#define NEV 100
static Vector2 abde[NEV];
void preopt() {
  for (int i = 0; i < NEV; i++) {
    abde[i] = spline_eval(ABDE, (double)i / (NEV - 1));
  }
}

double f_eval(const double *x, unsigned n) {
  memcpy(pw + 1, x, n * sizeof(double));
  for (int i = 1; i < 5; i++) {
    pw[i] += pw[i - 1]; // cumsum
  }
  //  normalize 0 to 1
  for (int i = 1; i < 5; i++) {
    pw[i] -= pw[0];
  }
  pw[0] = 0;
  pw[4] = 1;
  for (int i = 1; i < 5; i++) {
    pw[i] = MIN(pw[i], 1);
  }

  double result = 0;
  for (int i = 0; i < NEV; i++) {
    double t = i;
    t /= NEV - 1;
    Vector2 abcde = spline_eval(ABCDE, t);
    double d = Vector2DistanceSqr(abcde, abde[i]);
    if (d > result) {
      result = d;
      tstar = t;
    }
  }
  last_result = result;
  last_r = spline_warp_roughness();
  return result + regularize * last_r;
}

double f_gsl(const gsl_vector *x, void *params) {
  (void)params;
  double xs[3];
  for (int i = 0; i < 3; i++) {
    xs[i] = gsl_vector_get(x, i);
  }
  return f_eval(xs, 3);
}

void pw_opt() {
  const size_t n = 3;
  const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex2;
  gsl_multimin_fminimizer *s = NULL;
  gsl_vector *x = gsl_vector_alloc(n);
  gsl_vector *ss = gsl_vector_alloc(n);
  for (int i = 0; i < 3; i++) {
    gsl_vector_set(x, i, 0.2);
  }
  gsl_vector_set_all(ss, 0.1);

  gsl_multimin_function minex_func;
  minex_func.n = n;
  minex_func.f = f_gsl;
  minex_func.params = NULL;

  preopt();
  s = gsl_multimin_fminimizer_alloc(T, n);
  gsl_multimin_fminimizer_set(s, &minex_func, x, ss);

  size_t iter = 0;
  int status;
  double size;
  do {
    iter++;
    status = gsl_multimin_fminimizer_iterate(s);
    if (status) {
      break;
    }
    size = gsl_multimin_fminimizer_size(s);
    status = gsl_multimin_test_size(size, 1e-3);
  } while (status == GSL_CONTINUE && iter < 100);

  double xs[3];
  for (int i = 0; i < 3; i++) {
    xs[i] = gsl_vector_get(s->x, i);
  }
  f_eval(xs, 3);

  gsl_vector_free(x);
  gsl_vector_free(ss);
  gsl_multimin_fminimizer_free(s);
}

void draw_unit_line(Vector2 a[2], Color col) {
  double w = GetScreenWidth() - marginx;
  double h = GetScreenHeight() - marginy;
  DrawLine(a[0].x * w, a[0].y * h, a[1].x * w, a[1].y * h, col);
}

void draw_text_i(const char *str, int i, Color col) {
  double w = GetScreenWidth() - marginx;
  double h = GetScreenHeight() - marginy;
  DrawText(str, w * px[i], h * py[i], radius / 2, col);
}

void draw_circle_i(int i, double radius, Color col) {
  double w = GetScreenWidth() - marginx;
  double h = GetScreenHeight() - marginy;
  DrawCircle(w * px[i], h * py[i], radius, col);
}

void p_rescale() {
  // get the maximum component
  // divide by the maximum component
  double max = 0;
  (void)max;
  for (int i = 0; i < 5; i++) {
    px[i] = CLAMP(px[i], 0, 1);
    py[i] = CLAMP(py[i], 0, 1);
    pw[i] = CLAMP(pw[i], 0, 1);
  }
}

void p_reset_rescale() {
  for (int i = 0; i < 5; i++) {
    px[i] = random() * 1.0 / RAND_MAX;
    py[i] = random() * 1.0 / RAND_MAX;
    pw[i] = (double)i / 4;
  }
  p_rescale();
}

/// if (handle_dragging()) { // is dragging
/// }
bool handle_dragging() {
  const double radius2 = pow(radius, 2);
  double w = GetScreenWidth() - marginx;
  double h = GetScreenHeight() - marginy;
  static int dragging = -1;
  if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    dragging = -1;
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    Vector2 m = GetMousePosition();
    for (int i = 0; i < 5; i++) {
      double dx = m.x - w * px[i];
      double dy = m.y - h * py[i];
      if (dx * dx + dy * dy < radius2) {
        dragging = i;
      }
    }
  }
  if (dragging >= 0) {
    Vector2 md = GetMouseDelta();
    px[dragging] += md.x / w;
    py[dragging] += md.y / h;
    p_rescale();
  }
  return dragging >= 0;
}

void UpdateDrawFrame() {
  if (IsKeyPressed(KEY_R)) {
    p_reset_rescale();
    pw_opt();
  }
  if (handle_dragging()) {
    p_rescale();
    pw_opt();
  }

  BeginDrawing();

  ClearBackground(BLACK);
  {
    const int labelw = 260;
    const int slider_h = 30;
    const int label_h = (int)marginy - slider_h;
    const int panel_y = GetScreenHeight() - (int)marginy;
    char labeltxt[200];
    snprintf(labeltxt, sizeof(labeltxt),
             "pw: %.3f %.3f %.3f %.3f %.3f\nt*: %.3f r: %.3f res: %.3f", pw[0],
             pw[1], pw[2], pw[3], pw[4], tstar, last_r, last_result);
    GuiLabel((Rectangle){0, panel_y, labelw, label_h}, labeltxt);

    Rectangle plot = {labelw, panel_y, GetScreenWidth() - marginx - labelw,
                      label_h};
    const int nseg = 100;
    Vector2 prev = spline_eval(WARP, 0);
    for (int i = 1; i < nseg; i++) {
      double t = (double)i / (nseg - 1);
      Vector2 cur = spline_eval(WARP, t);
      int x0 = plot.x + prev.x * plot.width;
      int y0 = plot.y + plot.height - prev.y * plot.height;
      int x1 = plot.x + cur.x * plot.width;
      int y1 = plot.y + plot.height - cur.y * plot.height;
      DrawLine(x0, y0, x1, y1, GREEN);
      prev = cur;
    }

    if (GuiSlider((Rectangle){labelw, GetScreenHeight() - slider_h,
                              GetScreenWidth() - marginx - labelw, slider_h},
                  "-1", "10", &regularize, -1, 10))
      pw_opt();
  }

  const Color circle_colors[5] = {GRAY, GRAY, RAYWHITE, GRAY, GRAY};
  const char *circle_labels[5] = {"A", "B", "C", "D", "E"};
  for (int i = 0; i < 5; i++) {
    draw_circle_i(i, radius, circle_colors[i]);
  }
  Vector2 sp[2], tp[2];
  for (int i = 0; i < 100; i++) {
    double s = i / 100.0;
    sp[i % 2] = spline_eval(ABDE, s);
    tp[i % 2] = spline_eval(ABCDE, s);
    if (i != 0) {
      draw_unit_line(sp, BLUE);
      draw_unit_line(tp, YELLOW);
    }
  };
  sp[0] = spline_eval(ABCDE, tstar);
  sp[1] = spline_eval(ABDE, tstar);
  draw_unit_line(sp, RED);
  for (int i = 0; i < 5; i++) {
    draw_text_i(circle_labels[i], i, BLACK);
  }
  EndDrawing();
}

int main() {
  printf("starting\n");
  srandom((unsigned)time(NULL));
  p_reset_rescale();
  crs4 = gsl_interp_alloc(gsl_interp_cspline, 4);
  crs5 = gsl_interp_alloc(gsl_interp_cspline, 5);
  warp = gsl_interp_alloc(gsl_interp_steffen, 5);
  pw_opt();

  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  InitWindow(640, 480, "spline");
#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
  SetTargetFPS(20);
  while (!WindowShouldClose() && !IsKeyPressed(KEY_ESCAPE)) {
    UpdateDrawFrame();
  }
#endif

  CloseWindow();
  return 0;
}
