#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <windows.h>

#include <algorithm>

#pragma comment(lib, "xinput9_1_0")
#include <Xinput.h>

#define DOS_IMPLEMENTATION
#include "dos.h"

#include "maths.h" // instersection, geometric tests and conversion functions
#include "util.h"  // min, max, swap, smoothstep, scalar functions.. etc
#include "vec.h"   // vector of any dimension and type
#include "mat.h"   // matrix of any dimension and type
#include "quat.h"  // quaternion of any type

static int screenWidth = 320;
static int screenHeight = 200;
static uint8_t* screen;

/*
union v2f {
  struct {
    float x, y;
  };
  float v[2];
};

union v2i {
  struct {
    int x, y;
  };
  int v[2];
};

v2i operator+(const v2i& lhs, const v2i& rhs) {
  v2i r;
  for (int i = 0; i != 2; ++i)
    r.v[i] = lhs.v[i] + rhs.v[i];
  return r;
}
*/

typedef vec2f v2f;
typedef vec2i v2i;

void drawVLine(int x1, int x2, int y, uint8_t color) {
  int i = y * screenWidth;
  for (int x = x1; x <= x2; ++x) {
    screen[x + i] = color;
  }
}

// http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
void fillBottomFlatTriangle(int v1x, int v1y, int v2x, int v2y, int v3x,
                            uint8_t color) {
  float yinv = 1.0f / (v2y - v1y);
  float invslope1 = (v2x - v1x) * yinv;
  float invslope2 = (v3x - v1x) * yinv;

  float curx1 = v1x;
  float curx2 = v1x;

  for (int scanlineY = v1y; scanlineY <= v2y; scanlineY++) {
    drawVLine((int)curx1, (int)curx2, scanlineY, color);
    curx1 += invslope1;
    curx2 += invslope2;
  }
}

void fillBottomFlatTriangle(v2i v1, v2i v2, v2i v3, uint8_t color) {
  fillBottomFlatTriangle(v1.x, v1.y, v2.x, v2.y, v3.x, color);
}

void fillTopFlatTriangle(int v1x, int v1y, int v2x, int v3x, int v3y,
                         uint8_t color) {
  float yinv = 1.0f / (v3y - v1y);
  float invslope1 = (v3x - v1x) * yinv;
  float invslope2 = (v3x - v2x) * yinv;

  float curx1 = v3x;
  float curx2 = v3x;

  for (int scanlineY = v3y; scanlineY > v1y; scanlineY--) {
    drawVLine((int)curx1, (int)curx2, scanlineY, color);
    curx1 -= invslope1;
    curx2 -= invslope2;
  }
}

void fillTopFlatTriangle(v2i v1, v2i v2, v2i v3, uint8_t color) {
  fillTopFlatTriangle(v1.x, v1.y, v2.x, v3.x, v3.y, color);
}

void drawTriangle(v2i v1, v2i v2, v2i v3, uint8_t color) {
  /* at first sort the three vertices by y-coordinate ascending so v1 is the
   * topmost vertice */
  if (v1.y > v2.y) std::swap(v1, v2);
  if (v1.y > v3.y) std::swap(v1, v3);
  if (v2.y > v3.y) std::swap(v2, v3);

  /* here we know that v1.y <= v2.y <= v3.y */
  /* check for trivial case of bottom-flat triangle */
  if (v2.y == v3.y) {
    fillBottomFlatTriangle(v1, v2, v3, color);
  }
  /* check for trivial case of top-flat triangle */
  else if (v1.y == v2.y) {
    fillTopFlatTriangle(v1, v2, v3, color);
  } else {
    /* general case - split the triangle in a topflat and bottom-flat one */
    v2i v4;
    v4.x = (int)(v1.x + ((float)(v2.y - v1.y) / (float)(v3.y - v1.y)) * (v3.x - v1.x));
    v4.y = v2.y;
    fillTopFlatTriangle(v2, v4, v3, color);
    fillBottomFlatTriangle(v1, v2, v4, color);
  }
}

void drawTriangle(v2f v1, v2f v2, v2f v3, uint8_t color) {
  v2i a(v1.x, v1.y);
  v2i b(v2.x, v2.y);
  v2i c(v3.x, v3.y);

  drawTriangle(a, b, c, color);
}

int main(int argc, char* argv[]) {
  setvideomode(videomode_320x200);

  setdoublebuffer(1);
  screen = screenbuffer();

  screenWidth = 320;
  screenHeight = 200;

  
  v2f p1 = {-10, -10};
  v2f p2 = {-20, 10};
  v2f p3 = {10, 15};
  v2f p = {50, 50};

  while (!shuttingdown()) {
    waitvbl();
    clearscreen();

    XINPUT_STATE xstate;
    XInputGetState(0, &xstate);

    bool up = xstate.Gamepad.sThumbLY > 10000;
    bool down = xstate.Gamepad.sThumbLY < -10000;

    bool left = xstate.Gamepad.sThumbLX < -10000;
    bool right = xstate.Gamepad.sThumbLX > 10000;

    if (up) {
      p.y--;
    }

    if (down) {
      p.y++;
    }

    if (right) {
      p.x++;
    }

    if (left) {
      p.x--;
    }

    //rotate 
    { 
      mat4f rot = mat::create_z_rotation(xstate.Gamepad.sThumbRX * .000001f);

      v2f * arr[3] = {&p1,&p2,&p3};

      for (int i = 0; i != 3; ++i) {
        v2f * p = arr[i];
        vec4f a(p->x, p->y, 0, 1);
        a = rot * a;
        p->x = a.x;
        p->y = a.y;
      }

    }

    v2f a = p + p1;
    v2f b = p + p2;
    v2f c = p + p3;

    drawTriangle(a, b, c, 8);

    setcolor(12);
    outtextxy(10, 10, "UP/DOWN/LEFT/RIGHT - move/turn");
    outtextxy(10, 18, "R/F - change altitude");
    outtextxy(10, 26, "Q/W - change pitch");

    screen = swapbuffers();

    if (keystate(KEY_ESCAPE)) break;
  }

  return 0;
}
