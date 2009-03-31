#include <Gled/Gled.h>
#include <GledView/GledViewNS.h>

#include <FL/Fl.h>
#include <FL/fl_draw.h>

namespace {

  void menubar_box_draw(int x, int y, int w, int h, Fl_Color c) {
    int yp = y + h - 6;
    fl_color(c);
    fl_rectf(x, y, w, h - 6);
    fl_color(FL_DARK3);
    fl_xyline(x, yp, x+w);
    fl_xyline(x, yp+3, x+w);
    fl_color(FL_WHITE);
    fl_xyline(x, yp+1, x+w);
    fl_xyline(x, yp+2, x+w);
    fl_color(FL_BACKGROUND_COLOR);
    fl_rectf(x, yp+4, w, 2);
  }

}

/**************************************************************************/

void libGledCore_GLED_user_init_View()
{
  // Box types.
  GledViewNS::menubar_box = FL_FREE_BOXTYPE;
  Fl::set_boxtype(FL_FREE_BOXTYPE, menubar_box_draw, 1, 1, 2, 7);
}

void *GledCore_GLED_user_init_View = (void*)libGledCore_GLED_user_init_View;
