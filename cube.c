// Compile with:
// gcc cube.c -o cube `pkg-config --cflags --libs gtk+-3.0 epoxy` -lGLU -lm

#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <GL/glu.h>
#include <math.h>

static float rotation_x = 0.0f;
static float rotation_y = 0.0f;
static gboolean is_rotating = FALSE;
static gfloat last_x = 0.0f;
static gfloat last_y = 0.0f;

// Render callback
static gboolean on_render(GtkGLArea *area, GdkGLContext *context, gpointer data) {
    int width = gtk_widget_get_allocated_width(GTK_WIDGET(area));
    int height = gtk_widget_get_allocated_height(GTK_WIDGET(area));
    float aspect = (float)width / (float)height;

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.0, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set perspective
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, aspect, 0.1, 100.0);

    // Set model view
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(rotation_x, 1.0f, 0.0f, 0.0f);
    glRotatef(rotation_y, 0.0f, 1.0f, 0.0f);

    // Draw cube
    glBegin(GL_QUADS);
    
    // Front
    glColor3f(1, 0, 0);
    glVertex3f(-1, -1,  1);
    glVertex3f( 1, -1,  1);
    glVertex3f( 1,  1,  1);
    glVertex3f(-1,  1,  1);

    // Back
    glColor3f(0, 1, 0);
    glVertex3f(-1, -1, -1);
    glVertex3f(-1,  1, -1);
    glVertex3f( 1,  1, -1);
    glVertex3f( 1, -1, -1);

    // Left
    glColor3f(0, 0, 1);
    glVertex3f(-1, -1, -1);
    glVertex3f(-1, -1,  1);
    glVertex3f(-1,  1,  1);
    glVertex3f(-1,  1, -1);

    // Right
    glColor3f(1, 1, 0);
    glVertex3f(1, -1, -1);
    glVertex3f(1,  1, -1);
    glVertex3f(1,  1,  1);
    glVertex3f(1, -1,  1);

    // Top
    glColor3f(0, 1, 1);
    glVertex3f(-1, 1, -1);
    glVertex3f(-1, 1,  1);
    glVertex3f( 1, 1,  1);
    glVertex3f( 1, 1, -1);

    // Bottom
    glColor3f(1, 0, 1);
    glVertex3f(-1, -1, -1);
    glVertex3f( 1, -1, -1);
    glVertex3f( 1, -1,  1);
    glVertex3f(-1, -1,  1);

    glEnd();

    return TRUE;
}

// Timer to update rotation
static gboolean on_tick(gpointer data) {
    if (!is_rotating) {
        rotation_y += 0.5f;
    }
    gtk_gl_area_queue_render(GTK_GL_AREA(data));
    return G_SOURCE_CONTINUE;
}

// Mouse event handlers
static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    if (event->button == GDK_BUTTON_PRIMARY) {
        is_rotating = TRUE;
        last_x = event->x;
        last_y = event->y;
    }
    return TRUE;
}

static gboolean on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    if (event->button == GDK_BUTTON_PRIMARY) {
        is_rotating = FALSE;
    }
    return TRUE;
}

static gboolean on_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer data) {
    if (is_rotating) {
        gfloat dx = event->x - last_x;
        gfloat dy = event->y - last_y;
        
        rotation_y += dx * 0.5f;
        rotation_x += dy * 0.5f;
        
        last_x = event->x;
        last_y = event->y;
        
        gtk_gl_area_queue_render(GTK_GL_AREA(data));
    }
    return TRUE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Spinning Cube");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *gl_area = gtk_gl_area_new();
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(gl_area), TRUE);
    gtk_widget_set_events(gl_area, gtk_widget_get_events(gl_area) | 
                         GDK_BUTTON_PRESS_MASK | 
                         GDK_BUTTON_RELEASE_MASK | 
                         GDK_POINTER_MOTION_MASK);
    
    g_signal_connect(gl_area, "render", G_CALLBACK(on_render), NULL);
    g_signal_connect(gl_area, "button-press-event", G_CALLBACK(on_button_press), gl_area);
    g_signal_connect(gl_area, "button-release-event", G_CALLBACK(on_button_release), gl_area);
    g_signal_connect(gl_area, "motion-notify-event", G_CALLBACK(on_motion_notify), gl_area);
    
    gtk_container_add(GTK_CONTAINER(window), gl_area);

    gtk_widget_show_all(window);

    g_timeout_add(16, on_tick, gl_area);  // ~60 FPS

    gtk_main();
    return 0;
}