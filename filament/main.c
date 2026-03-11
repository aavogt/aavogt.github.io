// planar filament merging: what is the optimal wye fitting given the inlet and
// outlet positions/orientations
#include <gsl/gsl_interp.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_odeiv2.h>
#include <gsl/gsl_spline.h>

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

#define N 5

const float user_point_radius = 55.0f;
const float angle_pick_distance = 12.0f;
const double handle_length = 100;
const float base_screen_width = 640.0f;
const float base_screen_height = 480.0f;
struct {
  float x, y, theta;
} user_point[3];

struct {
  gsl_spline *sx1, *sy1, *sx2, *sy2, *sx12, *sy12;
  gsl_interp_accel *acc;
  double ts[N], x1[N], y1[N], x2[N], y2[N], x12[N], y12[N], kappa[N],
      kappa_ss[N];
} s;

void alloc_splines() {
  const gsl_interp_type *ty = gsl_interp_cspline;
  s.sx1 = gsl_spline_alloc(ty, N);
  s.sx2 = gsl_spline_alloc(ty, N);
  s.sx12 = gsl_spline_alloc(ty, N);
  s.sy1 = gsl_spline_alloc(ty, N);
  s.sy2 = gsl_spline_alloc(ty, N);
  s.sy12 = gsl_spline_alloc(ty, N);
  s.acc = gsl_interp_accel_alloc();
}

void init_splines() {
  gsl_spline_init(s.sx1, s.ts, s.x1, N);
  gsl_spline_init(s.sy1, s.ts, s.y1, N);
  gsl_spline_init(s.sx2, s.ts, s.x2, N);
  gsl_spline_init(s.sy2, s.ts, s.y2, N);
  gsl_spline_init(s.sx12, s.ts, s.x12, N);
  gsl_spline_init(s.sy12, s.ts, s.y12, N);
}

void free_splines() {
  gsl_spline_free(s.sx1);
  gsl_spline_free(s.sx2);
  gsl_spline_free(s.sx12);
  gsl_spline_free(s.sy1);
  gsl_spline_free(s.sy2);
  gsl_spline_free(s.sy12);
  gsl_interp_accel_free(s.acc);

  s.sx1 = NULL;
  s.sx2 = NULL;
  s.sx12 = NULL;
  s.sy1 = NULL;
  s.sy2 = NULL;
  s.sy12 = NULL;
  s.acc = NULL;
}

typedef struct {
  double EI[2], w1, kappa_h;
  enum { S1, S2, S12 } which;
} Params;

const double ei0 = 1e2;
/* GUI-controlled penalty weight */
double gui_lambda = 1e1;
Params params = {ei0, ei0, 1, 1e-4, S1};
typedef struct {
  double kappa, kappa_ss;
} Kappas;

