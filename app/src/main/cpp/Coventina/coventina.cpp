// vim: sw=4 expandtab

#include "enums.hpp"
#include "Coin.hpp"
#include "matrix.h"
#include "opengl/programs.h"
#include "glm/vec3.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include <array>
#include <cstdint>
#include <cmath>

#include <android/log.h>

#include "assets.hpp"

#define LOGe(...) __android_log_print(ANDROID_LOG_ERROR, "Coventina Error", __VA_ARGS__)
#define LOGd(...) __android_log_print(ANDROID_LOG_DEBUG, "Coventina Debug", __VA_ARGS__)


using namespace std;

static std::array<glm::vec3, 2> faceNormals = {{
	{ 0., 0., 1. },  // front face
	{ 0., 0., -1. }  // back face
}};

static GLuint coin_glbufs[3];

constexpr int FACE_VERT_COUNT = 22;
constexpr int HEAD_IDX = 0;
constexpr int TAIL_START_IDX = FACE_VERT_COUNT;
constexpr int EDGE_START_IDX = TAIL_START_IDX + FACE_VERT_COUNT;
constexpr int EDGE_VERT_COUNT = 42;

namespace game
{
    float Coin::yAngle = 0.;
    float Coin::yTrans = 0.;
    float Coin::yDir = 0.;

    void Coin::genGraphics()
    {
        std::vector<glm::vec3> coinVertices;
        std::vector<glm::vec3> coinNormals;
        std::vector<uint16_t> coinIndices;

        coinVertices.reserve(FACE_VERT_COUNT*2);
        coinNormals.reserve(FACE_VERT_COUNT*2);
        coinIndices.reserve(FACE_VERT_COUNT*2);

        // front face
        coinVertices.push_back(glm::vec3{0.0,0.0,0.05});
        coinNormals.push_back(glm::vec3{0.0,0.0,1.0});
        // back face
        coinVertices.push_back(glm::vec3{0.0,0.0,-0.05});
        coinNormals.push_back(glm::vec3{0.0,0.0,-1.0});
        for (int i=0; i<FACE_VERT_COUNT-1; ++i)
        {
            float angle = i * M_PI * 0.1;
            float x = std::sin(angle);
            float y = std::cos(angle);
            auto norm = glm::vec3{x,y,0.};
            coinVertices.push_back(glm::vec3{x*0.5,y*0.5,0.05});
            coinNormals.push_back(norm);
            coinVertices.push_back(glm::vec3{x*0.5,y*0.5,-0.05});
            coinNormals.push_back(norm);
            //printf("%d: %.2f,%.2f\n", i, x, y);
        }

        coinIndices.push_back(0);
        for (int i=0; i<FACE_VERT_COUNT-1; ++i)
        {
            coinIndices.push_back(2+i*2);
        }
        assert(coinIndices.size() == TAIL_START_IDX);
        coinIndices.push_back(1);
        for (int i=0; i<FACE_VERT_COUNT-1; ++i)
        {
            coinIndices.push_back(3+i*2);
        }

        glGenBuffers(3,coin_glbufs);
        glBindBuffer(GL_ARRAY_BUFFER, coin_glbufs[VERTBUF]);
        glBufferData(GL_ARRAY_BUFFER, coinVertices.size() * sizeof(glm::vec3), &coinVertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, coin_glbufs[NORMBUF]);
        glBufferData(GL_ARRAY_BUFFER, coinNormals.size() * sizeof(glm::vec3), &coinNormals[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,coin_glbufs[INDXBUF]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,coinIndices.size() * 2,&coinIndices[0],GL_STATIC_DRAW);
    }

    void Coin::draw()
    {
        float gold[] = {1.,0.843,0.,1.};
        auto prevModel = mat::model;
        mat::translate(pos[0]+0.5, pos[1]+0.5, pos[2]+0.5);
        auto vmmat = mat::view * mat::model;
        glm::vec3 lightPos = glm::vec3{vmmat * glm::vec4{2.,2.,2., 1.}};
        glm::vec3 light2Pos = glm::vec3{vmmat * glm::vec4{-2.,-2.,-2., 1.}};
        mat::translate(0., yTrans, 0.);
        mat::rotate(yAngle, 0., 1., 0);
        auto prog = static_cast<program::LightenFixedColor*>(program::getProgram(program::LIGHTEN_FIXED_COLOR));
        prog->bind();
        mat::downloadMVP();
        vmmat = mat::view * mat::model;
        glUniformMatrix4fv(prog->mvmUniform, 1, GL_FALSE, glm::value_ptr(vmmat));
        mat::model = prevModel;
        glBindBuffer(GL_ARRAY_BUFFER, coin_glbufs[VERTBUF]);
        glVertexAttribPointer(prog->vertHandle,3,GL_FLOAT,false,0,0);
        glEnableVertexAttribArray(prog->vertHandle);
        glBindBuffer(GL_ARRAY_BUFFER, coin_glbufs[NORMBUF]);
        glVertexAttribPointer(prog->normalHandle,3,GL_FLOAT,false,0,0);
        glEnableVertexAttribArray(prog->normalHandle);
        glUniform4fv(prog->colorHandle,1,gold);
        glUniform1f(prog->fogDensityHandle, 0.0);
        glUniform3fv(prog->lightPosUniform, 1, glm::value_ptr(lightPos));
        glUniform3fv(prog->light2PosUniform, 1, glm::value_ptr(light2Pos));
        glUniform1i(prog->doLightUniform, 1);
        glUniform1i(prog->doLight2Uniform, 1);
        glUniform1f(prog->shininessUniform, 1.);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,coin_glbufs[INDXBUF]);
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CW);
        glUniform1i(prog->doUniformNormalsUniform, 0);
        glDrawArrays(GL_TRIANGLE_STRIP, 2, EDGE_VERT_COUNT);
        glUniform1i(prog->doUniformNormalsUniform, 1);
        for (int i=0; i<2; ++i)
        {
            glUniform3fv(prog->normalUniform, 1, &faceNormals[i][0]);
            glDrawElements(GL_TRIANGLE_FAN,FACE_VERT_COUNT,GL_UNSIGNED_SHORT,reinterpret_cast<void*>(i*2*FACE_VERT_COUNT));
            glFrontFace(GL_CCW);
        }
        glDisable(GL_CULL_FACE);
        glDisableVertexAttribArray(prog->vertHandle);
        glDisableVertexAttribArray(prog->normalHandle);
    }

    void Coin::update(float dt)
    {
        yAngle += dt * 0.02/10.0;
        if (yAngle > 2.*M_PI)
            yAngle = 0.;
        yTrans += (dt * yDir) / 500.0;
        if (yTrans <= 0.0)
        {
            yTrans = 0.;
            yDir = 1;
        }
        else if (yTrans > 0.5)
        {
            yTrans = 0.5;
            yDir = -1;
        }
    }
}

// vim: sw=4 expandtab

#include "enums.hpp"
#include "Cube.hpp"
#include "CubeMesh.hpp"
#include "matrix.h"
#include "opengl/programs.h"
#include "glm/vec3.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include <array>
#include <cstdint>
#include <cmath>

using namespace std;

static std::array<glm::vec3, 8> cubeVertices = {{
	{ 0., 0., 0. }, { 0., 1., 0.}, // 0 (left,  bottom, back),  1 (left, top, back)
	{ 1., 0., 0. }, { 1., 1., 0.}, // 2 (right, bottom, back),  3 (right, top, back)
	{ 1., 0., 1. }, { 1., 1., 1.}, // 4 (right, bottom, front), 5 (right, top, front)
	{ 0., 0., 1. }, { 0., 1., 1.}  // 6 (left,  bottom, front), 7 (left,  top, front)
}};

#if 0
static std::array<glm::vec3, 6*4> cubeNormals = {{
	{ -1., 0., 0. }, { -1., 0., 0. }, { -1., 0., 0. }, { -1., 0., 0. }, // left face
	{ 0., 0., 1. }, { 0., 0., 1. }, { 0., 0., 1. }, { 0., 0., 1. },     // front face
	{ 1., 0., 0. }, { 1., 0., 0. }, { 1., 0., 0. }, { 1., 0., 0. },     // right face
	{ 0., 0., -1. }, { 0., 0., -1. }, { 0., 0., -1. }, { 0., 0., -1. }, // back face
	{ 0., 1., 0. }, { 0., 1., 0. }, { 0., 1., 0. }, { 0., 1., 0. },     // top face
        { 0., -1., 0.}, { 0., -1., 0.}, { 0., -1., 0.}, { 0., -1., 0.}      // bottom face
}};
#endif
static std::array<glm::vec3, 6> cubeNormals = {{
	{ -1., 0., 0. },  // left face
	{ 0., 0., 1. },   // front face
	{ 1., 0., 0. },   // right face
	{ 0., 0., -1. },  // back face
	{ 0., 1., 0. },   // top face
        { 0., -1., 0.},   // bottom face
}};

enum {
	LBB, LTB,
	RBB, RTB,
	RBF, RTF,
	LBF, LTF
};

static std::array<uint16_t, 24> cubeIndices = {{
    LTB, LBB, LTF, LBF, // left face
    LTF, LBF, RTF, RBF, // front face
    RTF, RBF, RTB, RBB, // right face
    RTB, RBB, LTB, LBB, // back face
    RTB, LTB, RTF, LTF, // top face
    LBB, RBB, LBF, RBF  // bottom face

}};

static GLuint cube_glbufs[3];

namespace game
{
    void Cube::genGraphics()
    {
        glGenBuffers(3,cube_glbufs);
        glBindBuffer(GL_ARRAY_BUFFER, cube_glbufs[VERTBUF]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices[0], GL_STATIC_DRAW);
        //glBindBuffer(GL_ARRAY_BUFFER, glbufs[NORMBUF]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), &cubeNormals[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,cube_glbufs[INDXBUF]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(cubeIndices),&cubeIndices[0],GL_STATIC_DRAW);
    }

    void Cube::draw()
    {
        float gray[] = {0.5,0.5,0.5,1.};
        //float green[] = {0.,1.,0.,1.};
        float blue[] = {0.0,0.5,1.0,1.};
        auto prevModel = mat::model;
        mat::translate(pos[0], pos[1], pos[2]);
        auto prog = static_cast<program::LightenFixedColor*>(program::getProgram(program::LIGHTEN_FIXED_COLOR));
        prog->bind();
        mat::downloadMVP();
        glUniformMatrix4fv(prog->mvmUniform, 1, GL_FALSE, glm::value_ptr(mat::view*mat::model));
        mat::model = prevModel;
        glBindBuffer(GL_ARRAY_BUFFER, cube_glbufs[VERTBUF]);
        glVertexAttribPointer(prog->vertHandle,3,GL_FLOAT,false,0,0);
        //glBindBuffer(GL_ARRAY_BUFFER, glbufs[NORMBUF]);
        if (type == WallBlockType)
            glUniform4fv(prog->colorHandle,1,gray);
        else
            glUniform4fv(prog->colorHandle,1,blue);
        glUniform1f(prog->fogDensityHandle, 0.02);
        auto lightPos = glm::vec3{mat::view * glm::vec4{10., 30., 20., 0.}};
        glUniform3fv(prog->lightPosUniform, 1, glm::value_ptr(lightPos));
        auto light2Pos = glm::vec3{mat::view * glm::vec4{-10., 30., -40., 0.}};
        glUniform3fv(prog->light2PosUniform, 1, glm::value_ptr(light2Pos));
        glUniform1i(prog->doLightUniform, 1);
        glUniform1i(prog->doLight2Uniform, 1);
        glUniform1i(prog->doUniformNormalsUniform, 1);
        glUniform1f(prog->shininessUniform, 0.);
        //glEnableVertexAttribArray(prog->normalHandle);
        glEnableVertexAttribArray(prog->vertHandle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_glbufs[INDXBUF]);
        for (int i=0; i<6; ++i)
        {
            //glVertexAttribPointer(prog->normalHandle,3,GL_FLOAT,false,0,reinterpret_cast<void*>(0));
            glUniform3fv(prog->normalUniform, 1, &cubeNormals[i][0]);
            glDrawElements(GL_TRIANGLE_STRIP,4,GL_UNSIGNED_SHORT,reinterpret_cast<void*>(i*2*4));
        }
        glDisableVertexAttribArray(prog->vertHandle);
    }
}

// vim: sw=4 expandtab

#include "CubeMesh.hpp"
#include "fileSystem.h"
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

extern unsigned WinScore;

constexpr int XD = 64;
constexpr int ZD = 64;
constexpr int YD = 8;

constexpr int XD2 = XD/2;
constexpr int ZD2 = ZD/2;
constexpr int YD2 = 1;

