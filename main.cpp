
#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

int winW = 800, winH = 600;
int currentSeason = 0;

float timeOfDay = 0.0f; // 0.0 to 1.0 (0=dawn, 0.5=dusk, 1.0=next dawn)
float timeSpeed = 0.001f;
bool isRaining = false;
bool isSnowing = false;
bool showLabels = true;

// Structure for particles (rain and snow)
struct Particle {
    float x, y;
    float speed;
    float size;
};

vector<Particle> raindrops;
vector<Particle> snowflakes;

// Structure for clouds
struct Cloud {
    float x, y;
    float size;
    float speed;
};

vector<Cloud> clouds;

// Human positions with gender
struct Human {
    float x, y;
    float direction; // -1 for left, 1 for right
    char gender; // 'M' or 'F'
};

const int MAX_HUMANS = 4; // 2 boys and 2 girls
Human humans[MAX_HUMANS];

// Updated Structure for birds
struct Bird {
    float x, y;
    float speed;
    float phase; // Renamed from flapPhase
};

const int MAX_BIRDS = 7;
Bird birds[MAX_BIRDS];

// Function prototypes
void initGL();
void display();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void update(int value);
void initBirds();
void updateBirds();
void initHumans();
void updateHumans();

void initClouds() {
    clouds.clear();
    for (int i = 0; i < 5; ++i) {
        Cloud c;
        c.x = rand() % 800 - 400;
        c.y = rand() % 100 + 150;
        c.size = 30 + rand() % 40;
        c.speed = 0.1f + (rand() % 20) / 100.0f;
        clouds.push_back(c);
    }
}

void initParticles() {
    raindrops.clear();
    for (int i = 0; i < 200; ++i) {
        Particle p;
        p.x = rand() % 800 - 400;
        p.y = rand() % 600 - 300;
        p.speed = 2.0f + (rand() % 10);
        p.size = 1.0f + (rand() % 3);
        raindrops.push_back(p);
    }

    snowflakes.clear();
    for (int i = 0; i < 100; ++i) {
        Particle p;
        p.x = rand() % 800 - 400;
        p.y = rand() % 600 - 300;
        p.speed = 0.5f + (rand() % 5) / 10.0f;
        p.size = 1.0f + (rand() % 3);
        snowflakes.push_back(p);
    }
}

void initBirds() {
    for (int i = 0; i < MAX_BIRDS; ++i) {
        birds[i].x = rand() % 800 - 400;
        birds[i].y = 100 + rand() % 50;
        birds[i].phase = (rand() % 100) / 10.0f;
        birds[i].speed = 0.5f + (rand() % 100) / 100.0f;
    }
}

void initHumans() {
    // 2 boys and 2 girls with a better starting y-coordinate
    humans[0] = {-300.0f, -225.0f, 1.0f, 'M'};
    humans[1] = { -200.0f, -225.0f, 1.0f, 'F'};
    humans[2] = {100.0f, -225.0f, -1.0f, 'M'};
    humans[3] = {300.0f, -225.0f, -1.0f, 'F'};
}

void drawRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
}

void drawCircle(float cx, float cy, float r, int segments = 30) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        glVertex2f(cx + cosf(angle) * r, cy + sinf(angle) * r);
    }
    glEnd();
}

void drawSun() {
    float sunPos = timeOfDay * 2.0f * M_PI;
    float sunX = cosf(sunPos) * 350;
    float sunY = sinf(sunPos) * 200 + 150;

    float intensity = 1.0f - abs(timeOfDay - 0.5f) * 1.5f;
    if (intensity < 0) intensity = 0;

    glColor3f(1.0f, 0.6f * intensity, 0.2f * intensity);
    drawCircle(sunX, sunY, 35);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 0.5f, 0.0f, 0.3f * intensity);
    drawCircle(sunX, sunY, 50);
    glDisable(GL_BLEND);
}

void drawMoon() {
    float moonPos = timeOfDay * 2.0f * M_PI + M_PI;
    float moonX = cosf(moonPos) * 350;
    float moonY = sinf(moonPos) * 200 + 150;

    float visibility = 1.0f - abs(timeOfDay - 0.5f) * 1.5f;
    if (visibility < 0.2f) visibility = 0.2f;

    glColor3f(0.8f * visibility, 0.8f * visibility, 0.8f * visibility);
    drawCircle(moonX, moonY, 25);

    if (visibility > 0.5f) {
        glColor3f(0.6f * visibility, 0.6f * visibility, 0.6f * visibility);
        drawCircle(moonX - 5, moonY + 5, 4);
        drawCircle(moonX + 8, moonY - 3, 6);
        drawCircle(moonX + 3, moonY + 8, 3);
    }

    if (visibility > 0.7f) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.9f, 0.9f, 0.9f, 0.2f * (visibility - 0.7f) * 3.0f);
        drawCircle(moonX, moonY, 35);
        glDisable(GL_BLEND);
    }
}

