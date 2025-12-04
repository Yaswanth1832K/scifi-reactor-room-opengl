/*
* Computer Graphics
Sci-Fi Reactor Room with Workers, Control Machines, Ceiling LEDs, and Ceiling Wall
*/

#define NOMINMAX 
#include <windows.h>
#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <cstdint>
#include <cstring> 
#include <vector> 

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- Custom Matrix Functions ---
struct Matrix {
    float data[16];
};

std::vector<Matrix> matrixStack;

void custom_push_matrix() {
    Matrix currentMatrix;
    glGetFloatv(GL_MODELVIEW_MATRIX, currentMatrix.data);
    matrixStack.push_back(currentMatrix);
}

void custom_pop_matrix() {
    if (matrixStack.empty()) {
        std::cerr << "Error: Matrix stack underflow detected!" << std::endl;
        return;
    }
    const Matrix& poppedMatrix = matrixStack.back();
    glLoadMatrixf(poppedMatrix.data);
    matrixStack.pop_back();
}

void setIdentity(float M[16]) {
    memset(M, 0, 16 * sizeof(float));
    M[0] = M[5] = M[10] = M[15] = 1.0f;
}

void makeTranslationMatrix(float M[16], float x, float y, float z) {
    setIdentity(M);
    M[12] = x;
    M[13] = y;
    M[14] = z;
}

void makeRotationMatrix(float M[16], float angle, float x, float y, float z) {
    setIdentity(M);
    float rad = angle * (M_PI / 180.0f);
    float c = std::cos(rad);
    float s = std::sin(rad);
    float one_minus_c = 1.0f - c;

    float len = std::sqrt(x * x + y * y + z * z);
    if (len == 0.0f) return;
    x /= len;
    y /= len;
    z /= len;

    M[0] = x * x * one_minus_c + c;
    M[1] = y * x * one_minus_c + z * s;
    M[2] = z * x * one_minus_c - y * s;

    M[4] = x * y * one_minus_c - z * s;
    M[5] = y * y * one_minus_c + c;
    M[6] = z * y * one_minus_c + x * s;

    M[8] = x * z * one_minus_c + y * s;
    M[9] = y * z * one_minus_c - x * s;
    M[10] = z * z * one_minus_c + c;
}

void makeScalingMatrix(float M[16], float x, float y, float z) {
    setIdentity(M);
    M[0] = x;
    M[5] = y;
    M[10] = z;
}

void custom_translate(float x, float y, float z) {
    float T[16];
    makeTranslationMatrix(T, x, y, z);
    glMultMatrixf(T);
}

void custom_rotate(float angle, float x, float y, float z) {
    float R[16];
    makeRotationMatrix(R, angle, x, y, z);
    glMultMatrixf(R);
}

void custom_scale(float x, float y, float z) {
    float S[16];
    makeScalingMatrix(S, x, y, z);
    glMultMatrixf(S);
}
// --- End Custom Matrix Functions ---

int winW = 1200, winH = 800;
float t = 0.0f;
bool animateOn = true, wireframe = false, showFloorLights = true;
float speed = 0.1f;

// ---------------- Material Helpers ----------------
void setMaterial(const GLfloat diffuse[4], const GLfloat specular[4], GLfloat shininess) {
    GLfloat ambient[4] = { diffuse[0] * 0.12f, diffuse[1] * 0.12f, diffuse[2] * 0.12f, diffuse[3] };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

void setEmissive(const GLfloat color[3], float intensity) {
    GLfloat em[4] = { color[0] * intensity, color[1] * intensity, color[2] * intensity, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, em);
}

void clearEmissive() {
    GLfloat zero[4] = { 0,0,0,1 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero);
}

// ---------------- Geometry ----------------
void drawCube(float sx, float sy, float sz) {
    custom_push_matrix();
    custom_scale(sx, sy, sz);
    glutSolidCube(1.0);
    custom_pop_matrix();
}

void drawSphere(float r, int lats = 24, int longs = 24) {
    for (int i = 0; i <= lats; i++) {
        double lat0 = M_PI * (-0.5 + (double)(i - 1) / lats);
        double lat1 = M_PI * (-0.5 + (double)i / lats);
        double z0 = sin(lat0), zr0 = cos(lat0);
        double z1 = sin(lat1), zr1 = cos(lat1);
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= longs; j++) {
            double lng = 2 * M_PI * (double)(j - 1) / longs;
            double x = cos(lng), y = sin(lng);
            glNormal3d(x * zr0, y * zr0, z0);
            glVertex3d(r * x * zr0, r * y * zr0, r * z0);
            glNormal3d(x * zr1, y * zr1, z1);
            glVertex3d(r * x * zr1, r * y * zr1, r * z1);
        }
        glEnd();
    }
}