Kappas eval_kappas(double t, Params *params) {
  double h = params->kappa_h;
  double xp[3], xpp[3], yp[3], ypp[3], v[3], kappa[3];

  for (int i = 0; i < 3; i++) {
    double tj = t + (i - 1) * h;

    switch (params->which) {
    case S1:
      xp[i] = gsl_spline_eval_deriv(s.sx1, tj, s.acc);
      xpp[i] = gsl_spline_eval_deriv2(s.sx1, tj, s.acc);
      yp[i] = gsl_spline_eval_deriv(s.sy1, tj, s.acc);
      ypp[i] = gsl_spline_eval_deriv2(s.sy1, tj, s.acc);
      break;
    case S2:
      xp[i] = gsl_spline_eval_deriv(s.sx2, tj, s.acc);
      xpp[i] = gsl_spline_eval_deriv2(s.sx2, tj, s.acc);
      yp[i] = gsl_spline_eval_deriv(s.sy2, tj, s.acc);
      ypp[i] = gsl_spline_eval_deriv2(s.sy2, tj, s.acc);
      break;
    case S12:
      xp[i] = gsl_spline_eval_deriv(s.sx12, tj, s.acc);
      xpp[i] = gsl_spline_eval_deriv2(s.sx12, tj, s.acc);
      yp[i] = gsl_spline_eval_deriv(s.sy12, tj, s.acc);
      ypp[i] = gsl_spline_eval_deriv2(s.sy12, tj, s.acc);
      break;
    }

    v[i] = hypot(xp[i], yp[i]);
    if (!isfinite(v[i]) || v[i] < 1e-12) {
      kappa[i] = 0.0;
    } else {
      kappa[i] = (xp[i] * ypp[i] - yp[i] * xpp[i]) / pow(v[i], 3);
    }
  }

  double kappa_dot = (kappa[2] - kappa[0]) / (2 * h);
  double kappa_ddot = (kappa[2] - 2 * kappa[1] + kappa[0]) / (h * h);
  double v_dot = xp[1] * xpp[1] + yp[1] * ypp[1];

  if (!isfinite(v[1]) || v[1] < 1e-12) {
    return (Kappas){0.0, 0.0};
  }

  double kappa_ss = (kappa_ddot * v[1] - kappa_dot * v_dot) / pow(v[1], 3);
  if (!isfinite(kappa_ss)) {
    kappa_ss = 0.0;
  }

  return (Kappas){kappa[1], kappa_ss};
}
int step(double t, const double y[], double dy[], void *paramsv) {
  Params *params = paramsv;
  double tc = Clamp(t, s.ts[0] + 2 * params->kappa_h,
                    s.ts[N - 1] - 2 * params->kappa_h);
  Kappas k = eval_kappas(tc, paramsv);
  double EI;
  switch (params->which) {
  case S1:
    EI = params->EI[0];
    break;
  case S2:
    EI = params->EI[1];
    break;
  case S12:
    EI = params->EI[0] + params->EI[1];
    break;
  }
  dy[0] = fabs(k.kappa * y[0] + EI * k.kappa_ss);
  return GSL_SUCCESS;
}

// evaluate the tension for the given segment
double tension(Params *params) {
  gsl_odeiv2_system sys = (gsl_odeiv2_system){&step, NULL, 1, params};
  gsl_odeiv2_driver *d = gsl_odeiv2_driver_alloc_y_new(
      &sys, gsl_odeiv2_step_rkf45, 1e-2, 1e-5, 1e-4);
  double t = 0, y = 0;
  gsl_odeiv2_driver_apply(d, &t, s.ts[N - 1], &y);
  gsl_odeiv2_driver_free(d);
  return isnormal(y) ? y : 1e10;
}

void to_vec(gsl_vector *x) {
  int i = 0;
  // clang-format off
  for (int j = 2; j < N-1; j++) gsl_vector_set(x, i++, s.x1[j]);
  for (int j = 2; j < N-1; j++) gsl_vector_set(x, i++, s.y1[j]);
  for (int j = 2; j < N-1; j++) gsl_vector_set(x, i++, s.x2[j]);
  for (int j = 2; j < N-1; j++) gsl_vector_set(x, i++, s.y2[j]);
  for (int j = 2; j < N-1; j++) gsl_vector_set(x, i++, s.x12[j]);
  for (int j = 2; j < N-1; j++) gsl_vector_set(x, i++, s.y12[j]);
  // clang-format on
  gsl_vector_set(x, i++, s.x1[N - 1]);
  gsl_vector_set(x, i++, s.y1[N - 1]);
}

