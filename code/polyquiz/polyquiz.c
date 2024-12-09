#include <libdragon.h>
#include "../../minigame.h"
#include "../../core.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/gl_integration.h>

#define MAX_VERTICES 100
#define MAX_FACES 200
#define NUM_BKGS 20

#define MAX_TIME  20.0f
#define FADEIN_TIME 2.0f
#define FADEOUT_TIME 3.0f

const MinigameDef minigame_def = {
    .gamename = "Polyquiz",
    .developername = "Rasky",
    .description = "Simple OpenGL game. Can you guess how many faces a polyhedron has?",
    .instructions = "D-Pad to change your guess, A to confirm",
};

typedef struct {
    float r, g, b;
} Color;

typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    int v1, v2, v3;
    int color_idx;
} Face;

Color palette[] = {
    {0.894f, 0.102f, 0.110f},  // Rosso brillante
    {0.216f, 0.494f, 0.722f},  // Blu brillante
    {0.302f, 0.686f, 0.290f},  // Verde brillante
    {0.596f, 0.306f, 0.639f},  // Viola
    {1.000f, 0.498f, 0.000f},  // Arancione
    {1.000f, 1.000f, 0.200f},  // Giallo
    {0.651f, 0.337f, 0.157f},  // Marrone
    {0.968f, 0.505f, 0.749f},  // Rosa
};
#define PALETTE_SIZE (sizeof(palette) / sizeof(palette[0]))

Color random_color_from_palette() {
    int index = rand() % PALETTE_SIZE;
    return palette[index];
}

Vertex vertices[MAX_VERTICES];
Face faces[MAX_FACES];
int num_vertices = 0;
int num_faces = 0;
rspq_block_t *poly = NULL;
sprite_t *bkg[NUM_BKGS];
rdpq_font_t *font = NULL;
#define FONT_TEXT 1

typedef enum {
    GS_FADEIN,
    GS_PLAY,
    GS_FADEOUT,
    GS_RESULT,
} GameState;

GameState state;
float state_time;

int cur_bkg = 0;
float angle = 0.0f;
float rotationSpeed = 40.0f;
float axisX = 0.0f, axisY = 1.0f, axisZ = 0.0f; 
float zoom = 1.0f;

struct {
    int guess;
    bool confirmed;
    float confirm_time;
    int ai_guesses[3];
    float ai_guess_times[3];
} player[4];

void generateRandomAxis() {
    axisX = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
    axisY = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
    axisZ = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;

    float length = sqrtf(axisX * axisX + axisY * axisY + axisZ * axisZ);
    if (length > 0.0f) {
        axisX /= length;
        axisY /= length;
        axisZ /= length;
    }
}

Vertex random_vertex(float range_min, float range_max) {
    Vertex v;

    float radius = (range_max - range_min)/2;
    float theta = ((float)rand() / RAND_MAX) * 2.0f * 3.1415628f;
    float phi = acosf(1.0f - 2.0f * ((float)rand() / RAND_MAX));

    v.x = radius * sinf(phi) * cosf(theta);
    v.y = radius * sinf(phi) * sinf(theta);
    v.z = radius * cosf(phi);

    return v;
}

Vertex cross_product(Vertex v1, Vertex v2) {
    Vertex result;
    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.x;
    return result;
}

Vertex subtract(Vertex v1, Vertex v2) {
    Vertex result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    return result;
}