namespace game
{
    static vector<MeshItem*> cubemesh(XD*ZD*YD);

    static MeshItem noNode;

    const MeshItem &CubeMesh::getNode(int x, int y, int z)
    {
        if (y + YD2 < 0 || y + YD2 >= YD ||
                x + XD2 < 0 || x + XD2 >= XD ||
                z + ZD2 < 0 || z + ZD2 >= ZD)
            return noNode;
        int index = (y+YD2)*XD*ZD + (z+ZD2)*XD + (x+XD2);
        if (index < 0 || index >= cubemesh.size())
            return noNode;
        if (cubemesh[index] == nullptr)
            return noNode;
        return *cubemesh[index];
    }

    void CubeMesh::clearNode(const MeshItem &node)
    {
        auto &n = *cubemesh[node.midx];
        if (n.type == 0)
            return;
        MeshItem *item{nullptr};
        switch (n.type)
        {
        case CoinType:
            item = coins[node.idx].get();
            break;
        case RingType:
            item = rings[node.idx].get();
            break;
        default:
            item = cubes[node.idx].get();
            break;
        }
        item->valid = false;
        n.type = 0;
    }

    void CubeMesh::addCube(Cube cube)
    {
        auto &pos = cube.pos;
        auto ctype = cube.type;
        int index = (pos[1]+YD2)*XD*ZD + (pos[2]+ZD2)*XD + (pos[0]+XD2);
        auto c = cubemesh[index];
        while (c != nullptr && c->type > 0 && c->type <= 2 && pos[1]+YD2 < YD)
        {
            ++pos[1];
            index = (pos[1]+YD2)*XD*ZD + (pos[2]+ZD2)*XD + (pos[0]+XD2);
        }
        if (c != nullptr)
        {
            //SDL_Log("overlap");
            return;
        }
        cubes.push_back(make_shared<Cube>(move(cube)));
        c = cubes.back().get();
        c->type = ctype;
        c->idx = cubes.size() - 1;
        c->midx = index;
    }

    void CubeMesh::addCoin(Coin coin)
    {
        auto &pos = coin.pos;
        int index = (pos[1]+YD2)*XD*ZD + (pos[2]+ZD2)*XD + (pos[0]+XD2);
        while (cubemesh[index] != nullptr && /*cubemesh[index]->type > 0 && cubemesh[index]->type <= 2 && */ pos[1]+YD2 < YD)
        {
            ++pos[1];
            index = (pos[1]+YD2)*XD*ZD + (pos[2]+ZD2)*XD + (pos[0]+XD2);
        }

        if (cubemesh[index] != nullptr)
        {
            //SDL_Log("overlap");
            return;
        }
        coins.push_back(make_shared<Coin>(move(coin)));
        cubemesh[index] = coins.back().get();
        cubemesh[index]->type = CoinType;
        cubemesh[index]->idx = coins.size() - 1;
        cubemesh[index]->midx = index;
    }

    void CubeMesh::addRing(Ring ring)
    {
        auto &pos = ring.pos;
        int index = (pos[1]+YD2)*XD*ZD + (pos[2]+ZD2)*XD + (pos[0]+XD2);
        while (cubemesh[index] != nullptr && /*cubemesh[index]->type > 0 && cubemesh[index]->type <= 2 && */ pos[1]+YD2 < YD)
        {
            ++pos[1];
            index = (pos[1]+YD2)*XD*ZD + (pos[2]+ZD2)*XD + (pos[0]+XD2);
        }

        if (cubemesh[index] != nullptr)
        {
            //SDL_Log("overlap");
            return;
        }
        rings.push_back(make_shared<Ring>(move(ring)));
        cubemesh[index] = rings.back().get();
        cubemesh[index]->type = RingType;
        cubemesh[index]->idx = rings.size() - 1;
        cubemesh[index]->midx = index;
    }

    void CubeMesh::readMap(const char *filename)
    {
        int z = -ZD2;
        int y = 0;

        wellCenter = glm::ivec2{0, -5};
        playerStart = glm::ivec2{0, 3};

        string line;
        std::string newFilename = (fileSystem::getAssetsPath()+std::string(filename));
        memstream f(newFilename);
        while (getline(f, line))
        {
            int numx = int(line.size());
            for (int i=0; i<numx; ++i)
            {
                int x = i-XD2;

                if (line[i] == '*')
                {
                    playerStart = glm::ivec2{x,z};
                }
                else if (line[i] != ' ')
                {
                    Cube cube;
                    cube.pos = {x,y,z};
                    if (line[i] == '~')
                        cube.type = WaterBlockType;
                    else if (line[i] == '+')
                    {
                        wellCenter = glm::ivec2{x,z};
                        cube.type = WaterBlockType;
                    }
                    else if (line[i] == '=')
                    {
                        cube.type = WallBlockType;
                    }
                    else
                    {
                        cube.type = WallBlockType;
                        addCube(cube);
                        cube.pos[1] += 1;
                    }
                    addCube(move(cube));
                }
            }
            ++z;
        }
    }

    void CubeMesh::readCubes(const char *filename)
    {
        string line;
        std::string newFilename = (fileSystem::getAssetsPath()+std::string(filename));
        memstream f(newFilename);
        int linenum = 0;
        while (getline(f, line))
        {
            ++linenum;
            auto p = line.find_first_not_of(" \t");
            if (p == string::npos)
                continue;
            else if (line[p] == '#')
                continue;
            int x, y, z, type;
            if (sscanf(line.c_str(), "%d %d %d %d", &x, &y, &z, &type) != 4)
            {
                cerr << filename << ": parse error on line " << linenum << ": " << line << endl;
                continue;
            }

            if (type == CoinType)
            {
                Coin coin;
                coin.pos = {x,y,z};
                addCoin(move(coin));
            }
            else if (type == RingType)
            {
                Ring ring;
                ring.pos = {x,y,z};
                addRing(move(ring));
            }
            else
            {
                Cube cube;
                cube.pos = {x,y,z};
                cube.type = type;
                addCube(move(cube));
            }
        }

    }
    void CubeMesh::addRandoms()
    {
        int nRandCoins = WinScore * 2 + 5;
        int nRandRings = WinScore + 3;
        int x, y, z;

#if 0
        for (int i=0; i<60; ++i)
        {
            do
            {
                x = float(rand()) * XD / RAND_MAX - XD2;
                y = 0;
                z = float(rand()) * ZD / RAND_MAX - ZD2;
            } while (std::abs(x) <= 7 && std::abs(z-1) <= 7);
            Cube cube;
            cube.pos = {x,y,z};
            cube.type = 1;
            addCube(move(cube));
        }
#endif

        for (int i=0; i<nRandCoins; ++i)
        {
            do
            {
                x = float(rand()) * XD / RAND_MAX - XD2;
                y = 0;
                z = float(rand()) * ZD / RAND_MAX - ZD2;
            } while (std::abs(x) <= 7 && std::abs(z-1) <= 7);
            Coin coin;
            coin.pos = {x,y,z};
            addCoin(move(coin));
        }
        for (int i=0; i<nRandRings; ++i)
        {
            do
            {
                x = float(rand()) * XD / RAND_MAX - XD2;
                y = 0;
                z = float(rand()) * ZD / RAND_MAX - ZD2;
            } while (std::abs(x) <= 7 && std::abs(z-1) <= 7);
            Ring ring;
            ring.pos = {x,y,z};
            addRing(move(ring));
        }
    }
}

// vim: sw=4 expandtab

#include "CuttleFish.h"

#include "screens/GameScreen.h"
#include "opengl/programs.h"
#include "CubeMesh.hpp"
#include "read.h"

#include "loop.h"

#include "matrix.h"

#include "extra.h"

std::vector<Shape*> CuttleFish::shapes;

TexturedShape* CuttleFish::poker;

unsigned short CuttleFish::pokerVertIndice = 0;

extern glm::ivec2 wellCenter;


CuttleFish::CuttleFish(glm::vec3 center)
    : center(center)
{
    pos = center;
    dir = {0.,0.,1.};
    speed = 0.01/3.0;
    angle = 0.;

#if 0
    pokerPosInc = {0,0,0};

    inc(&pokerPosInc.z,1.);
#endif

};

void CuttleFish::genGraphics(){

  shapes = read::holden("CuttelFish.holden");

#if 0
  std::vector<Shape*> tmp = read::holden("Cube.holden");

  SDL_Log(("tmp size: "+toString(tmp.size())).c_str());

  poker = dynamic_cast<TexturedShape*>(tmp[0]);
#endif
}

void CuttleFish::update(float dt)
{
    pos += speed * dir * dt;
    if (glm::length(pos - center) > 1.6)
    {
        //printf("before: angle = %f, pos=%.2f,%.2f,%.2f,dir=%.2f,%.2f,%.2f\n", angle, pos[0],pos[1],pos[2],dir[0],dir[1],dir[2]);
        pos = center + dir * 1.6f;
        angle += M_PI + float(rand()) / RAND_MAX * 1;
        if (angle > M_PI * 2.)
            angle = angle - M_PI * 2.;
        //printf("after: angle = %f, pos=%.2f,%.2f,%.2f\n", angle, pos[0],pos[1],pos[2]);
    }

#if 0
  movePokerPos(
      pokerPosInc.x,
      pokerPosInc.y,
      pokerPosInc.z
  );

  pos.x+=pokerPosInc.z;
#endif
}

void CuttleFish::draw(){

  program::bind(program::LIGHTEN_TEXTURED_FIXED_COLOR);

  program::LightenTexturedFixedColor* p = static_cast<program::LightenTexturedFixedColor*>(program::getBound());

  auto prevModel = mat::model;
  mat::translate(pos.x,pos.y,pos.z);
  mat::rotate(angle, 0., 1., 0.);

  auto mvmat = mat::view*mat::model;
  dir = {mvmat[2][0],0.,mvmat[2][2]};

  glEnableVertexAttribArray(p->vertHandle);
  glEnableVertexAttribArray(p->texCoordHandle);
  glEnableVertexAttribArray(p->normalHandle);

  glUniform1i(p->doTexOffsetHandle,1);
  glUniform2f(p->texCoordOffsetHandle,GameScreen::instance->offsetX,GameScreen::instance->offsetY);
  glUniform1i(p->doLightUniform,0);
  glUniform1i(p->doLight2Uniform,0);

  //auto vec = mvmat * glm::vec4(0,0,0,1);
  //glUniform3f(p->lightPosUniform,vec.x,vec.y,vec.z);
  //glUniform1f(p->shininessUniform, 0.);
  //glUniform1i(p->doUniformNormalsUniform, 0);

  for(int i=0;i<shapes.size();i++)
    shapes[i]->draw();

  //poker->draw();

  mat::model = prevModel;
}

void CuttleFish::setPokerPos(float x,float y,float z){

  pokerPos.x = x;
  pokerPos.y = y;
  pokerPos.z = z;

  glBindBuffer(GL_ARRAY_BUFFER,poker->vertBuf);
  glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(Vec),&pokerPos);

  //SDL_Log(("Poker pos z: "+toString(pokerPos.z)).c_str());

}

void CuttleFish::movePokerPos(float x,float y,float z){

  setPokerPos(
      pokerPos.x+x,
      pokerPos.y+y,
      pokerPos.z+z
  );

}
#include "extra.h"
#include <sstream>
#include "assets.hpp"

std::string toString(float in){
  std::ostringstream buff;
  buff<<in;
  return buff.str();
}
#include "fileSystem.h"
#include <android/log.h>
#include <sstream>

std::string fileSystem::getAssetsPath()
{
  std::string path;
#ifndef __ANDROID__
#ifdef BRYAN_
  char cwdbuf[512];
  std::string home = getenv("PWD");
  path = home + "/assets/";
#else
  //std::string home = getenv("HOME");
  //path = home + "/Desktop/Programming/c++/android/SDL2-2.0.3/android-project/assets/";
  path = "./assets/";
#endif
#endif
  return path;
}

// vim: sw=4 expandtab

#include "enums.hpp"
#include "Floor.hpp"
#include "Player.hpp"
#include "matrix.h"
#include "opengl/programs.h"
#include "glm/vec3.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include <cmath>

using namespace std;

static GLuint floor_glbufs[2];
static int nverts;
static int nindices;
GLuint texture;

Surface *surface;

enum {
	TEXCBUF = 1
};

namespace game
{
    Floor floor;

