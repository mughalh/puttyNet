#include <gtk/gtk.h>
#include <math.h>
#include <cairo.h>
#include <gdk/gdk.h>
#include <gio/gio.h>
#include <gst/gst.h>
#include <epoxy/gl.h>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <string>

// Networking headers
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <unistd.h>

// Constants
const int MESSAGE_PORT = 12345;
const int VOICE_PORT = 12346;
const int DISCOVERY_PORT = 12347;

// Global variables
static guint wave_timeout_id = 0;
static double wave_radius = 0;
static double icon_scale = 1.0;
static std::unordered_map<std::string, std::string> online_nodes; // IP -> Name
static std::mutex nodes_mutex;
static int discovery_socket = -1;
static bool running = true;

// GStreamer elements
GstElement *voice_pipeline = NULL;
GstElement *sound_pipeline = NULL;

// OpenGL variables
GLuint program;
GLuint vao;
GLuint vbo;

// Shader sources
const char *vertex_shader_source =
    "#version 330 core\n"
    "layout (location = 0) in vec2 position;\n"
    "void main() {\n"
    "    gl_Position = vec4(position, 0.0, 1.0);\n"
    "}\n";

const char *fragment_shader_source =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 color;\n"
    "void main() {\n"
    "    FragColor = color;\n"
    "}\n";

// Function prototypes
void init_gstreamer();
void init_opengl(GtkWidget *gl_area);
void draw_gl_scene(GtkWidget *gl_area);
void discover_nodes();
void handle_discovery_packet();
void send_discovery_packet();
void start_voice_chat(const std::string &ip);
void stop_voice_chat();
void play_sound_effect(const char *filename);
void draw_footer(GtkWidget *widget, cairo_t *cr, gpointer data);
void draw_icon(GtkWidget *widget, cairo_t *cr, gpointer data);
gboolean animate_wave(GtkWidget *widget);
gboolean on_hover(GtkWidget *widget, GdkEvent *event, gpointer data);
gboolean on_leave(GtkWidget *widget, GdkEvent *event, gpointer data);
gboolean render_gl(GtkWidget *widget, GdkGLContext *context, gpointer data);
void realize_gl(GtkWidget *widget, gpointer data);

// Initialize GStreamer
void init_gstreamer() {
    gst_init(NULL, NULL);

    // Pipeline for sound effects
    sound_pipeline = gst_parse_launch("playbin uri=file:///home/ed/gtk-apps/hover_sound.ogg", NULL);
    if (!sound_pipeline) {
        g_warning("Failed to create sound GStreamer pipeline");
    }

    // Pipeline for voice chat (will be initialized when needed)
    voice_pipeline = NULL;
}

