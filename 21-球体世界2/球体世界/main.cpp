//
//  main.cpp
//  球体世界
//
//  Created by Alan Ge on 2020/6/21.
//  Copyright © 2020 AlanGe. All rights reserved.
//

#include "GLTools.h"
#include "GLShaderManager.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"
#include "StopWatch.h"

#include <math.h>
#include <stdio.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

/*
 1.给球体世界添加纹理
 2.给球体世界添加反光的效果
 
 反光效果:小把戏，首先澧染翻转的场景.然后在上面渲染大理石地面.使用Alpha与背景的混合.
*/

//**4、添加附加随机球
#define NUM_SPHERES 50
GLFrame spheres[NUM_SPHERES];

GLShaderManager         shaderManager;      // 着色器管理器
GLMatrixStack           modelViewMatrix;    // 模型视图矩阵
GLMatrixStack           projectionMatrix;   // 投影矩阵
GLFrustum               viewFrustum;        // 视景体
GLGeometryTransform     transformPipeline;  // 几何图形变换管道

GLTriangleBatch         torusBatch;         // 花托批处理
GLBatch                 floorBatch;         // 地板批处理

// **2、定义公转球的批处理（公转自转）**
GLTriangleBatch         sphereBatch;        //球批处理

//**3、角色帧 照相机角色帧（全局照相机实例）
GLFrame                 cameraFrame;

// **5、添加纹理
// 纹理标记数组
GLuint uiTextures[3];

bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
    GLbyte *pBits;
    int nWidth, nHeight, nComponents;
    GLenum eFormat;
    
    // 读取纹理数据
    pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
    if(pBits == NULL)
        return false;
    
    // 2、设置纹理参数
    // 参数1：纹理维度
    // 参数2：为S/T坐标设置模式
    // 参数3：wrapMode,环绕模式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    
    // 参数1：纹理维度
    // 参数2：线性过滤
    // 参数3：wrapMode,环绕模式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
    // 3、精密包装像素数据
    // 参数1：GL_UNPACK_ALIGNMENT,指定OpenGL如何从数据缓存区中解包图像数据
    // 参数2:针对GL_UNPACK_ALIGNMENT 设置的值
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    // 载入纹理
    // 参数1：纹理维度
    // 参数2：mip贴图层次
    // 参数3：纹理单元存储的颜色成分（从读取像素图是获得）-将内部参数nComponents改为了通用压缩纹理格式GL_COMPRESSED_RGB
    // 参数4：加载纹理宽
    // 参数5：加载纹理高
    // 参数6：加载纹理的深度
    // 参数7：像素数据的数据类型（GL_UNSIGNED_BYTE，每个颜色分量都是一个8位无符号整数）
    // 参数8：指向纹理图像数据的指针
    glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, nWidth, nHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBits);
    
    // 使用完毕释放pBits
    free(pBits);
    
    // 只有minFilter 等于以下四种模式，才可以生成Mip贴图
    // GL_NEAREST_MIPMAP_NEAREST具有非常好的性能，并且闪烁现象非常弱
    // GL_LINEAR_MIPMAP_NEAREST常常用于对游戏进行加速，它使用了高质量的线性过滤器
    // GL_LINEAR_MIPMAP_LINEAR 和GL_NEAREST_MIPMAP_LINEAR 过滤器在Mip层之间执行了一些额外的插值，以消除他们之间的过滤痕迹。
    // GL_LINEAR_MIPMAP_LINEAR 三线性Mip贴图。纹理过滤的黄金准则，具有最高的精度。
    if(minFilter == GL_LINEAR_MIPMAP_LINEAR ||
       minFilter == GL_LINEAR_MIPMAP_NEAREST ||
       minFilter == GL_NEAREST_MIPMAP_LINEAR ||
       minFilter == GL_NEAREST_MIPMAP_NEAREST)
        // 加载Mip,纹理生成所有的Mip层
        // 参数：GL_TEXTURE_1D、GL_TEXTURE_2D、GL_TEXTURE_3D
        glGenerateMipmap(GL_TEXTURE_2D);
    
    return true;
}