    void Floor::genGraphics()
    {
        constexpr int xdim = 80;
        constexpr int zdim = 40;

        nverts = xdim*zdim;
        vector<glm::vec3> verts(nverts);
        vector<glm::vec2> texcoords(nverts);

        constexpr float xbase = -40.;
        constexpr float zbase = -20.;
        constexpr float w = 1.;

        int index = 0;
        for (int z=0; z<zdim; ++z)
        {
            for (int x=0; x<xdim; ++x)
            {
                texcoords[index] = {x*w, z*w};
                verts[index++] = {xbase+x*w, 0, zbase+z*w};
            }
        }

        glGenBuffers(3,floor_glbufs);
        glBindBuffer(GL_ARRAY_BUFFER, floor_glbufs[VERTBUF]);
        glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(verts[0]), glm::value_ptr(verts[0]), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, floor_glbufs[TEXCBUF]);
        glBufferData(GL_ARRAY_BUFFER, texcoords.size()*sizeof(texcoords[0]), glm::value_ptr(texcoords[0]), GL_STATIC_DRAW);

        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        surface = new Surface("grass.png");
        if (surface->components == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->data);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->data);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glDisable(GL_TEXTURE_2D);

        int ntriangles = (xdim-1) * (zdim-1) * 2;
        nindices = ntriangles * 3;
        vector<uint16_t> indices(nindices);

        index = 0;
        for (int z=0; z<zdim-1; ++z)
        {
            for (int x=0; x<xdim-1; ++x)
            {
                int baseidx = z*xdim + x;
                int baseidx1 = (z+1)*xdim + x;
                indices[index++] = baseidx1;
                indices[index++] = baseidx + 1;
                indices[index++] = baseidx;
                indices[index++] = baseidx1 + 1;
                indices[index++] = baseidx + 1;
                indices[index++] = baseidx1;
            }
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floor_glbufs[INDXBUF]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * 2, &indices[0], GL_STATIC_DRAW);
    }

    void Floor::draw()
    {
        float green[] = {0.05,0.7,0.05,1.};
        auto prevModel = mat::model;
        float rot = int((player.yangle+M_PI_4) * M_2_PI) * M_PI_2;
        float delta = (player.yangle - rot) * (1./M_PI_4);
        mat::translate(floorf(player.pos[0]), -0.01, floorf(player.pos[2]));
        mat::rotate(rot, 0., -1., 0.);
        mat::translate(floorf(delta * 15.), 0., -17.);
        auto prog = static_cast<program::TexturedFixedColor*>(program::getProgram(program::TEXTURED_FIXED_COLOR));
        prog->bind();
        mat::downloadMVP();
        glBindBuffer(GL_ARRAY_BUFFER, floor_glbufs[VERTBUF]);
        glVertexAttribPointer(prog->vertHandle,3,GL_FLOAT,false,0,0);
        glBindBuffer(GL_ARRAY_BUFFER, floor_glbufs[TEXCBUF]);
        glVertexAttribPointer(prog->texCoordHandle,2,GL_FLOAT,false,0,0);
        glUniform4fv(prog->colorHandle,1,green);
        glUniform1f(prog->fogDensityHandle, 0.1);
        glEnableVertexAttribArray(prog->vertHandle);
        glEnableVertexAttribArray(prog->texCoordHandle);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(prog->textureUniform, 0);
        glUniform1i(prog->doTexOffsetHandle,1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floor_glbufs[INDXBUF]);
        glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, nullptr);
        //glDrawArrays(GL_LINES, 0, nverts);
        mat::model = prevModel;
    }
}

#include "loop.h"
#include "extra.h"
#include "screen.h"
#include "screens/GameScreen.h"

#include "time.h"

float avgFps = 0.;

namespace loop{

  Screen* currentScreen = GameScreen::instance;

  unsigned long tick = 0;
  unsigned long tickCounter = 0;
  float fpsSum = 0.0;

  unsigned long prevTick;

  float calculatedFps;

  std::vector<float> incrementValues;
  std::vector<float*> incrementPointers;

  struct timespec now;

  uint64_t getTicks() {
      clock_gettime(CLOCK_MONOTONIC, &now);
      return now.tv_sec*1000000000LL + now.tv_nsec;
  }

  void init() {
    LOGd("entering init");
    tick = getTicks();
    LOGd("mid init");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    LOGd("leaving init");
  }

  void loop(){

    prevTick = tick;
    tick = getTicks();

    int dif = tick-prevTick;
    if (dif == 0) return;

    dif /= 1000;

    float dt = dif * 0.001;
    calculatedFps = 1.0 / dt;

    constexpr bool showFps = true;
    if (showFps)
    {
      fpsSum += calculatedFps;
      if (++tickCounter == 120)
      {
        avgFps = fpsSum / 120;
        fpsSum = 0.;
        tickCounter = 0;

        //SDL_Log("calculated fps: %.1f;  pixelBounds: %d, %d", avgFps, screen::pixelBounds[0], screen::pixelBounds[1]);
      }
    }

    for(int i=0;i<incrementValues.size();i++)
      *incrementPointers[i] = incrementValues[i]/calculatedFps;

    currentScreen->update(dt);

  }

}

void inc(float* increment,float movementPerSecond){

  loop::incrementValues.push_back(movementPerSecond);

  loop::incrementPointers.push_back(increment);

}
#include "MachineGun.h"
#include "Player.hpp"
#include "loop.h"
#include "matrix.h"
#include "read.h"

float HeldMachineGun::timeTillNextFireInc;

HeldMachineGun::HeldMachineGun(){

  inc(&timeTillNextFireInc,1.);

}

/*
void HeldMachineGun::keyInput(SDL_Event e){

  switch(e.type){
    case SDL_KEYDOWN:
      switch(e.key.keysym.sym){
	case SDLK_a:
	  firing = true;
	  break;
      }
      break;
    case SDL_KEYUP:
      switch(e.key.keysym.sym){
	case SDLK_a:
	  firing = false;
	  break;
      }
      break;
  }

}
 */

void HeldMachineGun::update(){

  if(firing){

    if(timeTillNextFireCache >= 1.){

      fire();

      timeTillNextFireCache = 0;
    }

    timeTillNextFireCache += timeTillNextFireInc;

  }

}

void HeldMachineGun::draw(){



}

void HeldMachineGun::fire(){

  //SDL_Log(("FIRE"));

  Bullet* bullet = new Bullet();

  bullet->acceleration = {game::player.eyeVector[0],game::player.eyeVector[1],game::player.eyeVector[2]};

  Vec* a = &bullet->acceleration;

  inc(&a->x,a->x);
  inc(&a->y,a->y);
  inc(&a->z,a->z);

  bullet->pos = {game::player.pos[0],game::player.pos[1],game::player.pos[2]};

  bullet->velocity = {a->x*2,a->y*2,a->z*2};

  Bullet::instances.push_back(bullet);

}

std::vector<Bullet*> Bullet::instances;

LightenTexturedShape* Bullet::shape;

void Bullet::update(){

  pos.x += velocity.x;
  pos.y += velocity.y;
  pos.z += velocity.z;

  velocity.x -= acceleration.x;
  velocity.y -= acceleration.y;
  velocity.z -= acceleration.z;

}

void Bullet::draw(){

  mat::translate(pos.x,pos.y,pos.z);

  shape->draw();

  mat::translate(-pos.x,-pos.y,-pos.z);

}

void Bullet::genGraphics(){

  shape = dynamic_cast<LightenTexturedShape*>(read::holden("Ring2.holden")[0]);

}
#include "main.h"
#include "screen.h"
#include "loop.h"
#include "opengl/programs.h"

static bool did_init = false;

int init(){

    if(did_init) {
        gl::init();
        program::init();
        return 0;
    } else {
        did_init = true;
    }

  srand(time(0));

  screen::init();

  gl::init();

  program::init();

  loop::init();

  return 0;

}
// vim: sw=2 expandtab
#include "matrix.h"

#include "opengl/opengl.h"
#include "opengl/programs.h"

namespace mat{

  glm::mat4 projection = glm::mat4(1);
  glm::mat4 view = glm::mat4(1);
  glm::mat4 model = glm::mat4(1);

  void downloadMVP(){
    auto mvp = projection*view*model;
    glUniformMatrix4fv(glGetUniformLocation(program::getBound()->id, "MVP"), 1, GL_FALSE, glm::value_ptr(mvp));
  }

  void clearMatrices(){
    projection = glm::mat4(1);
    view = glm::mat4(1);
    model = glm::mat4(1);
  }

  void ortho(float left,float right,float bottom,float top,float near,float far){
    projection = glm::ortho(left,right,bottom,top,near,far);
  }

  void perspective(float fov,float aspectRatio,float nearClip,float farClip){
    projection = glm::perspective(fov,aspectRatio,nearClip,farClip);
  }

  void translate(float x,float y,float z){
    model = glm::translate(model,glm::vec3(x,y,z));
  }

  void rotate(float radians,int x,int y,int z){
    model = glm::rotate(model,radians,glm::vec3(x,y,z));
  }

  void scale(float x,float y,float z){
    model = glm::scale(model,glm::vec3(x,y,z));
  }

}
// vim: expandtab sw=4
#include "Player.hpp"
#include "screen.h"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <GLES3/gl3.h>
#include <cstdio>

#include "screens/GameScreen.h"

constexpr float DEGTORAD = M_PI / 180.;
constexpr float PI_2 = M_PI * 2.;

constexpr float PlayerSpeed = 6.0;
constexpr float JumpSpeed = 5.0;
constexpr float HopSpeed = 1.0;
constexpr float RotationSpeed = 120.0 * DEGTORAD;
constexpr float GravityAccel = -9.0;
constexpr float PlayerHalfWidth = 0.4;
constexpr float PlayerHalfDepth = 0.3;
constexpr float PlayerHeight = 2.0;
constexpr float PlayerEyeLevel = 1.5;
const glm::vec3 PlayerBounds[2] = {
    { -PlayerHalfWidth, 0., -PlayerHalfDepth },
    { PlayerHalfWidth, PlayerHeight, PlayerHalfDepth }
};

enum {
    LeftKey = 0x1,
    RightKey = 0x2,
    ForwardKey = 0x4,
    BackwardKey = 0x8
};

uint8_t keys = 0;

extern game::CubeMesh cubeMesh;

namespace game
{
    Player player = {
        .pos = {0., 0., 3.},
        .yangle = 0.,
        .xangle = 0.,
        .zspeed = 0.0,
        .xspeed = 0.0,
        .yspeed = 0.0,
        .yaccel = 0.0,
        .inAir = false,
        .wallClimbing = false,
        .eyeVector = {0., 0., -1.},
        .xVector = {1., 0., 0.},
        .zVector = {0., 0., -1.},
        .velocity = {0., 0., 0.},
        .viewMat = glm::mat4{1.,0.,0.,0., 0.,1.,0.,0., 0.,0.,1.,0., 0.,-PlayerEyeLevel,0.,1.},
        .yAngularVelocity = 0.,
        .xAngularVelocity = 0.,
        .pointerMode = false,
        .isRotating = false,
        .zRunning = false,
        .xRunning = false,
        .ringCount = 0,
        .coinCount = 0,
        .score = 0
    };

    void Player::update(float dt)
    {
        static glm::vec3 lastPos = {0.,0.,0.};

        pos += velocity * dt;

        isRotating = true;
        yangle += (dt * 0.0005) / 5.0;

        if (yAngularVelocity > 0.01 || yAngularVelocity < -0.01)
        {
            isRotating = true;
            yangle += yAngularVelocity * dt;
            if (yangle >= PI_2)
                yangle -= PI_2;
            else if (yangle < 0.0)
                yangle = PI_2 + yangle;
        }
        if (xAngularVelocity > 0.01 || xAngularVelocity < -0.01)
        {
            isRotating = true;
            xangle += xAngularVelocity * dt;
            if (xangle > M_PI_2)
                xangle = M_PI_2;
            else if (xangle < -M_PI_2)
                xangle = -M_PI_2;
        }

        if (isRotating || pos != lastPos)
        {
            isRotating = false;
            viewMat = glm::rotate(glm::mat4(1.0f), yangle, glm::vec3(0.,1.,0.));
            zVector[0] = viewMat[2][0];
            zVector[1] = 0;
            zVector[2] = -viewMat[2][2];
            xVector[0] = viewMat[0][0];
            xVector[1] = 0;
            xVector[2] = -viewMat[0][2];
            viewMat = glm::rotate(glm::mat4(1.0f), xangle, glm::vec3(1.,0.,0.)) * viewMat;

            eyeVector[0] = viewMat[2][0];
            eyeVector[1] = viewMat[2][1];
            eyeVector[2] = -viewMat[2][2];

            glm::vec3 trans = {-pos[0], -pos[1] - PlayerEyeLevel, -pos[2]};
            viewMat = glm::translate(viewMat, trans);
        }

        lastPos = pos;

        velocity = {0., yspeed, 0.};
        if ((!inAir) || zRunning)
            velocity += (zspeed * zVector);
        if ((!inAir) || xRunning)
            velocity += (xspeed * xVector);
        yspeed += yaccel * dt;
        if (yspeed > JumpSpeed)
        {
            yspeed = JumpSpeed;
        }
        else if (yspeed < -JumpSpeed)
        {
            yspeed = -JumpSpeed;
        }
    }