void from_vec(const gsl_vector *x) {
  int i = 0;
  // clang-format off
  for (int j = 2; j < N-1; j++) s.x1[j] = gsl_vector_get(x, i++);
  for (int j = 2; j < N-1; j++) s.y1[j] = gsl_vector_get(x, i++);
  for (int j = 2; j < N-1; j++) s.x2[j] = gsl_vector_get(x, i++);
  for (int j = 2; j < N-1; j++) s.y2[j] = gsl_vector_get(x, i++);
  for (int j = 1; j < N-2; j++) s.x12[j] = gsl_vector_get(x, i++);
  for (int j = 1; j < N-2; j++) s.y12[j] = gsl_vector_get(x, i++);
  // clang-format on

  double xj = gsl_vector_get(x, i++);
  double yj = gsl_vector_get(x, i++);
  s.x1[N - 1] = xj;
  s.y1[N - 1] = yj;
  s.x2[N - 1] = xj;
  s.y2[N - 1] = yj;
  s.x12[0] = xj;
  s.y12[0] = yj;
}

/// set endpoints of the 3 splines
void from_user() {
  // endpoints
  const double tangent_len =
      10; // handle_length except for the simulation not UI
  s.x1[0] = user_point[0].x;
  s.y1[0] = user_point[0].y;
  s.x2[0] = user_point[1].x;
  s.y2[0] = user_point[1].y;
  s.x12[N - 1] = user_point[2].x;
  s.y12[N - 1] = user_point[2].y;
  // why not reuse the gui handle_length?
  s.x1[1] = s.x1[0] + tangent_len * cos(user_point[0].theta);
  s.x2[1] = s.x2[0] + tangent_len * cos(user_point[1].theta);
  s.x12[N - 2] = s.x12[N - 1] + tangent_len * cos(user_point[2].theta);

  s.y1[1] = s.y1[0] + tangent_len * sin(user_point[0].theta);
  s.y2[1] = s.y2[0] + tangent_len * sin(user_point[1].theta);
  s.y12[N - 2] = s.y12[N - 1] + tangent_len * sin(user_point[2].theta);
}

/// fills in values between those set by from_user()
void s_lerp_centroid() {
  // junction
  double xj = (s.x1[0] + s.x2[0] + s.x12[N - 1]) / 3;
  double yj = (s.y1[0] + s.y2[0] + s.y12[N - 1]) / 3;
  s.x1[N - 1] = xj;
  s.x2[N - 1] = xj;
  s.x12[0] = xj;

  s.y1[N - 1] = yj;
  s.y2[N - 1] = yj;
  s.y12[0] = yj;

  /* interpolate branch 1 and 2 between handle near user (index 1) and
     double-step point near junction (index N-3) */
  for (int i = 2; i < N - 1; i++) {
    double t = (double)(i - 1) / (N - 1);
    s.x1[i] = s.x1[1] + t * (xj - s.x1[1]);
    s.y1[i] = s.y1[1] + t * (yj - s.y1[1]);
    s.x2[i] = s.x2[1] + t * (xj - s.x2[1]);
    s.y2[i] = s.y2[1] + t * (yj - s.y2[1]);
  }

  /* interpolate middle branch between the double-step point (index 2)
     and the handle near outlet (index N-2) */
  for (int i = 1; i < N - 2; i++) {
    double t = (double)(i) / (N - 2);
    s.x12[i] = xj + t * (s.x12[N - 1] - xj);
    s.y12[i] = yj + t * (s.y12[N - 1] - yj);
  }
}

// draw evaluated spline sample points for all three splines
void draw_spline_points(void) {
  for (int i = 0; i < N; i++) {
    double t = s.ts[i];
    double x1 = s.x1[i];
    double y1 = s.y1[i];
    double x2 = s.x2[i];
    double y2 = s.y2[i];
    double x12 = s.x12[i];
    double y12 = s.y12[i];
    DrawCircleV((Vector2){(float)x1, (float)y1}, 3.0f, LIGHTGRAY);
    DrawCircleV((Vector2){(float)x2, (float)y2}, 3.0f, LIGHTGRAY);
    DrawCircleV((Vector2){(float)x12, (float)y12}, 3.0f, LIGHTGRAY);
  }
}