void drawTorus(float majorR, float minorR, int majorSteps = 48, int minorSteps = 12) {
    for (int i = 0; i < majorSteps; i++) {
        int ni = (i + 1) % majorSteps;
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= minorSteps; j++) {
            int jj = j % minorSteps;
            for (int k = 0; k < 2; k++) {
                int idx = (k == 0) ? i : ni;
                float u = (float)idx / majorSteps * 2 * M_PI;
                float v = (float)jj / minorSteps * 2 * M_PI;
                float cu = cos(u), su = sin(u), cv = cos(v), sv = sin(v);
                float x = (majorR + minorR * cv) * cu, y = (majorR + minorR * cv) * su, z = minorR * sv;
                glNormal3f(cv * cu, cv * su, sv); glVertex3f(x, y, z);
            }
        }
        glEnd();
    }
}

// ---------------- Scene Components ----------------
void drawCore() {
    GLfloat color[3] = { 0.0f,0.8f,1.0f };
    float pulse = 0.6f + 0.4f * sinf(t * 0.02f);
    setMaterial(color, color, 100.0f);
    setEmissive(color, pulse);
    drawSphere(2.0, 32, 32);
    clearEmissive();
}

void drawRings() {
    GLfloat ring1[4] = { 0.4f,0.7f,1.0f,1.0f }, ringSpec[4] = { 0.7f,0.9f,1.0f,1.0f };
    setMaterial(ring1, ringSpec, 60.0f);
    custom_push_matrix();
    custom_rotate(t * 2.0f, 0, 0, 1);
    drawTorus(3.5, 0.15, 64, 12);
    custom_pop_matrix();

    GLfloat ring2[4] = { 0.8f,0.3f,1.0f,1.0f };
    setMaterial(ring2, ringSpec, 60.0f);
    custom_push_matrix();
    custom_rotate(t * 3.0f, 0, 1, 0);
    drawTorus(5.0, 0.12, 64, 12);
    custom_pop_matrix();

    GLfloat ring3[4] = { 1.0f,0.6f,0.2f,1.0f };
    setMaterial(ring3, ringSpec, 40.0f);
    custom_push_matrix();
    custom_rotate(t * 4.0f, 1, 0, 0);
    drawTorus(6.5, 0.1, 64, 12);
    custom_pop_matrix();
}


void drawFloorLights() {
    if (!showFloorLights) return;
    glDisable(GL_LIGHTING);
    GLfloat col[4] = { 0.2f,1.0f,0.8f,1.0f }; glColor4fv(col);
    for (int i = -5; i <= 5; i++)
        for (int j = -5; j <= 5; j++) {
            custom_push_matrix();
            custom_translate(i * 2.5f, -5.4f, j * 2.5f);
            drawCube(0.2f, 0.05f, 0.2f);
            custom_pop_matrix();
        }
    glEnable(GL_LIGHTING);
}

void drawContainment() {
    for (int i = 0; i < 8; i++) {
        custom_push_matrix();
        custom_rotate(i * 45, 0, 0, 1);
        custom_translate(6.5, 0, 0);
        custom_scale(0.4f, 0.4f, 10.0f);
        GLfloat diff[4] = { 0.7f,0.7f,0.7f,1.0f }, spec[4] = { 0.3f,0.3f,0.3f,1.0f };
        setMaterial(diff, spec, 30.0f);
        drawCube(0.3f, 0.3f, 6.0f);
        custom_pop_matrix();
    }
    // Changed base color to match walls
    GLfloat baseDiff[4] = { 0.3f,0.3f,0.35f,1.0f }, baseSpec[4] = { 0.06f,0.06f,0.06f,1.0f };
    setMaterial(baseDiff, baseSpec, 5.0f);
    custom_push_matrix();
    custom_translate(0, -4.5f, 0);
    drawCube(16.0f, 1.0f, 16.0f);
    custom_pop_matrix();
}

void drawHolograms() {
    glDisable(GL_LIGHTING); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int i = 0; i < 6; i++) {
        custom_push_matrix();
        float ang = i * 60.0f + t * 0.006f;
        custom_rotate(ang, 0, 1, 0);
        custom_translate(0, 1.2f + sinf((t + i * 30) * 0.0002f) * 0.2f, 3.2f);
        custom_rotate(-30, 1, 0, 0);
        GLfloat col[4] = { 0.1f + 0.08f * i,0.8f,1.0f,0.35f };
        glColor4fv(col);
        glBegin(GL_QUADS);
        glVertex3f(-0.8f, 0, 0);
        glVertex3f(0.8f, 0, 0);
        glVertex3f(0.8f, 1.2f, 0); 
        glVertex3f(-0.8f, 1.2f, 0);
        glEnd();
        custom_pop_matrix();
    }
    glDisable(GL_BLEND); glEnable(GL_LIGHTING);
}

