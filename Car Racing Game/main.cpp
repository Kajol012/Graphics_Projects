#include <GL/glut.h>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <math.h>

using namespace std;

int isMoving = 1;
float _move = 0.0f;
float roadOffset = 0.0f;
float yellowLineOffset = 0.0f;
int timeLeft = 90;
bool gameRunning = true;
float speed = 0.009f;
float WORLD_SPEED = 1.2f;

bool isNight = false;

int currentPhase = 1;
bool waitingForNextPhase = false;
bool waitingToStart = true;
int phaseTime = 0;

float startLineY = 5.0f;
float finishLineY = 8.0f;
bool showStartLine = true;
bool showFinishLine = false;

float boosterY = 5.0f;
float boosterX = 0.0f;
bool boosterActive = false;
int boosterTimer = 0;
const int BOOSTER_DURATION = 5000;
float boosterFallSpeed = 0.03f;
bool boosterCollected = false;
const float MAX_SPEED = 0.05f;

float opponentY = 6.0f;
float opponentX = 0.0f;
float opponentFallSpeed = 0.05f;
bool carCrashed = false;

bool staticObjectActive = false;
float staticX = 0.0f;
float staticY = 4.5f;
const float STATIC_FALL_SPEED = 0.03f;
int staticSpawnTimer = 0;
bool hitStatic = false;

bool isRaining = false;
#define MAX_RAIN 100
float rainX[MAX_RAIN];
float rainY[MAX_RAIN];

float LEFT_LIMIT = -1.0f;
float RIGHT_LIMIT = 1.0f;


void drawText(const char* text, float x, float y);
void resetGame();
void init();
void drawHouse(float x, float y);
void drawTree(float x, float y);
void drawHill();
void drawHillGround();
void drawHillTriangles();
void drawFieldLeft();
void drawFieldRight();
void drawRoad();
void drawRoadSideColor();
void drawPowerBooster();
void drawHeadlightBeam();
void drawRain();
void drawMoon();
void drawPhaseLines();
void update(int value);
void timerUpdate(int value);
void keyboard(unsigned char key, int x, int y);
void display();


void resetGame() {
    _move = 0.0f;
    opponentY = 6.0f;
    opponentX = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
    timeLeft = 90;
    gameRunning = true;
    carCrashed = false;
    hitStatic = false;
    isMoving = 1;
    speed = 0.009f;
    boosterActive = false;
    staticObjectActive = false;
    currentPhase = 1;
    waitingForNextPhase = false;
    waitingToStart = true;
    phaseTime = 0;
    startLineY = 5.0f;
    finishLineY = 8.0f;
    showStartLine = true;
    showFinishLine = false;
}

void init() {
    if (isNight) glClearColor(0.02f, 0.02f, 0.1f, 1.0f);
    else glClearColor(0.78f, 0.88f, 0.90f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-4, 4, -5, 5);
    srand(time(NULL));
    for (int i = 0; i < MAX_RAIN; i++) {
        rainX[i] = (float)(rand() % 800 - 400) / 100.0f;
        rainY[i] = (float)(rand() % 1000 - 500) / 100.0f;
    }
}

