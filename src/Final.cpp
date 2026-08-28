#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL.h>
#include <fstream>
#include <vector>
#include <direct.h>

using namespace std;

// 기본 카메라 위치 및 방향 설정
float cameraX = 0.0f;
float cameraY = 3.5f;
float cameraZ = 0.0f;
float cameraYaw = 0.0f;
float cameraPitch = 0.0f;

int lastMouseX, lastMouseY;
bool mouseDown = false;

// 배터리 및 잠금장치 확인
bool item1Exists = true;
bool item2Exists = true;
bool item3Exists = true;
bool lockExists = true;

// 폴리곤을 읽기 위한 구조체
typedef struct {
    float x;
    float y;
    float z;
} Point;

typedef struct {
    unsigned int ip[3];
} Face;

GLuint textureFloor1, textureFloor2;

// 카메라 업데이트
void updateCamera() {
    glLoadIdentity();
    gluLookAt(
        cameraX, cameraY, cameraZ,
        cameraX + sin(cameraYaw), cameraY + tan(cameraPitch), cameraZ - cos(cameraYaw),
        0.0f, 1.0f, 0.0f
    );
}

class MapObject {
public:
    MapObject(const glm::vec3& position, const glm::vec3& direction, float radius, int texture, string polygonName = "")
        : position(position), direction(glm::normalize(direction)), radius(radius), texture(texture), polygonName(polygonName) {
        readModel(polygonName);
    }

    // Getter functions
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getDirection() const { return direction; }
    float getRadius() const { return radius; }
    int getTexture() const { return texture; }

    // Setter functions
    void setPosition(const glm::vec3& newPosition) { position = newPosition; }
    void setDirection(const glm::vec3& newDirection) { direction = glm::normalize(newDirection); }
    void setRadius(float newRadius) { radius = newRadius; }

    // Read model function
    virtual void readModel(string fname) {
        if (fname == "") return;
        // 파일 이름을 인자로 받아 해당 파일을 읽어온다.
        polygonName = fname;

        FILE* f1;
        char s[81];
        int i;

        if (mpoint != NULL) delete[] mpoint;
        if (mface != NULL) delete[] mface;

        _chdir("../bin/");


        if ((f1 = fopen(fname.c_str(), "rt")) == NULL) {
            printf("No file\n");
            return;
        }

        fscanf(f1, "%s", s);
        fscanf(f1, "%s", s);

        fscanf(f1, "%d", &pnum);

        mpoint = new Point[pnum];

        for (i = 0; i < pnum; i++) {
            fscanf(f1, "%f", &mpoint[i].x);
            fscanf(f1, "%f", &mpoint[i].y);
            fscanf(f1, "%f", &mpoint[i].z);
        }

        fscanf(f1, "%s", s);
        fscanf(f1, "%s", s);

        fscanf(f1, "%d", &fnum);

        mface = new Face[fnum];

        for (i = 0; i < fnum; i++) {
            fscanf(f1, "%d", &mface[i].ip[0]);
            fscanf(f1, "%d", &mface[i].ip[1]);
            fscanf(f1, "%d", &mface[i].ip[2]);
        }

        fclose(f1);
    }

    // Rendering function
    virtual void render() {
        {
            glPushMatrix();
            glm::mat4 model = glm::translate(glm::mat4(1.0f), position);

            // 방향 벡터의 각 성분에 대해 세 번 회전
            model = glm::rotate(model, glm::radians(glm::degrees(atan2(direction.x, direction.z))), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(glm::degrees(atan2(direction.y, direction.x))), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, glm::radians(glm::degrees(atan2(direction.z, direction.y))), glm::vec3(1.0f, 0.0f, 0.0f));

            glMultMatrixf(glm::value_ptr(model));
            glColor3f(1.0f, 0.7f, 0.1f);

            // 특정 폴리곤의 경우 색상을 다르게 설정
            if (polygonName == "planet1.dat") {
                glColor3f(0.0f, 0.0f, 1.0f);
            }

            if (polygonName == "planet2.dat") {
                glColor3f(1.0f, 0.5f, 0.1f);
            }

            if (polygonName == "planet3.dat") {
                glColor3f(1.0f, 1.0f, 0.9f);
            }

            if (polygonName == "planet_ring.dat") {
                glColor3f(0.8f, 0.6f, 0.1f);
            }

            if (polygonName == "planet_ring2.dat") {
                glColor3f(1, 1, 1);
            }

            // readModel로 읽어온 모델을 그린다.
            for (int i = 0; i < fnum; i++) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glBegin(GL_TRIANGLES);
                glVertex3f(mpoint[mface[i].ip[0]].x, mpoint[mface[i].ip[0]].y, mpoint[mface[i].ip[0]].z);
                glVertex3f(mpoint[mface[i].ip[1]].x, mpoint[mface[i].ip[1]].y, mpoint[mface[i].ip[1]].z);
                glVertex3f(mpoint[mface[i].ip[2]].x, mpoint[mface[i].ip[2]].y, mpoint[mface[i].ip[2]].z);
                glEnd();
            }
            glPopMatrix();
        }
    }

    // checkCollision with camera function
    virtual bool checkCollisionWithCamera(float cameraX, float cameraY, float cameraZ) const {
        glm::vec3 diff = position - glm::vec3(cameraX, cameraY, cameraZ);
        float dist = glm::length(diff);
        float sumRadius = radius + 2.0f;

        if (dist < sumRadius) {
            return true;
        }
        return false;
    }

