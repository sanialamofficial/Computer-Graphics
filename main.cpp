#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

// Constants
#define MAX_PLANETS 8
#define MAX_MOONS 5
#define MAX_STARS 400
#define MAX_ASTEROIDS 800

// Structs
struct Moon {
    float radius;
    float size;
    float speed;
};

struct Planet {
    const char* name;
    float orbitRadius;
    float size;
    float orbitSpeed;
    float selfRotateSpeed;
    float r, g, b;
    Moon moons[MAX_MOONS];
    int moonCount;
    float orbitAngle;
    float selfAngle;
};

struct Star {
    float x, y;
    float twinkleSpeed;
    float phase;
};

struct Asteroid {
    float angle;
    float radius;
    float drift;
};

// Globals
int winW = 1200, winH = 800;
float camZoom = 1.0f;
float camX = 0.0f, camY = 0.0f;
bool showInfo = true;
bool paused = false;
bool shootingActive = false;
float shootX = 0, shootY = 0, shootVX = 0, shootVY = 0, shootLife = 0;
unsigned int prevMs = 0;

Planet planets[MAX_PLANETS];
int planetCount = 0;

Star stars[MAX_STARS];
Asteroid belt[MAX_ASTEROIDS];

// Clamp function
float clamp(float v, float a, float b) {
    if (v < a) return a;
    if (v > b) return b;
    return v;
}

// Draw bitmap text
void bitmapText(float x, float y, const char* s) {
    glRasterPos2f(x, y);
    for (const char* p = s; *p; ++p) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *p);
    }
}

// Draw filled circle
void drawCircle(float cx, float cy, float r, int seg) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; ++i) {
        float a = (2.0f * 3.14159265358979323846f * i) / seg;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

// Draw orbit path (circle outline)
void drawOrbitPath(float r) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 180; ++i) {
        float a = (2.0f * 3.14159265358979323846f * i) / 180.0f;
        glVertex2f(r * cosf(a), r * sinf(a));
    }
    glEnd();
}

