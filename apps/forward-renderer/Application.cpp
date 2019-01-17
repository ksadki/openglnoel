#include "Application.hpp"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static glm::vec3 computeDirectionVector(float phiRadians, float thetaRadians)
    {
        const auto cosPhi = glm::cos(phiRadians);
        const auto sinPhi = glm::sin(phiRadians);
        const auto sinTheta = glm::sin(thetaRadians);
        return glm::vec3(sinPhi * sinTheta, glm::cos(thetaRadians), cosPhi * sinTheta);
}

int Application::run()
{
  // Put here code to run before rendering loop
	glClearColor(1,1,1,1);
  // Loop until the user closes the window
  for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
      const auto seconds = glfwGetTime();

      // Put here rendering code
      const auto fbSize = m_GLFWHandle.framebufferSize();
      glViewport(0, 0, fbSize.x, fbSize.y);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  	  
      glUniform1i(uKdSampler, 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D,textureSphere);

      glm::mat4 viewMatrix = viewController.getViewMatrix();
      glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f),(float)fbSize.x/fbSize.y,0.1f,100.f);

      /*Light----------------------------------------------------------------*/
      glm::vec3 lightDirection = computeDirectionVector(glm::radians(90.0f), glm::radians(45.0f));

      glUniform3fv(uDirectionalLightDir, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::normalize(lightDirection), 0))));
      glUniform3fv(uDirectionalLightIntensity, 1, glm::value_ptr(glm::vec3(1, 1, 1) * 1.0f));
      glUniform3fv(uPointLightPosition, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::vec3(0, 1, 0), 1))));
      glUniform3fv(uPointLightIntensity, 1, glm::value_ptr(glm::vec3(0, 1, 0) * 5.0f));
      glUniform3fv(uKd, 1, glm::value_ptr(glm::vec3(1, 0, 0)));

      /*Sphere----------------------------------------------------------------------------*/
      glm::mat4 MVMatrix = viewMatrix*glm::translate(glm::mat4(1.0f),glm::vec3(0,0,-5));
      glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
      glm::mat4 MVPMatrix = ProjMatrix * MVMatrix;
    
      glUniformMatrix4fv(uMVPMatrix,1,GL_FALSE,glm::value_ptr(MVPMatrix));
      glUniformMatrix4fv(uMVMatrix,1,GL_FALSE,glm::value_ptr(MVMatrix));
      glUniformMatrix4fv(uNormalMatrix,1,GL_FALSE,glm::value_ptr(NormalMatrix));

      glBindVertexArray(s_vao);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_ibo);
      glDrawElements(GL_TRIANGLES, sphere.indexBuffer.size() , GL_UNSIGNED_INT,0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D,0);

      /*Cube ---------------------------------------------------------------------------*/
	  /*MVMatrix = viewMatrix*glm::translate(glm::mat4(1.0f),glm::vec3(0,0,-5));
      MVMatrix = glm::scale(MVMatrix,glm::vec3(1.5,1.5,1.5));
      NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
      MVPMatrix = ProjMatrix * MVMatrix;

      glUniformMatrix4fv(uMVPMatrix,1,GL_FALSE,glm::value_ptr(MVPMatrix));
      glUniformMatrix4fv(uMVMatrix,1,GL_FALSE,glm::value_ptr(MVMatrix));
      glUniformMatrix4fv(uNormalMatrix,1,GL_FALSE,glm::value_ptr(NormalMatrix));
      
      glBindVertexArray(c_vao);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c_ibo);
      glDrawElements(GL_TRIANGLES, cube.indexBuffer.size() , GL_UNSIGNED_INT,0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindVertexArray(0);*/

      // GUI code:
      glmlv::imguiNewFrame();

      {
	ImGui::Begin("GUI");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
      }

      glmlv::imguiRenderFrame();

      glfwPollEvents(); // Poll for and process events

      m_GLFWHandle.swapBuffers(); // Swap front and back buffers

      auto ellapsedTime = glfwGetTime() - seconds;
      auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
      if (!guiHasFocus) {
        viewController.update(float(ellapsedTime));
      }

    }

  return 0;
}

Application::Application(int argc, char** argv):
  m_AppPath { glmlv::fs::path{ argv[0] } },
  m_AppName { m_AppPath.stem().string() },
  m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
  m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
  viewController{ m_GLFWHandle.window() }

