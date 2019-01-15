#include "Application.hpp"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int Application::run()
{
  // Put here code to run before rendering loop
  shader.use();
  // Loop until the user closes the window
  for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
      const auto seconds = glfwGetTime();

      // Put here rendering code
      const auto fbSize = m_GLFWHandle.framebufferSize();
      glViewport(0, 0, fbSize.x, fbSize.y);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
      glm::mat4 viewMatrix = glm::mat4(1.0f);
      glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f),1000.f/1000.f,0.1f,100.f);
      
      glm::mat4 MVMatrix = viewMatrix*glm::translate(glm::mat4(1.0f),glm::vec3(0,0,-5));

      glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
      glm::mat4 MVPMatrix = ProjMatrix * MVMatrix;
    
      glUniformMatrix4fv(uMVPMatrix,1,GL_FALSE,glm::value_ptr(MVPMatrix));
      glUniformMatrix4fv(uMVMatrix,1,GL_FALSE,glm::value_ptr(MVMatrix));
      glUniformMatrix4fv(uNormalMatrix,1,GL_FALSE,glm::value_ptr(NormalMatrix));
  
      /*glBindVertexArray(c_vao);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c_ibo);
      glDrawElements(GL_TRIANGLES, cube.indexBuffer.size() , GL_UNSIGNED_INT,0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindVertexArray(0);*/

      glBindVertexArray(s_vao);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_ibo);
      glDrawElements(GL_TRIANGLES, sphere.indexBuffer.size() , GL_UNSIGNED_INT,0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      
      

      // GUI code:
      glmlv::imguiNewFrame();

      {
	ImGui::Begin("GUI");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
      }

      glmlv::imguiRenderFrame();

      glfwPollEvents(); // Poll for and process events

      auto ellapsedTime = glfwGetTime() - seconds;
      auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
      if (!guiHasFocus) {
	// Put here code to handle user interactions
      }

      m_GLFWHandle.swapBuffers(); // Swap front and back buffers
    }

  return 0;
}

Application::Application(int argc, char** argv):
  m_AppPath { glmlv::fs::path{ argv[0] } },
  m_AppName { m_AppPath.stem().string() },
  m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
  m_ShadersRootPath { m_AppPath.parent_path() / "shaders" }

{
  shader = glmlv::compileProgram({m_ShadersRootPath / m_AppName / "forward.vs.glsl" ,
	m_ShadersRootPath / m_AppName / "forward.fs.glsl" });
  ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file
  
  uMVPMatrix = shader.getUniformLocation("uMVPMatrix");
  uMVMatrix = shader.getUniformLocation("uMVMatrix");
  uNormalMatrix = shader.getUniformLocation("uNormalMatrix");
  uTexture = shader.getUniformLocation("uTexture");
                    
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