void drawText(const char* text, float x, float y) {
    glRasterPos2f(x, y);
    for (int i = 0; text[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
}

void resetSpeed(int value) {
    speed /= 2.0f;
    if (speed < 0.009f) speed = 0.009f;
}

void spawnPowerBooster() {
    boosterX = ((float)rand() / RAND_MAX) * (RIGHT_LIMIT - LEFT_LIMIT) + LEFT_LIMIT;
    boosterY = 5.0f;
    boosterActive = true;
    boosterCollected = false;
    boosterTimer = 0;
}

void checkBoosterCollision() {
    if (!boosterActive || boosterCollected) return;
    float scale = 0.32f * 0.42f;
    float carLeft = _move - 1.0f * scale;
    float carRight = _move + 1.0f * scale;
    float carTop = -3.5f + 1.2f * scale;
    float carBottom = -3.5f - 1.0f * scale;
    float boosterLeft = boosterX - 0.2f;
    float boosterRight = boosterX + 0.2f;
    float boosterTop = boosterY + 0.2f;
    float boosterBottom = boosterY - 0.2f;

    if (carRight > boosterLeft && carLeft < boosterRight && carTop > boosterBottom && carBottom < boosterTop) {
        boosterCollected = true;
        boosterActive = false;
        speed *= 2.0f;
        if (speed > MAX_SPEED) speed = MAX_SPEED;
        glutTimerFunc(3000, resetSpeed, 0);
    }
}

void drawPhaseLines() {
    if (showStartLine) {
        float checkSize = 0.3f;
        int cols = 10;
        float startX = -1.5f;
        for (int i = 0; i < cols; i++) {
            for (int j = 0; j < 2; j++) {
                if ((i + j) % 2 == 0) glColor3f(1.0f, 1.0f, 1.0f); else glColor3f(0.0f, 0.0f, 0.0f);
                glBegin(GL_QUADS);
                glVertex2f(startX + i * checkSize, startLineY + j * 0.2f);
                glVertex2f(startX + (i + 1) * checkSize, startLineY + j * 0.2f);
                glVertex2f(startX + (i + 1) * checkSize, startLineY + (j + 1) * 0.2f);
                glVertex2f(startX + i * checkSize, startLineY + (j + 1) * 0.2f);
                glEnd();
            }
        }
    }
    if (showFinishLine) {
        float flagSize = 0.25f;
        int cols = 12;
        float startX = -1.5f;
        for (int i = 0; i < cols; i++) {
            if (i % 2 == 0) glColor3f(1.0f, 1.0f, 1.0f); else glColor3f(0.0f, 0.0f, 0.0f);
            glBegin(GL_QUADS);
            glVertex2f(startX + i * flagSize, finishLineY);
            glVertex2f(startX + (i + 1) * flagSize, finishLineY);
            glVertex2f(startX + (i + 1) * flagSize, finishLineY + 0.4f);
            glVertex2f(startX + i * flagSize, finishLineY + 0.4f);
            glEnd();
        }
    }
}

void drawHill() { if (isNight) glColor3f(0.05f, 0.2f, 0.05f); else glColor3f(0.25f, 0.65f, 0.60f); glBegin(GL_QUADS); glVertex2f(4.3f, 3.0f); glVertex2f(4.0f, 4.0f); glVertex2f(3.3f, 3.4f); glVertex2f(3.3f, 3.0f); glEnd(); }
void drawHillGround() { if (isNight) glColor3f(0.05f, 0.2f, 0.05f); else glColor3f(0.25f, 0.65f, 0.60f); glBegin(GL_QUADS); glVertex2f(-4.0f, 3.0f); glVertex2f(4.0f, 3.0f); glVertex2f(4.0f, 3.4f); glVertex2f(-4.0f, 3.4f); glEnd(); }
void drawHillTriangles() { if (isNight) glColor3f(0.05f, 0.2f, 0.05f); else glColor3f(0.25f, 0.65f, 0.60f); glBegin(GL_TRIANGLES); glVertex2f(3.3f, 3.4f); glVertex2f(2.5f, 4.3f); glVertex2f(1.7f, 3.4f); glVertex2f(1.7f, 3.4f); glVertex2f(1.0f, 4.0f); glVertex2f(0.3f, 3.4f); glVertex2f(0.3f, 3.4f); glVertex2f(-0.5f, 4.3f); glVertex2f(-1.3f, 3.4f); glVertex2f(-1.3f, 3.4f); glVertex2f(-2.0f, 4.0f); glVertex2f(-2.7f, 3.4f); glVertex2f(-2.7f, 3.4f); glVertex2f(-3.3f, 3.7f); glVertex2f(-4.0f, 3.4f); glEnd(); }
void drawFieldLeft() { if (isNight) glColor3f(0.05f, 0.15f, 0.05f); else glColor3f(0.45f, 0.75f, 0.35f); glBegin(GL_QUADS); glVertex2f(-1.7f, -5.0f); glVertex2f(-4.0f, -5.0f); glVertex2f(-4.0f, 3.0f); glVertex2f(-1.7f, 3.0f); glEnd(); }
void drawFieldRight() { if (isNight) glColor3f(0.05f, 0.15f, 0.05f); else glColor3f(0.45f, 0.75f, 0.35f); glBegin(GL_QUADS); glVertex2f(1.7f, -5.0f); glVertex2f(4.0f, -5.0f); glVertex2f(4.0f, 3.0f); glVertex2f(1.7f, 3.0f); glEnd(); }

void drawHouse(float x, float y) {
    if (isNight) glColor3f(0.3f, 0.3f, 0.4f); else glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + 0.6f, y); glVertex2f(x + 0.6f, y + 0.5f); glVertex2f(x, y + 0.5f);
    glEnd();
    glColor3f(0.4f, 0.2f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(x + 0.2f, y); glVertex2f(x + 0.4f, y); glVertex2f(x + 0.4f, y + 0.3f); glVertex2f(x + 0.2f, y + 0.3f);
    glEnd();
    glColor3f(0.6f, 0.1f, 0.1f);
    glBegin(GL_TRIANGLES);
        glVertex2f(x - 0.1f, y + 0.5f); glVertex2f(x + 0.7f, y + 0.5f); glVertex2f(x + 0.3f, y + 0.8f);
    glEnd();
}

void drawTree(float x, float y) {
    glColor3f(0.4f, 0.2f, 0.0f);
    glBegin(GL_QUADS); glVertex2f(x-0.05f, y); glVertex2f(x+0.05f, y); glVertex2f(x+0.05f, y+0.4f); glVertex2f(x-0.05f, y+0.4f); glEnd();
    if (isNight) glColor3f(0.0f, 0.4f, 0.05f); else glColor3f(0, 0.5f, 0);
    glBegin(GL_TRIANGLES);
        glVertex2f(x-0.25f, y+0.3f); glVertex2f(x+0.25f, y+0.3f); glVertex2f(x, y+0.6f);
        glVertex2f(x-0.2f, y+0.45f); glVertex2f(x+0.2f, y+0.45f); glVertex2f(x, y+0.75f);
        glVertex2f(x-0.15f, y+0.6f); glVertex2f(x+0.15f, y+0.6f); glVertex2f(x, y+0.85f);
    glEnd();
}

void drawRoad() {
    if (isNight) glColor3f(0.05f, 0.05f, 0.05f); else glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_POLYGON); glVertex2f(-1.5f, -5.0f); glVertex2f(1.5f, -5.0f); glVertex2f(1.5f, 5.0f); glVertex2f(-1.5f, 5.0f); glEnd();
    glColor3f(1.0f, 1.0f, 1.0f); glLineWidth(2.0f);
    for(float y = -5.0f + roadOffset; y < 5.0f; y += 0.8f) { glBegin(GL_LINES); glVertex2f(0.0f, y); glVertex2f(0.0f, y + 0.4f); glEnd(); }
}

void drawRoadSideColor() {
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS); glVertex2f(-1.5f, -5.0f); glVertex2f(-1.7f, -5.0f); glVertex2f(-1.7f, 5.0f); glVertex2f(-1.5f, 5.0f); glVertex2f(1.5f, -5.0f); glVertex2f(1.7f, -5.0f); glVertex2f(1.7f, 5.0f); glVertex2f(1.5f, 5.0f); glEnd();
    if (isNight) glColor3f(0.5f, 0.4f, 0.0f); else glColor3f(0.8f, 0.8f, 0.0f);
    for(float y = -5.0f + yellowLineOffset; y < 5.0f; y += 1.5f) {
        glBegin(GL_QUADS); glVertex2f(-1.5f, y); glVertex2f(-1.7f, y); glVertex2f(-1.7f, y + 0.5f); glVertex2f(-1.5f, y + 0.5f); glEnd();
        glBegin(GL_QUADS); glVertex2f(1.5f, y); glVertex2f(1.7f, y); glVertex2f(1.7f, y + 0.5f); glVertex2f(1.5f, y + 0.5f); glEnd();
    }
}