    //static int moveval = 0;

    void Player::runForward()
    {
        keys |= ForwardKey;
        player.zspeed = PlayerSpeed;
        if (!inAir)
        {
            player.zRunning = true;
        }
        //printf("forward %d r=%d\n", moveval++, player.zRunning);
    }

    void Player::runBackward()
    {
        keys |= BackwardKey;
        player.zspeed = -PlayerSpeed;
        if (!inAir)
        {
            player.zRunning = true;
        }
        //printf("backward %d r=%d\n", moveval++, player.zRunning);
    }

    void Player::stopForward()
    {
        keys &= ~ForwardKey;
        if (player.zspeed > 0)
        {
            //printf("zstop %d\n", moveval++);
            zspeed = 0.0;
            player.zRunning = false;
            if (keys & BackwardKey)
            {
                runBackward();
            }
        }
    }

    void Player::stopBackward()
    {
        keys &= ~BackwardKey;
        if (player.zspeed < 0)
        {
            //printf("zstop %d\n", moveval++);
            zspeed = 0.0;
            player.zRunning = false;
            if (keys & ForwardKey)
            {
                runForward();
            }
        }
    }

    void Player::runLeft()
    {
        keys |= LeftKey;
        player.xspeed = PlayerSpeed;
        if (!inAir)
        {
            player.xRunning = true;
        }
        //printf("left %d r=%d\n", moveval++, player.xRunning);
    }

    void Player::runRight()
    {
        keys |= RightKey;
        player.xspeed = -PlayerSpeed;
        if (!inAir)
        {
            player.xRunning = true;
        }
        //printf("right %d r=%d\n", moveval++, player.xRunning);
    }

    void Player::stopLeft()
    {
        keys &= ~LeftKey;
        if (xspeed > 0)
        {
            //printf("left stop %d\n", moveval++);
            xspeed = 0.0;
            player.xRunning = false;
            if (keys & RightKey)
            {
                runRight();
            }
        }
    }

    void Player::stopRight()
    {
        keys &= ~RightKey;
        if (xspeed < 0)
        {
            //printf("right stop %d\n", moveval++);
            xspeed = 0.0;
            player.xRunning = false;
            if (keys & LeftKey)
            {
                runLeft();
            }
        }
    }

    void Player::runDirection(Vec2 direction)
    {
        player.xspeed = direction.x * PlayerSpeed;
        player.zspeed = direction.y * PlayerSpeed;
        if (!inAir)
        {
            player.xRunning = true;
            player.zRunning = true;
        }
    }

    void Player::stopRunning()
    {
        //printf("stop %d\n", moveval++);
        xspeed = zspeed = 0.0;
        player.xRunning = false;
        player.zRunning = false;
    }

    void Player::setRotation(int dir)
    {
        player.yAngularVelocity = RotationSpeed * dir;
    }

    void Player::setXRotation(int dir)
    {
        player.xAngularVelocity = RotationSpeed * dir;
    }

    void Player::setPointerDelta(float x, float y)
    {
        if (pointerMode && player.yAngularVelocity == 0. && player.xAngularVelocity == 0.)
        {
            if (x != 0.)
            {
                float z2 = std::sqrt(1.0f - x*x);

                float cosAngle = z2;
                float qangle = std::acos(cosAngle) * 0.5;
                if (x < 0.)
                {
                    yangle -= qangle;
                    if (yangle < 0.0)
                        yangle = PI_2 + yangle;
                }
                else
                {
                    yangle += qangle;
                    if (yangle >= PI_2)
                        yangle -= PI_2;
                }

                isRotating = true;
            }

            if (y != 0.)
            {
                float z2 = std::sqrt(1.0f - y*y);

                float cosAngle = z2;
                float qangle = std::acos(cosAngle) * 0.5;
                if (y < 0.)
                {
                    xangle -= qangle;
                    if (xangle < -M_PI_2)
                        xangle = -M_PI_2;
                }
                else
                {
                    xangle += qangle;
                    if (xangle > M_PI_2)
                        xangle = M_PI_2;
                }

                isRotating = true;
            }
        }
    }

    void Player::togglePointerMode()
    {
        pointerMode = !pointerMode;
        if (pointerMode)
        {
            yAngularVelocity = 0.;
            xAngularVelocity = 0.;
        }
    }

    void Player::jump()
    {
        if (yspeed != 0.)
            return;
        yspeed = JumpSpeed;
        yaccel = GravityAccel;
        inAir = true;
    }

    void Player::handleBlocks(CubeMesh &mesh, float dt)
    {
        auto nextpos = pos + velocity * dt;

        glm::ivec3 curmins = {
            floorf(pos[0]+PlayerBounds[0][0]),
            floorf(pos[1]+PlayerBounds[0][1]),
            floorf(pos[2]+PlayerBounds[0][2])
        };
        glm::ivec3 curmaxs = {
            floorf(pos[0]+PlayerBounds[1][0]),
            floorf(pos[1]+PlayerBounds[1][1]),
            floorf(pos[2]+PlayerBounds[1][2])
        };

        glm::ivec3 nextmins = {
            floorf(nextpos[0]+PlayerBounds[0][0]),
            floorf(nextpos[1]+PlayerBounds[0][1]),
            floorf(nextpos[2]+PlayerBounds[0][2])
        };
        glm::ivec3 nextmaxs = {
            floorf(nextpos[0]+PlayerBounds[1][0]),
            floorf(nextpos[1]+PlayerBounds[1][1]),
            floorf(nextpos[2]+PlayerBounds[1][2])
        };

        constexpr bool ladderEffect = false; // true for unlimited wall climbing
        bool isWallClimbing = false;
        bool onfloor = false;

        for (int y=nextmins[1]; y<=nextmaxs[1]; ++y)
        {
            for (int x=nextmins[0]; x<=nextmaxs[0]; ++x)
            {
                for (int z=nextmins[2]; z<=nextmaxs[2]; ++z)
                {
                    auto &node = mesh.getNode(x, y, z);
                    auto cubeType = node.type;
                    if (cubeType >= Collectible)
                    {
                        gotCollectible(node);
                        continue;
                    }
                    if (cubeType == 0)
                        continue;

                    //printf("collision pos=%.2f,%f,%.2f next=%.2f,%f,%.2f\n",
                    //        pos[0], pos[1], pos[2], nextpos[0], nextpos[1], nextpos[2]);

                    if (x < curmins[0])
                    {
                        //printf("-x %d,%d,%d\n", x,y,z);
                        velocity[0] = 0.;
                        if (zRunning && inAir && yspeed > 0)
                            isWallClimbing = true;
                        pos[0] = curmins[0] - PlayerBounds[0][0];
                    }
                    else if (x > curmaxs[0])
                    {
                        //printf("+x %d,%d,%d\n", x,y,z);
                        velocity[0] = 0.;
                        if (zRunning && inAir && yspeed > 0)
                            isWallClimbing = true;
                        pos[0] = curmaxs[0] + 0.99999 - PlayerBounds[1][0];
                    }
                    if (z < curmins[2])
                    {
                        static int val=0;
                        // hitting a solid block in front of us
                        //printf("-z %d,%d,%d\n", x, y, z);
                        velocity[2] = 0.;
                        if (zRunning && inAir && yspeed > 0)
                            isWallClimbing = true;
                        pos[2] = curmins[2] - PlayerBounds[0][2];
                    }
                    else if (z > curmaxs[2])
                    {
                        // hitting a solid block behind us
                        //printf("+z %d,%d,%d\n", x,y,z);
                        velocity[2] = 0.;
                        if (zRunning && inAir && yspeed > 0)
                            isWallClimbing = true;
                        pos[2] = curmaxs[2] + 0.99999 - PlayerBounds[1][2];
                    }
                    if (y < curmins[1])
                    {
                        // hitting a solid block beneath us
                        //printf("-y %d %d %d\n", y, curmins[1], nextmins[1]);
                        velocity[1] = 0.;
                        inAir = false;
                        yspeed = 0.;
                        yaccel = 0.;
                        pos[1] = curmins[1] - PlayerBounds[0][1];
                        nextpos[1] = pos[1];
                        nextmins[1] = curmins[1];
                    }
                    else if (y > curmaxs[1])
                    {
                        // hitting a solid block above us
                        //printf("+y %d\n", y);
                        velocity[1] = 0.;
                        yspeed = 0.;
                        pos[1] = curmaxs[1] + 0.99999 - PlayerBounds[1][1];
                        nextpos[1] = pos[1];
                        break;
                    }
                }
            }
        }

        if (isWallClimbing)
        {
            wallClimbing = true;
            //printf("wall climbing = true\n");
            if (ladderEffect)
                yaccel = 0.0;
        }
        else if (wallClimbing)
        {
            wallClimbing = false;
            //printf("wall climbing = false\n");
            if (yspeed > 0)
            {
                yaccel = GravityAccel;
                yspeed = HopSpeed;
            }
        }

        if (yspeed < 0. && nextpos[1] <= 0.)
        {
            //printf("ground\n");
            inAir = false;
            velocity[0] = yspeed = yaccel = 0;
            pos[1] = 0. - PlayerBounds[0][1];
        }
        else if (yaccel > GravityAccel && nextmins[1] > 0)
        {
            bool onBlock = false;
            int y = nextmins[1] - 1;
            for (int x=nextmins[0]; x<=nextmaxs[0]; ++x)
            {
                for (int z=nextmins[2]; z<=nextmaxs[2]; ++z)
                {
                    glm::ivec3 underFoot = {x, y, z};
                    auto &node = mesh.getNode(underFoot);
                    auto cubeType = node.type;
                    if (cubeType >= Collectible)
                    {
                        gotCollectible(node);
                        continue;
                    }
                    if (cubeType != 0)
                    {
                        onBlock = true;
                        if (cubeType == WaterBlockType && coinCount > 1 && ringCount > 0)
                        {
                            ++score;
                            coinCount = 0;
                            ringCount = 0;
                        }
                        break;
                    }
                }
            }

            if (!onBlock)
            {
                //printf("falling %f %f %f\n", pos[0], pos[1], pos[2]);
                yaccel = GravityAccel;
                inAir = true;
            }
        }
    }

    void Player::gotCollectible(const MeshItem &node)
    {
        auto ctype = node.type;
        if (ctype == CoinType)
        {
            cubeMesh.clearNode(node);
            ++coinCount;
        }
        else if (ctype == RingType)
        {
            cubeMesh.clearNode(node);
            ++ringCount;
        }
        else
        {
            //SDL_Log("whats this #%d?!", ctype);
        }
    }
}

// vim: sw=2 expandtab
#include "read.h"
#include "extra.h"
#include "fileSystem.h"

