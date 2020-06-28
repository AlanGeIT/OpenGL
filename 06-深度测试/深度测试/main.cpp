//
//  main.cpp
//  深度测试
//
//  Created by Alan Ge on 2020/3/28.
//  Copyright © 2020 AlanGe. All rights reserved.
//

//demo演示了OpenGL背面剔除，深度测试，和多边形模式
#include "GLTools.h"
#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLGeometryTransform.h"

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

// 设置角色帧，作为相机
GLFrame             viewFrame;

// 使用GLFrustum类来设置透视投影
GLFrustum           viewFrustum;
GLTriangleBatch     torusBatch;         // 平面圈图形
GLMatrixStack       modelViewMatix;     // 堆栈矩阵
GLMatrixStack       projectionMatrix;
GLGeometryTransform transformPipeline;
GLShaderManager     shaderManager;

// 标记：背面剔除、深度测试
int iCull = 0;  // 是否启用背面剔除，0：不启用，1：启用
int iDepth = 0; // 是否启用深度测试，0：不启用，1：启用

// 右键菜单栏选项 点击右击菜单中的选项,则会调用到此处
void ProcessMenu(int value)
{
    switch (value) {
        case 1:
            iCull = !iCull;
            break;
        
        case 2:
            iDepth = !iDepth;
            break;
            
        case 3:
            // 填充渲染
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        
        case 4:
            // 线框渲染
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
            
        case 5:
            // 点渲染
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;
            
        default:
            break;
    }
    
    // 重新显示
    glutPostRedisplay();
}

// 召唤场景
void RenderScene(void)
{
    // 清除颜色缓冲区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (iCull) {
        // 实现背面剔除
        // 1.开启背面剔除功能
        glEnable(GL_CULL_FACE);
        // 2.指定模型
        glFrontFace(GL_CCW);
        // 3.执行剔除
        glCullFace(GL_BACK);
        
    } else {
        glDisable(GL_CULL_FACE);
    }
    
    // 设置深度测试
    if (iDepth) {
        glEnable(GL_DEPTH_TEST);// 开启深度测试
    } else {
        glDisable(GL_DEPTH_TEST);// 关闭深度测试
    }
    
    modelViewMatix.PushMatrix(viewFrame);
    
    GLfloat vRed[] = {1.0f,0.0f,0.0f,1.0f};
    
    // 默认光源着色器
    // 参数1：GLT_SHADER_DEFAULT_LIGHT，默认的光源类型
    // 参数2：模型视图矩阵
    // 参数3：投影矩阵
    // 参数4：颜色
    // transformPipeline 管理堆栈,模型视图堆栈\投影矩阵堆栈
    shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT,transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(),vRed);
    
    // 绘制
    torusBatch.Draw();
    
    // 出栈
    modelViewMatix.PopMatrix();

    glutSwapBuffers();
}

// context. 图像上下文
void SetupRC()
{
    // 设置清屏颜色--可以理解背景颜色-颜色缓存区中
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    
    // 初始化固定管线
    shaderManager.InitializeStockShaders();
    
    // 观察者
    // 参数1:表示Z轴的值
    viewFrame.MoveForward(7.0f);
    
    // 创建一个天天圈
    /*
     void gltMakeTorus(GLTriangleBatch& torusBatch, GLfloat majorRadius, GLfloat minorRadius, GLint numMajor, GLint numMinor);
     参数1:GLTriangleBatch：三角形批次帮助类
     参数2:外圆半径,从圆心到外圆的距离
     参数3:内边圆半径,从圆心到内圆的距离
     参数4:外环三角形带的数量 numMajor = 2 * numMinor
     参数5:内环三角形带的数量 numMajor = 2 * numMinor
          外环的三角形是内环的2倍
     */
    gltMakeTorus(torusBatch, 1.0f, 0.3f, 52, 26);
    
    // 设置点的大小
    glPointSize(4.0f);
}