float dot_product(Vertex v1, Vertex v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

int is_convex(Vertex p, Vertex q, Vertex r, Vertex s) {
    Vertex pq = subtract(q, p);
    Vertex pr = subtract(r, p);
    Vertex ps = subtract(s, p);
    Vertex n = cross_product(pq, pr);
    return dot_product(n, ps) >= 0;
}

// Gift Wrapping (Jarvis March) for 3D convex hull
void compute_convex_hull() {
    assertf(num_vertices >= 4, "Too few points to create a polyhedron");

    num_faces = 0;
    
    // Staring point: find the leftmost point
    int start = 0;
    for (int i = 1; i < num_vertices; i++) {
        if (vertices[i].x < vertices[start].x) {
            start = i;
        }
    }

    int p = start;
    do {
        for (int i = 0; i < num_vertices; i++) {
            for (int j = i + 1; j < num_vertices; j++) {
                if (i != p && j != p) {
                    int found = 1;
                    for (int k = 0; k < num_vertices; k++) {
                        if (k != p && k != i && k != j && !is_convex(vertices[p], vertices[i], vertices[j], vertices[k])) {
                            found = 0;
                            break;
                        }
                    }
                    if (found) {
                        faces[num_faces].v1 = p;
                        faces[num_faces].v2 = i;
                        faces[num_faces].v3 = j;
                        num_faces++;
                    }
                }
            }
        }
        p++;
        if (p == num_vertices) {
            p = 0;
        }
    } while (p != start && num_faces < MAX_FACES);
}

void color_polyhedron(void) {
    int are_faces_adjacent(Face *f1, Face *f2) {
        int shared_vertices = 0;
        if (f1->v1 == f2->v1 || f1->v1 == f2->v2 || f1->v1 == f2->v3) shared_vertices++;
        if (f1->v2 == f2->v1 || f1->v2 == f2->v2 || f1->v2 == f2->v3) shared_vertices++;
        if (f1->v3 == f2->v1 || f1->v3 == f2->v2 || f1->v3 == f2->v3) shared_vertices++;

        return (shared_vertices == 2);
    }    
    
    int is_valid_color(Face *faces, int face_index, int color_index, int num_faces) {
        for (int i = 0; i < num_faces; i++) {
            if (i == face_index) continue;
            if (are_faces_adjacent(&faces[face_index], &faces[i]) && faces[i].color_idx == color_index)
                return false;
        }
        return true;
    }


    for (int i = 0; i < num_faces; i++) {
        int idx = rand() % PALETTE_SIZE;
        for (int c = 0; c < PALETTE_SIZE; c++) {
            if (is_valid_color(faces, i, (idx+c)%PALETTE_SIZE, num_faces)) {
                faces[i].color_idx = (idx+c) % PALETTE_SIZE;
                break;
            }
        }
    }
}

void draw_polyhedron(void)
{
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < num_faces; i++) {
        Vertex v1 = vertices[faces[i].v1];
        Vertex v2 = vertices[faces[i].v2];
        Vertex v3 = vertices[faces[i].v3];

        Vertex normal;
        normal.x = (v2.y - v1.y) * (v3.z - v1.z) - (v2.z - v1.z) * (v3.y - v1.y);
        normal.y = (v2.z - v1.z) * (v3.x - v1.x) - (v2.x - v1.x) * (v3.z - v1.z);
        normal.z = (v2.x - v1.x) * (v3.y - v1.y) - (v2.y - v1.y) * (v3.x - v1.x);

        glNormal3f(normal.x, normal.y, normal.z);
        Color color = palette[faces[i].color_idx];
        glColor4f(color.r, color.g, color.b, 0.8f);

        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
        glVertex3f(v3.x, v3.y, v3.z);
    }
    glEnd();
}

void generate_random_polyhedron(int num_vertices_input, float range_min, float range_max) {
    num_vertices = num_vertices_input;
    for (int i = 0; i < num_vertices; i++) {
        vertices[i] = random_vertex(range_min, range_max);
    }
    compute_convex_hull();
    color_polyhedron();

    if (poly) rspq_block_free(poly);
    rspq_block_begin();
        draw_polyhedron();
    poly = rspq_block_end();

    cur_bkg = rand() % NUM_BKGS;
}

float gauss_random(float mean, float stddev) {
    static int has_spare = 0;
    static float spare;    

    if (has_spare) {
        has_spare = 0;
        return mean + stddev * spare;
    }

    has_spare = 1;
    float u, v, s;
    do {
        u = (rand() / ((float) RAND_MAX)) * 2.0 - 1.0;
        v = (rand() / ((float) RAND_MAX)) * 2.0 - 1.0;
        s = u * u + v * v;
    } while (s >= 1.0 || s == 0.0);

    s = sqrtf(-2.0 * logf(s) / s);
    spare = v * s;
    return mean + stddev * u * s;
}

float random_gaussian_truncated(float A, float B, float C, float sigma) {
    float x;
    do {
        float u1 = rand() / (float) RAND_MAX;
        float u2 = rand() / (float) RAND_MAX;
        float z0 = sqrtf(-2.0 * logf(u1)) * cosf(2.0 * 3.1415628f * u2);
        x = C + sigma * z0;
    } while (x < A || x > B);
    return x;
}