std::vector<Shape*> read::holden(std::string path){

  memstream fl(path);

  char flTest[7];

  flTest[6] = 0; //null termination

  char test[] = "holden";

  fl.read(flTest,6);

  if(strcmp(flTest,test)){
    __android_log_print(ANDROID_LOG_ERROR, "Reading Holden", "THIS WASNT A HOLDEN FILE: %s", path.c_str());
    __android_log_print(ANDROID_LOG_ERROR, "Reading Holden", "FILE HEADERS: %s AND %s", flTest, test );
    __android_log_print(ANDROID_LOG_ERROR, "Reading Holden", "header sizes: %i, %i", sizeof(flTest), sizeof(test));

    exit(0);
  }

  fl.read(test,1); //garbage

  unsigned short objectCount;
  fl.read(reinterpret_cast<char*>(&objectCount),2);

  //SDL_Log(("Object Count: "+toString(objectCount)).c_str());

  std::vector<Shape*> shapes;

  for(int i=0;i<objectCount;i++){
    //SDL_Log(std::string("FRESH").c_str());

    unsigned char type;

    fl.read(reinterpret_cast<char*>(&type),1); //type of objects

    //SDL_Log(("TYPE: "+toString(type)).c_str());

    test[0] = 1;

    while(test[0] != 0){
      fl.read(test,1);
    }

    Shape* shape;

    if(type>=6)
      shape = new LightenTexturedShape();
    else if(type>=4)
      shape = new TexturedShape();
    else
      shape = new Shape();

    shapes.push_back(shape);

    unsigned short vertNum;

    fl.read(reinterpret_cast<char*>(&vertNum),2);

    //SDL_Log(("SHAPE vert num: "+toString(vertNum)).c_str());

    shape->verts = std::vector<Vec>(vertNum);

    float fCache;

    for(int i=0;i<vertNum;i++){
      Vec* vec = &shape->verts[i];

      fl.read(reinterpret_cast<char*>(&fCache),4);
      vec->x = fCache;
      fl.read(reinterpret_cast<char*>(&fCache),4);
      vec->y = fCache;
      fl.read(reinterpret_cast<char*>(&fCache),4);
      vec->z = fCache;
    }

    unsigned short faceNum;
    fl.read(reinterpret_cast<char*>(&faceNum),2);

    //SDL_Log(("face num: "+toString(faceNum)).c_str());

    bool useTris = (type%2);

    /*
    if(useTris)
      //SDL_Log(("use tris: true"));
    else
      //SDL_Log(("use tris: false"));
      */

    int idxsPerFace = 6;
    if(useTris)
      idxsPerFace = 3;

    //SDL_Log(("IDXS per face : "+toString(idxsPerFace)).c_str());

    shape->indices = std::vector<unsigned short>(faceNum*idxsPerFace);

    for(int i=0;i<faceNum*idxsPerFace;i+=idxsPerFace){
      unsigned short v0;
      unsigned short v1;
      unsigned short v2;
      unsigned short v3;

      fl.read(reinterpret_cast<char*>(&v0),2);
      fl.read(reinterpret_cast<char*>(&v1),2);
      fl.read(reinterpret_cast<char*>(&v2),2);
      if(!useTris)
	fl.read(reinterpret_cast<char*>(&v3),2);


      std::vector<unsigned short>& is = shape->indices;

      is[i] = v0;
      is[i+1] = v1;
      is[i+2] = v2;

      if(!useTris){
	is[i+3] = v2;
	is[i+4] = v0;
	is[i+5] = v3;
      }


    }


    if(type>=4){ //save uv coords

      unsigned short uvNum;

      fl.read(reinterpret_cast<char*>(&uvNum),2);

      //SDL_Log(("uv num: "+toString(uvNum)).c_str());

      dynamic_cast<TexturedShape*>(shape)->texCoords = std::vector<Vec2>(uvNum);


      for(int i=0;i<uvNum;i++){
	Vec2 vec;

	fl.read(reinterpret_cast<char*>(&vec.x),4);
	fl.read(reinterpret_cast<char*>(&vec.y),4);

	dynamic_cast<TexturedShape*>(shape)->texCoords[i] = vec;
      }

    }


    if(type>=6){ //normals

      unsigned short normalNum = vertNum;

      LightenTexturedShape* tmp = dynamic_cast<LightenTexturedShape*>(shape);

      tmp->normals = std::vector<Vec>(normalNum);

      for(int i=0;i<normalNum;i++){

	Vec &vec = tmp->normals[i];

	fl.read(reinterpret_cast<char*>(&vec.x),4);
	fl.read(reinterpret_cast<char*>(&vec.y),4);
	fl.read(reinterpret_cast<char*>(&vec.z),4);

      }

    }
  }

  for (int i=0;i<shapes.size();i++) {
      shapes[i]->name = path;
  }

  return shapes;

}
// vim: sw=4 expandtab

#include "Ring.hpp"
#include "read.h"
#include "opengl/programs.h"
#include "screens/GameScreen.h"
#include "matrix.h"
#include "Surface.h"

using namespace std;

namespace game
{
    std::vector<Shape*> Ring::shapes;

    float Ring::yAngle = 0.;
    float Ring::yTrans = 0.;
    float Ring::yDir = 0.;

    void Ring::genGraphics()
    {
        shapes = read::holden("Ring.holden");
        for (auto &shape : shapes)
        {
            shape->drawMode = GL_TRIANGLES;
            shape->colors[0] = ColorRGB(1.0,1.0,1.0);
            auto texshape = dynamic_cast<TexturedShape*>(shape);
            texshape->surface = new Surface("gold.png");
        }
    }

    void Ring::draw()
    {
        program::bind(program::LIGHTEN_TEXTURED_FIXED_COLOR);
        //program::bind(program::TEXTURED_FIXED_COLOR);

        auto prevModel = mat::model;
        mat::translate(pos[0]+0.5, pos[1]+0.5+yTrans, pos[2]+0.5);
        mat::rotate(yAngle,0.,1.,0.);
        mat::scale(0.4, 0.4, 0.4);

        program::LightenTexturedFixedColor* p = static_cast<program::LightenTexturedFixedColor*>(program::getBound());
        //program::TexturedFixedColor* p = static_cast<program::LightenTexturedFixedColor*>(program::getBound());

        glEnableVertexAttribArray(p->vertHandle);
        glEnableVertexAttribArray(p->texCoordHandle);
        glEnableVertexAttribArray(p->normalHandle);

        glUniform1i(p->doTexOffsetHandle,1);
        glUniform2f(p->texCoordOffsetHandle,GameScreen::instance->offsetX,GameScreen::instance->offsetY);
        glUniform1i(p->doLightUniform,0);
        glUniform1i(p->doLight2Uniform, 0);

        glUniform3f(p->lightPosUniform,1,1,1);

        for (auto &shape : shapes)
        {
            //shape->pos.x = pos[0];
            //shape->pos.y = pos[1];
            //shape->pos.z = pos[2];
            shape->draw();
        }

        mat::model = prevModel;
    }

    void Ring::update(float dt)
    {
        yAngle += dt * 0.03/10.0;
        if (yAngle > 2.*M_PI)
            yAngle = 0.;

        yTrans += (dt * yDir) / 500.0;
        if (yTrans <= 0.0)
        {
            yTrans = 0.;
            yDir = 1;
        }
        else if (yTrans > 0.5)
        {
            yTrans = 0.5;
            yDir = -1;
        }
    }
}

#include "screen.h"

namespace screen{

  float bounds[2];

  int si;

  int pixelBounds[2];

  void init(){

    bounds[0] = 1;
    //bounds[1] = float(pixelBounds[1])/pixelBounds[0];
    bounds[1] = 1;



    if(bounds[0]<bounds[1])
      si = 0;
    else
      si = 1;
  }
}


std::vector<Screen*> Screen::screens = std::vector<Screen*>();

Screen::Screen(){

  screens.push_back(this);

}

void Screen::genGraphics() { };
void Screen::update(float dt) { };


#include "shapes.h"
#include <math.h>
#include "matrix.h"
#include "extra.h"
#include "opengl/programs.h"
#include <sstream>
#include <glm/glm.hpp>

namespace shape{

  Shape* circle(float radius,std::vector<ColorRGB> colors,unsigned int outerVertNum,Shape* shape){

    if(shape == nullptr)
      shape = new Shape();

    if(outerVertNum == 0)
      outerVertNum = (unsigned int)((radius/0.1)*90);

    shape->verts = std::vector<Vec>(outerVertNum+1);

    Vec vec;
    vec.x = 0;
    vec.y = 0;
    vec.z = 0;

    shape->verts.push_back(vec);

    if(colors.size() > 0)
      shape->colors = colors;

    float radInc = (M_PI*2)/float(outerVertNum);

    float radCache = 0;
    for(int i=1;i<outerVertNum;i++){
      shape->verts[i].x = cos(radCache)*radius;
      shape->verts[i].y = sin(radCache)*radius;

      radCache+=radInc;
    }

    shape->indices = std::vector<unsigned short>((outerVertNum-1)*3);

    for(int i=3;i<(outerVertNum-1)*3;i+=3){

      shape->indices[i-3] = i/3;
      shape->indices[i-3+1] = (i/3)+1;
      shape->indices[i-3+2] = 0;

    }

    int tmp = (outerVertNum-2)*3;

    shape->indices[tmp] = 1;
    shape->indices[tmp+1] = shape->verts.size()-3;
    shape->indices[tmp+2] = 0;

    shape->type = CIRCLE;


  }


  namespace outline{

    SimpleShape* circle(float radius,std::vector<ColorRGB> colors,unsigned int vertNum,SimpleShape* shape){

      if(shape == nullptr)
	shape = new SimpleShape();

      if(vertNum == 0)
	vertNum = (unsigned int)((radius/0.1)*90);

      shape->verts = std::vector<Vec>(vertNum);

      if(colors.size() > 0)
	shape->colors = colors;

      float radInc = (M_PI*2)/float(vertNum);

      float radCache = 0;
      for(int i=0;i<vertNum;i++){
	shape->verts[i].x = cos(radCache)*radius;
	shape->verts[i].y = sin(radCache)*radius;

	radCache+=radInc;
      }

      shape->type = OUTLINE_CIRCLE;


      return shape;
    }

  }

}


std::vector<SimpleShape*> SimpleShape::instances = std::vector<SimpleShape*>();

SimpleShape::SimpleShape(ColorRGB color,bool multipleColors){

  instances.push_back(this);

  if(!multipleColors){
    colors = std::vector<ColorRGB>(1);
    colors[0] = color;
  }

}

void SimpleShape::draw(){

  if(verts.size() == colors.size()){ //draw colors



  }else{ //assumes colors.size() == 1 and a the bound program is fixedColor

    //glUniform4fv(program::fixedColor::colorHandle,1,&colors[0].r);

  }

  mat::translate(pos.x,pos.y,pos.z);

  mat::downloadMVP();

  mat::translate(-pos.x,-pos.y,-pos.z);

  glBindBuffer(GL_ARRAY_BUFFER,vertBuf);

  glVertexAttribPointer(program::getBound()->vertHandle,3,GL_FLOAT,false,0,0);

  glUniform4fv(program::getBound()->colorHandle,1,&colors[0].r);

  glDrawArrays(drawMode,0,verts.size());


}

void SimpleShape::genGraphics(){

  glGenBuffers(1,&vertBuf);
  glBindBuffer(GL_ARRAY_BUFFER,vertBuf);
  glBufferData(GL_ARRAY_BUFFER,verts.size()*sizeof(Vec),&verts[0].x,GL_DYNAMIC_DRAW);

}

void TexturedSimpleShape::genGraphics(){

  SimpleShape::genGraphics();

  glGenBuffers(1,&texCoordBuf);
  glBindBuffer(GL_ARRAY_BUFFER,texCoordBuf);
  glBufferData(GL_ARRAY_BUFFER,texCoords.size()*sizeof(Vec2),&texCoords[0],GL_DYNAMIC_DRAW);


  glEnable(GL_TEXTURE_2D);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  if (surface->components == 3)
  {
	  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->data);
  }
  else
  {
	  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->data);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glDisable(GL_TEXTURE_2D);

}

void TexturedSimpleShape::draw(){

  mat::translate(pos.x,pos.y,pos.z);

  mat::downloadMVP();

  mat::translate(-pos.x,-pos.y,-pos.z);

  program::TexturedFixedColor* p = static_cast<program::TexturedFixedColor*>(program::getBound());

  glUniform4fv(program::getBound()->colorHandle,1,&colors[0].r);

  glBindBuffer(GL_ARRAY_BUFFER,texCoordBuf);

  glVertexAttribPointer(p->texCoordHandle,2,GL_FLOAT,GL_FALSE,0,0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glUniform1i(p->textureUniform, 0);


  glBindBuffer(GL_ARRAY_BUFFER,vertBuf);

  glVertexAttribPointer(p->vertHandle,3,GL_FLOAT,false,0,0);

  glDrawArrays(drawMode,0,verts.size());

}

Shape::Shape(ColorRGB color,bool multipleColors){

  drawMode = GL_TRIANGLES;

}

void Shape::genGraphics(){

  SimpleShape::genGraphics();

  glGenBuffers(1,&idxBuf);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,idxBuf);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(unsigned short),&indices[0],GL_DYNAMIC_DRAW);

}

void Shape::draw(){

  if(verts.size() == colors.size()){ //draw colors



  }else{ //assumes colors.size() == 1 and a the bound program is fixedColor

    //glUniform4fv(program::fixedColor::colorHandle,1,&colors[0].r);

  }

  mat::translate(pos.x,pos.y,pos.z);

  mat::downloadMVP();

  mat::translate(-pos.x,-pos.y,-pos.z);

  glBindBuffer(GL_ARRAY_BUFFER,vertBuf);

  glVertexAttribPointer(program::getBound()->vertHandle,3,GL_FLOAT,false,0,0);

  glUniform4fv(program::getBound()->colorHandle,1,&colors[0].r);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,idxBuf);

  glDrawElements(drawMode,indices.size(),GL_UNSIGNED_SHORT,0);

  /*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

  ColorRGB color;
  color.r = 0.5;
  color.g = 1;
  color.b = 0.5;

  glBindBuffer(GL_ARRAY_BUFFER,vertBuf);

  glVertexAttribPointer(program::fixedColor::vertHandle,2,GL_FLOAT,false,0,0);

  glUniform4fv(program::fixedColor::colorHandle,1,&color.r);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,tmp);  

  glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_SHORT,0);*/


  //glDrawArrays(drawMode,0,verts.size());

  //SDL_Log(("verts.size: "+std::to_string(verts.size())+" indices.size: "+std::to_string(indices.size())+" colors.size: "+std::to_string(colors.size())+" mode equals: "+std::to_string(GL_TRIANGLES == drawMode)+" pos: ("+std::to_string(pos.x)+","+std::to_string(pos.y)+") vertBuf: "+std::to_string(vertBuf)+" idx buf: "+std::to_string(idxBuf)+" color: ("+std::to_string(colors[0].r)+","+std::to_string(colors[0].g)+","+std::to_string(colors[0].b)).c_str());

}