void drawMoon() { if (!isNight) return; glColor3f(0.9f, 0.9f, 0.8f); glBegin(GL_POLYGON); for(int i = 0; i < 360; i++) { float theta = i * 3.14159 / 180; glVertex2f(3.0f + 0.3f * cos(theta), 4.0f + 0.3f * sin(theta)); } glEnd(); }

void drawCarBody(float r, float g, float b) {
    glPushMatrix(); glScalef(0.32f, 0.42f, 1.0f); glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS); glVertex2f(-0.85f, 0.5f); glVertex2f(-1.0f, 0.5f); glVertex2f(-1.0f, 0.9f); glVertex2f(-0.85f, 0.9f); glVertex2f(0.85f, 0.5f); glVertex2f(1.0f, 0.5f); glVertex2f(1.0f, 0.9f); glVertex2f(0.85f, 0.9f); glVertex2f(-0.85f, -0.9f); glVertex2f(-1.0f, -0.9f); glVertex2f(-1.0f, -0.5f); glVertex2f(-0.85f, -0.5f); glVertex2f(0.85f, -0.9f); glVertex2f(1.0f, -0.9f); glVertex2f(1.0f, -0.5f); glVertex2f(0.85f, -0.5f); glEnd();
    glBegin(GL_POLYGON); glColor3f(r-0.2f, g-0.2f, b-0.2f); glVertex2f(-0.8f, -1.0f); glVertex2f(0.8f, -1.0f); glColor3f(r, g, b); glVertex2f(0.85f, 0.0f); glColor3f(r+0.15f, g+0.15f, b+0.15f); glVertex2f(0.8f, 0.9f); glVertex2f(0.5f, 1.2f); glVertex2f(-0.5f, 1.2f); glVertex2f(-0.8f, 0.9f); glColor3f(r, g, b); glVertex2f(-0.85f, 0.0f); glEnd();
    glColor3f(0.1f, 0.2f, 0.3f); glBegin(GL_QUADS); glVertex2f(-0.65f, 0.2f); glVertex2f(0.65f, 0.2f); glVertex2f(0.55f, 0.75f); glVertex2f(-0.55f, 0.75f); glVertex2f(-0.68f, -0.1f); glVertex2f(0.68f, -0.1f); glVertex2f(0.72f, -0.8f); glVertex2f(-0.72f, -0.8f); glEnd();
    if (isNight) glColor3f(1.0f, 1.0f, 0.8f); else glColor3f(1.0f, 1.0f, 0.6f);
    glBegin(GL_QUADS); glVertex2f(-0.75f, 0.95f); glVertex2f(-0.45f, 1.05f); glVertex2f(-0.45f, 1.15f); glVertex2f(-0.75f, 1.1f); glVertex2f(0.75f, 0.95f); glVertex2f(0.45f, 1.05f); glVertex2f(0.45f, 1.15f); glVertex2f(0.75f, 1.1f); glEnd();
    if (isNight) glColor3f(1.0f, 0.2f, 0.2f); else glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS); glVertex2f(-0.75f, -0.95f); glVertex2f(-0.55f, -0.95f); glVertex2f(-0.55f, -0.85f); glVertex2f(-0.75f, -0.85f); glVertex2f(0.75f, -0.95f); glVertex2f(0.55f, -0.95f); glVertex2f(0.55f, -0.85f); glVertex2f(0.75f, -0.85f); glEnd();
    glPopMatrix();
}