void SetupRC()
{
    // 初始化着色器管理器
    shaderManager.InitializeStockShaders();
    
    // 开启深度测试
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    // 开启多边形模型
    // **4、关闭线框渲染模式
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    // 设置清屏颜色到颜色缓存区
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // 绘制甜甜圈
    gltMakeTorus(torusBatch, 0.4f, 0.15f, 30, 30);
    
    // **2、 绘制球(公转自转)**
    gltMakeSphere(sphereBatch, 0.1f, 26, 13);
    
    // 往地板floorBatch批处理中添加顶点数据
    GLfloat texSize = 10.0f;
    floorBatch.Begin(GL_TRIANGLE_FAN, 4,1);
    floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    floorBatch.Vertex3f(-20.f, -0.41f, 20.0f);
    
    floorBatch.MultiTexCoord2f(0, texSize, 0.0f);
    floorBatch.Vertex3f(20.0f, -0.41f, -20.f);
    
    floorBatch.MultiTexCoord2f(0, texSize, texSize);
    floorBatch.Vertex3f(20.0f, -0.41f, -20.0f);
    
    floorBatch.MultiTexCoord2f(0, 0.0f, texSize);
    floorBatch.Vertex3f(-20.0f, -0.41f, -20.0f);
    floorBatch.End();
    
    // 绑定3个纹理
    glGenTextures(3, uiTextures);
    
    // 将TGA文件加载为2D纹理。
    // 参数1：纹理文件名称
    // 参数2&参数3：需要缩小&放大的过滤器
    // 参数4：纹理坐标环绕模式
    glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
    LoadTGATexture("marble.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
    LoadTGATexture("marslike.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
    LoadTGATexture("moonlike.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    
    // **3、在场景中随机位置对球体进行初始化
    // 随机放置球体
    for (int i = 0; i < NUM_SPHERES; i++) {
        
        // y轴不变，X,Z产生随机值
        GLfloat x = ((GLfloat)((rand() % 400) - 200 ) * 0.1f);
        GLfloat z = ((GLfloat)((rand() % 400) - 200 ) * 0.1f);
        
        // 在y方向，将球体设置为0.0的位置，这使得它们看起来是飘浮在眼睛的高度
        // 对spheres数组中的每一个顶点，设置顶点数据
        spheres[i].SetOrigin(x, 0.0f, z);
    }
}

void ShutdownRC(void)
{
    glDeleteTextures(3, uiTextures);
}

// 屏幕更改大小或已初始化
void ChangeSize(int nWidth, int nHeight)
{
    glViewport(0, 0, nWidth, nHeight);
    
    // 创建投影矩阵，。
    viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
    
    // viewFrustum.GetProjectionMatrix()  获取viewFrustum投影矩阵
    // 并将其加载到投影矩阵堆栈上
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    // 设置变换管道以使用两个矩阵堆栈（变换矩阵modelViewMatrix ，投影矩阵projectionMatrix）
    // 初始化GLGeometryTransform 的实例transformPipeline.通过将它的内部指针设置为模型视图矩阵堆栈 和 投影矩阵堆栈实例，来完成初始化
    // 当然这个操作也可以在SetupRC 函数中完成，但是在窗口大小改变时或者窗口创建时设置它们并没有坏处。而且这样可以一次性完成矩阵和管线的设置。
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    modelViewMatrix.LoadIdentity();
}

void DrawSongAndDance(GLfloat yRot)
{
    static GLfloat vWhite[] = {1.0f, 1.0f, 1.0f, 1.0f};
    static GLfloat vLightPos[] = {0.0f, 3.0f, 0.0f, 1.0f};
    
    // **4、添加光源
    // 光源位置的全局坐标存储在vLightPos变量中，其中包含了光源位置x坐标、y坐标、z坐标和w坐标。我们必须保留w坐标为1.0。因为无法用一个3分量去乘以4*4矩阵
    M3DVector4f vLightTransformed;
    
    // 定义一个4*4的相机矩阵
    M3DMatrix44f mCamera;
    // 从modelViewMatrix获取矩阵堆栈顶部的值
    modelViewMatrix.GetMatrix(mCamera);
    // 将照相机矩阵mCamera 与 光源矩阵vLightPos 相乘获得vLigthTransformed矩阵
    m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
    
    // 将结果压栈
    modelViewMatrix.PushMatrix();
    // 仿射变换，平移
    modelViewMatrix.Translatev(vLightPos);
    
    // 绘制（平面着色器）
    /*
     参数1:GLT_SHADER_FLAT
     参数2：4*4矩阵（模型视图投影矩阵）
     参数3：颜色数组
     */
    shaderManager.UseStockShader(GLT_SHADER_FLAT,
                                 transformPipeline.GetModelViewProjectionMatrix(),
                                 vWhite);
    sphereBatch.Draw();
    
    // 恢复矩阵
    modelViewMatrix.PopMatrix();
    
    // 绘制悬浮球体
    // 绑定纹理
    glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
    
    // 思路：循环绘制50个蓝色悬浮球体，绘制一个压栈一个，绘制完成出栈一个
    for(int i = 0; i < NUM_SPHERES; i++) {
        modelViewMatrix.PushMatrix();
        modelViewMatrix.MultMatrix(spheres[i]);
        
        /**绘制光源，修改着色器管理器
         参数1：GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF
         参数2：模型视图矩阵
         参数3：投影矩阵
         参数4：视点坐标系中的光源位置
         参数5：基本漫反射颜色
         参数6：颜色（使用纹理则不用颜色）
         */
        shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                     modelViewMatrix.GetMatrix(),
                                     transformPipeline.GetProjectionMatrix(),
                                     vLightTransformed,
                                     vWhite,
                                     0);
        sphereBatch.Draw();
        
        modelViewMatrix.PopMatrix();
    }
    
    // 绘制旋转甜甜圈
    // modelViewMatrix 顶部矩阵沿着z轴移动2.5单位
    modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);
    
    // 保存平移（公转自转）
    modelViewMatrix.PushMatrix();
    
    // modelViewMatrix 顶部矩阵旋转yRot度
    modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
    
    // 绑定纹理
    glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
   
    // 绘制光源，修改着色器管理器
    /**绘制光源，修改着色器管理器
     参数1：GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF
     参数2：模型视图矩阵
     参数3：投影矩阵
     参数4：视点坐标系中的光源位置
     参数5：基本漫反射颜色
     参数6：颜色（使用纹理则不用颜色）
     */
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                 modelViewMatrix.GetMatrix(),
                                 transformPipeline.GetProjectionMatrix(),
                                 vLightTransformed,
                                 vWhite,
                                 0);
    // 绘制
    torusBatch.Draw();
    
    // 恢复矩阵
    modelViewMatrix.PopMatrix();
    
    // 绘制公转球体（公转自转)
    modelViewMatrix.Rotate(yRot * -2.0f, 0.0f, 1.0f, 0.0f);
    modelViewMatrix.Translate(0.8f, 0.0f, 0.0f);
    
    // 绑定纹理
    glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
    
    // 绘制光源，着色器管理器
    /**绘制光源，修改着色器管理器
     参数1：GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF
     参数2：模型视图矩阵
     参数3：投影矩阵
     参数4：视点坐标系中的光源位置
     参数5：基本漫反射颜色
     参数6：颜色（使用纹理则不用颜色）
     */
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                 modelViewMatrix.GetMatrix(),
                                 transformPipeline.GetProjectionMatrix(),
                                 vLightTransformed,
                                 vWhite,
                                 0);
    
    sphereBatch.Draw();
}