// Initialize OpenGL
void init_opengl(GtkWidget *gl_area) {
    // Compile vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    // Compile fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    // Link shaders
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    // Clean up shaders
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Set up vertex data
    GLfloat vertices[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.0f,  0.5f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Draw OpenGL scene
void draw_gl_scene(GtkWidget *gl_area) {
    gtk_gl_area_make_current(GTK_GL_AREA(gl_area));

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);

    // Set color (blue)
    GLint color_loc = glGetUniformLocation(program, "color");
    glUniform4f(color_loc, 0.0f, 0.0f, 1.0f, 1.0f);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

// Node discovery thread
void discover_nodes() {
    discovery_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (discovery_socket < 0) {
        perror("socket");
        return;
    }

    // Set socket to allow broadcast
    int broadcast = 1;
    if (setsockopt(discovery_socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("setsockopt");
        close(discovery_socket);
        return;
    }

    // Bind to discovery port
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(DISCOVERY_PORT);

    if (bind(discovery_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(discovery_socket);
        return;
    }

    // Start discovery thread
    std::thread discovery_thread([]() {
        while (running) {
            handle_discovery_packet();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    discovery_thread.detach();

    // Send initial discovery packet
    send_discovery_packet();
}

void handle_discovery_packet() {
    char buffer[1024];
    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);

    int n = recvfrom(discovery_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&from, &from_len);
    if (n > 0) {
        buffer[n] = '\0';
        std::string ip = inet_ntoa(from.sin_addr);
        std::string name(buffer);

        std::lock_guard<std::mutex> lock(nodes_mutex);
        online_nodes[ip] = name;
    }
}

void send_discovery_packet() {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    addr.sin_port = htons(DISCOVERY_PORT);

    const char *name = "MyNode";
    sendto(discovery_socket, name, strlen(name), 0, (struct sockaddr*)&addr, sizeof(addr));
}

void start_voice_chat(const std::string &ip) {
    if (voice_pipeline) {
        gst_element_set_state(voice_pipeline, GST_STATE_NULL);
        gst_object_unref(voice_pipeline);
    }

    std::string pipeline_str = "autoaudiosrc ! audioconvert ! opusenc ! rtpopuspay ! "
                              "udpsink host=" + ip + " port=" + std::to_string(VOICE_PORT) + " "
                              "udpsrc port=" + std::to_string(VOICE_PORT + 1) + " ! "
                              "application/x-rtp,media=audio,encoding-name=OPUS ! "
                              "rtpjitterbuffer ! rtpopusdepay ! opusdec ! audioconvert ! autoaudiosink";

    voice_pipeline = gst_parse_launch(pipeline_str.c_str(), NULL);
    if (!voice_pipeline) {
        g_warning("Failed to create voice GStreamer pipeline");
        return;
    }

    gst_element_set_state(voice_pipeline, GST_STATE_PLAYING);
    play_sound_effect("call_start.ogg");
}

void stop_voice_chat() {
    if (voice_pipeline) {
        gst_element_set_state(voice_pipeline, GST_STATE_NULL);
        gst_object_unref(voice_pipeline);
        voice_pipeline = NULL;
    }
    play_sound_effect("call_end.ogg");
}

void play_sound_effect(const char *filename) {
    if (sound_pipeline) {
        g_object_set(sound_pipeline, "uri", g_strdup_printf("file://%s", filename), NULL);
        gst_element_set_state(sound_pipeline, GST_STATE_PLAYING);
    }
}

// GTK Drawing Functions

void draw_footer(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgba(cr, 0.1, 0.1, 0.1, 0.7);
    cairo_rectangle(cr, 0, height - 40, width, 40);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);

    std::lock_guard<std::mutex> lock(nodes_mutex);
    std::string status = "Online nodes: " + std::to_string(online_nodes.size());
    cairo_move_to(cr, 20, height - 15);
    cairo_show_text(cr, status.c_str());
}

void draw_icon(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    GtkImage *icon = GTK_IMAGE(data);
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(icon);

    if (pixbuf != NULL) {
        int img_width = gdk_pixbuf_get_width(pixbuf);
        int img_height = gdk_pixbuf_get_height(pixbuf);

        int scaled_width = (int)(img_width * icon_scale);
        int scaled_height = (int)(img_height * icon_scale);

        int x = (width - scaled_width) / 2;
        int y = (height - scaled_height) / 2;

        gdk_cairo_set_source_pixbuf(cr, pixbuf, x, y);
        cairo_paint(cr);
    }
}

gboolean animate_wave(GtkWidget *widget) {
    wave_radius += 10;
    if (wave_radius > 200) {
        wave_radius = 0;
    }
    gtk_widget_queue_draw(widget);
    return TRUE;
}

gboolean on_hover(GtkWidget *widget, GdkEvent *event, gpointer data) {
    play_sound_effect("hover_sound.ogg");
    icon_scale = 1.1;
    gtk_widget_queue_draw(widget);
    return FALSE;
}

gboolean on_leave(GtkWidget *widget, GdkEvent *event, gpointer data) {
    icon_scale = 1.0;
    gtk_widget_queue_draw(widget);
    return FALSE;
}

gboolean render_gl(GtkWidget *widget, GdkGLContext *context, gpointer data) {
    draw_gl_scene(widget);
    return TRUE;
}

void realize_gl(GtkWidget *widget, gpointer data) {
    gtk_gl_area_make_current(GTK_GL_AREA(widget));
    if (gtk_gl_area_get_error(GTK_GL_AREA(widget))) {
        g_warning("Failed to initialize OpenGL");
        return;
    }
    init_opengl(widget);
}

// Callback for node selection
void on_node_selected(GtkWidget *widget, gpointer data) {
    const char *ip = (const char *)data;
    start_voice_chat(ip);
}

// Create the main window
GtkWidget* create_main_window() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Decentralized Network");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK([](GtkWidget *widget, gpointer data) {
        running = false;
        if (discovery_socket != -1) close(discovery_socket);
        gtk_main_quit();
    }), NULL);

    // Set dark theme
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "* { background-color: #121212; color: #ffffff; }"
        "button { background-color: #333333; border-radius: 5px; padding: 5px; }"
        "button:hover { background-color: #444444; }"
        "scrollbar { background-color: #333333; }"
        "scrollbar slider { background-color: #666666; }", -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Main container
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Header with OpenGL animation
    GtkWidget *gl_area = gtk_gl_area_new();
    gtk_widget_set_size_request(gl_area, 800, 200);
    g_signal_connect(gl_area, "realize", G_CALLBACK(realize_gl), NULL);
    g_signal_connect(gl_area, "render", G_CALLBACK(render_gl), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), gl_area, FALSE, FALSE, 0);

    // Online nodes list
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *nodes_list = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(nodes_list), GTK_SELECTION_SINGLE);
    gtk_container_add(GTK_CONTAINER(scrolled), nodes_list);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

    // Footer
    GtkWidget *footer = gtk_drawing_area_new();
    gtk_widget_set_size_request(footer, -1, 40);
    g_signal_connect(footer, "draw", G_CALLBACK(draw_footer), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), footer, FALSE, FALSE, 0);

    // Start node discovery
    discover_nodes();

    // Timer to update node list
    g_timeout_add(1000, [](gpointer data) -> gboolean {
        GtkWidget *list = (GtkWidget *)data;

        // Clear current list
        GList *children = gtk_container_get_children(GTK_CONTAINER(list));
        for (GList *iter = children; iter != NULL; iter = iter->next) {
            gtk_container_remove(GTK_CONTAINER(list), GTK_WIDGET(iter->data));
        }
        g_list_free(children);

        // Add online nodes
        std::lock_guard<std::mutex> lock(nodes_mutex);
        for (const auto &node : online_nodes) {
            GtkWidget *row = gtk_list_box_row_new();
            GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
            gtk_container_add(GTK_CONTAINER(row), hbox);

            GtkWidget *label = gtk_label_new(node.second.c_str());
            gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);

            GtkWidget *button = gtk_button_new_with_label("Call");
            g_signal_connect(button, "clicked", G_CALLBACK(on_node_selected), (gpointer)node.first.c_str());
            gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

            gtk_container_add(GTK_CONTAINER(list), row);
        }

        gtk_widget_show_all(list);
        return TRUE;
    }, nodes_list);

    return window;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    init_gstreamer();

    GtkWidget *window = create_main_window();
    gtk_widget_show_all(window);

    gtk_main();

    // Clean up
    if (voice_pipeline) {
        gst_element_set_state(voice_pipeline, GST_STATE_NULL);
        gst_object_unref(voice_pipeline);
    }
    if (sound_pipeline) {
        gst_element_set_state(sound_pipeline, GST_STATE_NULL);
        gst_object_unref(sound_pipeline);
    }

    return 0;
}