void drawParticles() {
    glDisable(GL_LIGHTING);
    GLfloat col[4] = { 0.2f,1.0f,1.0f,1.0f }; glColor4fv(col);
    for (int i = 0; i < 50; i++) {
        float angle = i * 360.0f / 50 + t * 0.012f;
        float radius = 2.5f + 0.2f * sinf(t * 0.005f + i);
        float height = 0.5f * sinf(t * 0.007f + i);

        float scale = 0.1f + 2.0f * sinf(t * 0.03f + i * 0.15f); 

        custom_push_matrix();
        custom_translate(radius * cosf(angle), height, radius * sinf(angle));
        custom_scale(scale, scale, scale); // Apply scaling to make particles grow and shrink
        glutSolidSphere(0.05f, 8, 8);
        custom_pop_matrix();
    }
    glEnable(GL_LIGHTING);
}

// ---------------- Humans ----------------
void drawOperator(float x, float z, float phase = 0.0f) {
    GLfloat body[4] = { 0.8f,0.3f,0.1f,1.0f }, head[4] = { 1.0f,0.8f,0.6f,1.0f }, limb[4] = { 0.7f,0.3f,0.1f,1.0f };
    float moveRadius = 1.0f; 
    float mx = x + moveRadius * sinf(t * 0.05f + phase);
    float mz = z + moveRadius * cosf(t * 0.05f + phase); 

    custom_push_matrix();
    custom_translate(mx, -3.5f, mz); 
    setMaterial(body, body, 10.0f);
    drawCube(0.4f, 1.2f, 0.3f);
    setMaterial(head, head, 10.0f); 
    custom_translate(0, 0.9f, 0);
    drawSphere(0.2f, 8, 8);
    setMaterial(limb, limb, 10.0f);

    custom_push_matrix();
    custom_translate(-0.35f, 0.3f, 0); 
    drawCube(0.1f, 0.6f, 0.1f);
    custom_pop_matrix();
    custom_push_matrix(); 
    custom_translate(0.35f, 0.3f, 0); 
    drawCube(0.1f, 0.6f, 0.1f);
    custom_pop_matrix();
    custom_push_matrix(); 
    custom_translate(-0.15f, -0.9f, 0);
    drawCube(0.1f, 0.8f, 0.1f);
    custom_pop_matrix();
    custom_push_matrix(); 
    custom_translate(0.15f, -0.9f, 0);
    drawCube(0.1f, 0.8f, 0.1f);
    custom_pop_matrix();
    custom_pop_matrix();
}

void drawPeople() {
    drawOperator(-5, 5, 0); drawOperator(5, 5, 1.0f); drawOperator(-5, -5, 2.0f); drawOperator(5, -5, 3.0f);
}

// ---------------- Machines ----------------
void drawMachine(float x, float z) {
    GLfloat diff[4] = { 0.2f,0.2f,0.5f,1.0f }, spec[4] = { 0.5f,0.5f,0.7f,1.0f };
    setMaterial(diff, spec, 30.0f);
    custom_push_matrix();
    custom_translate(x, -4.0f, z); 
    drawCube(1.0f, 1.0f, 0.5f);
    custom_pop_matrix();

    custom_push_matrix();
    custom_translate(x, -3.5f, z + 0.3f); 
    custom_rotate(-30, 1, 0, 0);
    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 1.0f, 0.5f);
    glBegin(GL_QUADS);
    glVertex3f(-0.4f, 0, 0);
    glVertex3f(0.4f, 0, 0);
    glVertex3f(0.4f, 0.6f, 0);
    glVertex3f(-0.4f, 0.6f, 0);
    glEnd();
    glEnable(GL_LIGHTING);
    custom_pop_matrix();
}

void drawMachinesAndOperators() {
    drawMachine(-3, 7);
    drawOperator(-3, 7, 0.5f);
    drawMachine(3, 7); 
    drawOperator(3, 7, 1.5f);
    drawMachine(-3, -7);
    drawOperator(-3, -7, 2.5f);
    drawMachine(3, -7);
    drawOperator(3, -7, 3.5f);
}

