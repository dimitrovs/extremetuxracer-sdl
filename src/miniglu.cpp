/* Minimal GLU subset for Extreme Tux Racer on SDL+gl4es (no X11/GLX). */
#include <GL/gl.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

extern "C" {

const GLubyte* gluErrorString(GLenum error)
{
    switch (error) {
        case GL_NO_ERROR:          return (const GLubyte*)"no error";
        case GL_INVALID_ENUM:      return (const GLubyte*)"invalid enum";
        case GL_INVALID_VALUE:     return (const GLubyte*)"invalid value";
        case GL_INVALID_OPERATION: return (const GLubyte*)"invalid operation";
        case GL_STACK_OVERFLOW:    return (const GLubyte*)"stack overflow";
        case GL_STACK_UNDERFLOW:   return (const GLubyte*)"stack underflow";
        case GL_OUT_OF_MEMORY:     return (const GLubyte*)"out of memory";
        default:                   return (const GLubyte*)"unknown error";
    }
}

void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
    const GLdouble fH = tan(fovy * (3.14159265358979323846 / 360.0)) * zNear;
    const GLdouble fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}

void gluLookAt(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ,
               GLdouble centerX, GLdouble centerY, GLdouble centerZ,
               GLdouble upX, GLdouble upY, GLdouble upZ)
{
    GLdouble F[3] = { centerX - eyeX, centerY - eyeY, centerZ - eyeZ };
    GLdouble f_len = sqrt(F[0]*F[0] + F[1]*F[1] + F[2]*F[2]); if (f_len == 0.0) f_len = 1.0;
    F[0]/=f_len; F[1]/=f_len; F[2]/=f_len;

    GLdouble UP[3] = { upX, upY, upZ };
    GLdouble up_len = sqrt(UP[0]*UP[0] + UP[1]*UP[1] + UP[2]*UP[2]); if (up_len == 0.0) up_len = 1.0;
    UP[0]/=up_len; UP[1]/=up_len; UP[2]/=up_len;

    GLdouble S[3] = { F[1]*UP[2] - F[2]*UP[1],
                      F[2]*UP[0] - F[0]*UP[2],
                      F[0]*UP[1] - F[1]*UP[0] };
    GLdouble s_len = sqrt(S[0]*S[0] + S[1]*S[1] + S[2]*S[2]); if (s_len == 0.0) s_len = 1.0;
    S[0]/=s_len; S[1]/=s_len; S[2]/=s_len;

    GLdouble U[3] = { S[1]*F[2] - S[2]*F[1],
                      S[2]*F[0] - S[0]*F[2],
                      S[0]*F[1] - S[1]*F[0] };

    const GLfloat M[16] = {
        (GLfloat) S[0], (GLfloat) U[0], (GLfloat)-F[0], 0.0f,
        (GLfloat) S[1], (GLfloat) U[1], (GLfloat)-F[1], 0.0f,
        (GLfloat) S[2], (GLfloat) U[2], (GLfloat)-F[2], 0.0f,
        0.0f,           0.0f,           0.0f,           1.0f
    };
    glMultMatrixf(M);
    glTranslatef((GLfloat)-eyeX, (GLfloat)-eyeY, (GLfloat)-eyeZ);
}

int gluBuild2DMipmaps(GLenum target, GLint internalFormat,
                      GLsizei width, GLsizei height, GLenum format,
                      GLenum type, const void* data)
{
    glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, data);
    return 0;
}

/*** Minimal quadric emulation ***/
struct MiniQuadric { GLint drawStyle, orientation, normals; };

void* gluNewQuadric(void)
{
    MiniQuadric* q = (MiniQuadric*)malloc(sizeof(MiniQuadric));
    if (q) { q->drawStyle=0; q->orientation=0; q->normals=0; }
    return q;
}
void gluQuadricDrawStyle(void*, GLenum) {}
void gluQuadricOrientation(void*, GLenum) {}
void gluQuadricNormals(void*, GLenum) {}
void gluDeleteQuadric(void* q) { if (q) free(q); }

void gluSphere(void*, GLdouble radius, GLint slices, GLint stacks)
{
    for (int i = 0; i < stacks; ++i) {
        GLdouble lat0 = M_PI * (-0.5 + (double)i / stacks);
        GLdouble z0  = sin(lat0), zr0 = cos(lat0);
        GLdouble lat1 = M_PI * (-0.5 + (double)(i+1) / stacks);
        GLdouble z1  = sin(lat1), zr1 = cos(lat1);

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; ++j) {
            GLdouble lng = 2 * M_PI * (double)j / slices;
            GLdouble x = cos(lng), y = sin(lng);

            glNormal3f((GLfloat)(x*zr0), (GLfloat)(y*zr0), (GLfloat)z0);
            glVertex3f((GLfloat)(radius*x*zr0), (GLfloat)(radius*y*zr0), (GLfloat)(radius*z0));

            glNormal3f((GLfloat)(x*zr1), (GLfloat)(y*zr1), (GLfloat)z1);
            glVertex3f((GLfloat)(radius*x*zr1), (GLfloat)(radius*y*zr1), (GLfloat)(radius*z1));
        }
        glEnd();
    }
}

} // extern "C"
