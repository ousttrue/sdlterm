#include "vtermapp.h"
#include "TERM_Rect.h"
#include "sdlrenderer.h"
#include "vterm.h"
#include <iostream>

int damage(VTermRect rect, void *user) {
  // printf("damage: [%d, %d, %d, %d]\n", rect.start_col,
  //			rect.start_row, rect.end_col, rect.end_row);
  return 1;
}

int moverect(VTermRect dest, VTermRect src, void *user) { return 1; }

int movecursor(VTermPos pos, VTermPos oldpos, int visible, void *user) {
  auto state = (VTermApp *)user;
  state->MoveCursorCallback(pos.row, pos.col, visible);
  return 1;
}

int settermprop(VTermProp prop, VTermValue *val, void *user) { return 1; }

int bell(void *user) {
  auto state = (VTermApp *)user;
  state->BellCallback();
  return 1;
}

int sb_pushline(int cols, const VTermScreenCell *cells, void *user) {
  return 1;
}

int sb_popline(int cols, VTermScreenCell *cells, void *user) { return 1; }

VTermScreenCallbacks callbacks = {
    .damage = damage,
    .movecursor = movecursor,
    .bell = bell,
    .sb_pushline = sb_pushline,
};

VTermApp::~VTermApp() {
  std::cout << "VTermApp::~VTermApp\n";
  vterm_free(this->vterm);
}

void VTermApp::Initialize(int row, int col) {
  vterm = vterm_new(row, col);
  vterm_set_utf8(this->vterm, 1);
  this->screen = vterm_obtain_screen(this->vterm);
  vterm_screen_reset(this->screen, 1);
  this->termstate = vterm_obtain_state(this->vterm);
  vterm_screen_set_callbacks(this->screen, &callbacks, this);
}

void VTermApp::Write(const char *bytes, size_t len) {
  vterm_input_write(vterm, bytes, len);
}

size_t VTermApp::GetText(char *buffer, size_t len, const TERM_Rect &rect) {
  return vterm_screen_get_text(this->screen, buffer, len,
                               {
                                   .start_row = rect.start_row,
                                   .end_row = rect.end_row,
                                   .start_col = rect.start_col,
                                   .end_col = rect.end_col,
                               });
}

const VTermScreenCell *VTermApp::Cell(const VTermPos &pos) {
  if (vterm_screen_get_cell(screen, pos, &cell)) {
    vterm_state_convert_color_to_rgb(termstate, &cell.fg);
    vterm_state_convert_color_to_rgb(termstate, &cell.bg);
    return &cell;
  } else {
    return nullptr;
  }
}

void VTermApp::Resize(int rows, int cols) { vterm_set_size(vterm, rows, cols); }