double f_gsl(const gsl_vector *x, void *paramsv) {
  Params *params = paramsv;
  from_user();
  from_vec(x);
  init_splines();
  params->which = S1;
  double t1 = tension(params);
  params->which = S2;
  double t2 = tension(params);
  params->which = S12;
  double t12 = tension(params);

  // penalty to force tangent endpoints of branch1 and branch2 to be
  // collinear and opposite to the start tangent of branch12
  double penalty = 0.0;
  // compute tangent vectors using finite differences
  double v1x = s.x1[N - 1] - s.x1[N - 2];
  double v1y = s.y1[N - 1] - s.y1[N - 2];
  double v2x = s.x2[N - 1] - s.x2[N - 2];
  double v2y = s.y2[N - 1] - s.y2[N - 2];
  double v12x = s.x12[1] - s.x12[0];
  double v12y = s.y12[1] - s.y12[0];

  double n1 = hypot(v1x, v1y);
  double n2 = hypot(v2x, v2y);
  double n12 = hypot(v12x, v12y);
  if (n1 > 1e-8 && n12 > 1e-8) {
    double u1x = v1x / n1, u1y = v1y / n1;
    double u12x = v12x / n12, u12y = v12y / n12;
    double dx = u1x + u12x;
    double dy = u1y + u12y;
    penalty -= dx * dx + dy * dy;
  }
  if (n2 > 1e-8 && n12 > 1e-8) {
    double u2x = v2x / n2, u2y = v2y / n2;
    double u12x = v12x / n12, u12y = v12y / n12;
    double dx = u2x + u12x;
    double dy = u2y + u12y;
    penalty -= dx * dx + dy * dy;
  }
  // weight for penalty (tunable)
  // controlled from GUI via global gui_lambda

  return params->w1 * t1 + t2 + (1 + params->w1) * t12 + gui_lambda * penalty;
}

typedef struct {
  size_t n;
  gsl_multimin_fminimizer *fm;
  gsl_vector *x;
  gsl_vector *ss;
  gsl_multimin_function minex_func;
  bool alloc, init;
} Opt;

static Opt opt = {0, NULL, NULL, NULL, {0, 0, NULL}, false, false};

void alloc_opt() {
  if (opt.alloc)
    return;
  opt.n = (N - 3) * 6 + 2;

  opt.x = gsl_vector_alloc(opt.n);
  opt.ss = gsl_vector_alloc(opt.n);
  opt.alloc = true;
}

void init_opt() {
  if (!opt.alloc)
    alloc_opt();
  to_vec(opt.x);
  gsl_vector_set_all(opt.ss, 30);
  gsl_vector_set(opt.ss, opt.n - 1, 30);

  opt.minex_func.n = opt.n;
  opt.minex_func.f = f_gsl;
  opt.minex_func.params = &params;

  const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex2;
  opt.fm = gsl_multimin_fminimizer_alloc(T, opt.n);
  gsl_multimin_fminimizer_set(opt.fm, &opt.minex_func, opt.x, opt.ss);

  opt.init = true;
}

int step_opt() {
  if (!opt.init)
    init_opt();
  int status = gsl_multimin_fminimizer_iterate(opt.fm);
  if (status)
    return status;
  double size = gsl_multimin_fminimizer_size(opt.fm);
  status = gsl_multimin_test_size(size, 1e-3);
  return status;
}

void free_opt() {
  if (!opt.init)
    return;
  gsl_vector_free(opt.x);
  gsl_vector_free(opt.ss);
  gsl_multimin_fminimizer_free(opt.fm);
  opt.x = NULL;
  opt.ss = NULL;
  opt.fm = NULL;
  opt.init = false;
  opt.alloc = false;
}

