// Minimal GLU replacement for GLES/gl4es targets (Trimui port)
#ifndef GLU_COMPAT_H
#define GLU_COMPAT_H
#include <GL/gl.h>

void gluPerspective(double fovy, double aspect, double zNear, double zFar);
void gluLookAt(double ex, double ey, double ez, double cx, double cy, double cz,
               double ux, double uy, double uz);
const unsigned char* gluErrorString(GLenum err);
void etrDrawUnitSphere(int slices, int stacks);
#endif