void generate_ai_guesses(void)
{
    float stddev[] = { 7.0f, 4.5f, 2.0f };
    float times[] = { 15.0f, 10.0f, 5.0f };

    AiDiff diff = core_get_aidifficulty();
    for (int i=core_get_playercount(); i<4; i++) {
        player[i].ai_guesses[2] = gauss_random(num_faces, stddev[diff]*1.0f);
        player[i].ai_guesses[1] = gauss_random(num_faces, stddev[diff]*2.0f);
        player[i].ai_guesses[0] = gauss_random(num_faces, stddev[diff]*3.0f);

        for (int j=0;j<3;j++) {
            if (player[i].ai_guesses[j] < 0) player[i].ai_guesses[j] = 0;
            if (player[i].ai_guesses[j] > 40) player[i].ai_guesses[j] = 40;
        }

        float t2 = random_gaussian_truncated(times[diff]/2, MAX_TIME*1.3f, times[diff], stddev[diff]*2);
        float t1 = random_gaussian_truncated(t2*0.4f, t2*0.8f, t2*0.6f, stddev[diff]*2);
        float t0 = random_gaussian_truncated(t1*0.4f, t1*0.8f, t1*0.6f, stddev[diff]*2);

        player[i].ai_guess_times[2] = t2;
        player[i].ai_guess_times[1] = t1;
        player[i].ai_guess_times[0] = t0;
    }
}

void minigame_init()
{
    display_init(RESOLUTION_640x480, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);
    gl_init();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  // Colore di sfondo

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat light_dir[] = { -1.0f, -1.0f, -1.0f, 0.0f };
    GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_dir);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    int w = display_get_width(), h = display_get_height();
    float near_plane = 1.0f;
    float far_plane = 50.0f;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat)w / (GLfloat)h, near_plane, far_plane);

    int num_vertices = rand() % 10 + 5;
    generate_random_polyhedron(num_vertices, -1.0f, 1.0f);

    for (int i=0; i<NUM_BKGS; i++) {
        char fn[64];
        sprintf(fn, "rom:/polyquiz/plaster%d.ci4.sprite", i+1);
        bkg[i] = sprite_load(fn);
    }
    font = rdpq_font_load("rom:/polyquiz/abaddon.font64");
    rdpq_text_register_font(FONT_TEXT, font);
    rdpq_font_style(font, 0, &(rdpq_fontstyle_t){
        .color = RGBA32(0xFF, 0xFF, 0xFF, 0xFF), .outline_color = RGBA32(0x0, 0x0, 0x0, 0xFF),
    });
    rdpq_font_style(font, 1, &(rdpq_fontstyle_t){
        .color = RGBA32(palette[2].r*255, palette[2].g*255, palette[2].b*255, 0xFF),
        .outline_color = RGBA32(0x0, 0x0, 0x0, 0xFF),
    });
    rdpq_font_style(font, 2, &(rdpq_fontstyle_t){
        .color = RGBA32(palette[0].r*255, palette[0].g*255, palette[0].b*255, 0xFF),
        .outline_color = RGBA32(0x0, 0x0, 0x0, 0xFF),
    });

    generate_ai_guesses();

    state = GS_FADEIN;
    state_time = FADEIN_TIME;
    zoom = 0.001f;
}

void minigame_cleanup()
{
    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_font_free(font);
    for (int i=0; i<NUM_BKGS; i++) {
        sprite_free(bkg[i]);
    }
    if (poly) rspq_block_free(poly);
    gl_close();
    display_close();
}

void minigame_fixedloop(float dt)
{
    state_time -= dt;
    switch (state) {
    case GS_PLAY:
        angle += rotationSpeed * dt;
        if (angle > 360.0f) {
            angle -= 360.0f;
            generateRandomAxis();
        }
        break;
    case GS_FADEIN:
        // Zoom in the polyhedron
        zoom = 1.0f - (state_time / FADEIN_TIME);
        if (state_time <= 0) {
            state = GS_PLAY;
            state_time = 20.0f;
        }
        return;
    case GS_FADEOUT:
        // Zoom out the polyhedron
        zoom = 1.0f * (state_time / FADEOUT_TIME);
        if (state_time <= 0) {
            state = GS_RESULT;
            state_time = 5.0f;
        }
        return;
    case GS_RESULT:
        if (state_time <= 0) {
            minigame_end();
        }
        return;
    }

    int num_humans = core_get_playercount();
    for (int i=num_humans; i<4; i++) {
        if (player[i].confirmed) continue;
        for (int j=0; j<3; j++) 
            player[i].ai_guess_times[j] -= dt;
        for (int j=0; j<3; j++) {
            if (player[i].ai_guess_times[j] > 0) {
                int target_guess = player[i].ai_guesses[j] * (1 - player[i].ai_guess_times[j] / MAX_TIME);
                if (player[i].guess < target_guess) player[i].guess++;
                if (player[i].guess > target_guess) player[i].guess--;
                break;
            } else if (j == 2 && !player[i].confirmed) {
                player[i].confirmed = true;
                player[i].guess = player[i].ai_guesses[j];
            }
        }
    }

    // Check if all players have confirmed
    bool all_confirmed = true;
    for (int i=0; i<4; i++) {
        if (!player[i].confirmed) {
            all_confirmed = false;
            break;
        }
    }
    if (all_confirmed || state_time <= 0) {
        state = GS_FADEOUT;
        state_time = FADEOUT_TIME;
    }
}