// 进行调用以绘制场景
void RenderScene(void)
{
    // 颜色值 地板颜色、甜甜圈颜色
    static GLfloat vFloorColor[] = {1.0f, 1.0f, 0.0f, 0.75f};
    
    // 基于时间动画
    static CStopWatch rotTimer;
    float yRot = rotTimer.GetElapsedSeconds() * 60.0f;
    
    // 清除颜色缓存区和深度缓冲区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 将当前的模型视图矩阵压入矩阵堆栈（单位矩阵）
    /*因为我们先绘制地面，而地面是不需要有任何变换的。所以在开始渲染时保证矩阵状态，
     然后在结束时使用相应的PopMatrix恢复它。这样就不必在每一次渲染时重载单位矩阵了。
     */
    modelViewMatrix.PushMatrix();
    
    // **3、设置照相机矩阵
    M3DMatrix44f mCamera;
    // **3、从camraFrame中获取一个4*4的矩阵；
    cameraFrame.GetCameraMatrix(mCamera);
    // **3、将照相机矩阵压入模型视图堆栈中
    modelViewMatrix.MultMatrix(mCamera);
    
    // 压栈
    modelViewMatrix.PushMatrix();
    
    // ---添加反光效果---
    modelViewMatrix.Scale(1.0f, -1.0f, 1.0f); //翻转Y轴
    modelViewMatrix.Translate(0.0f, 0.8f, 0.0f); // scootch世界缩小一点...
    /*
     定义前向和背向的多边形
     GL_CCW：表示传入的mode会选择逆时针为前向
     GL_CW：表示顺时针为前向。
     */
    glFrontFace(GL_CW);
    // 绘制地面以外其他部分
    DrawSongAndDance(yRot);
    glFrontFace(GL_CCW);
    // 绘制完，恢复矩阵
    modelViewMatrix.PopMatrix();
    
    // 开启混合功能
    glEnable(GL_BLEND);
    
    // 绑定地面纹理
    glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
    
    /* glBlendFunc 颜色混合方程式
     参数1：目标颜色
     参数2：源颜色
     */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    /*
     纹理调整着色器(将一个基本色乘以一个取自纹理的单元nTextureUnit的纹理)
     参数1：GLT_SHADER_TEXTURE_MODULATE
     参数2：模型视图矩阵
     参数3：颜色
     参数4：纹理单元（第0层的纹理单元）
     */
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE,
                                 transformPipeline.GetModelViewProjectionMatrix(),
                                 vFloorColor,
                                 0);
    // 开始绘制
    floorBatch.Draw();
    
    // 取消混合
    glDisable(GL_BLEND);
    
    // 绘制地面以外其他部分
    DrawSongAndDance(yRot);
    
    // 绘制完，恢复矩阵
    modelViewMatrix.PopMatrix();
    
    glutSwapBuffers();
    glutPostRedisplay();
}

// 3.移动照相机参考帧，来对方向键作出响应
void SpeacialKeys(int key,int x,int y)
{
    float linear = 0.1f;
    float angular = float(m3dDegToRad(5.0f));
    
    if (key == GLUT_KEY_UP) {
        
        // MoveForward 平移
        cameraFrame.MoveForward(linear);
    }
    
    if (key == GLUT_KEY_DOWN) {
        cameraFrame.MoveForward(-linear);
    }
    
    if (key == GLUT_KEY_LEFT) {
        // RotateWorld 旋转
        cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);
    }
    
    if (key == GLUT_KEY_RIGHT) {
        cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);
    }
}


int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    
    glutCreateWindow("OpenGL SphereWorld");
    
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpeacialKeys);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    SetupRC();
    glutMainLoop();
    ShutdownRC();
    return 0;
}
