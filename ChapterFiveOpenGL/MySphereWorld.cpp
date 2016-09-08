//
//  MySphereWorld.cpp
//  ChapterFiveOpenGL
//
//  Created by XuLiang on 16/9/7.
//  Copyright © 2016年 XuLiang. All rights reserved.
//

#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#include <math.h>
#include <stdio.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLShaderManager		shaderManager;			// Shader Manager

GLMatrixStack		modelViewMatrix;		// Modelview Matrix
GLMatrixStack		projectionMatrix;		// Projection Matrix
GLGeometryTransform	transformPipeline;		// Geometry Transform Pipeline几何变换管道
GLFrustum			viewFrustum;			// View Frustum
GLFrame				cameraFrame;			// Camera frame

GLBatch             floorBatch;
GLBatch             cubeBatch;

GLuint				uiTextures[3];


void DrawSongAndDance(){
    
    static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    static GLfloat vLightPos[] = { 0.0f, 3.0f, 0.0f, 1.0f };
    
    M3DVector4f	vLightTransformed;
    M3DMatrix44f mCamera;
    modelViewMatrix.GetMatrix(mCamera);
    m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
    
    // Draw the light source
    modelViewMatrix.PushMatrix();
    modelViewMatrix.Translatev(vLightPos);
    shaderManager.UseStockShader(GLT_SHADER_FLAT,
                                 transformPipeline.GetModelViewProjectionMatrix(),
                                 vWhite);
    cubeBatch.Draw();
    modelViewMatrix.PopMatrix();
    
    modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);
    modelViewMatrix.PushMatrix();
    // Draw stuff relative to the camera
    glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                 modelViewMatrix.GetMatrix(),
                                 transformPipeline.GetProjectionMatrix(),
                                 vLightTransformed,
                                 vWhite,
                                 0);
    cubeBatch.Draw();
    modelViewMatrix.PopMatrix();
    
    modelViewMatrix.Translate(0.8f, 0.0f, 0.0f);
}

bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
    GLbyte *pBits;
    int nWidth, nHeight, nComponents;
    GLenum eFormat;
    
    // Read the texture bits
    pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
    if(pBits == NULL)
        return false;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//紧密包装像素数据
    glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, nWidth, nHeight, 0,
                 eFormat, GL_UNSIGNED_BYTE, pBits);
    
    free(pBits);
    
    if(minFilter == GL_LINEAR_MIPMAP_LINEAR ||
       minFilter == GL_LINEAR_MIPMAP_NEAREST ||
       minFilter == GL_NEAREST_MIPMAP_LINEAR ||
       minFilter == GL_NEAREST_MIPMAP_NEAREST)
        glGenerateMipmap(GL_TEXTURE_2D);
    
    return true;
}

//////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering
// context.
void SetupRC(){
    // Make sure OpenGL entry points are set
    glewInit();
    
    // Initialze Shader Manager
    shaderManager.InitializeStockShaders();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // This makes a sphere
    gltMakeCube(cubeBatch,40);
    
    // Make the solid ground
    GLfloat texSize = 10.0f;
    floorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
    floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    floorBatch.Vertex3f(-20.0f, -0.41f, 20.0f);
    
    floorBatch.MultiTexCoord2f(0, texSize, 0.0f);
    floorBatch.Vertex3f(20.0f, -0.41f, 20.0f);
    
    floorBatch.MultiTexCoord2f(0, texSize, texSize);
    floorBatch.Vertex3f(20.0f, -0.41f, -20.0f);
    
    floorBatch.MultiTexCoord2f(0, 0.0f, texSize);
    floorBatch.Vertex3f(-20.0f, -0.41f, -20.0f);
    floorBatch.End();
    
    glGenTextures(3, uiTextures);
    // Load the Marble
    glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
    LoadTGATexture("marble.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
    LoadTGATexture("marslike.tga", GL_LINEAR_MIPMAP_LINEAR,
                   GL_LINEAR, GL_CLAMP_TO_EDGE);
    
    glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
    LoadTGATexture("moonlike.tga", GL_LINEAR_MIPMAP_LINEAR,
                   GL_LINEAR, GL_CLAMP_TO_EDGE);
    
}

void RenderScene(void){
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    modelViewMatrix.PushMatrix();
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    modelViewMatrix.MultMatrix(mCamera);
  
    // Draw the world upside down
    modelViewMatrix.PushMatrix();
    modelViewMatrix.Scale(1.0f, -1.0f, 1.0f); // Flips the Y Axis
    modelViewMatrix.Translate(0.0f, 0.8f, 0.0f); // Scootch the world down a bit...
    glFrontFace(GL_CW);
    DrawSongAndDance();
    glFrontFace(GL_CCW);
    modelViewMatrix.PopMatrix();
   
    // Draw the solid ground
    glEnable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    static GLfloat vFloorColor[] = { 1.0f, 1.0f, 1.0f, 0.75f};
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE,
                                 transformPipeline.GetModelViewProjectionMatrix(),
                                 vFloorColor,
                                 0);
    floorBatch.Draw();
    glDisable(GL_BLEND);
    
    DrawSongAndDance();
    
    modelViewMatrix.PopMatrix();
    
    
    glutSwapBuffers();
    glutPostRedisplay();
}

void ChangeSize(int nWidth, int nHeight)
{
    glViewport(0, 0, nWidth, nHeight);
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
//
    viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    modelViewMatrix.LoadIdentity();
}

// Respond to arrow keys by moving the camera frame of reference
void SpecialKeys(int key, int x, int y)
{
    float linear = 0.1f;
    float angular = float(m3dDegToRad(5.0f));
    
    if(key == GLUT_KEY_UP)
        cameraFrame.MoveForward(linear);
    
    if(key == GLUT_KEY_DOWN)
        cameraFrame.MoveForward(-linear);
    
    if(key == GLUT_KEY_LEFT)
        cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);
    
    if(key == GLUT_KEY_RIGHT)
        cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);
}

// Do shutdown for the rendering context
void ShutdownRC(void)
{
    glDeleteTextures(3, uiTextures);
}

int main(int argc, char * argv[]){
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    
    glutCreateWindow("OpenGL SphereWorld");
    
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);
    
    SetupRC();
    glutMainLoop();
    ShutdownRC();
    return 0;
}