void drawStars() {
    float nightIntensity = abs(timeOfDay - 0.5f) * 2.0f;
    if (nightIntensity > 1.0f) nightIntensity = 2.0f - nightIntensity;

    if (nightIntensity > 0.7f) {
        glColor3f(1.0f, 1.0f, 1.0f);
        srand(0);
        for (int i = 0; i < 100; ++i) {
            float x = rand() % 800 - 400;
            float y = rand() % 300 + 50;
            float size = 0.5f + (rand() % 5) / 10.0f;
            float brightness = 0.5f + (rand() % 5) / 10.0f;
            glColor3f(brightness, brightness, brightness);
            drawCircle(x, y, size);
        }
    }
}

void drawTree(float x, float y) {
    glColor3f(0.55f, 0.27f, 0.07f);
    drawRect(x, y, 20, 60);

    if (currentSeason == 3 && !isSnowing) return;

    if (currentSeason == 0) glColor3f(0.3f, 0.8f, 0.3f);
    else if (currentSeason == 1) glColor3f(0.0f, 0.6f, 0.2f);
    else if (currentSeason == 2) glColor3f(0.8f, 0.4f, 0.1f);
    else glColor3f(0.8f, 0.9f, 1.0f);

    drawCircle(x + 10, y + 70, 30);
    drawCircle(x - 10, y + 60, 25);
    drawCircle(x + 30, y + 60, 25);
}

void drawBench(float x, float y) {
    glColor3f(0.6f, 0.3f, 0.1f);
    drawRect(x, y, 80, 10);
    drawRect(x, y + 10, 80, 10);
    drawRect(x + 5, y - 20, 5, 20);
    drawRect(x + 70, y - 20, 5, 20);
}

void drawLamppost(float x, float y) {
    glColor3f(0.3f, 0.3f, 0.3f);
    drawRect(x, y, 10, 80);

    float lightIntensity = 0.0f;
    if (timeOfDay < 0.25f || timeOfDay > 0.75f) {
        lightIntensity = 1.0f;
    } else if (timeOfDay < 0.3f || timeOfDay > 0.7f) {
        lightIntensity = 0.5f;
    }

    if (lightIntensity > 0) {
        glColor3f(1.0f, 1.0f, 0.8f * lightIntensity);
        drawRect(x - 5, y + 80, 20, 15);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 0.5f, 0.3f * lightIntensity);
        drawCircle(x, y + 70, 30);
        glDisable(GL_BLEND);
    }
}

void drawCloud(Cloud &c, bool isDark) {
    if (isDark) {
        glColor3f(0.3f, 0.3f, 0.4f);
    } else {
        glColor3f(0.9f, 0.9f, 1.0f);
    }

    drawCircle(c.x, c.y, c.size);
    drawCircle(c.x + c.size * 0.7f, c.y, c.size * 0.8f);
    drawCircle(c.x - c.size * 0.7f, c.y, c.size * 0.8f);
    drawCircle(c.x, c.y + c.size * 0.5f, c.size * 0.7f);
}

void drawRain() {
    glColor3f(0.5f, 0.5f, 1.0f);
    glBegin(GL_LINES);
    for (size_t i = 0; i < raindrops.size(); ++i) {
        glVertex2f(raindrops[i].x, raindrops[i].y);
        glVertex2f(raindrops[i].x - 2.0f, raindrops[i].y - 5.0f);
    }
    glEnd();
}

void drawSnow() {
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < snowflakes.size(); ++i) {
        glVertex2f(snowflakes[i].x, snowflakes[i].y);
    }
    glEnd();
}