void TexturedShape::genGraphics(){
    LOGd("Surface Path: %s", surface->path.c_str());

  Shape::genGraphics();

    LOGd("Surface Path: %s", surface->path.c_str());

  glGenBuffers(1,&texCoordBuf);
  glBindBuffer(GL_ARRAY_BUFFER,texCoordBuf);
  glBufferData(GL_ARRAY_BUFFER,texCoords.size()*sizeof(Vec2),&texCoords[0],GL_DYNAMIC_DRAW);

    LOGd("Surface Path: %s", surface->path.c_str());

    //glClear(GL_COLOR_BUFFER_BIT);

    /*
    if (type == shape::CIRCLE) {
        LOGd("Skipping circle, %s", name.c_str());
        return;
    }
     */

    LOGd("Not skipping: %s", name.c_str());

  glEnable(GL_TEXTURE_2D);

    LOGd("Surface Path: %s", surface->path.c_str());

  glGenTextures(1, &texture);
    LOGd("Surface Path: %s", surface->path.c_str());
  glBindTexture(GL_TEXTURE_2D, texture);
    LOGd("Surface Path: %s", surface->path.c_str());
    surface->components = 3;
  if (surface->components == 3)
  {
	  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->data);
  }
  else
  {
      //LOGd("surface->data end: %i", surface->data[surface->w*surface->h*4]);
      for (int i=0; i < (surface->w*surface->h*4); i++) {
          char tmp = surface->data[i];
      }
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->data);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glDisable(GL_TEXTURE_2D);

}

void TexturedShape::draw(){

  mat::translate(pos.x,pos.y,pos.z);

  mat::downloadMVP();

  mat::translate(-pos.x,-pos.y,-pos.z);

  glBindBuffer(GL_ARRAY_BUFFER,vertBuf);

  glVertexAttribPointer(program::getBound()->vertHandle,3,GL_FLOAT,false,0,0);

  glUniform4fv(program::getBound()->colorHandle,1,&colors[0].r);

  glBindBuffer(GL_ARRAY_BUFFER,texCoordBuf);

  program::TexturedFixedColor* p = static_cast<program::TexturedFixedColor*>(program::getBound());
  glVertexAttribPointer(p->texCoordHandle,2,GL_FLOAT,GL_FALSE,0,0);

  glActiveTexture(GL_TEXTURE0);
  //SDL_Log("10 GL errors: %s",gluErrorString(glGetError()));
  glBindTexture(GL_TEXTURE_2D, texture);
  //SDL_Log("11 GL errors: %s",gluErrorString(glGetError()));
  glUniform1i(p->textureUniform, 0);


  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,idxBuf);

  glDrawElements(drawMode,indices.size(),GL_UNSIGNED_SHORT,0);

}

void LightenTexturedShape::draw(){

  program::LightenTexturedFixedColor* p = static_cast<program::LightenTexturedFixedColor*>(program::getBound());


  mat::translate(pos.x,pos.y,pos.z);

  mat::downloadMVP();

  glUniformMatrix4fv(p->mvmUniform, 1, GL_FALSE, glm::value_ptr(mat::view*mat::model));

  glUniform3f(p->lightPosUniform,lightPos.x,lightPos.y,lightPos.z);

  mat::translate(-pos.x,-pos.y,-pos.z);

  glBindBuffer(GL_ARRAY_BUFFER,vertBuf);

  glVertexAttribPointer(program::getBound()->vertHandle,3,GL_FLOAT,false,0,0);

  glUniform4fv(program::getBound()->colorHandle,1,&colors[0].r);

  glBindBuffer(GL_ARRAY_BUFFER,texCoordBuf);

  glVertexAttribPointer(p->texCoordHandle,2,GL_FLOAT,GL_FALSE,0,0);

  glBindBuffer(GL_ARRAY_BUFFER,normalBuf);
  glVertexAttribPointer(p->normalHandle,3,GL_FLOAT,GL_FALSE,0,0);

  glActiveTexture(GL_TEXTURE0);
  //SDL_Log("10 GL errors: %s",gluErrorString(glGetError()));
  glBindTexture(GL_TEXTURE_2D, texture);
  //SDL_Log("11 GL errors: %s",gluErrorString(glGetError()));
  glUniform1i(p->textureUniform, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,idxBuf);

  glDrawElements(drawMode,indices.size(),GL_UNSIGNED_SHORT,0);

}

void LightenTexturedShape::genGraphics(){

  TexturedShape::genGraphics();

  glGenBuffers(1,&normalBuf);
  glBindBuffer(GL_ARRAY_BUFFER,normalBuf);
  glBufferData(GL_ARRAY_BUFFER,normals.size()*sizeof(Vec),&normals[0],GL_DYNAMIC_DRAW);

}



#include "shapes.h"
#include "opengl/programs.h"
#include "Surface.h"

using namespace std;

static TexturedShape *spash;

void genSplash()
{
	spash = new TexturedShape();
	spash->verts = vector<Vec>(4);
	spash->verts[0] = Vec{-8,-1,-1};
	spash->verts[1] = Vec{8,-1,-8};
	spash->verts[2] = Vec{8,7,-8};
	spash->verts[3] = Vec{-8,7,-1};

	spash->indices = vector<unsigned short>(4);
	spash->indices[0] = 3;
	spash->indices[1] = 0;
	spash->indices[2] = 2;
	spash->indices[3] = 1;

	spash->texCoords = vector<Vec2>(4);
	spash->texCoords[0] = {0,1};
	spash->texCoords[1] = {1,1};
	spash->texCoords[2] = {1,0};
	spash->texCoords[3] = {0,0};

	spash->drawMode = GL_TRIANGLE_STRIP;
	spash->surface = new Surface("ggj-splash.png");
}

void drawSplash()
{
	program::bind(program::TEXTURED_FIXED_COLOR);

	program::TexturedFixedColor* p = static_cast<program::TexturedFixedColor*>(program::getBound());

	glEnableVertexAttribArray(p->vertHandle);
	glEnableVertexAttribArray(p->texCoordHandle);
	glUniform1i(p->doTexOffsetHandle,0);
	spash->draw();
}

#include "Surface.h"
#include "assets.hpp"
#include <android/log.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Surface::Surface(std::string path){

    if (assets.find(path) == assets.end()) {
        //SDL_Log(("CANNOT LOAD IMAGE: "+path).c_str());
        __android_log_print(ANDROID_LOG_DEBUG, "all.cpp", "CANNOT LOAD IMAGE: %s", path.c_str());
        exit(0);
    }

    Buffer png_buf = assets[path];

    data = stbi_load_from_memory((stbi_uc*) png_buf.data, png_buf.size, &w, &h, &components, 3);

    this->path = path;

    LOGd("Surface Initialization Path and Channels: %s, %i", path.c_str(), components);
}
/*#include "Texture.h"

GLuint Texture::texCoordBuf;

std::vector<Vec2> Texture::texCoords;

void Texture::init(){

  glGenBuffers(1,&texCoordBuf);
  glBindBuffer(GL_ARRAY_BUFFER,texCoordBuf);
  glBufferData(GL_ARRAY_BUFFER,texCoords.size()*sizeof(Vec2),&texCoords[0],GL_DYNAMIC_DRAW);


  glEnable(GL_TEXTURE_2D);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  if (surface->components == 3)
  {
	  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->data);
  }
  else
  {
	  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->data);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glDisable(GL_TEXTURE_2D);

}*/
// vim: sw=2 expandtab
#include "opengl/opengl.h"
#include <math.h>
#include <cstdlib>
#include <string>
#include "screen.h"
#include "shapes.h"
#include "fileSystem.h"
#include "Surface.h"

#include <cstring>

namespace gl{

  void init(){

    //glViewport(0,0,screen::pixelBounds[0],screen::pixelBounds[1]);

    //glClearColor(0.5,1,0.5,1);
    //
    glClearColor(0.2,0.3,0.1,0);

    glEnable(GL_DEPTH_TEST);

    for(int i=0;i<Screen::screens.size();i++)
      Screen::screens[i]->genGraphics();

    Surface* defaultSurface = new Surface("MapTexture.png");

    for(int i=0;i<SimpleShape::instances.size();i++){

      if(TexturedShape* shape = dynamic_cast<TexturedShape*>(SimpleShape::instances[i])){
        if (shape->surface == nullptr)
          shape->surface = defaultSurface;
      }

      SimpleShape::instances[i]->genGraphics();
    }

  }

}

ColorRGB::ColorRGB(float r,float g,float b){
  this->r = r;
  this->g = g;
  this->b = b;
}

float AngularVelocity::cache = 0;

AngularVelocity::AngularVelocity(){
  /*angle = (rand()/float(INT_MAX))*(2*M_PI);

  AngularVelocity::cache+=0.2;

  if(cache>=(M_PI*2))
    cache = 0;

  if(angle == M_PI)
    angle = 1.5;
  else if(angle == 0)
    angle = 1.5;

  if(angle<M_PI){
    accel = -accel;
    resistance = -resistance;
  }*/
}
// vim: sw=4 expandtab

#include "opengl/programs.h"
#include "extra.h"

namespace program{

  std::vector<Program*> programs;

  unsigned int bound;

  void init(){

    programs.push_back( new FixedColor() );
    programs.push_back( new TexturedFixedColor() );
    programs.push_back( new LightenTexturedFixedColor() );
    programs.push_back( new LightenFixedColor() );

  }

  void bind(EnumProgramIndices e){
    programs[e]->bind();
  }

  Program* getBound(){
    return programs[bound];
  }

  Program* getProgram(EnumProgramIndices e)
  {
    return programs[e];
  }

  const GLchar* fogVertShaderSource =
      "uniform float fogDensity;\n"
      "vec4 mixFog(vec4 color, vec3 ecPosition) {\n"
      "     vec3 fogColor = vec3(0.0, 0.0, 0.0);\n"
      "     float fogFragCoord = abs(ecPosition.z);\n"
      "     float fog = exp(-fogDensity * fogFragCoord);\n"
      "     fog = clamp(fog, 0.0, 1.0);\n"
      "     return vec4(mix(fogColor, color.rgb, fog), color.a);\n"
      "}\n";

    const GLchar* lightVertShaderSource =
        "uniform vec3 u_LightPos;\n"
        "uniform vec3 u_Light2Pos;\n"
        "uniform vec3 u_Normal;\n"
        "uniform float u_Shininess;\n"
        "uniform bool do_light;\n"
        "uniform bool do_light2;\n"
        "uniform bool do_uniformNormals;\n"
        "attribute vec3 a_Normal;\n"
        "vec3 doLight(vec3 color, vec3 ecPos, vec3 ecNormal, vec3 lightPos) {\n"
        "   vec3 lightVector = normalize(lightPos - ecPos);\n"
        "   float nDotL = dot(ecNormal, lightVector);\n"
        "   vec3 ambient = 0.1 * color;\n"
        "   vec3 diffuse = 0.6 * max(0., nDotL) * color;\n"
        "   vec3 specular = vec3(0.,0.,0.);\n"
        "   if (u_Shininess > 0.) {\n"
        "       vec3 reflection = (2.0 * ecNormal * nDotL) - lightVector;\n"
        "       vec3 viewDirection = -normalize(ecPos);\n"
        "       float rDotV = dot(reflection, viewDirection);\n"
        "       float specPower = 1.5;\n"
        "       specular = color * u_Shininess * pow(max(0.,rDotV), specPower);\n"
        "   }\n"
        //"   return vec4(ambient + diffuse + specular, inColor.a);\n"
        "   return vec3(ambient + diffuse + specular);\n"
        "}\n"
        "vec4 addLight(vec4 inColor, mat4 MVM, vec4 vertPos) {\n"
        "   if (!do_light && !do_light2) return inColor;\n"
        "   vec4 modelViewVertex = MVM * vertPos;\n"
        "   vec3 ecPos = vec3(modelViewVertex);\n"
        "   vec3 ecNormal;\n"
        "   if (do_uniformNormals) {\n"
        "       ecNormal = vec3(MVM * vec4(u_Normal, 0.0));\n"
        "   }\n"
        "   else {\n"
        "       ecNormal = vec3(MVM * vec4(a_Normal, 0.0));\n"
        "   }\n"
        "   vec3 color = vec3(0.,0.,0.);\n"
        "   if (do_light)\n"
        "       color = color + doLight(inColor.rgb, ecPos, ecNormal, u_LightPos);\n"
        "   if (do_light2)\n"
        "       color = color + doLight(inColor.rgb, ecPos, ecNormal, u_Light2Pos);\n"

