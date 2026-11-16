// Minimal GLU replacement for GLES/gl4es targets (Trimui port)
#include "glu_compat.h"
#include <cmath>
#include <vector>

void gluPerspective(double fovy, double aspect, double zNear, double zFar) {
	double f = 1.0 / tan(fovy * M_PI / 360.0);
	GLdouble m[16] = {0};
	m[0] = f / aspect;
	m[5] = f;
	m[10] = (zFar + zNear) / (zNear - zFar);
	m[11] = -1.0;
	m[14] = (2.0 * zFar * zNear) / (zNear - zFar);
	glMultMatrixd(m);
}

static void norm3(double v[3]) {
	double l = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	if (l > 0) { v[0] /= l; v[1] /= l; v[2] /= l; }
}

static void cross3(const double a[3], const double b[3], double r[3]) {
	r[0] = a[1]*b[2] - a[2]*b[1];
	r[1] = a[2]*b[0] - a[0]*b[2];
	r[2] = a[0]*b[1] - a[1]*b[0];
}

void gluLookAt(double ex, double ey, double ez, double cx, double cy, double cz,
               double ux, double uy, double uz) {
	double f[3] = {cx - ex, cy - ey, cz - ez};
	double up[3] = {ux, uy, uz};
	double s[3], u[3];
	norm3(f);
	cross3(f, up, s);
	norm3(s);
	cross3(s, f, u);
	GLdouble m[16] = {
		s[0], u[0], -f[0], 0,
		s[1], u[1], -f[1], 0,
		s[2], u[2], -f[2], 0,
		0, 0, 0, 1
	};
	glMultMatrixd(m);
	glTranslated(-ex, -ey, -ez);
}

const unsigned char* gluErrorString(GLenum err) {
	switch (err) {
		case GL_NO_ERROR: return (const unsigned char*)"no error";
		case GL_INVALID_ENUM: return (const unsigned char*)"invalid enumerant";
		case GL_INVALID_VALUE: return (const unsigned char*)"invalid value";
		case GL_INVALID_OPERATION: return (const unsigned char*)"invalid operation";
		case GL_STACK_OVERFLOW: return (const unsigned char*)"stack overflow";
		case GL_STACK_UNDERFLOW: return (const unsigned char*)"stack underflow";
		case GL_OUT_OF_MEMORY: return (const unsigned char*)"out of memory";
		default: return (const unsigned char*)"unknown error";
	}
}

// Same orientation as gluSphere(radius 1): poles on the Z axis, outside normals.
void etrDrawUnitSphere(int slices, int stacks) {
	if (slices < 3) slices = 3;
	if (stacks < 2) stacks = 2;
	std::vector<GLfloat> v;
	v.reserve((slices + 1) * 2 * 3);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	for (int i = 0; i < stacks; i++) {
		double rho0 = M_PI * i / stacks;
		double rho1 = M_PI * (i + 1) / stacks;
		v.clear();
		for (int j = 0; j <= slices; j++) {
			double theta = (j == slices) ? 0.0 : 2.0 * M_PI * j / slices;
			double st = -sin(theta), ct = cos(theta);
			v.push_back(st * sin(rho0)); v.push_back(ct * sin(rho0)); v.push_back(cos(rho0));
			v.push_back(st * sin(rho1)); v.push_back(ct * sin(rho1)); v.push_back(cos(rho1));
		}
		glVertexPointer(3, GL_FLOAT, 0, &v[0]);
		glNormalPointer(GL_FLOAT, 0, &v[0]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)(v.size() / 3));
	}
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}