void drawHuman(Human& h) {
    // Head
    glColor3f(1.0f, 0.8f, 0.6f);
    drawCircle(h.x, h.y + 15, 8);

    // Body
    if (isRaining) {
        glColor3f(0.2f, 0.2f, 0.8f);
    } else {
        if (h.gender == 'M') {
            glColor3f(0.8f, 0.2f, 0.2f);
        } else {
            glColor3f(0.8f, 0.2f, 0.8f);
        }
    }
    drawRect(h.x - 5, h.y - 5, 10, 15);

    // Hair
    glColor3f(0.3f, 0.1f, 0.0f);
    if (h.gender == 'F') {
         drawCircle(h.x, h.y + 15, 10);
    } else {
        drawRect(h.x - 5, h.y + 15, 10, 5);
    }

    // Legs
    glColor3f(0.2f, 0.2f, 0.2f);
    drawRect(h.x - 6, h.y - 25, 4, 20);
    drawRect(h.x + 2, h.y - 25, 4, 20);

    // Arms
    float armAngle = sinf(glutGet(GLUT_ELAPSED_TIME) * 0.01f) * 0.3f;
    glColor3f(1.0f, 0.8f, 0.6f);

    glPushMatrix();
    glTranslatef(h.x + 7, h.y + 5, 0);
    glRotatef(armAngle * 30 * h.direction, 0, 0, 1);
    drawRect(0, -3, 12 * h.direction, 4);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(h.x - 7, h.y + 5, 0);
    glRotatef(-armAngle * 30 * h.direction, 0, 0, 1);
    drawRect(-12 * h.direction, -3, 12 * h.direction, 4);
    glPopMatrix();

    // Umbrella
    if (isRaining) {
        glColor3f(0.8f, 0.1f, 0.1f);
        drawCircle(h.x, h.y + 30, 15);
        glColor3f(0.6f, 0.6f, 0.6f);
        drawRect(h.x - 1, h.y + 10, 2, 20);
    }
}

void drawBird(Bird& b) {
    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix();
    glTranslatef(b.x, b.y, 0);

    glBegin(GL_TRIANGLES);
    glVertex2f(0, 0);
    glVertex2f(-15, 5);
    glVertex2f(-15, -5);
    glEnd();

    float wingFlap = sinf(b.phase) * 20.0f;

    glPushMatrix();
    glTranslatef(-10, 2, 0);
    glRotatef(wingFlap, 0, 0, 1);
    glBegin(GL_TRIANGLES);
    glVertex2f(0, 0);
    glVertex2f(-15, 8);
    glVertex2f(-10, 0);
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-10, -2, 0);
    glRotatef(-wingFlap, 0, 0, 1);
    glBegin(GL_TRIANGLES);
    glVertex2f(0, 0);
    glVertex2f(-15, -8);
    glVertex2f(-10, 0);
    glEnd();
    glPopMatrix();

    glPopMatrix();
}

void drawHouse(float x, float y) {
    glColor3f(0.9f, 0.7f, 0.5f);
    drawRect(x, y, 100, 80);

    glColor3f(0.6f, 0.2f, 0.2f);
    drawTriangle(x - 10, y + 80, x + 110, y + 80, x + 50, y + 130);

    glColor3f(0.4f, 0.2f, 0.0f);
    drawRect(x + 40, y, 20, 40);

    glColor3f(0.7f, 0.8f, 0.9f);
    drawRect(x + 15, y + 30, 25, 25);
    drawRect(x + 60, y + 30, 25, 25);
}

void drawGroundAndRoad() {
    // Grassy area
    if (isSnowing) {
        glColor3f(0.95f, 0.95f, 1.0f);
    } else if (currentSeason == 0) {
        glColor3f(0.4f, 0.9f, 0.4f);
    } else if (currentSeason == 1) {
        glColor3f(0.2f, 0.8f, 0.2f);
    } else if (currentSeason == 2) {
        glColor3f(0.8f, 0.5f, 0.2f);
    } else {
        glColor3f(0.7f, 0.7f, 0.7f);
    }
    drawRect(-400, -300, 800, 150);

    // Zigzag Road
    if (isSnowing) {
        glColor3f(0.95f, 0.95f, 1.0f);
    } else {
        glColor3f(0.4f, 0.4f, 0.4f);
    }

    glBegin(GL_TRIANGLE_STRIP);
    for (float x = -400; x <= 400; x += 1.0f) {
        float y = -225 + 20 * sinf(x * 0.02f);
        glVertex2f(x, y + 15); // Widened from 10 to 15
        glVertex2f(x, y - 15); // Widened from -10 to -15
    }
    glEnd();

    // Lane markers
    if (!isSnowing) {
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINES);
        for (float x = -350; x < 350; x += 50) {
            float y = -225 + 20 * sinf(x * 0.02f);
            glVertex2f(x, y);
            glVertex2f(x + 20, y);
        }
        glEnd();
    }
}

void drawSky() {
    float skyR, skyG, skyB;

    if (timeOfDay < 0.25f) {
        float t = timeOfDay * 4.0f;
        skyR = 0.2f + t * 0.1f;
        skyG = 0.4f + t * 0.3f;
        skyB = 0.6f + t * 0.2f;
    } else if (timeOfDay < 0.75f) {
        skyR = 0.4f;
        skyG = 0.7f;
        skyB = 0.9f;
    } else {
        float t = (timeOfDay - 0.75f) * 4.0f;
        skyR = 0.4f - t * 0.2f;
        skyG = 0.7f - t * 0.3f;
        skyB = 0.9f - t * 0.3f;
    }

    glColor3f(skyR, skyG, skyB);
    drawRect(-400, -150, 800, 450);
}

