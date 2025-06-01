// Compile with: gcc galaxy.c -o galaxy `pkg-config --cflags --libs gtk+-3.0 epoxy` -lm

#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <GL/glu.h>


#define NUM_STARS 1000

typedef struct {
    float x, y, z;
    float r, g, b;
} Star;

static Star stars[NUM_STARS];
static float rotation = 0.0f;

// Initialize stars in a spiral galaxy formation
void init_galaxy() {
    srand(time(NULL));
    for (int i = 0; i < NUM_STARS; i++) {
        float angle = ((float) rand() / RAND_MAX) * 6.28f * 3; // up to 3 full spirals
        float radius = ((float) rand() / RAND_MAX) * 2.0f;
        float arm_offset = ((float) rand() / RAND_MAX - 0.5f) * 0.2f;

        float spiral_x = cos(angle) * radius + arm_offset;
        float spiral_y = sin(angle) * radius + arm_offset;
        float z = ((float) rand() / RAND_MAX - 0.5f) * 0.2f;

        stars[i].x = spiral_x;
        stars[i].y = spiral_y;
        stars[i].z = z;

        // Color: mostly white with slight tints
        stars[i].r = 0.8f + ((float) rand() / RAND_MAX) * 0.2f;
        stars[i].g = 0.8f + ((float) rand() / RAND_MAX) * 0.2f;
        stars[i].b = 0.8f + ((float) rand() / RAND_MAX) * 0.2f;
    }
}

static gboolean render(GtkGLArea *area, GdkGLContext *context, gpointer user_data) {
    int width = gtk_widget_get_allocated_width(GTK_WIDGET(area));
    int height = gtk_widget_get_allocated_height(GTK_WIDGET(area));
    glViewport(0, 0, width, height);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glPointSize(2.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2, 2, -2, 2, 1.0, 10.0);  // flat orthographic view for now

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -3.0f);  // move camera back

    glBegin(GL_POINTS);
    for (int i = 0; i < NUM_STARS; i++) {
        glColor3f(stars[i].r, stars[i].g, stars[i].b);
        glVertex3f(stars[i].x, stars[i].y, stars[i].z);
    }
    glEnd();

    return TRUE;
}


static gboolean tick(gpointer data) {
    GtkGLArea *area = GTK_GL_AREA(data);
    rotation += 0.2f;
    gtk_gl_area_queue_render(area);
    return G_SOURCE_CONTINUE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    init_galaxy();

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "3D Galaxy Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 800);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *gl_area = gtk_gl_area_new();
    gtk_container_add(GTK_CONTAINER(window), gl_area);

    g_signal_connect(gl_area, "render", G_CALLBACK(render), NULL);
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(gl_area), TRUE);

    gtk_widget_show_all(window);

    // Animation timer (~60 FPS)
    g_timeout_add(16, tick, gl_area);

    gtk_main();
    return 0;
}

//galaxy.h / galaxy.c for star logic, gl_render.c for rendering, main.c for GTK app setup
//Add mouse/touch controls to rotate it manually
//Switch to shaders (GLSL) for glow effects
//Make stars orbit (animated position updates)
