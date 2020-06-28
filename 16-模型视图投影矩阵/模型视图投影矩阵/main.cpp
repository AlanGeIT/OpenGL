//
//  main.cpp
//  模型视图投影矩阵
//
//  Created by Alan Ge on 2020/4/19.
//  Copyright © 2020 AlanGe. All rights reserved.
//

#include <stdio.h>
#include "GLTools.h"
#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLGeometryTransform.h"
#include "GLBatch.h"
#include "StopWatch.h"

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif


GLFrustum           viewFrustum;    // 视景体-投影矩阵通过它来设置
GLShaderManager     shaderManager;  // 固定着色管理器
GLTriangleBatch     torusBatch;     // 三角形批次类，绘制图形


// 设置视口和投影矩阵
void ChangeSize(int w, int h)
{
    if (h==0) {
        h = 1;
    }
    glViewport(0, 0, w, h);
    
    // 创建投影矩阵，并将它载入投影矩阵堆栈中
    // setPerspective函数的参数是一个从顶点方向看去的视场角度（用角度值表示）
    // 设置透视模式，初始化其透视矩阵
    // 第一个参数一般是35，或者45
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 1000.0f);
}

// 召唤场景
void RenderScene(void)
{
   // 建立一个基于时间变化的动画
    static CStopWatch rotTimer;
    
    // 当前时间 * 60s
    float yRot = rotTimer.GetElapsedSeconds() * 60.0f;
    // 清除颜色缓冲区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 矩阵变量 mRotate：旋转，mModelView：模型视图，mModelViewProjection：模型矩阵最终保存到这
    M3DMatrix44f mTranlate,mRotate,mModelView,mModelViewProjection;
    
    // 将圆球像Z轴负方向移动2.5个单位长度 (mTranlate(结果保存的地方)，x，y，z)
    m3dTranslationMatrix44(mTranlate, 0.0f, 0.0f,-2.5f);
    
    // 围绕y轴旋转（mRotate(旋转结果保存的地方)，m3dDegToRad(yRot)(旋转角度)，x,y,z）
    m3dRotationMatrix44(mRotate, m3dDegToRad(yRot), 0.0f, 1.0f, 0.0f);
    
    // 将平移和旋转的矩阵进行叉乘，产生一个新的矩阵mModelView
    // mModelView：最终结果矩阵，mTranlate:平移矩阵，mRotate：旋转矩阵
    m3dMatrixMultiply44(mModelView, mTranlate, mRotate);
    
    // 模型视图矩阵 和 投影矩阵 相乘
    // 将投影矩阵 与 模型视图矩阵进行叉乘，将变化最终结果通过矩阵叉乘的方式应用到mModelViewProjection中来
    // mModelViewProjection：结果，viewFrustum.GetProjectionMatrix()：获取投影矩阵，mModelView：上面处理的结果矩阵
    m3dMatrixMultiply44(mModelViewProjection, viewFrustum.GetProjectionMatrix(), mModelView);
    
    // 定义一个颜色
    GLfloat vBlack[] = {0.0f,0.0f,0.0f,1.0f};
    
    // 调用平面着色器来渲染图像
    shaderManager.UseStockShader(GLT_SHADER_FLAT,mModelViewProjection,vBlack);
    
    // 开始绘图
    torusBatch.Draw();
    
    glutSwapBuffers();// 交换两个缓冲区指针
    glutPostRedisplay();
}

void SetupRC()
{
    // 设置背影颜色
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    // 开启深度测试
    glEnable(GL_DEPTH_TEST);
    // 初始化着色管理器
    shaderManager.InitializeStockShaders();
    //球
    /*
     gltMakeSphere(GLTriangleBatch& sphereBatch, GLfloat fRadius, GLint iSlices, GLint iStacks);
     参数1:三角形批次类对象
     参数2:球体的半径
     参数3:从球体底部到顶部的三角形带的数量.其实球体是一圈一圈的三角形带组成的.
     参数4:围绕球体一圈排列的三角形对数
     */
    gltMakeSphere(torusBatch, 0.4f, 10, 20);
    // 图形填充方式(前面和背面，线段)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("ModelViewProjection Example");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    SetupRC();
    
    glutMainLoop();
    return 0;
}