private:
    glm::vec3 position;
    glm::vec3 direction;
    float radius;
    int texture;
    string polygonName;
    Point* mpoint = NULL;
    Face* mface = NULL;
    int pnum;
    int fnum;
};

class PlaneObject : public MapObject {
public:
    PlaneObject(const glm::vec3& position, const glm::vec3& direction, int texture, const glm::vec3& direction_height, float width, float height)
        : MapObject(position, direction, 0.0f, texture), direction_height(glm::normalize(direction_height)), width(width), height(height) {
        // 만약 direction과 direction_height가 서로 수직이 아니라면 경고 메시지를 출력하고 direction_height를 direction에 수직인 벡터로 만든다.
        if (glm::dot(direction, direction_height) != 0.0f) {
            std::cout << "Warning: direction and direction_height are not perpendicular!" << std::endl;
            this->direction_height = glm::normalize(glm::cross(direction, direction_height));
        }
        initTexture();
    }

    // Getter functions
    glm::vec3 getDirection_height() const { return direction_height; }
    float getWidth() const { return width; }
    float getHeight() const { return height; }

    // Setter functions
    void setDirection_height(const glm::vec3& newDirection_height) { direction_height = glm::normalize(newDirection_height); }
    void setWidth(float newWidth) { width = newWidth; }
    void setHeight(float newHeight) { height = newHeight; }

