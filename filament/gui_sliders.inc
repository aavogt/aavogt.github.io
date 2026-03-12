static float gui_layout_padding = 10.0f;
static float gui_layout_y = 10.0f;
static float gui_layout_h = 20.0f;
static float gui_layout_row_gap = 4.0f;
static float gui_layout_label_w = 120.0f;
static float gui_layout_value_w = 60.0f;
static float gui_layout_gap = 4.0f;
static float gui_layout_col_x = 0.0f;
static float gui_layout_col_w = 0.0f;
static float gui_layout_slider_w = 0.0f;
static float gui_layout_x0 = 0.0f;
static float gui_layout_x1 = 0.0f;
static bool gui_layout_single_col = false;
static int gui_layout_seq = 0;

// Reset layout state before drawing rows
void rect_reset() {
  gui_layout_padding = 10.0f;
  gui_layout_y = 10.0f;
  gui_layout_h = 20.0f;
  gui_layout_row_gap = 4.0f;
  gui_layout_label_w = 120.0f;
  gui_layout_value_w = 60.0f;
  gui_layout_gap = 4.0f;
  float screen_w = (float)GetScreenWidth();
  gui_layout_single_col = GetScreenWidth() < 400;
  if (gui_layout_single_col) {
    gui_layout_col_x = gui_layout_padding;
    gui_layout_col_w = screen_w - 2 * gui_layout_padding;
    gui_layout_slider_w = gui_layout_col_w - gui_layout_label_w - gui_layout_value_w - 2 * gui_layout_gap;
  } else {
    gui_layout_col_w = (screen_w - 3 * gui_layout_padding) / 2.0f;
    gui_layout_x0 = gui_layout_padding;
    gui_layout_x1 = gui_layout_padding * 2.0f + gui_layout_col_w;
    gui_layout_slider_w = gui_layout_col_w - gui_layout_label_w - gui_layout_value_w - 2 * gui_layout_gap;
  }
  gui_layout_seq = 0;
}

// Return the next label Rectangle (variable name) in the sequence used by the GUI layout.
Rectangle rect_lab1() {
  Rectangle r = {0, 0, 0, 0};
  if (gui_layout_single_col) {
    float x = gui_layout_col_x;
    float y = gui_layout_y;
    int idx = gui_layout_seq % 3; // label, slider, value
    if (idx == 0) {
      r.x = x;
      r.y = y;
      r.width = gui_layout_label_w;
      r.height = gui_layout_h;
    } else {
      // fallback: place at label position
      r.x = x;
      r.y = y;
      r.width = gui_layout_label_w;
      r.height = gui_layout_h;
    }
  } else {
    // two-column layout: sequence of 6 per row: left label, left slider, left value, right label, right slider, right value
    float y = gui_layout_y;
    int idx = gui_layout_seq % 6;
    if (idx == 0) {
      r.x = gui_layout_x0;
      r.y = y;
      r.width = gui_layout_label_w;
      r.height = gui_layout_h;
    } else if (idx == 3) {
      r.x = gui_layout_x1;
      r.y = y;
      r.width = gui_layout_label_w;
      r.height = gui_layout_h;
    } else {
      // fallback: choose left or right label based on position
      if (idx < 3) {
        r.x = gui_layout_x0;
      } else {
        r.x = gui_layout_x1;
      }
      r.y = y;
      r.width = gui_layout_label_w;
      r.height = gui_layout_h;
    }
  }
  gui_layout_seq++;
  return r;
}

// Return the next value Rectangle (variable value / value_text) in the sequence used by the GUI layout.
Rectangle rect_lab2() {
  Rectangle r = {0, 0, 0, 0};
  if (gui_layout_single_col) {
    float x = gui_layout_col_x;
    float y = gui_layout_y;
    int idx = gui_layout_seq % 3; // label, slider, value
    if (idx == 2) {
      r.x = x + gui_layout_label_w + gui_layout_gap + gui_layout_slider_w + gui_layout_gap;
      r.y = y;
      r.width = gui_layout_value_w;
      r.height = gui_layout_h;
      // advance to next row after value
      gui_layout_y += gui_layout_h + gui_layout_row_gap;
    } else {
      // fallback: place at value position
      r.x = x + gui_layout_label_w + gui_layout_gap + gui_layout_slider_w + gui_layout_gap;
      r.y = y;
      r.width = gui_layout_value_w;
      r.height = gui_layout_h;
    }
  } else {
    // two-column layout: sequence of 6 per row: left label, left slider, left value, right label, right slider, right value
    float y = gui_layout_y;
    int idx = gui_layout_seq % 6;
    if (idx == 2) {
      r.x = gui_layout_x0 + gui_layout_label_w + gui_layout_gap + gui_layout_slider_w + gui_layout_gap;
      r.y = y;
      r.width = gui_layout_value_w;
      r.height = gui_layout_h;
    } else {
      r.x = gui_layout_x1 + gui_layout_label_w + gui_layout_gap + gui_layout_slider_w + gui_layout_gap;
      r.y = y;
      r.width = gui_layout_value_w;
      r.height = gui_layout_h;
      // advance to next row after right value
      gui_layout_y += gui_layout_h + gui_layout_row_gap;
    }
  }
  gui_layout_seq++;
  return r;
}