// 键位设置，通过不同的键位对其进行设置
// 控制Camera的移动，从而改变视口
void SpecialKeys(int key, int x, int y)
{
    // 通过移动世界坐标系来产生物体旋转的功能
    /*
     RotateWorld(float fAngle, float x, float y, float z)
     fAngle:旋转度数
     x,y,z：0或者1，0：不旋转，1：旋转
     */
    // 上下：围绕x轴旋转
    if(key == GLUT_KEY_UP)
        viewFrame.RotateWorld(m3dDegToRad(-5.0), 1.0f, 0.0f, 0.0f);
    
    if(key == GLUT_KEY_DOWN)
        viewFrame.RotateWorld(m3dDegToRad(5.0), 1.0f, 0.0f, 0.0f);
    
    // 左右：围绕y轴旋转
    if(key == GLUT_KEY_LEFT)
        viewFrame.RotateWorld(m3dDegToRad(-5.0), 0.0f, 1.0f, 0.0f);
    
    if(key == GLUT_KEY_RIGHT)
        viewFrame.RotateWorld(m3dDegToRad(5.0), 0.0f, 1.0f, 0.0f);
    
    // 重新刷新window
    glutPostRedisplay();
}


void ChangeSize(int w, int h)
{
    // 防止h变为0
    if(h == 0)
        h = 1;
    
    // 设置视口窗口尺寸
    glViewport(0, 0, w, h);
    
    // setPerspective函数的参数是一个从顶点方向看去的视场角度（用角度值表示）
    // 设置透视模式，初始化其透视矩阵
    // 第一个参数一般是35，或者45
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);
    
    // 把透视矩阵加载到透视矩阵对阵中
    // viewFrustum.GetProjectionMatrix() 获取投影矩阵
    // projectionMatrix.LoadMatrix(矩阵) 把矩阵加载到projectionMatrix中来
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    // 初始化渲染管线
    transformPipeline.SetMatrixStacks(modelViewMatix, projectionMatrix);
}


int main(int argc, char* argv[])
{
    // 设置当前工作目录，针对MAC OS X
    gltSetWorkingDirectory(argv[0]);
    // 初始化GLUT库
    glutInit(&argc, argv);
    
    /* 初始化双缓冲窗口，其中标志GLUT_DOUBLE、GLUT_RGBA、GLUT_DEPTH、GLUT_STENCIL分别指
        双缓冲窗口、RGBA颜色模式、深度测试、模板缓冲区*/
    // 申请一个眼神缓存区、深度缓存区、双缓存区、模板缓存区
    // GLUT_DOUBLE:双缓存区
    // GLUT_RGBA:颜色缓存区：存储颜色值
    // GLUT_DEPTH:深度缓冲区：用来记录每个像素上的深度值。深度测试、背面剔除。为了确保像素的遮挡关系。
    // GLUT_STENCIL:模板缓存区：
    // 初始化双缓冲窗口
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
     // 设置window的尺寸
    glutInitWindowSize(800, 600);
    // 设置window的名称
    glutCreateWindow("Geometry Test Program");
    
    // 注册回调函数
    glutReshapeFunc(ChangeSize);    // 修改尺寸
    glutSpecialFunc(SpecialKeys);   // 特殊键位函数（上下左右）
    glutDisplayFunc(RenderScene);   // 显示函数
    
    // 设置右击菜单选项
    glutCreateMenu(ProcessMenu);
    
    // 添加右击菜单栏选项
    // 参数1: 菜单栏中选项的内容
    // 参数2: 1
    // glutAddMenuEntry(菜单栏中选项的内容, 值);
    glutAddMenuEntry("正面背面剔除", 1);
    glutAddMenuEntry("深度测试", 2);
    glutAddMenuEntry("Set Fill Mode", 3);
    glutAddMenuEntry("Set line Mode", 4);
    glutAddMenuEntry("Set Point Mode", 5);

    // 添加到右击菜单
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    SetupRC();
    
    glutMainLoop();
    return 0;
}