    // Texture initialization function
    void initTexture() {
        // [수정] 이 함수는 생성자에서 호출되므로 textureID 는 아직 유효한 값이 아니다.
        // 미초기화 값으로 glDeleteTextures 를 호출하던 부분을 제거한다.
        const char* textureName;

        // 인자로 받은 정수에 따라 텍스처를 다르게 설정한다.
        switch (getTexture())
        {
        case 1:
            textureName = "../bin/Texture/dia_panel.bmp";
            break;

        case 2:
            textureName = "../bin/Texture/metal_panel.bmp";
            break;

        case 3:
            textureName = "../bin/Texture/Rcircuit.bmp";
            break;

        case 4:
            textureName = "../bin/Texture/Gcircuit.bmp";
            break;

        case 5:
            textureName = "../bin/Texture/spacetile.bmp";
            break;

        case 6:
            textureName = "../bin/Texture/steel.bmp";
            break;

        case 7:
            textureName = "../bin/Texture/steel.bmp";
            break;

        default:
            textureName = "../bin/Texture/check.bmp";
            break;
        }

        textureID = SOIL_load_OGL_texture(
            textureName,
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_INVERT_Y
        );
        if (!textureID) {
            printf("텍스처를 불러올 수 없습니다. : %s\n", SOIL_last_result());
        }

        // 텍스처 설정
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    virtual void readModel(string fname) {
        // PlaneObject는 polygon.dat 파일을 읽지 않는다.
    }

    // Rendering function
    virtual void render() {
        glm::vec3 v1 = getPosition() + getDirection() * width + direction_height * height;
        glm::vec3 v2 = getPosition() + getDirection() * width - direction_height * height;
        glm::vec3 v3 = getPosition() - getDirection() * width - direction_height * height;
        glm::vec3 v4 = getPosition() - getDirection() * width + direction_height * height;

        glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);

        if (getTexture() == 5) {
            glBegin(GL_QUADS);
            glColor3f(1.0f, 1.0f, 1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(v1.x, v1.y, v1.z);
            glTexCoord2f(10.0f, 0.0f); glVertex3f(v2.x, v2.y, v2.z);
            glTexCoord2f(10.0f, 10); glVertex3f(v3.x, v3.y, v3.z);
            glTexCoord2f(0.0f, 10); glVertex3f(v4.x, v4.y, v4.z);
            glEnd();

            glDisable(GL_TEXTURE_2D);
            glPopMatrix();

            return;
        }

        if (getTexture() == 6) {
            glBegin(GL_QUADS);
            glColor3f(1.0f, 1.0f, 1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(v1.x, v1.y, v1.z);
            glTexCoord2f(3.0f, 0.0f); glVertex3f(v2.x, v2.y, v2.z);
            glTexCoord2f(3.0f, 9.0f); glVertex3f(v3.x, v3.y, v3.z);
            glTexCoord2f(0.0f, 9.0f); glVertex3f(v4.x, v4.y, v4.z);
            glEnd();

            glDisable(GL_TEXTURE_2D);
            glPopMatrix();

            return;
        }

        if (getTexture() == 7) {
            glBegin(GL_QUADS);
            glColor3f(1.0f, 1.0f, 1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(v1.x, v1.y, v1.z);
            glTexCoord2f(5.0f, 0.0f); glVertex3f(v2.x, v2.y, v2.z);
            glTexCoord2f(5.0f, 5.0f); glVertex3f(v3.x, v3.y, v3.z);
            glTexCoord2f(0.0f, 5.0f); glVertex3f(v4.x, v4.y, v4.z);
            glEnd();

            glDisable(GL_TEXTURE_2D);
            glPopMatrix();

            return;
        }

        // 사각형 그리기
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(v1.x, v1.y, v1.z);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(v2.x, v2.y, v2.z);
        glTexCoord2f(1.0f, width / height); glVertex3f(v3.x, v3.y, v3.z);
        glTexCoord2f(0.0f, width / height); glVertex3f(v4.x, v4.y, v4.z);
        glEnd();

        glDisable(GL_TEXTURE_2D);
        glPopMatrix();
    }

    // 안 씀
    bool checkCollisionWithCamera(float cameraX, float cameraY, float cameraZ) const {
        return false;
    }

    // 충돌에 따라 카메라의 이동 방향 및 거리를 결정하는 함수
    glm::vec3 collisionNavigation(float cameraX, float cameraY, float cameraZ, glm::vec3 direction) {

        float deltaX = cameraX + direction.x;
        float deltaY = cameraY;
        float deltaZ = cameraZ + direction.z;
        glm::vec3 moveDirection = direction;

        glm::vec3 normal = glm::normalize(glm::cross(getDirection(), getDirection_height()));
        float dist_position = glm::abs(glm::dot(getPosition() - glm::vec3(deltaX, deltaY, deltaZ), normal));
        float dist_direction = glm::abs(glm::dot(getDirection(), glm::vec3(deltaX, deltaY, deltaZ) - getPosition()));
        float dist_direction_height = glm::abs(glm::dot(getDirection_height(), glm::vec3(deltaX, deltaY, deltaZ) - getPosition()));

        if (dist_position < 2.0f && dist_direction < width + 2 && dist_direction_height < height + 2) {
            moveDirection = glm::dot(moveDirection, getDirection()) * getDirection();
            return moveDirection;
        }
        return moveDirection;
    }

private:
    glm::vec3 direction_height;
    float width;
    float height;
    GLuint textureID = 0;   // [수정] 미초기화 상태로 glDeleteTextures 에 넘겨져
                            // 다른 객체의 텍스처를 삭제하던 버그의 원인이었음
};

// 배터리 3개 생성
MapObject* item1 = new MapObject(glm::vec3(-25.0f, 3.0f, 150.0f), glm::vec3(0.0f, 1.0f, 1.0f), 1.0f,
    0, "battery.dat");
MapObject* item2 = new MapObject(glm::vec3(-20.0f, 3.0f, -140.0f), glm::vec3(1.0f, 1.0f, 0.0f), 1.0f,
    0, "battery.dat");
MapObject* item3 = new MapObject(glm::vec3(-145.0f, 3.0f, -5.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f,
    0, "battery.dat");

// 트로피 생성
MapObject* trophy = new MapObject(glm::vec3(125.0f, 5.0f, 0.0f), glm::vec3(1.0f, 0.2f, 0.0f), 1.0f,
    0, "trophy.dat");

// 행성 1, 2, 3 생성
MapObject* planet1 = new MapObject(glm::vec3(-350.0f, -100.0f, 250.0f), glm::vec3(0.0f, 1.0f, 0.0f), 1.0f,
    0, "planet1.dat");
MapObject* planet2 = new MapObject(glm::vec3(700.0f, 150.0f, -200.0f), glm::vec3(0.0f, 1.0f, 0.0f), 1.0f,
    0, "planet2.dat");
MapObject* planet3 = new MapObject(glm::vec3(350.0f, -20.0f, 500.0f), glm::vec3(0.0f, 1.0f, 0.0f), 1.0f,
    0, "planet3.dat");

// 행성 고리 1, 2 생성
MapObject* planet2_ring = new MapObject(glm::vec3(700.0f, 150.0f, -200.0f), glm::vec3(0.2f, -0.3f, 0.2f), 1.0f,
    0, "planet_ring.dat");
MapObject* planet3_ring = new MapObject(glm::vec3(350.0f, -20.0f, 500.0f), glm::vec3(-0.7f, 0.3f, 0.2f), 1.0f,
    0, "planet_ring2.dat");

// PlaneObject를 저장할 벡터 생성
vector<PlaneObject*> wallsMain;
vector<PlaneObject*> wallsRight;
vector<PlaneObject*> wallsLeft;
vector<PlaneObject*> wallsBack;
vector<PlaneObject*> wallsLock;

// 메인 구역 생성
void generateWallsMain() {
    // Main 구역 외벽
    wallsMain.push_back(new PlaneObject(glm::vec3(40.0f, 4.0f, -25.0f), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 15.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(40.0f, 4.0f, 25.0f), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 15.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-40.0f, 4.0f, -25.0f), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 15.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-40.0f, 4.0f, 25.0f), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 15.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-25.0f, 4.0f, -40.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 15.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(25.0f, 4.0f, -40.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 15.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-25.0f, 4.0f, 40.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 15.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(25.0f, 4.0f, 40.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 15.0f, 4.0f));

    // Main 구역 내부 1
    wallsMain.push_back(new PlaneObject(glm::vec3(32.5f, 4.0f, -15.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 7.5f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(35.0f, 4.0f, -10.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 5.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(25.0f, 4.0f, 5.0f), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 20.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(30.0f, 4.0f, 7.5f), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 17.5f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(27.5f, 4.0f, 25.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 2.5f, 4.0f));

    // Main 구역 내부 2
    wallsMain.push_back(new PlaneObject(glm::vec3(-10.0f, 4.0f, 15.0f), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 25.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-15.0f, 4.0f, 15.0f), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 25.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-12.5f, 4.0f, -10.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 2.5f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(0.0f, 4.0f, 15.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 10.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(0.0f, 4.0f, 20.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 10.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(10.0f, 4.0f, 17.5f), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 2.5f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(0.0f, 4.0f, -10.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 10.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(0.0f, 4.0f, -5.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 10.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(10.0f, 4.0f, -15.0), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 10.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(5.0f, 4.0f, -15.0), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 10.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(0.0f, 4.0f, -25.0), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 30.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(0.0f, 4.0f, -20.0), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 30.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-30.0f, 4.0f, -22.5), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 2.5f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(30.0f, 4.0f, -22.5), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 2.5f, 4.0f));

    // Main 구역 내부 3
    wallsMain.push_back(new PlaneObject(glm::vec3(-30.0f, 4.0f, -10.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 10.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-30.0f, 4.0f, -15.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0f, 0.0f), 10.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-20.0f, 4.0f, 5.0f), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0, 0.0f), 20.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-25.0f, 4.0f, 5.0f), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0, 0.0f), 20.0f, 4.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-22.5f, 4.0f, 25.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0, 0.0f), 2.5f, 4.0f));

    // Right 구역 복도
    wallsMain.push_back(new PlaneObject(glm::vec3(10.0f, 0.5f, 70.0f), glm::vec3(0.0f, 0.0, 1.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 0.5f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-10.0f, 0.5f, 70.0f), glm::vec3(0.0f, 0.0, 1.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 0.5f));
    wallsMain.push_back(new PlaneObject(glm::vec3(10.0f, 5.5f, 70.0f), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 2.5f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-10.0f, 5.5f, 70.0f), glm::vec3(0.0f, 0.0, 1.0f), 1, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 2.5f));

    // Left 구역 복도
    wallsMain.push_back(new PlaneObject(glm::vec3(10.0f, 0.5f, -70.0f), glm::vec3(0.0f, 0.0, -1.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 0.5f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-10.0f, 0.5f, -70.0f), glm::vec3(0.0f, 0.0, -1.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 0.5f));
    wallsMain.push_back(new PlaneObject(glm::vec3(10.0f, 5.5f, -70.0f), glm::vec3(0.0f, 0.0, -1.0f), 1, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 2.5f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-10.0f, 5.5f, -70.0f), glm::vec3(0.0f, 0.0, -1.0f), 1, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 2.5f));

    // Back 구역 복도
    wallsMain.push_back(new PlaneObject(glm::vec3(-70.0f, 0.5f, 10.0f), glm::vec3(1.0f, 0.0, 0.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 0.5f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-70.0f, 0.5f, -10.0f), glm::vec3(1.0f, 0.0, 0.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 0.5f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-70.0f, 5.5f, 10.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 2.5f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-70.0f, 5.5f, -10.0f), glm::vec3(1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 2.5f));

    // Front 구역 복도
    wallsMain.push_back(new PlaneObject(glm::vec3(70.0f, 0.5f, 10.0f), glm::vec3(-1.0f, 0.0, 0.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 0.5f));
    wallsMain.push_back(new PlaneObject(glm::vec3(70.0f, 0.5f, -10.0f), glm::vec3(-1.0f, 0.0, 0.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 0.5f));
    wallsMain.push_back(new PlaneObject(glm::vec3(70.0f, 5.5f, 10.0f), glm::vec3(-1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 2.5f));
    wallsMain.push_back(new PlaneObject(glm::vec3(70.0f, 5.5f, -10.0f), glm::vec3(-1.0f, 0.0, 0.0f), 1, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 2.5f));

    // Main 바닥
    wallsMain.push_back(new PlaneObject(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0, 1.0f), 5, glm::vec3(1.0f, 0.0, 0.0f), 40.0f, 40.0f));

    // 복도 바닥
    wallsMain.push_back(new PlaneObject(glm::vec3(0.0f, 0.0f, -70.0f), glm::vec3(0.0f, 0.0, 1.0f), 6, glm::vec3(1.0f, 0.0, 0.0f), 30.0f, 10.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(0.0f, 0.0f, 70.0f), glm::vec3(0.0f, 0.0, -1.0f), 6, glm::vec3(-1.0f, 0.0, 0.0f), 30.0f, 10.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(70.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0, 0.0f), 6, glm::vec3(0.0f, 0.0, 1.0f), 30.0f, 10.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-70.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0, 0.0f), 6, glm::vec3(0.0f, 0.0, -1.0f), 30.0f, 10.0f));

    // 최종 목적지
    wallsMain.push_back(new PlaneObject(glm::vec3(125.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0, 1.0f), 7, glm::vec3(1.0f, 0.0, 0.0f), 25.0f, 25.0f));

    // Main 천장
    wallsMain.push_back(new PlaneObject(glm::vec3(0.0f, 8.0f, 0.0f), glm::vec3(0.0f, 0.0, 1.0f), 5, glm::vec3(1.0f, 0.0, 0.0f), 40.0f, 40.0f));

    // 복도 천장
    wallsMain.push_back(new PlaneObject(glm::vec3(0.0f, 8.0f, -70.0f), glm::vec3(0.0f, 0.0, 1.0f), 6, glm::vec3(1.0f, 0.0, 0.0f), 30.0f, 10.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(0.0f, 8.0f, 70.0f), glm::vec3(0.0f, 0.0, -1.0f), 6, glm::vec3(-1.0f, 0.0, 0.0f), 30.0f, 10.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(70.0f, 8.0f, 0.0f), glm::vec3(-1.0f, 0.0, 0.0f), 6, glm::vec3(0.0f, 0.0, 1.0f), 30.0f, 10.0f));
    wallsMain.push_back(new PlaneObject(glm::vec3(-70.0f, 8.0f, 0.0f), glm::vec3(1.0f, 0.0, 0.0f), 6, glm::vec3(0.0f, 0.0, -1.0f), 30.0f, 10.0f));
}

// 메인 구역 삭제
void releaseWallsMain() {
    for (int i = 0; i < wallsMain.size(); i++) {
        delete wallsMain[i];
    }
    wallsMain.clear();
}

// Right 구역 생성
void generateWallsRight() {

    // Right 구역 외벽
    wallsRight.push_back(new PlaneObject(glm::vec3(-20.0f, 4.0f, 100.0f), glm::vec3(1.0f, 0.0, 0.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(20.0f, 4.0f, 100.0f), glm::vec3(1.0f, 0.0, 0.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(0.0f, 4.0f, 160.0f), glm::vec3(1.0f, 0.0, 0.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(30.0f, 4.0f, 130.0f), glm::vec3(0.0f, 0.0, 1.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(-30.0f, 4.0f, 130.0f), glm::vec3(0.0f, 0.0, 1.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 4.0f));

    // Right 구역 내부 1
    wallsRight.push_back(new PlaneObject(glm::vec3(0.0f, 4.0f, 110.0f), glm::vec3(1.0f, 0.0, 0.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 15.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(0.0f, 4.0f, 115.0f), glm::vec3(1.0f, 0.0, 0.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 15.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(15.0f, 4.0f, 125.0f), glm::vec3(0.0f, 0.0, 1.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 15.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(10.0f, 4.0f, 125.0f), glm::vec3(0.0f, 0.0, 1.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 15.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(-15.0f, 4.0f, 120.0f), glm::vec3(0.0f, 0.0, 1.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(-10.0f, 4.0f, 120.0f), glm::vec3(0.0f, 0.0, 1.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(-20.0f, 4.0f, 130.0f), glm::vec3(1.0f, 0.0, 0.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(-20.0f, 4.0f, 125.0f), glm::vec3(1.0f, 0.0, 0.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(12.5f, 4.0f, 140.0f), glm::vec3(1.0f, 0.0, 0.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 2.5f, 4.0f));

    // Right 구역 내부 2
    wallsRight.push_back(new PlaneObject(glm::vec3(2.5f, 4.0f, 145.0f), glm::vec3(0.0f, 0.0, 1.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 15.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(-2.5f, 4.0f, 145.0f), glm::vec3(0.0f, 0.0, 1.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 15.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(0.0f, 4.0f, 130.0f), glm::vec3(1.0f, 0.0, 0.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 2.5f, 4.0f));

    // Right 구역 내부 3
    wallsRight.push_back(new PlaneObject(glm::vec3(-20.0f, 4.0f, 140.0f), glm::vec3(1.0f, 0.0, 0.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(-20.0f, 4.0f, 145.0f), glm::vec3(1.0f, 0.0, 0.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsRight.push_back(new PlaneObject(glm::vec3(-10.0f, 4.0f, 142.5f), glm::vec3(0.0f, 0.0, 1.0f), 2, glm::vec3(0.0f, 1.0, 0.0f), 2.5f, 4.0f));

    // Right 바닥
    wallsRight.push_back(new PlaneObject(glm::vec3(0.0f, 0.0f, 130.0f), glm::vec3(0.0f, 0.0, 1.0f), 5, glm::vec3(1.0f, 0.0, 0.0f), 30.0f, 30.0f));

    // Right 천장
    wallsRight.push_back(new PlaneObject(glm::vec3(0.0f, 8.0f, 130.0f), glm::vec3(0.0f, 0.0, 1.0f), 5, glm::vec3(1.0f, 0.0, 0.0f), 30.0f, 30.0f));
}

// Right 구역 삭제
void releaseWallsRight() {
    for (int i = 0; i < wallsRight.size(); i++) {
        delete wallsRight[i];
    }
    wallsRight.clear();
}

// Left 구역 생성
void generateWallsLeft() {
    // Left 구역 외벽
    wallsLeft.push_back(new PlaneObject(glm::vec3(-20.0f, 4.0f, -100.0f), glm::vec3(1.0f, 0.0, 0.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(20.0f, 4.0f, -100.0f), glm::vec3(1.0f, 0.0, 0.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(0.0f, 4.0f, -160.0f), glm::vec3(1.0f, 0.0, 0.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(30.0f, 4.0f, -130.0f), glm::vec3(0.0f, 0.0, -1.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(-30.0f, 4.0f, -130.0f), glm::vec3(0.0f, 0.0, -1.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 4.0f));

    // Left 구역 내부 1
    wallsLeft.push_back(new PlaneObject(glm::vec3(2.5f, 4.0f, -135.0f), glm::vec3(0.0f, 0.0, 1.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 25.0f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(-2.5f, 4.0f, -135.0f), glm::vec3(0.0f, 0.0, 1.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 25.0f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(0.0f, 4.0f, -110.0f), glm::vec3(1.0f, 0.0, 0.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 22.5f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(0.0f, 4.0f, -115.0f), glm::vec3(1.0f, 0.0, 0.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 22.5f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(10.0f, 4.0f, -150.0f), glm::vec3(1.0f, 0.0, 0.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(10.0f, 4.0f, -145.0f), glm::vec3(1.0f, 0.0, 0.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(20.0f, 4.0f, -147.5f), glm::vec3(0.0f, 0.0, 1.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 2.5f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(22.5f, 4.0f, -112.5f), glm::vec3(0.0f, 0.0, 1.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 2.5f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(-22.5f, 4.0f, -112.5f), glm::vec3(0.0f, 0.0, -1.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 2.5f, 4.0f));

    // Left 구역 내부 2
    wallsLeft.push_back(new PlaneObject(glm::vec3(20.0f, 4.0f, -130.0f), glm::vec3(1.0f, 0.0, 0.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(20.0f, 4.0f, -135.0f), glm::vec3(1.0f, 0.0, 0.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(10.0f, 4.0f, -132.5f), glm::vec3(0.0f, 0.0, 1.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 2.5f, 4.0f));

    // Left 구역 내부 3
    wallsLeft.push_back(new PlaneObject(glm::vec3(-20.0f, 4.0f, -125.0f), glm::vec3(1.0f, 0.0, 0.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(-20.0f, 4.0f, -130.0f), glm::vec3(1.0f, 0.0, 0.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(-10.0f, 4.0f, -137.5f), glm::vec3(0.0f, 0.0, 1.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 12.5f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(-15.0f, 4.0f, -137.5f), glm::vec3(0.0f, 0.0, 1.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 12.5f, 4.0f));
    wallsLeft.push_back(new PlaneObject(glm::vec3(-12.5f, 4.0f, -150.0f), glm::vec3(1.0f, 0.0, 0.0f), 3, glm::vec3(0.0f, 1.0, 0.0f), 2.5f, 4.0f));

    // Left 바닥
    wallsLeft.push_back(new PlaneObject(glm::vec3(0.0f, 0.0f, -130.0f), glm::vec3(0.0f, 0.0, 1.0f), 5, glm::vec3(1.0f, 0.0, 0.0f), 30.0f, 30.0f));

    // Left 천장
    wallsLeft.push_back(new PlaneObject(glm::vec3(0.0f, 8.0f, -130.0f), glm::vec3(0.0f, 0.0, 1.0f), 5, glm::vec3(1.0f, 0.0, 0.0f), 30.0f, 30.0f));
}

// Left 구역 삭제
void releaseWallsLeft() {
    for (int i = 0; i < wallsLeft.size(); i++) {
        delete wallsLeft[i];
    }
    wallsLeft.clear();
}

// Back 구역 생성
void generateWallsBack() {
    // Back 구역 외벽
    wallsBack.push_back(new PlaneObject(glm::vec3(-100.0f, 4.0f, -20.0f), glm::vec3(0.0f, 0.0, 1.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-100.0f, 4.0f, 20.0f), glm::vec3(0.0f, 0.0, 1.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-160.0f, 4.0f, 0.0f), glm::vec3(0.0f, 0.0, 1.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-130.0f, 4.0f, 30.0f), glm::vec3(1.0f, 0.0, 0.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-130.0f, 4.0f, -30.0f), glm::vec3(1.0f, 0.0, 0.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 30.0f, 4.0f));

    // Back 구역 내부 1
    wallsBack.push_back(new PlaneObject(glm::vec3(-117.5f, 4.0f, 15.0f), glm::vec3(1.0f, 0.0, 0.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 7.5f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-117.5f, 4.0f, 20.0f), glm::vec3(1.0f, 0.0, 0.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 7.5f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-120.0f, 4.0f, 22.5f), glm::vec3(0.0f, 0.0, 1.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 7.5f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-125.0f, 4.0f, 22.5f), glm::vec3(0.0f, 0.0, 1.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 7.5f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-110.0f, 4.0f, 17.5f), glm::vec3(0.0f, 0.0, 1.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 2.5f, 4.0f));

    // Back 구역 내부 2
    wallsBack.push_back(new PlaneObject(glm::vec3(-135.0f, 4.0f, 5.0f), glm::vec3(1.0f, 0.0, 0.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 25.0f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-135.0f, 4.0f, 0.0f), glm::vec3(1.0f, 0.0, 0.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 25.0f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-130.0f, 4.0f, -15.0f), glm::vec3(1.0f, 0.0, 0.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 20.0f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-130.0f, 4.0f, -20.0f), glm::vec3(1.0f, 0.0, 0.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 20.0f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-110.0f, 4.0f, 2.5f), glm::vec3(0.0f, 0.0, 1.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 2.5f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-140.0f, 4.0f, 0.0f), glm::vec3(0.0f, 0.0, 1.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 20.0f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-135.0f, 4.0f, 0.0f), glm::vec3(0.0f, 0.0, 1.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 20.0f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-110.0f, 4.0f, -15.0f), glm::vec3(0.0f, 0.0, 1.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 5.0f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-115.0f, 4.0f, -15.0f), glm::vec3(0.0f, 0.0, 1.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 5.0f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-112.5f, 4.0f, 0.0f), glm::vec3(1.0f, 0.0, 0.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 2.5f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-140.0f, 4.0f, 20.0f), glm::vec3(1.0f, 0.0, 0.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 5.0f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-140.0f, 4.0f, 15.0f), glm::vec3(1.0f, 0.0, 0.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 5.0f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-145.0f, 4.0f, 17.5f), glm::vec3(0.0f, 0.0, 1.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 2.5f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-150.0f, 4.0f, -17.5f), glm::vec3(0.0f, 0.0, 1.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 2.5f, 4.0f));
    wallsBack.push_back(new PlaneObject(glm::vec3(-112.5f, 4.0f, -10.0f), glm::vec3(1.0f, 0.0, 0.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 2.5f, 4.0f));

    // Back 바닥
    wallsBack.push_back(new PlaneObject(glm::vec3(-130.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0, 0.0f), 5, glm::vec3(0.0f, 0.0, 1.0f), 30.0f, 30.0f));

    // Back 천장
    wallsBack.push_back(new PlaneObject(glm::vec3(-130.0f, 8.0f, 0.0f), glm::vec3(1.0f, 0.0, 0.0f), 5, glm::vec3(0.0f, 0.0, 1.0f), 30.0f, 30.0f));
}

// Back 구역 삭제
void releaseWallsBack() {
    for (int i = 0; i < wallsBack.size(); i++) {
        delete wallsBack[i];
    }
    wallsBack.clear();
}

// 잠금 장치 생성
void generateWallsLock() {
    wallsLock.push_back(new PlaneObject(glm::vec3(40.0f, 4.0f, 0.0f), glm::vec3(0.0f, 0.0, 1.0f), 4, glm::vec3(0.0f, 1.0, 0.0f), 10.0f, 4.0f));
}

// 잠금 장치 삭제
void releaseWallsLock() {
    for (int i = 0; i < wallsLock.size(); i++) {
        delete wallsLock[i];
    }
    wallsLock.clear();
}

// 벽 충돌 계산, 카메라 방향 조정
glm::vec3 collisionCalculation(float cameraX, float cameraY, float cameraZ, float moveX, float moveZ) {
    glm::vec3 moveDirection = glm::vec3(moveX, 0.0f, moveZ);

    if (cameraX < 100.0f && cameraX > -100.0f && cameraZ < 100.0f && cameraZ > -100.0f) {
        for (int i = 0; i < wallsMain.size(); i++) {
            moveDirection = wallsMain[i]->collisionNavigation(cameraX, cameraY, cameraZ, moveDirection);
        }
        for (int i = 0; i < wallsLock.size(); i++) {
            moveDirection = wallsLock[i]->collisionNavigation(cameraX, cameraY, cameraZ, moveDirection);
        }
    }

    if (cameraZ > 40.0f) {
        for (int i = 0; i < wallsRight.size(); i++) {
            moveDirection = wallsRight[i]->collisionNavigation(cameraX, cameraY, cameraZ, moveDirection);
        }
    }

    if (cameraZ < -40.0f) {
        for (int i = 0; i < wallsLeft.size(); i++) {
            moveDirection = wallsLeft[i]->collisionNavigation(cameraX, cameraY, cameraZ, moveDirection);
        }
    }

    if (cameraX < -40.0f) {
        for (int i = 0; i < wallsBack.size(); i++) {
            moveDirection = wallsBack[i]->collisionNavigation(cameraX, cameraY, cameraZ, moveDirection);
        }
    }



    return moveDirection;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0f, 0.0f, 0.0f);

    // 배터리 render
    if (item1Exists) {
        item1->render();
    }
    if (item2Exists) {
        item2->render();
    }
    if (item3Exists) {
        item3->render();
    }

    // 각종 오브젝트 render
    trophy->render();
    planet1->render();
    planet2->render();
    planet3->render();
    planet2_ring->render();
    planet3_ring->render();

    // Main 구역 Render
    if (cameraX < 200.0f && cameraX > -140.0f && cameraZ < 140.0f && cameraZ > -140.0f) {
        // Main 구역 벽
        for (int i = 0; i < wallsMain.size(); i++) {
            wallsMain[i]->render();
        }
        // 잠금 장치
        for (int i = 0; i < wallsLock.size(); i++) {
            wallsLock[i]->render();
        }
    }

    // Right 구역 Render
    if (cameraZ > 0.0f) {
        for (int i = 0; i < wallsRight.size(); i++) {
            wallsRight[i]->render();
        }
    }

    // Left 구역 Render
    if (cameraZ < 0.0f) {
        for (int i = 0; i < wallsLeft.size(); i++) {
            wallsLeft[i]->render();
        }
    }

    // Back 구역 Render
    if (cameraX < 0.0f) {
        for (int i = 0; i < wallsBack.size(); i++) {
            wallsBack[i]->render();
        }
    }

    glutSwapBuffers();
}

// 카메라 기본 설정
void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)width / height, 0.1f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
    updateCamera();
}

// 마우스 좌클릭 상태 확인
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseDown = true;
            lastMouseX = x;
            lastMouseY = y;
        }
        else if (state == GLUT_UP) {
            mouseDown = false;
        }
    }
}

// 마우스 좌클릭 드래그 시 카메라 회전
void motion(int x, int y) {
    if (mouseDown) {
        int deltaX = x - lastMouseX;
        int deltaY = y - lastMouseY;

        cameraYaw += deltaX * 0.005f;
        cameraPitch -= deltaY * 0.005f;

        if (cameraPitch > 1.5f) cameraPitch = 1.5f;
        if (cameraPitch < -1.5f) cameraPitch = -1.5f;

        updateCamera();

        lastMouseX = x;
        lastMouseY = y;

        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y) {
    // 카메라 이동 속도 및 방향 변수
    const float cameraSpeed = 0.9f;
    float moveX = cameraSpeed * sin(cameraYaw);
    float moveZ = -1 * cameraSpeed * cos(cameraYaw);

    // 구역 생성 체크
    static bool isMainGenerated = false;
    static bool isRightGenerated = false;
    static bool isLeftGenerated = false;
    static bool isBackGenerated = false;
    static bool isFrontGenerated = false;

    switch (key) {
    case 'w': // 전진
    {
        glm::vec3 moveDirection = collisionCalculation(cameraX, cameraY, cameraZ, moveX, moveZ);
        cameraX += moveDirection.x;
        cameraZ += moveDirection.z;
        break;
    }
    case 's': // 후진
    {
        glm::vec3 moveDirection = collisionCalculation(cameraX, cameraY, cameraZ, -moveX, -moveZ);
        cameraX += moveDirection.x;
        cameraZ += moveDirection.z;
        break;
    }
    case 'a': // 좌로 이동
    {
        glm::vec3 moveDirection = collisionCalculation(cameraX, cameraY, cameraZ, moveZ, -moveX);
        cameraX += moveDirection.x;
        cameraZ += moveDirection.z;
        break;
    }
    case 'd': // 우로 이동
    {
        glm::vec3 moveDirection = collisionCalculation(cameraX, cameraY, cameraZ, -moveZ, moveX);
        cameraX += moveDirection.x;
        cameraZ += moveDirection.z;
        break;
    }

    //  case 'q': // 디버그 용 상하 이동
          //cameraY += cameraSpeed;
          //break;

    //  case 'e':
    //      cameraY -= cameraSpeed;
    //      break;

    case 'r': // 시작 지점으로 이동
        cameraX = 0.0f;
        cameraY = 2.0f;
        cameraZ = 0.0f;
        cameraYaw = 0.0f;
        cameraPitch = 0.0f;
        break;

    case 27: // 종료
        exit(0);
        break;
    }

    // 카메라 좌표에 따른 벽 생성 및 제거
    if (cameraZ > 40.0f && !isRightGenerated) {
        generateWallsRight();
        isRightGenerated = true;
    }
    if (cameraZ < 30.0f && isRightGenerated) {
        releaseWallsRight();
        isRightGenerated = false;
    }

    if (cameraZ < -40.0f && !isLeftGenerated) {
        generateWallsLeft();
        isLeftGenerated = true;
    }

    if (cameraZ > -30.0f && isLeftGenerated) {
        releaseWallsLeft();
        isLeftGenerated = false;
    }

    if (cameraX < -40.0f && !isBackGenerated) {
        generateWallsBack();
        isBackGenerated = true;
    }

    if (cameraX > -30.0f && isBackGenerated) {
        releaseWallsBack();
        isBackGenerated = false;
    }

    // 배터리 획득 검사
    if (item1Exists) {
        if (item1->checkCollisionWithCamera(cameraX, cameraY, cameraZ)) {
            item1Exists = false;
            delete item1;
        }
    }

    if (item2Exists) {
        if (item2->checkCollisionWithCamera(cameraX, cameraY, cameraZ)) {
            item2Exists = false;
            delete item2;
        }
    }

    if (item3Exists) {
        if (item3->checkCollisionWithCamera(cameraX, cameraY, cameraZ)) {
            item3Exists = false;
            delete item3;
        }
    }

    // 배터리 전부 획득 시 잠금 장치 제거
    if (item1Exists == false && item2Exists == false && item3Exists == false && lockExists) {
        releaseWallsLock();
    }

    updateCamera();
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1200, 900);
    glutCreateWindow("3DMaze_20201772");

    glEnable(GL_DEPTH_TEST);

    // 메인 구역 잠금 장치 생성
    generateWallsMain();
    generateWallsLock();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}
