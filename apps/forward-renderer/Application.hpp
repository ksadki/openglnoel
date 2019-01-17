#pragma once
#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glmlv/Image2DRGBA.hpp>
#include <iostream>

using namespace std;
const GLint VERTEX_ATTR_POSITION = 0;
const GLint VERTEX_ATTR_NORMAL = 1;
const GLint VERTEX_ATTR_TEXTURE = 2;


class Application
{
public:
  Application(int argc, char** argv);

  int run();
private:
  const size_t m_nWindowWidth = 1280;
  const size_t m_nWindowHeight = 720;
  glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

  const glmlv::fs::path m_AppPath;
  const std::string m_AppName;
  const std::string m_ImGuiIniFilename;
  const glmlv::fs::path m_ShadersRootPath;

  glmlv::ViewController viewController;

  glmlv::SimpleGeometry cube;
  GLuint c_vbo,c_vao,c_ibo;
 
  glmlv::SimpleGeometry sphere;
  glmlv::Image2DRGBA imgTextureSphere;
  GLuint s_vbo,s_vao,s_ibo,textureSphere;

  glmlv::GLProgram shader;
  
  GLint uMVPMatrix;
  GLint uMVMatrix;
  GLint uNormalMatrix;
  GLint uKdSampler;

  GLint uDirectionalLightDir;
  GLint uDirectionalLightIntensity;
  GLint uPointLightPosition;
  GLint uPointLightIntensity;
  GLint uKd;
 
};