void drawHeadlightBeam() { if (!isNight || !isMoving) return; glPushMatrix(); glTranslatef(_move, -3.5f, 0.0f); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glColor4f(1.0f, 1.0f, 0.7f, 0.3f); glBegin(GL_TRIANGLES); glVertex2f(-0.13f, 0.13f); glVertex2f(-1.0f, 2.0f); glVertex2f(-0.5f, 2.0f); glEnd(); glBegin(GL_TRIANGLES); glVertex2f(0.13f, 0.13f); glVertex2f(1.0f, 2.0f); glVertex2f(0.5f, 2.0f); glEnd(); glDisable(GL_BLEND); glPopMatrix(); }
void drawCar() { glPushMatrix(); glTranslatef(_move, -3.5f, 0.0f); drawCarBody(0.1f, 0.35f, 0.85f); glPopMatrix(); }
void drawOpponentCar() { glPushMatrix(); float wobble = sin(opponentY * 1.5f) * 0.25f; glTranslatef(opponentX + wobble, opponentY, 0.0f); drawCarBody(0.85f, 0.1f, 0.15f); glPopMatrix(); }

void drawPowerBooster() {
    if (!boosterActive) return;
    glPushMatrix(); glTranslatef(boosterX, boosterY, 0.0f);
    if (isNight) glColor3f(1.0f, 0.9f, 0.3f); else glColor3f(1.0f, 0.8f, 0.0f);
    glBegin(GL_QUADS); glVertex2f(-0.2f, -0.2f); glVertex2f(0.2f, -0.2f); glVertex2f(0.2f, 0.2f); glVertex2f(-0.2f, 0.2f); glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES); glVertex2f(-0.1f, 0.15f); glVertex2f(0.1f, 0.15f); glVertex2f(0.0f, 0.05f); glEnd();
    glBegin(GL_TRIANGLES); glVertex2f(-0.1f, -0.15f); glVertex2f(0.1f, -0.15f); glVertex2f(0.0f, -0.05f); glEnd();
    glBegin(GL_QUADS); glVertex2f(-0.05f, 0.05f); glVertex2f(0.05f, 0.05f); glVertex2f(0.05f, -0.05f); glVertex2f(-0.05f, -0.05f); glEnd();
    glPopMatrix();
}