void minigame_loop(float dt)
{
    joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    if (btn.start) {
        debugf("Minigame ended by player (faces=%d)\n", num_faces);
        minigame_end();
    }

    if (state == GS_PLAY) {
        int num_humans = core_get_playercount();
        for (int i=0; i<num_humans; i++) {
            if (player[i].confirmed) break;
            joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1+i);
            if (btn.d_up) player[i].guess++;
            if (btn.d_down) player[i].guess--;
            if (btn.d_left) player[i].guess -= 10;
            if (btn.d_right) player[i].guess += 10;
            if (player[i].guess < 0) player[i].guess = 0;
            if (player[i].guess > 40) player[i].guess = 40;
            if (btn.a) player[i].confirmed = true;
        }
    }
    
    surface_t *disp = display_get();
    rdpq_attach(disp, NULL);

    rdpq_set_mode_copy(false);
    rdpq_sprite_upload(TILE0, bkg[cur_bkg], &(rdpq_texparms_t){
        .s.repeats = REPEAT_INFINITE, .t.repeats = REPEAT_INFINITE,
    });
    rdpq_texture_rectangle(TILE0, 0, 0, display_get_width(), display_get_height(), 0, 0);

    gl_context_begin();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 4.0,  // Camera pos
              0.0, 0.0, 0.0,  // Look at
              0.0, 1.0, 0.0); // Up vector

    glScalef(zoom, zoom, zoom);
    glRotatef(angle, axisX, axisY, axisZ);

    rspq_block_run(poly);

    gl_context_end();

    rdpq_set_mode_standard();
    switch (state) {
    case GS_FADEIN:
        rdpq_text_printf(&(rdpq_textparms_t){
            .width = display_get_width(), .align = ALIGN_CENTER,
        }, FONT_TEXT, 0, 50, "Guess the number of faces!");
        break;
    case GS_PLAY: {
        char buf[16];
        sprintf(buf, "%02d:%02d", (int)state_time, (int)((state_time - (int)state_time) * 100));
        rdpq_text_printf(NULL, FONT_TEXT, 250, 50, "%c", buf[0]);
        rdpq_text_printf(NULL, FONT_TEXT, 270, 50, "%c", buf[1]);
        rdpq_text_printf(NULL, FONT_TEXT, 295, 50, "%c", ':');
        rdpq_text_printf(NULL, FONT_TEXT, 310, 50, "%c", buf[3]);
        rdpq_text_printf(NULL, FONT_TEXT, 330, 50, "%c", buf[4]);
    }   break;
    case GS_RESULT:
        rdpq_text_printf(&(rdpq_textparms_t){
            .width = display_get_width(), .align = ALIGN_CENTER,
        }, FONT_TEXT, 0, 50, "Faces: %d", num_faces);

        // Found the player with the closest guess, and between evens,
        // the one with the earliest guess
        int closest = -1;
        int closest_diff = 1000;
        for (int i=0; i<4; i++) {
            if (!player[i].confirmed) continue;
            int diff = abs(player[i].guess - num_faces);
            if (diff < closest_diff || (diff == closest_diff && player[i].guess < player[closest].guess)) {
                closest = i;
                closest_diff = diff;
            }
        }

        if (closest != -1) {
            rdpq_text_printf(&(rdpq_textparms_t){
                .width = display_get_width(), .align = ALIGN_CENTER,
                .style_id = 1,
            }, FONT_TEXT, 0, 240, "Player %d wins!", closest+1);
        } else {
            rdpq_text_printf(&(rdpq_textparms_t){
                .width = display_get_width(), .align = ALIGN_CENTER,
                .style_id = 1,
            }, FONT_TEXT, 0, 240, "Nobody wins");
        }
        break;
    default:
        break;
    }

    if (state != GS_FADEIN) {
        for (int i=0; i<4; i++) {
            rdpq_textparms_t parms = {
                .style_id = player[i].confirmed ? 1 : (state == GS_PLAY ? 0 : 2),
            };
            rdpq_text_printf(&parms, FONT_TEXT, 100+i*140, 460, "%d", player[i].guess);
        }
    }

    rdpq_detach_show();
}