void drawLabels() {
    if (!showLabels) return;

    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(-390, 280);

    string label = "Time: ";
    if (timeOfDay < 0.25f) label += "Dawn";
    else if (timeOfDay < 0.5f) label += "Day";
    else if (timeOfDay < 0.75f) label += "Evening";
    else label += "Night";

    if (isRaining) label += " | Rain";
    if (isSnowing) label += " | Snow";

    label += " | Season: ";
    if (currentSeason == 0) label += "Spring";
    else if (currentSeason == 1) label += "Summer";
    else if (currentSeason == 2) label += "Autumn";
    else label += "Winter";

    for (size_t i = 0; i < label.size(); ++i) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, label[i]);
    }
}

void updateParticles() {
    if (isRaining) {
        for (size_t i = 0; i < raindrops.size(); ++i) {
            raindrops[i].y -= raindrops[i].speed;
            if (raindrops[i].y < -300) {
                raindrops[i].y = 300;
                raindrops[i].x = rand() % 800 - 400;
            }
        }
    }

    if (isSnowing) {
        for (size_t i = 0; i < snowflakes.size(); ++i) {
            snowflakes[i].y -= snowflakes[i].speed;
            snowflakes[i].x += sinf(glutGet(GLUT_ELAPSED_TIME) * 0.001f + i) * 0.5f;
            if (snowflakes[i].y < -300) {
                snowflakes[i].y = 300;
                snowflakes[i].x = rand() % 800 - 400;
            }
        }
    }
}

void updateClouds() {
    for (size_t i = 0; i < clouds.size(); ++i) {
        clouds[i].x += clouds[i].speed;
        if (clouds[i].x > 450) {
            clouds[i].x = -450;
            clouds[i].y = rand() % 100 + 150;
        }
    }
}

void updateHumans() {
    for (int i = 0; i < MAX_HUMANS; ++i) {
        humans[i].x += humans[i].direction * 0.5f;
        humans[i].y = -225 + 20 * sinf(humans[i].x * 0.02f);

        if (humans[i].x > 350 || humans[i].x < -350) humans[i].direction *= -1;
    }
}

void updateBirds() {
    for (int i = 0; i < MAX_BIRDS; ++i) {
        birds[i].x += birds[i].speed;
        birds[i].phase += 0.1f;

        if (birds[i].x > 400) {
            birds[i].x = -400;
            birds[i].y = 100 + rand() % 50;
            birds[i].speed = 0.5f + (rand() % 100) / 100.0f;
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawSky();
    drawStars();
    drawSun();
    drawMoon();
    drawGroundAndRoad();

    for (size_t i = 0; i < clouds.size(); ++i) {
        drawCloud(clouds[i], isRaining);
    }

    for (int i = 0; i < MAX_BIRDS; ++i) {
        drawBird(birds[i]);
    }

    drawHouse(-300, -150);
    drawTree(-200, -150);
    drawTree(250, -150);
    drawTree(300, -150);
    drawTree(-350, -150);
    drawBench(-40, -150);
    drawBench(200, -150);
    drawLamppost(-150, -150);
    drawLamppost(150, -150);
    drawLamppost(0, -150);

    for (int i = 0; i < MAX_HUMANS; ++i) {
        drawHuman(humans[i]);
    }

    if (isRaining) drawRain();
    if (isSnowing) drawSnow();

    drawLabels();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-400, 400, -300, 300);
    glMatrixMode(GL_MODELVIEW);
}

void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    srand(time(NULL));
    initClouds();
    initParticles();
    initBirds();
    initHumans();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        currentSeason = (currentSeason + 1) % 4;
        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'r': case 'R':
            isRaining = !isRaining;
            isSnowing = false;
            break;
        case 's': case 'S':
            isSnowing = !isSnowing;
            isRaining = false;
            break;
        case '+':
            timeSpeed *= 1.5f;
            break;
        case '-':
            timeSpeed /= 1.5f;
            break;
        case '0':
            timeOfDay = 0.0f;
            timeSpeed = 0.001f;
            isRaining = false;
            isSnowing = false;
            break;
        case 'l': case 'L':
            showLabels = !showLabels;
            break;
    }
    glutPostRedisplay();
}

void update(int value) {
    timeOfDay += timeSpeed;
    if (timeOfDay > 1.0f) timeOfDay -= 1.0f;

    updateParticles();
    updateClouds();
    updateHumans();
    updateBirds();

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Enhanced 2D Park with Day/Night Cycle and Weather");
    initGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, update, 0);
    glutMainLoop();
    return 0;
}
