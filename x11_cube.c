// Compile: gcc x11_cube.c -o x11_cube -lX11 -lGL -lGLU -lm

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

Display *dpy;
Window win;
XWindowAttributes winattr;

void DrawCube(void) {
    float verts[] = {
        0.5f,  0.5f,  0.5f,
       -0.5f,  0.5f,  0.5f,
       -0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
       -0.5f,  0.5f, -0.5f,
       -0.5f, -0.5f, -0.5f,
    };

    GLubyte indices[] = {
        0,1,2,3, // Front
        0,3,4,5, // Right
        5,4,7,6, // Back
        1,6,7,2, // Left
        0,5,6,1, // Top
        3,2,7,4  // Bottom
    };

    GLubyte colors[] = {
        255,0,0,
        0,255,0,
        0,0,255,
        255,255,0,
        0,255,255,
        255,0,255,
        255,255,255,
        0,0,0,
    };

    static int angle = 0;

    XGetWindowAttributes(dpy, win, &winattr);
    glViewport(0, 0, winattr.width, winattr.height);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)winattr.width / winattr.height, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -3.0f);
    glRotatef(angle, 1.0f, 1.0f, 0.0f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, verts);
    glColorPointer(3, GL_UNSIGNED_BYTE, 0, colors);
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, indices);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glXSwapBuffers(dpy, win);

    angle += 1;
    usleep(16000); // ~60 FPS
}

int main() {
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Failed to open display\n");
        exit(1);
    }

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask;

    win = XCreateWindow(dpy, root, 100, 100, 800, 600, 0,
                        vi->depth, InputOutput, vi->visual,
                        CWColormap | CWEventMask, &swa);

    XMapWindow(dpy, win);
    XStoreName(dpy, win, "Raw X11 + GLX Cube");

    GLXContext glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);

    while (1) {
        while (XPending(dpy)) {
            XEvent e;
            XNextEvent(dpy, &e);
            if (e.type == KeyPress)
                exit(0);
        }

        DrawCube();
    }

    return 0;
}
