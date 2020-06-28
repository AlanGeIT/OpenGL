//
//  main.cpp
//  图形移动(矩阵变换)
//
//  Created by Alan Ge on 2020/3/29.
//  Copyright © 2020 AlanGe. All rights reserved.
//


/*
 案例：实现矩阵的移动，利用矩阵的平移、旋转、综合变化等
 */
#include "GLTools.h"
#include "GLShaderManager.h"
#include "math3d.h"

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLBatch             squareBatch;
GLShaderManager     shaderManager;

GLfloat blockSize = 0.1f;
GLfloat vVerts[] = {
    -blockSize, -blockSize, 0.0f,
    blockSize, -blockSize, 0.0f,
    blockSize,  blockSize, 0.0f,
    -blockSize,  blockSize, 0.0f};

GLfloat xPos = 0.0f;
GLfloat yPos = 0.0f;

M3DVector3f vVector;

M3DVector4f vVector4 = {0.0f,0.0f,0.0f,1.0f};

M3DVector3f vVert[] = {
    -0.5f,0.0f,0.0f,
    0.5f,0.0f,0.0f,
    0.0f,0.5f,0.0f
};

void SetupRC()
{
    // 1.定义2个向量
    M3DVector3f v1 = {1.0f,0.0f,0.0f};
    M3DVector3f v2 = {0.0f,1.0f,0.0f};
  
    // 2.求得2个向量夹角
    // 向量就是一维矩阵
    /*
     m3dDotProduct3(v1, v2);
     参数1:向量A
     参数2:向量B
     求得向量V1,V2的夹角的cos值
     V1.V2 表示V1,V2向量点乘
     */
    GLfloat value1 = m3dDotProduct3(v1, v2);
    printf("v1v2余弦值:%f\n",value1);
    
    // 求得v1,v2的弧度值
    GLfloat value2 = acos(value1);
    printf("v1v2弧度值:%f\n",value2);
    
    // 方法2
    GLfloat value3 = m3dGetAngleBetweenVectors3(v1, v2);
    printf("v1v2弧度值:%f\n",value3);

    // 弧度->度数
    GLfloat value4 = m3dRadToDeg(value3);
    // 度数->弧度
    GLfloat value5 = m3dDegToRad(90);
    
    // 定义2个向量
    M3DVector3f vector2 = {0.0f,0.0f,0.0f};
    
    // 实现叉乘
    /*
      m3dCrossProduct3(M3DVector3f result, const M3DVector3f u, const M3DVector3f v)
     参数1:结果向量
     参数2:V1
     参数3:V2
     */
    m3dCrossProduct3(vector2, v1, v2);
    printf("%f,%f,%f\n",vector2[0],vector2[1],vector2[2]);
    
    // 矩阵:坐标变换 旋转\缩放\平移
    // 定义矩阵 3*3
    M3DMatrix33f mv1;
    
    // 定义一个4 * 4矩阵
    M3DMatrix44f mv2;
    
    // 定义一个4 * 4矩阵
    // OpenGL 常用一维数组来表示矩阵 -- 推荐
    GLfloat matrix[16];
    
    // OpenGL 可以使用二维数据来表示矩阵,但是不推荐!
    GLfloat matrix2[4][4];
    
    //创建单元矩阵 -- 列矩阵
    GLfloat m[] = {
        1.0f,0.0f,0.0f,0.0f,//X
        0.0f,1.0f,0.0f,0.0f,//Y
        0.0f,0.0f,1.0f,0.0f,//Z
        0.0f,0.0f,0.0f,1.0f,//变换
    };
    
    // 创建单元矩阵 理解常量1
    M3DMatrix44f m3;
    m3dLoadIdentity44(m3);
    
    // 平移
    /*
      m3dTranslationMatrix44(M3DMatrix44f m, float x, float y, float z)
     参数1:结果矩阵,平移之后的结果矩阵
     参数2:沿着X轴移动多少,正数\负数
     参数3:沿着Y轴移动多少,正数\负数
     参数4:沿着Z轴移动多少,正数\负数
     */
    m3dTranslationMatrix44(m3, 0.0f, 10.0f, 0.0f);
    
    // 旋转
    /*
      m3dRotationMatrix44(M3DMatrix44f m, float angle, float x, float y, float z);
     参数1:结果矩阵,旋转之后的结果矩阵
     参数2:旋转多少弧度
     参数3:是否围绕X轴旋转,是(1),不是(0)
     参数4:是否围绕Y轴旋转,是(1),不是(0)
     参数5:是否围绕Z轴旋转,是(1),不是(0)
     */
    m3dRotationMatrix44(m3, m3dDegToRad(45.0f), 1.0f, 0.0f, 0.0f);
     
    // 缩放
    /*
     void m3dScaleMatrix44(M3DMatrix44f m, float xScale, float yScale, float zScale)
     参数1:结果矩阵
     参数2:围绕X轴放大\缩小;放大x>1,缩小:0.5f
     参数3:围绕Y轴放大\缩小;放大x>1,缩小:0.5f
     参数4:围绕Z轴放大\缩小;放大x>1,缩小:0.5f
     */
    m3dScaleMatrix44(m3, 1.0f, 10.0f, 1.0f);
    
    // 背景颜色
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f );
    
    shaderManager.InitializeStockShaders();
    
    // 加载三角形
    squareBatch.Begin(GL_TRIANGLE_FAN, 4);
    squareBatch.CopyVertexData3f(vVerts);
    squareBatch.End();
}

// 移动（移动只是计算了X,Y移动的距离，以及碰撞检测）
void SpecialKeys(int key, int x, int y)
{
    GLfloat stepSize = 0.025f;
    
    if(key == GLUT_KEY_UP)
        yPos += stepSize;
    
    if(key == GLUT_KEY_DOWN)
        yPos -= stepSize;
    
    if(key == GLUT_KEY_LEFT)
        xPos -= stepSize;
    
    if(key == GLUT_KEY_RIGHT)
        xPos += stepSize;
    
    // 碰撞检测
    if(xPos < (-1.0f + blockSize)) xPos = -1.0f + blockSize;
    
    if(xPos > (1.0f - blockSize)) xPos = 1.0f - blockSize;
    
    if(yPos < (-1.0f + blockSize))  yPos = -1.0f + blockSize;
    
    if(yPos > (1.0f - blockSize)) yPos = 1.0f - blockSize;
    
    glutPostRedisplay();
}

void RenderScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    
    M3DMatrix44f mFinalTransform, mTranslationMatrix, mRotationMatrix;
    
    // 平移 xPos,yPos
    m3dTranslationMatrix44(mTranslationMatrix, xPos, yPos, 0.0f);
    
    // 每次重绘时，旋转5度
    static float yRot = 0.0f;
    yRot += 5.0f;
    m3dRotationMatrix44(mRotationMatrix, m3dDegToRad(yRot), 0.0f, 0.0f, 1.0f);
    
    // 将旋转和移动的结果合并到mFinalTransform 中
    m3dMatrixMultiply44(mFinalTransform, mTranslationMatrix, mRotationMatrix);
    
    // 将矩阵结果提交到固定着色器（平面着色器）中。
    shaderManager.UseStockShader(GLT_SHADER_FLAT, mFinalTransform, vRed);
    squareBatch.Draw();
    
    // 执行缓冲区交换
    glutSwapBuffers();
}

void ChangeSize(int w, int h)
{
    glViewport(0, 0, w, h);
}

int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Move Block with Arrow Keys");
    
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);
    
    SetupRC();
    
    glutMainLoop();
    return 0;
}