{
  shader = glmlv::compileProgram({m_ShadersRootPath / m_AppName / "forward.vs.glsl" ,
	m_ShadersRootPath / m_AppName / "forward.fs.glsl" });
  ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file
  
  /*Chargement de la texture pour la sphere*/
  glmlv::fs::path path = "/home/2ins2/ksadki01/Documents/synthese_avance/openglnoel/apps/forward-renderer/assets/textures/JupiterMap.jpg";
  imgTextureSphere = glmlv::readImage(path);
  glGenTextures(1,&textureSphere);
    glBindTexture(GL_TEXTURE_2D, textureSphere);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,imgTextureSphere.width(),imgTextureSphere.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,imgTextureSphere.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

  /*Recuperation des id des matrices*/
  uMVPMatrix = shader.getUniformLocation("uMVPMatrix");
  uMVMatrix = shader.getUniformLocation("uMVMatrix");
  uNormalMatrix = shader.getUniformLocation("uNormalMatrix");
  uKdSampler = shader.getUniformLocation("uKdSampler");
  uDirectionalLightDir = shader.getUniformLocation("uDirectionalLightDir");
  uDirectionalLightIntensity = shader.getUniformLocation("uDirectionalLightIntensity");
  uPointLightPosition = shader.getUniformLocation("uPointLightPosition");
  uPointLightIntensity = shader.getUniformLocation("uPointLightIntensity");
  uKd = shader.getUniformLocation("uKd");
                    
  shader.use();
  
  // Put here initialization code
  glEnable(GL_DEPTH_TEST);
  cube = glmlv::makeCube();
  sphere = glmlv::makeSphere(10);

  cout << "Content created" << endl;
  
  //Init cube
  //VBO
  glGenBuffers(1,&c_vbo);
  glBindBuffer(GL_ARRAY_BUFFER,c_vbo);
  glBufferStorage(GL_ARRAY_BUFFER,cube.vertexBuffer.size()*sizeof(glmlv::Vertex3f3f2f),cube.vertexBuffer.data(),GL_DYNAMIC_STORAGE_BIT);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  //IBO
  glGenBuffers(1,&c_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,c_ibo);
  glBufferStorage(GL_ELEMENT_ARRAY_BUFFER,cube.indexBuffer.size()*sizeof(uint32_t),cube.indexBuffer.data(),GL_DYNAMIC_STORAGE_BIT);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
  
  //VAO
  glGenVertexArrays(1,&c_vao);
  glBindVertexArray(c_vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c_ibo);
  
  glBindVertexBuffer(0,c_vbo,0,sizeof(glmlv::Vertex3f3f2f));
  
  glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
  glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, position));
  glVertexAttribBinding(0, 0);
  glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
  glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, normal));
  glVertexAttribBinding(1, 0);
  glEnableVertexAttribArray(VERTEX_ATTR_TEXTURE);
  glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, texCoords));
  glVertexAttribBinding(2, 0);
  
  glBindBuffer(GL_ARRAY_BUFFER,0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glEnable(GL_DEPTH_TEST);
  glBindVertexArray(0);
  
  //Init sphere
  //VBO
  glGenBuffers(1,&s_vbo);
  glBindBuffer(GL_ARRAY_BUFFER,s_vbo);
  glBufferStorage(GL_ARRAY_BUFFER,sphere.vertexBuffer.size()*sizeof(glmlv::Vertex3f3f2f),sphere.vertexBuffer.data(),GL_DYNAMIC_STORAGE_BIT);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  //IBO
  glGenBuffers(1,&s_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,s_ibo);
  glBufferStorage(GL_ELEMENT_ARRAY_BUFFER,sphere.indexBuffer.size()*sizeof(uint32_t),sphere.indexBuffer.data(),GL_DYNAMIC_STORAGE_BIT);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
  
  //VAO
  glGenVertexArrays(1,&s_vao);
  glBindVertexArray(s_vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_ibo);
  
  glBindVertexBuffer(0,s_vbo,0,sizeof(glmlv::Vertex3f3f2f));
  
  glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
  glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, position));
  glVertexAttribBinding(0, 0);
  glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
  glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, normal));
  glVertexAttribBinding(1, 0);
  glEnableVertexAttribArray(VERTEX_ATTR_TEXTURE);
  glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, texCoords));
  glVertexAttribBinding(2, 0);
  
  glBindBuffer(GL_ARRAY_BUFFER,0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glEnable(GL_DEPTH_TEST);
  glBindVertexArray(0);
  
}