        "   return vec4(color, inColor.a);\n"
        "}\n";

  FixedColor::FixedColor(){

    const GLchar* vertShaderSource =
        "uniform mat4 MVP;\n"
        "uniform vec4 vColor;\n"
        "attribute vec4 vPosition;\n"
        "uniform float m_gl_PointSize;\n"
        "varying vec4 v_Color;\n"
        "void main() {\n"
            "gl_Position = MVP * vec4(vPosition[0],vPosition[1],vPosition[2],1);\n"
            "gl_PointSize = m_gl_PointSize;\n"
            "v_Color = mixFog(vColor, gl_Position.xyz);\n"
        "}\n";

    const GLchar* fragShaderSource =
        "varying vec4 v_Color;"
        "void main() {"
            "gl_FragColor = v_Color;"
        "}";

    const GLchar* vertShaderSources[2] = {program::fogVertShaderSource, vertShaderSource};
    LOGd("Init Fixed Color Program");
    defaultInit(vertShaderSources,2,fragShaderSource,FIXED_COLOR);
    fogInit();

    colorHandle = glGetUniformLocation(id,"vColor");

  }

  TexturedFixedColor::TexturedFixedColor(){

    const GLchar* tmpVertShader =
        "uniform mat4 MVP;"
        "attribute vec4 vPosition;"
        "uniform float m_gl_PointSize;"
        "attribute vec2 TexCoordIn;"
        "uniform bool doTexOffset;"
        "uniform vec2 TexCoordOffset;"
        "varying vec2 TexCoordOut;"
        "vec2 bridge;"
        "void main() {"
            "gl_Position = MVP * vec4(vPosition[0],vPosition[1],vPosition[2],1);"
            "gl_PointSize = m_gl_PointSize;"
            "bridge.x = TexCoordIn.x;"
            "bridge.y = TexCoordIn.y;"
            "if (doTexOffset) {"
            "   bridge.x += TexCoordOffset.x;"
            "   bridge.y += TexCoordOffset.y;"
            "   if(bridge.x > 1.){"
            "           bridge.x = TexCoordIn.x-1.;"
            "   }"
            "   if(bridge.y > 1.){"
            "           bridge.y = TexCoordIn.y-1.;"
            "   }"
            "}"
            "TexCoordOut = bridge;"
        "}";

    const GLchar* tmpFragShader =
        "uniform vec4 vColor;"
        "varying vec2 TexCoordOut;"
        "uniform sampler2D Texture;"
        "void main() {"
            "gl_FragColor =  texture2D(Texture, TexCoordOut);"
        "}";


      LOGd("Init Textured Fixed Color Program");
    defaultInit(&tmpVertShader,1,tmpFragShader,TEXTURED_FIXED_COLOR);


    texCoordHandle = glGetAttribLocation(id, "TexCoordIn");

    textureUniform = glGetUniformLocation(id, "Texture");

    doTexOffsetHandle = glGetUniformLocation(id,"doTexOffset");
    texCoordOffsetHandle = glGetUniformLocation(id,"TexCoordOffset");


  }



  LightenTexturedFixedColor::LightenTexturedFixedColor(){

    const GLchar* tmpVertShader =
        "uniform mat4 MVP;\n"
        "uniform mat4 MVM;\n"
        "uniform float m_gl_PointSize;\n"
        "uniform bool doTexOffset;\n"
        "uniform vec2 TexCoordOffset;\n"
        "uniform vec4 vColor;\n"

        "attribute vec4 vPosition;\n"
        "attribute vec2 TexCoordIn;\n"

        "varying vec2 TexCoordOut;\n"
        "varying vec4 v_Color;\n"

        "vec2 bridge;\n"

        "void main() {\n"
            "v_Color = addLight(vec4(1,1,1,1), MVM, vPosition);\n"

            "gl_Position = MVP * vec4(vPosition[0],vPosition[1],vPosition[2],1);\n"
            "gl_PointSize = m_gl_PointSize;\n"

            "bridge.x = TexCoordIn.x;\n"
            "bridge.y = TexCoordIn.y;\n"
            "if (doTexOffset) {\n"
            "   bridge.x += TexCoordOffset.x;\n"
            "   bridge.y += TexCoordOffset.y;\n"
            "   if(bridge.x > 1.){\n"
            "           bridge.x = TexCoordIn.x-1.;\n"
            "   }\n"
            "   if(bridge.y > 1.){\n"
            "           bridge.y = TexCoordIn.y-1.;\n"
            "   }\n"
            "}\n"
            "TexCoordOut = bridge;\n"
        "}\n";

    const GLchar* tmpFragShader =
        "varying vec4 v_Color;\n"
        "varying vec2 TexCoordOut;\n"

        "uniform sampler2D Texture;\n"
        "void main() {\n"
            "gl_FragColor =  v_Color * texture2D(Texture, TexCoordOut);\n"
        "}\n";


    const GLchar* vertShaderSources[3] = {
        program::fogVertShaderSource,
        program::lightVertShaderSource,
        tmpVertShader
    };
      LOGd("Init Lightened Textured Fixed Color Program");
    defaultInit(vertShaderSources,3,tmpFragShader,LIGHTEN_TEXTURED_FIXED_COLOR);
    lightInit(id);

    texCoordHandle = glGetAttribLocation(id, "TexCoordIn");

    textureUniform = glGetUniformLocation(id, "Texture");

    mvmUniform = glGetUniformLocation(id,"MVM");

    doTexOffsetHandle = glGetUniformLocation(id,"doTexOffset");
    texCoordOffsetHandle = glGetUniformLocation(id,"TexCoordOffset");

  }


  LightenFixedColor::LightenFixedColor(){

    const GLchar* tmpVertShader =
        "uniform mat4 MVP;\n"
        "uniform mat4 MVM;\n"
        "uniform float m_gl_PointSize;\n"
        "uniform vec4 vColor;\n"
        "attribute vec4 vPosition;\n"
        "varying vec4 v_Color;\n"
        "void main() {\n"
        "    v_Color = addLight(vColor, MVM, vPosition);\n"
        "    gl_Position = MVP * vec4(vPosition.xyz,1);\n"
        "    v_Color = mixFog(v_Color, gl_Position.xyz);\n"
        "    gl_PointSize = m_gl_PointSize;\n"
        "}\n";

    const GLchar* tmpFragShader =
        "varying vec4 v_Color;\n"
        "void main() {\n"
            "gl_FragColor = v_Color;\n"
        "}\n";

    const GLchar* vertShaderSources[3] = {
        program::fogVertShaderSource,
        program::lightVertShaderSource,
        tmpVertShader
    };

      LOGd("Init Lightened Fixed Color Program");
    defaultInit(vertShaderSources,3,tmpFragShader,LIGHTEN_FIXED_COLOR);
    fogInit();
    lightInit(id);

    colorHandle = glGetUniformLocation(id,"vColor");

    mvmUniform = glGetUniformLocation(id,"MVM");

  }

    void DummyLighten::lightInit(GLuint id)
    {
        normalHandle = glGetAttribLocation(id,"a_Normal");
        normalUniform = glGetUniformLocation(id,"u_Normal");
        shininessUniform = glGetUniformLocation(id,"u_Shininess");
        lightPosUniform = glGetUniformLocation(id,"u_LightPos");
        light2PosUniform = glGetUniformLocation(id,"u_Light2Pos");
        doLightUniform = glGetUniformLocation(id,"do_light");
        doLight2Uniform = glGetUniformLocation(id,"do_light2");
        doUniformNormalsUniform = glGetUniformLocation(id,"do_uniformNormals");
    }

}


Program::Program() :
    idx(program::programs.size())
{
}

void compileShader(GLuint shader)
{
    glCompileShader(shader);
    GLint shaderCompiled = GL_FALSE;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &shaderCompiled );
    if( shaderCompiled != GL_TRUE ) {
        __android_log_print(ANDROID_LOG_DEBUG, "Compile Shader", "Unable to compile shader %i", shader);
        int infoLogLength = 0;
        int maxLength = infoLogLength;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );
        char* infoLog = new char[ maxLength ];
        glGetShaderInfoLog( shader, maxLength, &infoLogLength, infoLog );
        __android_log_print(ANDROID_LOG_DEBUG, "Compile Shader", "Shader log: %s", infoLog);
        delete[] infoLog;
    }
}

void Program::defaultInit(const GLchar** vertSources, int vcnt, const GLchar* fragSource,program::EnumProgramIndices type){

  id = glCreateProgram();

  vertShader = glCreateShader(GL_VERTEX_SHADER);

  //SDL_Log(("Shader id: "+toString(type)).c_str());
  glShaderSource(vertShader,vcnt,vertSources,nullptr);

  for (int i=0; i<vcnt; i++) {
      LOGd("Vertex Source: %s", vertSources[i]);
  }

  LOGd("Compiling vert shader");
  compileShader(vertShader);

  fragShader = glCreateShader(GL_FRAGMENT_SHADER);

  const GLchar* fragSources[1] = {fragSource};
  glShaderSource(fragShader,1,fragSources,nullptr);

  LOGd("Compiling frag shader");
  compileShader(fragShader);

  glAttachShader(id,vertShader);
  glAttachShader(id,fragShader);
  glLinkProgram(id);

  vertHandle = glGetAttribLocation(id,"vPosition");

}

void Program::fogInit()
{
    fogDensityHandle = glGetUniformLocation(id,"fogDensity");
}

void Program::bind()
{
    program::bound = idx;

    glUseProgram(id);
}

// vim: sw=4 expandtab

#include "screens/GameScreen.h"
#include "opengl/programs.h"
#include "matrix.h"
#include "loop.h"
#include "extra.h"
#include "read.h"
#include "Player.hpp"
#include "Floor.hpp"
#include "Cube.hpp"
#include "CubeMesh.hpp"
#include "Ring.hpp"
#if USE_ANTBAR == 1
#include "AntTweakBar.h"
#endif

using game::player;

bool frozen = false;
unsigned WinScore = 6;

constexpr float DEGRAD = M_PI / 180.;

game::CubeMesh cubeMesh;
glm::ivec2 wellCenter;

GameScreen* GameScreen::instance = new GameScreen();


InputEvent* GameScreen::inputEvent;
Thumbstick* GameScreen::thumbstick = new Thumbstick();
unsigned GameScreen::level = 1;

GameScreen::GameScreen(){

  loop::currentScreen = this;

  inputEvent = new InputEvent();
#ifdef __ANDROID__
  thumbstick = new Thumbstick();
#endif

}

constexpr bool infinitePlay = false;
constexpr bool freezePlayerWhenTimesUp = false;
static unsigned timeleftSeconds = infinitePlay ? 0 : 90;
static char result[10] = "";
bool finished = false;

struct Thing
{
    static void genGraphics();
    void draw();

    static std::vector<Shape*> scene;

    static Shape* shs;

    glm::vec4 lightPos = {0,0,2,1};

} thing;

//static float colorVar[] = { 0., 1., 0. };

#if USE_ANTBAR == 1
TwBar *bar;
#endif

void initUI()
{
#if USE_ANTBAR == 1
    extern float avgFps;

    if (!TwInit(TW_OPENGL_CORE, NULL)) {
      SDL_Log("Failed to init TweakBar: %s\n", TwGetLastError());
      return;
    }

    int width = screen::pixelBounds[0];
    int height = screen::pixelBounds[1];
    TwWindowSize(width, height);
    bar = TwNewBar("Coventina");
    TwDefine(" Coventina size='200 130' help='Is this helpful?\n' ");
    //TwAddVarRW(bar, "Color", TW_TYPE_COLOR3F, &colorVar, " help='color' ");
    TwAddVarRW(bar, "Framerate", TW_TYPE_FLOAT, &avgFps, " help='framerate' ");
    TwAddVarRW(bar, "Coins", TW_TYPE_UINT32, &player.coinCount, " help='coins' ");
    TwAddVarRW(bar, "Rings", TW_TYPE_UINT32, &player.ringCount, " help='rings' ");
    TwAddVarRW(bar, "Time Left", TW_TYPE_UINT32, &timeleftSeconds, " help='seconds remaining' ");
    TwAddVarRW(bar, "Score", TW_TYPE_UINT32, &player.score, " help='score' ");
    TwAddVarRW(bar, "Result", TW_TYPE_CSSTRING(10), &result, " help='results' ");
#endif
}