static float DistancePointSegment(Vector2 p, Vector2 a, Vector2 b) {
  Vector2 ab = Vector2Subtract(b, a);
  float ab_len2 = Vector2DotProduct(ab, ab);
  if (ab_len2 <= 1e-6f) {
    return Vector2Distance(p, a);
  }
  float t = Vector2DotProduct(Vector2Subtract(p, a), ab) / ab_len2;
  t = Clamp(t, 0.0f, 1.0f);
  Vector2 proj = Vector2Add(a, Vector2Scale(ab, t));
  return Vector2Distance(p, proj);
}

static bool update_user_points() {
  static int drag_point = -1;
  static int drag_angle = -1;

  Vector2 m = GetMousePosition();

  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    for (int i = 0; i < 3; i++) {
      Vector2 p = {user_point[i].x, user_point[i].y};
      Vector2 h = {p.x + (float)(handle_length * cos(user_point[i].theta)),
                   p.y + (float)(handle_length * sin(user_point[i].theta))};

      if (Vector2Distance(m, p) <= user_point_radius) {
        drag_point = i;
        drag_angle = -1;
        return true;
      }
      if (DistancePointSegment(m, p, h) <= angle_pick_distance) {
        drag_angle = i;
        drag_point = -1;
        return true;
      }
    }
  }

  if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    if (drag_point >= 0) {
      user_point[drag_point].x = m.x;
      user_point[drag_point].y = m.y;
      return true;
    }

    if (drag_angle >= 0) {
      Vector2 p = {user_point[drag_angle].x, user_point[drag_angle].y};
      user_point[drag_angle].theta = atan2f(m.y - p.y, m.x - p.x);
      return true;
    }
  }

  if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    drag_point = -1;
    drag_angle = -1;
  }

  return false;
}
void user_points_fit() {
  for (int i = 0; i < 3; i++) {
    user_point[i].x =
        Clamp(user_point[i].x, handle_length, GetScreenWidth() - handle_length);
    user_point[i].y = Clamp(user_point[i].y, handle_length,
                            GetScreenHeight() - handle_length);
  }
}

void user_points_rand() {
  double xj = 0, yj = 0;
  for (int i = 0; i < 3; i++) {
    user_point[i].x =
        ((float)rand() / RAND_MAX) * (GetScreenWidth() - handle_length) +
        handle_length;
    user_point[i].y =
        ((float)rand() / RAND_MAX) * (GetScreenHeight() - handle_length) +
        handle_length;

    xj += user_point[i].x;
    yj += user_point[i].y;
  }
  xj /= 3;
  yj /= 3;
  for (int i = 0; i < 3; i++) {
    user_point[i].theta = atan2(yj - user_point[i].y, xj - user_point[i].x);
  }
}

void draw_spline_points_dense() {
  const int nseg = 100; // number of samples along each spline
  double tmin = s.ts[0];
  double tmax = s.ts[N - 1];
  init_splines();
  for (int i = 0; i < nseg; i++) {
    double t = tmin + (tmax - tmin) * (double)i / (nseg - 1);
    // clang-format off
    double x1 = gsl_spline_eval(s.sx1, t, s.acc);
    double y1 = gsl_spline_eval(s.sy1, t, s.acc);
    double x2 = gsl_spline_eval(s.sx2, t, s.acc);
    double y2 = gsl_spline_eval(s.sy2, t, s.acc);
    double x12 = gsl_spline_eval(s.sx12, t, s.acc);
    double y12 = gsl_spline_eval(s.sy12, t, s.acc);
    // clang-format on
    DrawCircle(x1, y1, 2.0f, DARKGRAY);
    DrawCircle(x2, y2, 2.0f, DARKGRAY);
    DrawCircle(x12, y12, 2.0f, DARKGRAY);
  }
}