void drawRain() { if (!isRaining) return; glColor3f(0.6f, 0.7f, 1.0f); glBegin(GL_LINES); for (int i = 0; i < MAX_RAIN; i++) { glVertex2f(rainX[i], rainY[i]); glVertex2f(rainX[i], rainY[i] - 0.2f); } glEnd(); }

void update(int value) {
    if (gameRunning) {
        if (showStartLine && waitingToStart) { startLineY = -2.5f; }
        if (isMoving && !waitingToStart) {
            roadOffset -= 0.05f * WORLD_SPEED;
            yellowLineOffset -= 0.08f * WORLD_SPEED;
            if (roadOffset < -0.8f) roadOffset = 0;
            if (yellowLineOffset < -1.5f) yellowLineOffset = 0;

            if (showFinishLine) {
                finishLineY -= 0.04f * WORLD_SPEED;
                if (finishLineY < -4.0f) {
                    waitingForNextPhase = true;
                    isMoving = 0;
                    showFinishLine = false;
                }
            }
            if (!waitingForNextPhase) {
                phaseTime++;
                if (phaseTime >= 1500 && !showFinishLine) {
                    showFinishLine = true;
                    finishLineY = 8.0f;
                }
            }
        }
        if (isRaining) {
            for (int i = 0; i < MAX_RAIN; i++) {
                rainY[i] -= 0.15f * WORLD_SPEED;
                if (rainY[i] < -5.0f) { rainY[i] = 5.0f; rainX[i] = (float)(rand() % 800 - 400) / 100.0f; }
            }
        }
        if (isMoving && !waitingToStart && !waitingForNextPhase) {
            if (currentPhase == 1) {
                if (boosterActive) {
                    boosterY -= boosterFallSpeed * WORLD_SPEED;
                    boosterTimer += 16;
                    checkBoosterCollision();
                    if (boosterY < -5.0f || boosterTimer > BOOSTER_DURATION) boosterActive = false;
                } else {
                    boosterTimer += 16;
                    if (boosterTimer >= 3000) { spawnPowerBooster(); boosterTimer = 0; }
                }
            }
            else if (currentPhase == 2) {
                if (staticObjectActive) {
                    staticY -= STATIC_FALL_SPEED * WORLD_SPEED;
                    if (staticX > _move - 0.4f && staticX < _move + 0.4f && staticY > -3.8f && staticY < -3.0f) {
                        hitStatic = true; gameRunning = false; isMoving = 0;
                    }
                    if (staticY < -5) staticObjectActive = false;
                } else {
                    staticSpawnTimer++;
                    if (staticSpawnTimer >= 100) {
                        staticObjectActive = true; staticY = 4.5f;
                        staticX = ((float)rand()/RAND_MAX)*2.0f-1.0f; staticSpawnTimer = 0;
                    }
                }
            }
            else if (currentPhase == 3) {
                opponentY -= (opponentFallSpeed + 0.01f) * WORLD_SPEED;
                float ox = opponentX + (sin(opponentY * 1.5f) * 0.25f);
                if (abs(_move - ox) < 0.5f && abs(-3.5f - opponentY) < 0.75f) {
                    gameRunning = false; carCrashed = true; isMoving = 0;
                }
                if (opponentY < -6.0f) { opponentY = 6.0f; opponentX = ((float)rand()/RAND_MAX)*2.0f-1.0f; }
            }
        }
    }
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void timerUpdate(int value) {
    if (gameRunning && isMoving && !waitingToStart && !waitingForNextPhase) {
        if (timeLeft > 0) {
            timeLeft--;
            if (timeLeft <= 60 && currentPhase == 1) showFinishLine = true;
            else if (timeLeft <= 30 && currentPhase == 2) showFinishLine = true;
        } else {
            gameRunning = false; isMoving = 0;
        }
    }
    glutTimerFunc(1000, timerUpdate, 0);
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 's': case 'S': if (!waitingForNextPhase && !waitingToStart) isMoving = 0; break;
        case 'm': case 'M': if (!gameRunning) resetGame(); else if (!waitingForNextPhase && !waitingToStart) isMoving = 1; break;
        case 'p': case 'P':
            if (waitingForNextPhase) {
                if (currentPhase < 3) {
                    currentPhase++;
                    waitingForNextPhase = false;
                    waitingToStart = true;
                    phaseTime = 0;
                    isMoving = 1;
                    startLineY = -2.5f;
                    finishLineY = 8.0f;
                    showStartLine = true;
                    showFinishLine = false;
                    staticObjectActive = false;
                    boosterActive = false;
                }
            } else if (waitingToStart) {
                waitingToStart = false;
                isMoving = 1;
                showStartLine = false;
            }
            break;
        case 'n': case 'N': isNight = !isNight; init(); break;
        case 'r': case 'R': isRaining = !isRaining; break;
        case 'a': case 'A': if(gameRunning && !waitingForNextPhase) { _move -= 0.1f; if (_move < LEFT_LIMIT) _move = LEFT_LIMIT; } break;
        case 'l': case 'L': if(gameRunning && !waitingForNextPhase) { _move += 0.1f; if (_move > RIGHT_LIMIT) _move = RIGHT_LIMIT; } break;
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    if (isNight) drawMoon();
    drawHill(); drawHillGround(); drawHillTriangles();
    drawFieldLeft(); drawFieldRight(); drawRoad(); drawRoadSideColor();
    drawPhaseLines(); drawRain();


    drawTree(-2.3f, 2.2f);
    drawTree(-2.1f, 0.5f);
    drawTree(-2.4f, -1.2f);
    drawTree(-2.2f, -3.5f);


    drawTree(2.3f, 2.2f);
    drawTree(2.1f, 0.5f);
    drawTree(2.4f, -1.2f);
    drawTree(2.2f, -3.5f);


    drawHouse(-3.5f, 1.2f);
    drawHouse(-3.6f, -2.5f);


    drawHouse(2.9f, 1.2f);
    drawHouse(3.0f, -2.5f);

    if (isNight && isMoving && !waitingToStart) drawHeadlightBeam();
    if (currentPhase == 1 && !waitingForNextPhase && !waitingToStart) drawPowerBooster();
    else if (currentPhase == 2 && !waitingForNextPhase && !waitingToStart && staticObjectActive) {
        glColor3f(0, 0, 1); glBegin(GL_QUADS); glVertex2f(staticX-0.2f, staticY-0.2f); glVertex2f(staticX+0.2f, staticY-0.2f); glVertex2f(staticX+0.2f, staticY+0.2f); glVertex2f(staticX-0.2f, staticY+0.2f); glEnd();
    } else if (currentPhase == 3 && !waitingForNextPhase && !waitingToStart) drawOpponentCar();
    drawCar();

    if (waitingToStart) {
        glColor3f(0.0f, 1.0f, 0.0f);
        if (currentPhase == 1) drawText("PHASE 1 - READY TO START", -1.5f, 1.0f);
        else if (currentPhase == 2) drawText("PHASE 2 - READY TO START", -1.5f, 1.0f);
        else if (currentPhase == 3) drawText("FINAL PHASE - READY TO START", -1.8f, 1.0f);
        drawText("Press 'P' to Start Race", -1.3f, 0.5f);
    }

    if (waitingForNextPhase) {
        glColor3f(0.0f, 1.0f, 0.0f);
        if (currentPhase == 1) drawText("PHASE 1 COMPLETED!", -1.2f, 1.0f);
        else if (currentPhase == 2) drawText("PHASE 2 COMPLETED!", -1.2f, 1.0f);
        else drawText("ALL PHASES COMPLETED! YOU WIN!", -1.8f, 1.0f);
        if (currentPhase < 3) drawText("Press 'P' for Next Phase", -1.5f, 0.5f);
    }

    char buffer[50];
    if (isNight) glColor3f(1, 1, 1); else glColor3f(0, 0, 0);
    int displayTime = (timeLeft < 0) ? 0 : timeLeft;
    sprintf(buffer, "TIME: %ds", displayTime); drawText(buffer, 2.2f, 4.5f);
    glColor3f(1.0f, 0.8f, 0.0f); sprintf(buffer, "PHASE %d", currentPhase); drawText(buffer, -3.5f, 4.5f);

    if (!gameRunning && !waitingForNextPhase) {
        glColor3f(1.0f, 0.0f, 0.0f);
        if (carCrashed) drawText("CRASHED! GAME OVER", -1.2f, 0.5f);
        else if (hitStatic) drawText("HIT OBJECT! GAME OVER", -1.2f, 0.5f);
        else if (timeLeft <= 0) drawText("TIME UP! GAME OVER", -1.0f, 0.5f);
        drawText("Press 'M' to Restart", -1.0f, 0.0f);
    }
    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 800);
    glutCreateWindow("Car Racing Game main - 3 Phases");
    init();
    glutDisplayFunc(display);
    glutTimerFunc(16, update, 0);
    glutTimerFunc(1000, timerUpdate, 0);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