// Initialize planets and moons
void initPlanets() {
    planetCount = 0;

    // Mercury
    planets[planetCount].name = "Mercury";
    planets[planetCount].orbitRadius = 60.0f;
    planets[planetCount].size = 6.0f;
    planets[planetCount].orbitSpeed = 47.4f;
    planets[planetCount].selfRotateSpeed = 6.0f;
    planets[planetCount].r = 0.8f; planets[planetCount].g = 0.8f; planets[planetCount].b = 0.7f;
    planets[planetCount].moonCount = 0;
    planets[planetCount].orbitAngle = 0.0f;
    planets[planetCount].selfAngle = 0.0f;
    planetCount++;

    // Venus
    planets[planetCount].name = "Venus";
    planets[planetCount].orbitRadius = 95.0f;
    planets[planetCount].size = 10.0f;
    planets[planetCount].orbitSpeed = 35.0f;
    planets[planetCount].selfRotateSpeed = -2.0f;
    planets[planetCount].r = 0.95f; planets[planetCount].g = 0.8f; planets[planetCount].b = 0.5f;
    planets[planetCount].moonCount = 0;
    planets[planetCount].orbitAngle = 0.0f;
    planets[planetCount].selfAngle = 0.0f;
    planetCount++;

    // Earth
    planets[planetCount].name = "Earth";
    planets[planetCount].orbitRadius = 135.0f;
    planets[planetCount].size = 11.0f;
    planets[planetCount].orbitSpeed = 29.8f;
    planets[planetCount].selfRotateSpeed = 15.0f;
    planets[planetCount].r = 0.2f; planets[planetCount].g = 0.5f; planets[planetCount].b = 1.0f;
    planets[planetCount].moonCount = 1;
    planets[planetCount].moons[0].radius = 18.0f;
    planets[planetCount].moons[0].size = 3.0f;
    planets[planetCount].moons[0].speed = 120.0f;
    planets[planetCount].orbitAngle = 0.0f;
    planets[planetCount].selfAngle = 0.0f;
    planetCount++;

    // Mars
    planets[planetCount].name = "Mars";
    planets[planetCount].orbitRadius = 175.0f;
    planets[planetCount].size = 8.0f;
    planets[planetCount].orbitSpeed = 24.1f;
    planets[planetCount].selfRotateSpeed = 14.0f;
    planets[planetCount].r = 1.0f; planets[planetCount].g = 0.3f; planets[planetCount].b = 0.2f;
    planets[planetCount].moonCount = 2;
    planets[planetCount].moons[0].radius = 12.0f;
    planets[planetCount].moons[0].size = 2.2f;
    planets[planetCount].moons[0].speed = 160.0f;
    planets[planetCount].moons[1].radius = 16.0f;
    planets[planetCount].moons[1].size = 1.8f;
    planets[planetCount].moons[1].speed = 90.0f;
    planets[planetCount].orbitAngle = 0.0f;
    planets[planetCount].selfAngle = 0.0f;
    planetCount++;

    // Jupiter
    planets[planetCount].name = "Jupiter";
    planets[planetCount].orbitRadius = 235.0f;
    planets[planetCount].size = 20.0f;
    planets[planetCount].orbitSpeed = 13.1f;
    planets[planetCount].selfRotateSpeed = 25.0f;
    planets[planetCount].r = 0.9f; planets[planetCount].g = 0.6f; planets[planetCount].b = 0.3f;
    planets[planetCount].moonCount = 4;
    planets[planetCount].moons[0].radius = 24.0f;
    planets[planetCount].moons[0].size = 2.5f;
    planets[planetCount].moons[0].speed = 200.0f;
    planets[planetCount].moons[1].radius = 30.0f;
    planets[planetCount].moons[1].size = 2.8f;
    planets[planetCount].moons[1].speed = 170.0f;
    planets[planetCount].moons[2].radius = 36.0f;
    planets[planetCount].moons[2].size = 3.1f;
    planets[planetCount].moons[2].speed = 140.0f;
    planets[planetCount].moons[3].radius = 42.0f;
    planets[planetCount].moons[3].size = 3.3f;
    planets[planetCount].moons[3].speed = 110.0f;
    planets[planetCount].orbitAngle = 0.0f;
    planets[planetCount].selfAngle = 0.0f;
    planetCount++;

    // Saturn
    planets[planetCount].name = "Saturn";
    planets[planetCount].orbitRadius = 300.0f;
    planets[planetCount].size = 18.0f;
    planets[planetCount].orbitSpeed = 9.7f;
    planets[planetCount].selfRotateSpeed = 24.0f;
    planets[planetCount].r = 0.95f; planets[planetCount].g = 0.85f; planets[planetCount].b = 0.55f;
    planets[planetCount].moonCount = 1;
    planets[planetCount].moons[0].radius = 28.0f;
    planets[planetCount].moons[0].size = 2.4f;
    planets[planetCount].moons[0].speed = 180.0f;
    planets[planetCount].orbitAngle = 0.0f;
    planets[planetCount].selfAngle = 0.0f;
    planetCount++;

    // Uranus
    planets[planetCount].name = "Uranus";
    planets[planetCount].orbitRadius = 360.0f;
    planets[planetCount].size = 14.0f;
    planets[planetCount].orbitSpeed = 6.8f;
    planets[planetCount].selfRotateSpeed = 20.0f;
    planets[planetCount].r = 0.6f; planets[planetCount].g = 0.9f; planets[planetCount].b = 0.95f;
    planets[planetCount].moonCount = 1;
    planets[planetCount].moons[0].radius = 20.0f;
    planets[planetCount].moons[0].size = 1.8f;
    planets[planetCount].moons[0].speed = 140.0f;
    planets[planetCount].orbitAngle = 0.0f;
    planets[planetCount].selfAngle = 0.0f;
    planetCount++;

    // Neptune
    planets[planetCount].name = "Neptune";
    planets[planetCount].orbitRadius = 420.0f;
    planets[planetCount].size = 14.0f;
    planets[planetCount].orbitSpeed = 5.4f;
    planets[planetCount].selfRotateSpeed = 19.0f;
    planets[planetCount].r = 0.3f; planets[planetCount].g = 0.5f; planets[planetCount].b = 1.0f;
    planets[planetCount].moonCount = 1;
    planets[planetCount].moons[0].radius = 18.0f;
    planets[planetCount].moons[0].size = 1.8f;
    planets[planetCount].moons[0].speed = 130.0f;
    planets[planetCount].orbitAngle = 0.0f;
    planets[planetCount].selfAngle = 0.0f;
    planetCount++;
}