void draw_spline_lines() {
  const int nseg = 100; // number of samples along each spline
  double tmin = s.ts[0];
  double tmax = s.ts[N - 1];
  init_splines();
  double px[3][2], py[3][2];
  for (int i = 0; i < nseg; i++) {
    double t = tmin + (tmax - tmin) * (double)i / (nseg - 1);
    // clang-format off
    px[0][i % 2] = gsl_spline_eval(s.sx1, t, s.acc);
    py[0][i % 2] = gsl_spline_eval(s.sy1, t, s.acc);
    px[1][i % 2] = gsl_spline_eval(s.sx2, t, s.acc);
    py[1][i % 2] = gsl_spline_eval(s.sy2, t, s.acc);
    px[2][i % 2] = gsl_spline_eval(s.sx12, t, s.acc);
    py[2][i % 2] = gsl_spline_eval(s.sy12, t, s.acc);

    if (i > 0) for (int j=0; j < 3 ; j++) DrawLine(px[j][0], py[j][0], px[j][1], py[j][1], DARKGRAY);
    // clang-format on
  }
}

void gui_sliders() {
  static bool gui_inited = false;
  static float slider_logEI0, slider_logEI1, slider_logEIgm, slider_w1,
      slider_kappa_h, slider_lambda_f;
  static float prev_logEI0, prev_logEI1, prev_logEIgm, prev_w1, prev_kappa_h,
      prev_lambda_f;
  if (!gui_inited) {
    slider_logEI0 = log10f((float)params.EI[0]);
    slider_logEI1 = log10f((float)params.EI[1]);
    slider_logEIgm = 0.5f * (slider_logEI0 + slider_logEI1);
    slider_w1 = (float)params.w1;
    slider_kappa_h = (float)params.kappa_h;
    slider_lambda_f = (float)gui_lambda;
    prev_logEI0 = slider_logEI0;
    prev_logEI1 = slider_logEI1;
    prev_logEIgm = slider_logEIgm;
    prev_w1 = slider_w1;
    prev_kappa_h = slider_kappa_h;
    prev_lambda_f = slider_lambda_f;
    gui_inited = true;
  }

  char value_text[6][32];
  sprintf(value_text[0], "%g", params.EI[0]);
  sprintf(value_text[1], "%g", params.EI[1]);
  sprintf(value_text[2], "%g", sqrt(params.EI[0] * params.EI[1]));
  sprintf(value_text[3], "%g", params.w1);
  sprintf(value_text[4], "%g", params.kappa_h);
  sprintf(value_text[5], "%g", gui_lambda);

  float padding = 10;
  float y = 10;
  float h = 20;
  float row_gap = 4;
  float label_w = 120;
  float value_w = 60;
  float gap = 4;
  bool single_col = GetScreenWidth() < 400;

  if (single_col) {
    float col_x = padding;
    float col_w = GetScreenWidth() - 2 * padding;
    float slider_w = col_w - label_w - value_w - 2 * gap;

    GuiLabel((Rectangle){col_x, y, label_w, h}, "log10 EI[0]");
    GuiSlider((Rectangle){col_x + label_w + gap, y, slider_w, h}, "", "",
              &slider_logEI0, -6, 6);
    GuiLabel((Rectangle){col_x + label_w + gap + slider_w + gap, y, value_w, h},
             value_text[0]);
    y += h + row_gap;

    GuiLabel((Rectangle){col_x, y, label_w, h}, "log10 EI[1]");
    GuiSlider((Rectangle){col_x + label_w + gap, y, slider_w, h}, "", "",
              &slider_logEI1, -6, 6);
    GuiLabel((Rectangle){col_x + label_w + gap + slider_w + gap, y, value_w, h},
             value_text[1]);
    y += h + row_gap;

    GuiLabel((Rectangle){col_x, y, label_w, h}, "log10 EI[0] + EI[1]");
    GuiSlider((Rectangle){col_x + label_w + gap, y, slider_w, h}, "", "",
              &slider_logEIgm, -6, 6);
    GuiLabel((Rectangle){col_x + label_w + gap + slider_w + gap, y, value_w, h},
             value_text[2]);
    y += h + row_gap;

    GuiLabel((Rectangle){col_x, y, label_w, h}, "w");
    GuiSlider((Rectangle){col_x + label_w + gap, y, slider_w, h}, "", "",
              &slider_w1, 0.0f, 100.0f);
    GuiLabel((Rectangle){col_x + label_w + gap + slider_w + gap, y, value_w, h},
             value_text[3]);
    y += h + row_gap;

    GuiLabel((Rectangle){col_x, y, label_w, h}, "kappa_h");
    GuiSlider((Rectangle){col_x + label_w + gap, y, slider_w, h}, "", "",
              &slider_kappa_h, 1e-6f, 1.0f);
    GuiLabel((Rectangle){col_x + label_w + gap + slider_w + gap, y, value_w, h},
             value_text[4]);
    y += h + row_gap;

    GuiLabel((Rectangle){col_x, y, label_w, h}, "lambda");
    GuiSlider((Rectangle){col_x + label_w + gap, y, slider_w, h}, "", "",
              &slider_lambda_f, 0.0f, 1e4f);
    GuiLabel((Rectangle){col_x + label_w + gap + slider_w + gap, y, value_w, h},
             value_text[5]);
  } else {
    float col_w = (GetScreenWidth() - 3 * padding) / 2;
    float x0 = padding;
    float x1 = padding * 2 + col_w;
    float slider_w = col_w - label_w - value_w - 2 * gap;

    GuiLabel((Rectangle){x0, y, label_w, h}, "log10 EI[0]");
    GuiSlider((Rectangle){x0 + label_w + gap, y, slider_w, h}, "", "",
              &slider_logEI0, -6, 6);
    GuiLabel((Rectangle){x0 + label_w + gap + slider_w + gap, y, value_w, h},
             value_text[0]);
    GuiLabel((Rectangle){x1, y, label_w, h}, "log10 EI[1]");
    GuiSlider((Rectangle){x1 + label_w + gap, y, slider_w, h}, "", "",
              &slider_logEI1, -6, 6);
    GuiLabel((Rectangle){x1 + label_w + gap + slider_w + gap, y, value_w, h},
             value_text[1]);
    y += h + row_gap;

    GuiLabel((Rectangle){x0, y, label_w, h}, "log10 EI[0] + EI[1]");
    GuiSlider((Rectangle){x0 + label_w + gap, y, slider_w, h}, "", "",
              &slider_logEIgm, -6, 6);
    GuiLabel((Rectangle){x0 + label_w + gap + slider_w + gap, y, value_w, h},
             value_text[2]);
    GuiLabel((Rectangle){x1, y, label_w, h}, "w");
    GuiSlider((Rectangle){x1 + label_w + gap, y, slider_w, h}, "", "",
              &slider_w1, 0.0f, 100.0f);
    GuiLabel((Rectangle){x1 + label_w + gap + slider_w + gap, y, value_w, h},
             value_text[3]);
    y += h + row_gap;

    GuiLabel((Rectangle){x0, y, label_w, h}, "kappa_h");
    GuiSlider((Rectangle){x0 + label_w + gap, y, slider_w, h}, "", "",
              &slider_kappa_h, 1e-6f, 1.0f);
    GuiLabel((Rectangle){x0 + label_w + gap + slider_w + gap, y, value_w, h},
             value_text[4]);
    GuiLabel((Rectangle){x1, y, label_w, h}, "lambda");
    GuiSlider((Rectangle){x1 + label_w + gap, y, slider_w, h}, "", "",
              &slider_lambda_f, 0.0f, 1e4f);
    GuiLabel((Rectangle){x1 + label_w + gap + slider_w + gap, y, value_w, h},
             value_text[5]);
  }

  bool changed = false;
  if (fabsf(slider_logEI0 - prev_logEI0) > 1e-6f) {
    params.EI[0] = pow(10.0, slider_logEI0);
    changed = true;
  }
  if (fabsf(slider_logEI1 - prev_logEI1) > 1e-6f) {
    params.EI[1] = pow(10.0, slider_logEI1);
    changed = true;
  }
  if (fabsf(slider_logEIgm - prev_logEIgm) > 1e-6f) {
    double r = params.EI[0] / params.EI[1];
    if (r <= 0)
      r = 1.0;
    double new_gm = pow(10.0, slider_logEIgm);
    double sqrt_r = sqrt(r);
    params.EI[0] = new_gm * sqrt_r;
    params.EI[1] = new_gm / sqrt_r;
    changed = true;
  }
  if (fabsf(slider_w1 - prev_w1) > 1e-6f) {
    params.w1 = slider_w1;
    changed = true;
  }
  if (fabsf(slider_kappa_h - prev_kappa_h) > 1e-8f) {
    params.kappa_h = slider_kappa_h;
    changed = true;
  }
  if (fabsf(slider_lambda_f - prev_lambda_f) > 1e-6f) {
    gui_lambda = slider_lambda_f;
    changed = true;
  }

  if (changed) {
    slider_logEI0 = log10f((float)params.EI[0]);
    slider_logEI1 = log10f((float)params.EI[1]);
    slider_logEIgm = 0.5f * (slider_logEI0 + slider_logEI1);
    slider_w1 = (float)params.w1;
    slider_kappa_h = (float)params.kappa_h;
    slider_lambda_f = (float)gui_lambda;
    prev_logEI0 = slider_logEI0;
    prev_logEI1 = slider_logEI1;
    prev_logEIgm = slider_logEIgm;
    prev_w1 = slider_w1;
    prev_kappa_h = slider_kappa_h;
    prev_lambda_f = slider_lambda_f;
    init_opt();
  }
}

