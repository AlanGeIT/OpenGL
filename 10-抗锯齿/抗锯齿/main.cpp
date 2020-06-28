//
//  main.cpp
//  抗锯齿
//
//  Created by Alan Ge on 2020/3/29.
//  Copyright © 2020 AlanGe. All rights reserved.
//

#include "GLTools.h"
#include "GLFrustum.h"

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLShaderManager shaderManager;
GLFrustum viewFrustum;
GLBatch smallStarBatch;
GLBatch mediumStarBatch;
GLBatch largeStarBatch;
GLBatch mountainRangeBatch;
GLBatch moonBatch;


#define SMALL_STARS     100
#define MEDIUM_STARS     40
#define LARGE_STARS      15

#define SCREEN_X        800
#define SCREEN_Y        600


// 选择菜单
void ProcessMenu(int value)
{
    switch (value) {
        case 1:
            // 打开抗锯齿功能
            // 1.开启混合功能
            glEnable(GL_BLEND);
            
            // 2.指定混合因子
            // 注意:如果你修改了混合方程式,当你使用混合抗锯齿功能时,请一定要改为默认混合方程式
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            // 3.开启对点\线\多边形的抗锯齿功能
            glEnable(GL_POINT_SMOOTH);
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_POLYGON_SMOOTH);
            
            break;
        
        case 2:
            // 关闭抗锯齿功能
            glDisable(GL_BLEND);
            glDisable(GL_LINE_SMOOTH);
            glDisable(GL_POINT_SMOOTH);
            glDisable(GL_POLYGON_SMOOTH);
            
            break;
        default:
            break;
    }
    
    glutPostRedisplay();
}


// 场景召唤
void RenderScene()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    // 定义白色
    GLfloat vWhite[] = {1.0f,1.0f,1.0f,1.0f};
    
    // 使用存储着色器中平面着色器
    // 参数1:类型,平面着色器
    // 参数2:投影矩阵
    // 参数3:颜色,白色
    shaderManager.UseStockShader(GLT_SHADER_FLAT,viewFrustum.GetProjectionMatrix(),vWhite);
    
    // 绘制小星星
    glPointSize(1.0f);
    smallStarBatch.Draw();
    
    // 绘制中等大小的星星
    glPointSize(4.0f);
    mediumStarBatch.Draw();
    
    // 绘制大星星
    glPointSize(8.0f);
    largeStarBatch.Draw();
    
    // 绘制地平线
    glLineWidth(3.5f);
    mountainRangeBatch.Draw();
    
    // 绘制月亮
    moonBatch.Draw();
    
    glutSwapBuffers();
    
}


// 对渲染进行必要的初始化。
void SetupRC()
{
    M3DVector3f vVerts[SMALL_STARS];
    int i;
    
    shaderManager.InitializeStockShaders();
    
    // 小星星
    smallStarBatch.Begin(GL_POINTS, SMALL_STARS);
    for(i = 0; i < SMALL_STARS; i++)
    {
        vVerts[i][0] = (GLfloat)(rand() % SCREEN_X);
        vVerts[i][1] = (GLfloat)(rand() % (SCREEN_Y - 100)) + 100.0f;
        vVerts[i][2] = 0.0f;
    }
    smallStarBatch.CopyVertexData3f(vVerts);
    smallStarBatch.End();
    
    // 中星星
    mediumStarBatch.Begin(GL_POINTS, MEDIUM_STARS);
    for(i = 0; i < MEDIUM_STARS; i++)
    {
        vVerts[i][0] = (GLfloat)(rand() % SCREEN_X);
        vVerts[i][1] = (GLfloat)(rand() % (SCREEN_Y - 100)) + 100.0f;
        vVerts[i][2] = 0.0f;
    }
    mediumStarBatch.CopyVertexData3f(vVerts);
    mediumStarBatch.End();
    
    // 大星星
    largeStarBatch.Begin(GL_POINTS, LARGE_STARS);
    for(i = 0; i < LARGE_STARS; i++)
    {
        vVerts[i][0] = (GLfloat)(rand() % SCREEN_X);
        vVerts[i][1] = (GLfloat)(rand() % (SCREEN_Y - 100)) + 100.0f;
        vVerts[i][2] = 0.0f;
    }
    largeStarBatch.CopyVertexData3f(vVerts);
    largeStarBatch.End();
    
    M3DVector3f vMountains[12] = { 0.0f, 25.0f, 0.0f,
        50.0f, 100.0f, 0.0f,
        100.0f, 25.0f, 0.0f,
        225.0f, 125.0f, 0.0f,
        300.0f, 50.0f, 0.0f,
        375.0f, 100.0f, 0.0f,
        460.0f, 25.0f, 0.0f,
        525.0f, 100.0f, 0.0f,
        600.0f, 20.0f, 0.0f,
        675.0f, 70.0f, 0.0f,
        750.0f, 25.0f, 0.0f,
        800.0f, 90.0f, 0.0f };
    
    mountainRangeBatch.Begin(GL_LINE_STRIP, 12);
    mountainRangeBatch.CopyVertexData3f(vMountains);
    mountainRangeBatch.End();
    
    // 月亮
    GLfloat x = 700.0f;
    GLfloat y = 500.0f;
    GLfloat r = 50.0f;
    GLfloat angle = 0.0f;
    
    moonBatch.Begin(GL_TRIANGLE_FAN, 34);
    int nVerts = 0;
    vVerts[nVerts][0] = x;
    vVerts[nVerts][1] = y;
    vVerts[nVerts][2] = 0.0f;
    for(angle = 0; angle < 2.0f * 3.141592f; angle += 0.2f) {
        nVerts++;
        vVerts[nVerts][0] = x + float(cos(angle)) * r;
        vVerts[nVerts][1] = y + float(sin(angle)) * r;
        vVerts[nVerts][2] = 0.0f;
    }
    nVerts++;
    
    vVerts[nVerts][0] = x + r;;
    vVerts[nVerts][1] = y;
    vVerts[nVerts][2] = 0.0f;
    moonBatch.CopyVertexData3f(vVerts);
    moonBatch.End();
    
    // 设置黑色背景
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
}



void ChangeSize(int w, int h)
{
    
    if(h == 0)
        h = 1;
    
    glViewport(0, 0, w, h);
    viewFrustum.SetOrthographic(0.0f, SCREEN_X, 0.0f, SCREEN_Y, -1.0f, 1.0f);
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Smoothing Out The Jaggies");
    
    // 创建菜单
    glutCreateMenu(ProcessMenu);
    glutAddMenuEntry("Antialiased Rendering",1);
    glutAddMenuEntry("Normal Rendering",2);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
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