// Initialize stars
void initStars() {
    for (int i = 0; i < MAX_STARS; ++i) {
        stars[i].x = (float)(rand() % 2000 - 1000);
        stars[i].y = (float)(rand() % 2000 - 1000);
        stars[i].twinkleSpeed = 0.5f + (rand() % 100) / 100.0f;
        stars[i].phase = (rand() % 628) / 100.0f;
    }
}

// Initialize asteroid belt
void initBelt() {
    for (int i = 0; i < MAX_ASTEROIDS; ++i) {
        belt[i].angle = (rand() % 36000) / 100.0f;
        belt[i].radius = 205.0f + (rand() % 60);
        belt[i].drift = ((rand() % 200) - 100) / 20000.0f;
    }
}

// Spawn shooting star
void spawnShootingStar() {
    if (shootingActive) return;
    shootingActive = true;
    int edge = rand() % 4;
    float speed = 500.0f;
    if (edge == 0) {
        shootX = -700; shootY = 500; shootVX = speed; shootVY = -speed * 0.4f;
    }
    else if (edge == 1) {
        shootX = 700; shootY = 500; shootVX = -speed; shootVY = -speed * 0.4f;
    }
    else if (edge == 2) {
        shootX = -700; shootY = -500; shootVX = speed; shootVY = speed * 0.4f;
    }
    else {
        shootX = 700; shootY = -500; shootVX = -speed; shootVY = speed * 0.4f;
    }
    shootLife = 1.2f;
}

// Draw background with stars
void drawBackground() {
    glBegin(GL_QUADS);
    glColor3f(0.02f, 0.02f, 0.05f);
    glVertex2f(-10000, -10000);
    glVertex2f(10000, -10000);
    glColor3f(0.02f, 0.02f, 0.10f);
    glVertex2f(10000, 10000);
    glVertex2f(-10000, 10000);
    glEnd();

    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_STARS; ++i) {
        float a = 0.4f + 0.6f * (0.5f + 0.5f * sinf(stars[i].phase));
        glColor4f(1, 1, 1, a);
        glVertex2f(stars[i].x, stars[i].y);
    }
    glEnd();
}

// Draw sun
void drawSun() {
    glColor3f(1.0f, 0.85f, 0.1f);
    drawCircle(0, 0, 28, 120);
}

// Draw asteroid belt
void drawAsteroidBelt(float dt) {
    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_ASTEROIDS; ++i) {
        belt[i].angle += belt[i].drift * dt * 60.0f;
        float rad = belt[i].angle * 3.14159265358979323846f / 180.0f;
        float x = belt[i].radius * cosf(rad);
        float y = belt[i].radius * sinf(rad);
        glColor3f(0.7f, 0.7f, 0.7f);
        glVertex2f(x, y);
    }
    glEnd();
}

// Draw shooting star
void drawShootingStar() {
    if (!shootingActive) return;
    glBegin(GL_LINES);
    glColor3f(1, 1, 1);
    glVertex2f(shootX, shootY);
    glVertex2f(shootX - shootVX * 0.05f, shootY - shootVY * 0.05f);
    glEnd();
}

// Draw planet and moons
void drawPlanetAndMoons(Planet* p) {
    // Draw orbit path
    glColor3f(0.25f, 0.25f, 0.35f);
    drawOrbitPath(p->orbitRadius);

    // Draw planet
    glPushMatrix();
    glRotatef(p->orbitAngle, 0, 0, 1);
    glTranslatef(p->orbitRadius, 0, 0);
    glColor3f(p->r, p->g, p->b);
    drawCircle(0, 0, p->size, 60);

    if (showInfo) {
        glColor3f(1, 1, 1);
        bitmapText(p->size + 4, p->size + 4, p->name);
    }

    // Draw moons
    for (int i = 0; i < p->moonCount; ++i) {
        glPushMatrix();
        float moonAngle = p->selfAngle * (p->moons[i].speed / 60.0f);
        glRotatef(moonAngle, 0, 0, 1);
        glTranslatef(p->moons[i].radius, 0, 0);
        glColor3f(0.85f, 0.85f, 0.85f);
        drawCircle(0, 0, p->moons[i].size, 24);
        glPopMatrix();
    }

    glPopMatrix();
}