void UpdateDrawFrame() {
#if defined(PLATFORM_WEB)
  SetMouseScale((float)GetScreenWidth() / 640, (float)GetScreenHeight() / 480);
#endif

  if (update_user_points()) {
    // update spline control points from user inputs
    from_user();
    // s_lerp_centroid();
    init_opt();
  }

  // removed interactive junction rotation - tangents are enforced via penalty

  static bool play = true;
  if (IsKeyPressed(KEY_O)) {
    play = !play;
    init_opt();
  }
  if (play) {
    for (int i = 0; i < 5; i++)
      step_opt();
  }

  if (IsKeyPressed(KEY_SPACE)) {
    user_points_rand();
    from_user();
    s_lerp_centroid();
    init_opt();
  }

  BeginDrawing();

  ClearBackground(BLACK);

  // draw sampled points from each spline
  draw_spline_lines();
  draw_spline_points();

  // GUI sliders at top
  gui_sliders();

  Color col[] = {BLUE, YELLOW, GREEN};
  for (int i = 0; i < 3; i++) {
    Vector2 p = {user_point[i].x, user_point[i].y};
    Vector2 h = {p.x + (float)(handle_length * cos(user_point[i].theta)),
                 p.y + (float)(handle_length * sin(user_point[i].theta))};
    DrawLineV(p, h, col[i]);
    DrawCircleV(p, user_point_radius, col[i]);
  }
  EndDrawing();
}

int main() {
  printf("starting\n");
  for (int i = 0; i < N; i++)
    s.ts[i] = (double)i / (N - 1);
  srandom((unsigned)time(NULL));

  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  InitWindow(640, 480, "wye");
#if defined(PLATFORM_WEB)
  SetMouseScale((float)GetScreenWidth() / 640, (float)GetScreenHeight() / 480);
#endif
  user_points_rand();
  from_user();
  s_lerp_centroid();
  alloc_splines();
  alloc_opt();
  init_opt();
#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
  SetTargetFPS(20);
  while (!WindowShouldClose() && !IsKeyPressed(KEY_ESCAPE)) {
    UpdateDrawFrame();
  }
#endif

  free_splines();
  free_opt();
  CloseWindow();
  return 0;
}