void genSplash();
void drawSplash();

void GameScreen::genGraphics(){

    initUI();

    if (thumbstick)
        thumbstick->genGraphics();

    timeleft = timeleftSeconds;

    //Bullet::genGraphics();

    genSplash();
    game::Floor::genGraphics();
    game::Cube::genGraphics();
    game::Coin::genGraphics();
    game::Ring::genGraphics();
    CuttleFish::genGraphics();
    Thing::genGraphics();
    thumbstick->genGraphics();

    cubeMesh.readMap("maze.grid");
    //cubeMesh.readCubes("map.in");
    cubeMesh.addRandoms();
    wellCenter = cubeMesh.wellCenter;
    player.pos[0] = cubeMesh.playerStart[0];
    player.pos[2] = cubeMesh.playerStart[1];

    cuttleFish = new CuttleFish(glm::vec3{wellCenter[0], 4., wellCenter[1]-1.});
}

void GameScreen::keyInput()
{
    /*
    if (thumbstick)
        thumbstick->keyInput(event,inputEvent);
        */
}

static float aspect_ratio;

void GameScreen::render(float dt)
{

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat::clearMatrices();

    mat::perspective(60*DEGRAD,aspect_ratio);

    mat::view = player.viewMat;

    game::floor.draw();

    {
        static int cnt = 0;
        if (cnt < 60 * 1)
        {
            drawSplash();
            ++cnt;
            return;
        }
    }

    if (timeleft > 0)
    {
        timeleftSeconds = unsigned(timeleft);
        timeleft -= dt;
    }
    else if (!finished && !infinitePlay)
    {
        finished = true;
        timeleftSeconds = 0;
        if (player.score >= WinScore)
        {
            strcpy(result, "Winner!");
        }
        else
        {
            strcpy(result, "Loser!");
        }
        printf("%s\n", result);
    }

    for (auto &cube : cubeMesh.cubes)
    {
        if (cube->valid)
            cube->draw();
    }

    for (auto &coin : cubeMesh.coins)
    {
        if (coin->valid)
            coin->draw();
    }

    for (auto &ring : cubeMesh.rings)
    {
        if (ring->valid)
            ring->draw();
    }

    //gun.draw();

    /*for(int i=0;i<Bullet::instances.size();i++)
      Bullet::instances[i]->draw();*/

    cuttleFish->draw();

    thing.draw();

    mat::clearMatrices();


    /*
    if (thumbstick)
        thumbstick->draw();
    */


#if USE_ANTBAR == 1
    TwDraw();
#endif

    if (!frozen)
    {
        offsetX+=0.005;
        offsetY+=0.005;

        if(offsetX>1)
            offsetX = 0;
        if(offsetY>1)
            offsetY = 0;
    }
}

void GameScreen::update(float dt)
{
    keyInput();

    if (thumbstick)
        thumbstick->update();

    if (!frozen)
    {
        if (timeleftSeconds > 0 || !freezePlayerWhenTimesUp)
        {
            player.handleBlocks(cubeMesh, dt);
            player.update(dt);
        }

        game::Coin::update(dt);
        game::Ring::update(dt);
        cuttleFish->update(dt);

        //gun.update();

        /*for(int i=0;i<Bullet::instances.size();i++)
          Bullet::instances[i]->update();*/

    }

#if USE_ANTBAR == 1
    TwRefreshBar(bar);
#endif

    render(dt);

}

std::vector<Shape*> Thing::scene;
Shape* Thing::shs;

void Thing::genGraphics()
{
    //scene = read::holden("WithNormals.holden");
    scene = read::holden("Ring.holden");

    shs = 0;
    if (true)
    {
        shs = new Shape();

        shs->verts = scene[0]->verts;
        shs->indices = scene[0]->indices;
        shs->colors = std::vector<ColorRGB>(1);
        shs->colors[0] = ColorRGB(1,0,0);

        //SDL_Log(("Verts Size: "+toString(shs->verts.size())+" indices size: "+toString(shs->indices.size())).c_str());
    }

}

void Thing::draw()
{
    mat::translate(wellCenter[0]+0.5, 2, wellCenter[1]);

    if(false){
        program::bind(program::FIXED_COLOR);

        program::FixedColor* p2 = static_cast<program::FixedColor*>(program::getBound());

        glEnableVertexAttribArray(p2->vertHandle);

        shs->pos.x = 0; shs->pos.y = 0; shs->pos.z = lightPos.z;
        mat::scale(0.1,0.1,0.1);
        shs->draw();
        mat::scale(10,10,10);
        shs->pos.z = 0;
    }

    static float angle = 0;

    mat::rotate(angle,0,1,0);
    mat::rotate(fabs(angle-(2*M_PI)),1,0,0);

    if (!frozen)
    {
        angle += 0.005;
        if(angle>=(2*M_PI))
            angle = 0;
    }

    if (shs)
    {
        program::bind(program::FIXED_COLOR);

        program::FixedColor* p2 = static_cast<program::FixedColor*>(program::getBound());

        glEnableVertexAttribArray(p2->vertHandle);

        mat::downloadMVP();

        shs->drawMode = GL_LINE_LOOP;

        shs->draw();

    }

    if (true)
    {
        program::bind(program::LIGHTEN_TEXTURED_FIXED_COLOR);

        mat::downloadMVP();

        program::LightenTexturedFixedColor* p = static_cast<program::LightenTexturedFixedColor*>(program::getBound());

        glEnableVertexAttribArray(p->vertHandle);
        glEnableVertexAttribArray(p->texCoordHandle);
        glEnableVertexAttribArray(p->normalHandle);

        glUniform1i(p->doTexOffsetHandle,1);
        glUniform2f(p->texCoordOffsetHandle,GameScreen::instance->offsetX,GameScreen::instance->offsetY);

        auto vec = (mat::view*mat::model) * lightPos;

        glUniform3f(p->lightPosUniform,vec.x,vec.y,vec.z);
        glUniform1i(p->doLightUniform,1);
        glUniform1i(p->doLight2Uniform,0);
        glUniform1f(p->shininessUniform, 0.);
        glUniform1i(p->doUniformNormalsUniform, 0);

        for(int i=0;i<scene.size();i++){

            scene[i]->drawMode = GL_TRIANGLES;
            scene[i]->colors[0] = ColorRGB(0.75,0.75,0.75);
            scene[i]->draw();

            //SDL_Log(("Tex Coords size: "+std::to_string(scene[i]->texCoords.size())).c_str());

#if 0
            scene[i]->drawMode = GL_LINE_LOOP;
            scene[i]->colors[0] = ColorRGB(1,0.5,1);
            scene[i]->draw();
#endif
        }
    }
}
#include "ui/InputEvent.h"

#include "screen.h"

InputEvent* InputEvent::instance;


InputEvent::InputEvent(){

  instance = this;

}

void InputEvent::update(){

    /*
  SDL_GetMouseState(&px,&py);

  x = ((px/float(screen::pixelBounds[0]))*2)-1;
  y = ((py/float(screen::pixelBounds[1]))*(screen::bounds[1]*2))-screen::bounds[1];
     */

}
// vim: sw=4 expandtab
#include "ui/Thumbstick.h"
#include "Surface.h"
#include "matrix.h"
#include "screen.h"

#include "opengl/programs.h"
#include "extra.h"
#include <math.h>
#include "Player.hpp"

void Thumbstick::draw()
{
    mat::ortho(-screen::bounds[0],screen::bounds[0],screen::bounds[1],-screen::bounds[1],-1,1);

    mat::translate(pos.x,pos.y,0);

    mat::downloadMVP();

    program::bind(program::TEXTURED_FIXED_COLOR);

    program::TexturedFixedColor* p2 = static_cast<program::TexturedFixedColor*>(program::getBound());

    glEnableVertexAttribArray(p2->vertHandle);
    glEnableVertexAttribArray(p2->texCoordHandle);

    innerCircle->draw();

    program::bind(program::FIXED_COLOR);
    program::FixedColor* p = static_cast<program::FixedColor*>(program::getBound());
    glEnableVertexAttribArray(p->vertHandle);
    mat::downloadMVP();

    outerRing->draw();

    mat::translate(-pos.x,-pos.y,0);
}

Thumbstick::Thumbstick(){

  
}

void Thumbstick::genGraphics(){

  innerCircle = new Texture();

  innerCircle->surface = new Surface("ui/thumbstick.png");
  innerCircle->drawMode = GL_TRIANGLES;

  float size = 0.1;

  Vec verts[4] = {
    -0.1,-0.1,0,
    0.1,-0.1,0,
    0.1,0.1,0,
    -0.1,0.1,0
  };

  Vec2 texCoords[4] = {
    0,0,
    1,0,
    1,1,
    0,1
  };

  innerCircle->verts.push_back(verts[0]);
  innerCircle->verts.push_back(verts[1]);
  innerCircle->verts.push_back(verts[2]);
  innerCircle->verts.push_back(verts[3]);

  innerCircle->texCoords.push_back(texCoords[0]);
  innerCircle->texCoords.push_back(texCoords[1]);
  innerCircle->texCoords.push_back(texCoords[2]);
  innerCircle->texCoords.push_back(texCoords[3]);

  outerRing = shape::outline::circle();

}

void Thumbstick::update(){

  if(InputEvent::instance->pressed)
    //SDL_Log(("PRESSED"));
  if(active)
    //SDL_Log(("ACTIVE"));

  if(InputEvent::instance->pressed && active){

    Vec2 vec;
    vec.x = direction.x * (innerCirclePosLength/radius);
    vec.y = direction.y * (innerCirclePosLength/radius);

    game::player.runDirection(vec);

  }

}

/*
void Thumbstick::keyInput(SDL_Event e,InputEvent* ie){

  switch(e.type){
    case SDL_MOUSEBUTTONDOWN:
      {
      
      float difX = ie->x-outerRing->pos.x;
      float difY = ie->y-outerRing->pos.y;

      float pathag = sqrt(pow(fabs(difX),2)+pow(fabs(difY),2));

      if(pathag<=radius){
	game::player.zRunning = true;
	game::player.xRunning = true;
	active = true;
      }


      mouseHandling(ie);
      }
      break;
    case SDL_MOUSEBUTTONUP:
      active = false;
      game::player.stopRunning();
      break;
    case SDL_MOUSEMOTION:
      mouseHandling(ie);
      break;
  }

}

 */

void Thumbstick::mouseHandling(InputEvent* e){

  float difX = e->x-outerRing->pos.x;
  float difY = e->y-outerRing->pos.y;

  float pathag = sqrt(pow(fabs(e->x-outerRing->pos.x),2)+pow(fabs(e->y-outerRing->pos.y),2));

  innerCirclePosLength = pathag;

  float x = difX/pathag;
  float y = difY/pathag;

  direction.x = x;
  direction.y = -y;

  //SDL_Log(("X: "+toString(difX)+" Y: "+toString(difY)+" pathag: "+toString(pathag)+" hypot: "+toString(hypot(fabs(difX-pos.x),fabs(difY-pos.y)))).c_str());

  if(hypot(fabs(difX-pos.x),fabs(difY-pos.y))<=radius){
    innerCircle->pos.x = e->x;
    innerCircle->pos.y = e->y;
  }else{
    //SDL_Log(("atan: "+toString(atan(fabs(y-outerMovementRing->pos.y)/fabs(x-outerMovementRing->pos.x)))).c_str());
    //normalizing the vector

    innerCircle->pos.x = (x*radius)+outerRing->pos.x;
    innerCircle->pos.y = (y*radius)+outerRing->pos.y;
  }

}

#include <jni.h>

extern "C" {

JNIEXPORT void JNICALL
Java_org_voxim_coventinaandroid_CoventinaView_init(
        JNIEnv* env,
        jobject /* this */)
{
    init();
}

JNIEXPORT void JNICALL
Java_org_voxim_coventinaandroid_CoventinaView_resize(
        JNIEnv* env,
        jobject, /* this */
        int width,
        int height
        )

{
    glViewport(0, 0, width, height);
    aspect_ratio = float(width)/float(height);
}

JNIEXPORT void JNICALL
Java_org_voxim_coventinaandroid_CoventinaView_redraw(
        JNIEnv* env,
jobject /* this */)
{
    /*
    float r = (rand()%255) / 255.0;
    float g = (rand()%255) / 255.0;
    float b = (rand()%255) / 255.0;
    __android_log_print(ANDROID_LOG_DEBUG, "Coventina", "Red: %f", r);
    glViewport(0, 0, 500, 500);
    glClearColor(r, g, b, 0.5);
    glClear(GL_COLOR_BUFFER_BIT);
     */

    loop::loop();
}

}