// Display callback
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glScalef(camZoom, camZoom, 1.0f);
    glTranslatef(camX, camY, 0.0f);

    drawBackground();
    drawSun();
    drawAsteroidBelt(1.0f);

    for (int i = 0; i < planetCount; ++i) {
        drawPlanetAndMoons(&planets[i]);
    }

    drawShootingStar();

    if (showInfo) {
        glLoadIdentity();

                glColor3f(1, 1, 1);
        bitmapText(-winW * 0.45f, winH * 0.45f, "Arrows=Pan, +/-=Zoom, Space=Pause, I=Info, 0=Reset");
    }

    glutSwapBuffers();
}

// Update simulation state
void update(float dt) {
    // Update stars twinkle phase
    for (int i = 0; i < MAX_STARS; ++i) {
        stars[i].phase += stars[i].twinkleSpeed * dt;
    }

    // Possibly spawn shooting star
    if (!shootingActive && (rand() % 600) == 0) {
        spawnShootingStar();
    }

    // Update shooting star position and life
    if (shootingActive) {
        shootX += shootVX * dt;
        shootY += shootVY * dt;
        shootLife -= dt;
        if (shootLife <= 0) {
            shootingActive = false;
        }
    }

    // Update planets orbit and rotation angles if not paused
    if (!paused) {
        for (int i = 0; i < planetCount; ++i) {
            planets[i].orbitAngle += planets[i].orbitSpeed * dt;
            if (planets[i].orbitAngle > 360.0f) planets[i].orbitAngle -= 360.0f;

            planets[i].selfAngle += planets[i].selfRotateSpeed * dt;
            if (planets[i].selfAngle > 360.0f) planets[i].selfAngle -= 360.0f;
        }
    }
}

// Timer callback for animation
void timer(int) {
    unsigned int now = glutGet(GLUT_ELAPSED_TIME);
    if (prevMs == 0) prevMs = now;
    unsigned int ms = now - prevMs;
    prevMs = now;

    float dt = clamp(ms / 1000.0f, 0.0f, 0.05f);

    update(dt);

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// Keyboard input
void keyboard(unsigned char key, int, int) {
    if (key == ' ') paused = !paused;
    else if (key == 'i' || key == 'I') showInfo = !showInfo;
    else if (key == '+') camZoom = clamp(camZoom * 1.1f, 0.1f, 4.0f);
    else if (key == '-') camZoom = clamp(camZoom / 1.1f, 0.1f, 4.0f);
    else if (key == '0') {
        camZoom = 1.0f;
        camX = 0.0f;
        camY = 0.0f;
    }
}

// Special keys (arrow keys) for panning
void special(int key, int, int) {
    float step = 20.0f / camZoom;
    if (key == GLUT_KEY_LEFT) camX += step;
    else if (key == GLUT_KEY_RIGHT) camX -= step;
    else if (key == GLUT_KEY_UP) camY -= step;
    else if (key == GLUT_KEY_DOWN) camY += step;
}

// Window reshape callback
void reshape(int w, int h) {
    winW = w;
    winH = h;
    if (h == 0) h = 1;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (float)w / (float)h;
    float view = 550.0f;

    glOrtho(-view * aspect, view * aspect, -view, view, -1, 1);

    glMatrixMode(GL_MODELVIEW);
}

// OpenGL initialization
void initGL() {
    glClearColor(0, 0, 0, 1);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    srand((unsigned int)time(NULL));

    initPlanets();
    initStars();
    initBelt();
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Scenario 2 — 2D Solar System (Beginner Friendly)");

    initGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();

    return 0;
}
