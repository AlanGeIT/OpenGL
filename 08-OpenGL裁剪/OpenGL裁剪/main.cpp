//
//  main.cpp
//  OpenGL裁剪
//
//  Created by Alan Ge on 2020/3/28.
//  Copyright © 2020 AlanGe. All rights reserved.
//

//demo OpenGL 裁剪
#include "GLTools.h"
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

//召唤场景
void RenderScene(void)
{
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    //一 现在裁剪出小红色分区
    //1.指定裁剪区的颜色为红色--设置!
    glClearColor(1.0f, 0.0f, 0.0f,1.0f);
    
    //2.设置裁剪尺寸
    glScissor(100,100,600, 400);
    
    //3.开启裁剪
    glEnable(GL_SCISSOR_TEST);
    
    //4.开启清屏,执行裁剪
    glClear(GL_COLOR_BUFFER_BIT);
    
    //二 裁剪另外一个绿色的小矩形
    //1.指定裁剪区的颜色为绿色--设置!
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    
    //2.设置裁剪尺寸
    glScissor(200,200,400, 200);
    
    //3.开启裁剪
    glEnable(GL_SCISSOR_TEST);
    
    //4.开启清屏,执行裁剪
    glClear(GL_COLOR_BUFFER_BIT);

    //关闭裁剪测试
    glDisable(GL_SCISSOR_TEST);
    
    //强制执行缓存区
    glutSwapBuffers();
}

void ChangeSize(int w, int h)
{
    //保证高度不能为0
    if(h == 0)
        h = 1;
    
    // 将视口设置为窗口尺寸
    glViewport(0, 0, w, h);
}

//程序入口
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800,600);
    glutCreateWindow("OpenGL Scissor");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutMainLoop();
    
    return 0;
}