// ---------------- Floor ----------------
void drawFloor() {
    // Floor color remains same as walls
    GLfloat floorCol[4] = { 0.3f, 0.3f, 0.35f, 1.0f };
    GLfloat floorSpec[4] = { 0.05f, 0.05f, 0.05f, 1.0f };
    setMaterial(floorCol, floorSpec, 5.0f);

    custom_push_matrix();
    custom_translate(0, -5.5f, 0);
    drawCube(30.5f, 0.2f, 30.5f); 
    custom_pop_matrix();
}

// ---------------- Walls ----------------
void drawWalls() {
    GLfloat wallCol[4] = { 0.3f, 0.3f, 0.35f, 1.0f };
    GLfloat wallSpec[4] = { 0.2f, 0.2f, 0.25f, 1.0f };

    // Back wall
    setMaterial(wallCol, wallSpec, 5.0f);
    custom_push_matrix();
    custom_translate(0, 5, -15);
    drawCube(30.5f, 10.2f, 0.5f); 
    custom_pop_matrix();

    // Left wall
    setMaterial(wallCol, wallSpec, 5.0f);
    custom_push_matrix();
    custom_translate(-15, 5, 0);
    drawCube(0.5f, 10.2f, 30.5f); 
    custom_pop_matrix();

    // Ceiling wall
    setMaterial(wallCol, wallSpec, 5.0f);
    custom_push_matrix();
    custom_translate(0, 10.0f, 0);
    drawCube(30.5f, 0.5f, 30.5f); 
    custom_pop_matrix();
}


// ---------------- Ceiling LEDs ----------------
void drawCeilingLights() {
    glDisable(GL_LIGHTING);
    GLfloat ledColor[3] = { 1.0f, 1.0f, 0.6f };
    int rows = 6, cols = 6;
    float spacingX = 4.0f, spacingZ = 4.0f;
    float ceilingY = 9.75f; 

    for (int i = -rows / 2; i <= rows / 2; i++) {
        for (int j = -cols / 2; j <= cols / 2; j++) {
            custom_push_matrix();
            custom_translate(i * spacingX, ceilingY, j * spacingZ);
            float pulse = 0.5f + 0.5f * sinf(t * 0.05f + i + j);
            setEmissive(ledColor, pulse);
            drawCube(0.3f, 0.05f, 0.3f);
            clearEmissive();
            custom_pop_matrix();
        }
    }
    glEnable(GL_LIGHTING);
}

// ---------------- Lighting ----------------
void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); 
    glEnable(GL_LIGHT1);
    GLfloat ambient[] = { 0.2f,0.2f,0.25f,1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    GLfloat pos0[] = { 0,0,0,1 }, diff0[] = { 0.9f,0.9f,1.0f,1.0f }, spec0[] = { 1.0f,1.0f,1.2f,1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, pos0); 
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff0); 
    glLightfv(GL_LIGHT0, GL_SPECULAR, spec0);
    GLfloat pos1[] = { -20,20,40,1 }, diff1[] = { 0.6f,0.6f,0.8f,1 }, spec1[] = { 0.6f,0.6f,0.7f,1 };
    glLightfv(GL_LIGHT1, GL_POSITION, pos1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diff1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, spec1);
}

// ---------------- Render ----------------
void renderScene() {
    drawFloor();
    drawFloorLights(); 
    drawContainment();
    drawCore(); 
    drawRings(); 
    drawHolograms(); 
    drawParticles();
    drawWalls(); 
    drawCeilingLights(); 
    drawPeople();
    drawMachinesAndOperators();
}

// ---------------- GLUT ----------------
void display() {
    glClearColor(0.05f, 0.05f, 0.08f, 1); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); 
    glLoadIdentity();
    float camX = 18.0f * cosf(t * 0.005f), camZ = 24.0f * sinf(t * 0.005f);
    gluLookAt(camX, 8.0f, camZ, 0, 0, 0, 0, 1, 0);
    renderScene();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    winW = w; winH = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (float)h, 0.1, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

void idle() {
    if (animateOn) t += speed;
    if (t > 360.0f) t -= 360.0f;
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27: exit(0); break;
    case ' ': animateOn = !animateOn; break;
    case 'v': wireframe = !wireframe; 
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL); break;
    case '+': speed += 0.05f; break;
    case '-': speed -= 0.05f; if (speed < 0.01f) speed = 0.01f; break;
    case 'l': showFloorLights = !showFloorLights; break;
    }
}

void initGL() {
    glEnable(GL_DEPTH_TEST); 
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH); 
    setupLighting();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv); 
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winW, winH); 
    glutCreateWindow("Reactor Room with Workers, Machines");
    initGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard); 
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}
