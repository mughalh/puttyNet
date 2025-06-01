#include <gtk/gtk.h>
#include <math.h>
#include <cairo.h>
#include <gdk/gdk.h>
#include <gio/gio.h>
#include <gst/gst.h>
#include <epoxy/gl.h>

static guint wave_timeout_id = 0;  // Timeout for the wave animation
static double wave_radius = 0;     // Current radius of the wave

// Current icon scaling factor
static double icon_scale = 1.0;

// GStreamer pipeline for sound playback
GstElement *pipeline = NULL;

// Function to initialize GStreamer
void init_gstreamer() {
    gst_init(NULL, NULL);
    pipeline = gst_parse_launch("playbin uri=file:///home/ed/gtk-apps/hover_sound.ogg", NULL);
    if (!pipeline) {
        g_warning("Failed to create GStreamer pipeline");
    }
}

// Function to play the hover sound
void play_hover_sound() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
    } else {
        g_warning("GStreamer pipeline not initialized");
    }
}

// Function to stop the sound (optional)
void stop_hover_sound() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
    }
}

// Function to draw footer (same as before)
void draw_footer(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    // Draw the footer background (transparent grey)
    cairo_set_source_rgba(cr, 0.1, 0.1, 0.1, 0.7);  // Semi-transparent grey
    cairo_rectangle(cr, 0, height - 40, width, 40); // Footer size
    cairo_fill(cr);

    // Draw the footer text
    cairo_set_source_rgb(cr, 1, 1, 1);  // White text
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);
    cairo_move_to(cr, 20, height - 15);  // Position of the text
    cairo_show_text(cr, "More nodes will show as they come online");
}

// Function to draw the icon
void draw_icon(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    // Load the icon image (png)
    GtkImage *icon = GTK_IMAGE(data);
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(icon);

    if (pixbuf != NULL) {
        int img_width = gdk_pixbuf_get_width(pixbuf);
        int img_height = gdk_pixbuf_get_height(pixbuf);

        // Apply scaling to the icon
        int scaled_width = (int)(img_width * icon_scale);
        int scaled_height = (int)(img_height * icon_scale);

        // Calculate the center position for the image
        int x = (width - scaled_width) / 2;
        int y = (height - scaled_height) / 2;

        // Draw the image with scaling
        gdk_cairo_set_source_pixbuf(cr, pixbuf, x, y);
        cairo_paint(cr);
    }
}

// Function to animate the wave
gboolean animate_wave(GtkWidget *widget) {
    wave_radius += 10;  // Increase the wave radius
    if (wave_radius > 200) {  // Reset after reaching a maximum size
        wave_radius = 0;
    }

    gtk_widget_queue_draw(widget);  // Redraw the widget

    return TRUE;  // Continue calling this function at intervals
}

// Function to handle hover effect on the icon
gboolean on_hover(GtkWidget *widget, GdkEvent *event, gpointer data) {
    // Play sound using GStreamer
    play_hover_sound();

    // Apply zoom effect by scaling the icon
    icon_scale = 1.1;  // Zoom in the icon

    gtk_widget_queue_draw(widget);  // Redraw the widget with updated scale
    return FALSE;  // Continue with normal behavior
}

// Function to handle when the mouse leaves the icon (reset zoom)
gboolean on_leave(GtkWidget *widget, GdkEvent *event, gpointer data) {
    // Reset zoom effect
    icon_scale = 1.0;  // Restore the icon to its original size

    gtk_widget_queue_draw(widget);  // Redraw the widget with original scale
    return FALSE;  // Continue with normal behavior
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    init_gstreamer();  // Initialize GStreamer

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Putty");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Set the background color (black)
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "* { background-color: black; }", -1, NULL);
    gtk_style_context_add_provider(gtk_widget_get_style_context(window), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Create a container for the window content
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Load the icon and create an image widget
    GtkWidget *icon_widget = gtk_image_new_from_file("icon.png");  // Replace with your actual image path
    gtk_widget_set_size_request(icon_widget, 100, 100);  // Set the size of the icon (adjust as needed)
    
    // Create a drawing area for the content (background, footer, etc.)
    GtkWidget *canvas = gtk_drawing_area_new();
    gtk_widget_set_size_request(canvas, 400, 400);

    // Connect signals for drawing the icon and footer
    g_signal_connect(canvas, "draw", G_CALLBACK(draw_icon), icon_widget);
    g_signal_connect(canvas, "draw", G_CALLBACK(draw_footer), NULL);

    // Add canvas and icon to the vbox
    gtk_box_pack_start(GTK_BOX(vbox), canvas, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), icon_widget, FALSE, FALSE, 0);

    // Start wave animation
    wave_timeout_id = g_timeout_add(2000, (GSourceFunc)animate_wave, canvas);

    // Mouse hover effect for icon
    g_signal_connect(icon_widget, "enter-notify-event", G_CALLBACK(on_hover), NULL);
    g_signal_connect(icon_widget, "leave-notify-event", G_CALLBACK(on_leave), NULL);

    // Show all widgets
    gtk_widget_show_all(window);

    gtk_main();

    // Clean up GStreamer resources
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }

    return 0;
}