// Return the next slider Rectangle in the sequence used by the GUI layout.
Rectangle rect_bar() {
  Rectangle r = {0, 0, 0, 0};
  if (gui_layout_single_col) {
    float x = gui_layout_col_x;
    float y = gui_layout_y;
    r.x = x + gui_layout_label_w + gui_layout_gap;
    r.y = y;
    r.width = gui_layout_slider_w;
    r.height = gui_layout_h;
  } else {
    // two-column layout: sequence of 6 per row: left label, left slider, left value, right label, right slider, right value
    float y = gui_layout_y;
    int idx = gui_layout_seq % 6;
    if (idx == 1) {
      r.x = gui_layout_x0 + gui_layout_label_w + gui_layout_gap;
      r.y = y;
      r.width = gui_layout_slider_w;
      r.height = gui_layout_h;
    } else {
      r.x = gui_layout_x1 + gui_layout_label_w + gui_layout_gap;
      r.y = y;
      r.width = gui_layout_slider_w;
      r.height = gui_layout_h;
    }
  }
  gui_layout_seq++;
  return r;
}


void gui_sliders() {
  static bool gui_inited = false;
  static float slider_logEI0, slider_logEI1, slider_logEIgm, slider_w1;
  static float prev_logEI0, prev_logEI1, prev_logEIgm, prev_w1, prev_kappa_h;
  if (!gui_inited) {
    slider_logEI0 = log10f((float)params.EI[0]);
    slider_logEI1 = log10f((float)params.EI[1]);
    slider_logEIgm = 0.5f * (slider_logEI0 + slider_logEI1);
    slider_w1 = (float)params.w1;
    prev_logEI0 = slider_logEI0;
    prev_logEI1 = slider_logEI1;
    prev_logEIgm = slider_logEIgm;
    prev_w1 = slider_w1;
    gui_inited = true;
  }

  char value_text[6][32];
  sprintf(value_text[0], "%g", params.EI[0]);
  sprintf(value_text[1], "%g", params.EI[1]);
  sprintf(value_text[2], "%g", sqrt(params.EI[0] * params.EI[1]));
  sprintf(value_text[3], "%g", params.w1);
  sprintf(value_text[4], "%g", params.kappa_h);
  sprintf(value_text[5], "%g", params.gui_lambda);

  // initialize layout state
  rect_reset();

  // Draw controls. Use rect_lab() for labels/values and rect_bar() for sliders.
  // For sliders that write directly into params, supply &params.foo. For others use the local slider_* variables.
  bool changed = false;

  // log10 EI[0]
  GuiLabel(rect_lab1(), "log10 EI[0]");
  if (GuiSlider(rect_bar(), "", "", &slider_logEI0, -6, 6)) {
    params.EI[0] = pow(10.0, slider_logEI0);
    changed = true;
  }
  GuiLabel(rect_lab2(), value_text[0]);

  // log10 EI[1]
  GuiLabel(rect_lab1(), "log10 EI[1]");
  if (GuiSlider(rect_bar(), "", "", &slider_logEI1, -6, 6)) {
    params.EI[1] = pow(10.0, slider_logEI1);
    changed = true;
  }
  GuiLabel(rect_lab2(), value_text[1]);

  // log10 EI[0] + EI[1]
  GuiLabel(rect_lab1(), "log10 EI[0] + EI[1]");
  if (GuiSlider(rect_bar(), "", "", &slider_logEIgm, -6, 6)) {
    double r = params.EI[0] / params.EI[1];
    if (r <= 0)
      r = 1.0;
    double new_gm = pow(10.0, slider_logEIgm);
    double sqrt_r = sqrt(r);
    params.EI[0] = new_gm * sqrt_r;
    params.EI[1] = new_gm / sqrt_r;
    changed = true;
  }
  GuiLabel(rect_lab2(), value_text[2]);

  // w (use params.w1 directly)
  GuiLabel(rect_lab1(), "w");
  if (GuiSlider(rect_bar(), "", "", &params.w1, 0.0f, 100.0f)) {
    changed = true;
  }
  GuiLabel(rect_lab2(), value_text[3]);

  // kappa_h (use params.kappa_h directly)
  GuiLabel(rect_lab1(), "kappa_h");
  if (GuiSlider(rect_bar(), "", "", &params.kappa_h, 1e-6f, 1.0f)) {
    changed = true;
  }
  GuiLabel(rect_lab2(), value_text[4]);

  // lambda (use params.gui_lambda directly)
  GuiLabel(rect_lab1(), "lambda");
  if (GuiSlider(rect_bar(), "", "", &params.gui_lambda, 0.0f, 1e4f)) {
    changed = true;
  }
  GuiLabel(rect_lab2(), value_text[5]);


  if (changed) {
    // Re-sync local slider copies from params after changes and re-init optimisation
    slider_logEI0 = log10f((float)params.EI[0]);
    slider_logEI1 = log10f((float)params.EI[1]);
    slider_logEIgm = 0.5f * (slider_logEI0 + slider_logEI1);
    slider_w1 = (float)params.w1;
    prev_logEI0 = slider_logEI0;
    prev_logEI1 = slider_logEI1;
    prev_logEIgm = slider_logEIgm;
    prev_w1 = slider_w1;
    init_opt();
  }
}

