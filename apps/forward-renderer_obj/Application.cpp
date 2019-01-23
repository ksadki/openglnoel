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

      const auto sceneDiagonalSize = glm::length(_scene.bboxMax - _scene.bboxMin);
      viewController.setSpeed(sceneDiagonalSize * 0.1f); 

      glm::mat4 ProjMatrix = glm::perspective(70.f, float(fbSize.x) / fbSize.y, 0.01f * sceneDiagonalSize, sceneDiagonalSize);
      glm::mat4 viewMatrix = viewController.getViewMatrix();


      glUniform1i(uKdSampler, 0);

      /*Light----------------------------------------------------------------*/
      glm::vec3 lightDirection = computeDirectionVector(glm::radians(90.0f), glm::radians(45.0f));

      glUniform3fv(uDirectionalLightDir, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::normalize(lightDirection), 0))));
      glUniform3fv(uDirectionalLightIntensity, 1, glm::value_ptr(glm::vec3(1, 1, 1) * 1.0f));
      glUniform3fv(uPointLightPosition, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::vec3(0, 1, 0), 1))));
      glUniform3fv(uPointLightIntensity, 1, glm::value_ptr(glm::vec3(0, 1, 0) * 5.0f));
      glUniform3fv(uKd, 1, glm::value_ptr(glm::vec3(1, 1, 1)));

      /*Sphere----------------------------------------------------------------------------*/
      glm::mat4 MVMatrix = viewMatrix*glm::translate(glm::mat4(1.0f),glm::vec3(0,0,-5));
      glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
      glm::mat4 MVPMatrix = ProjMatrix * MVMatrix;
    
      glUniformMatrix4fv(uMVPMatrix,1,GL_FALSE,glm::value_ptr(MVPMatrix));
      glUniformMatrix4fv(uMVMatrix,1,GL_FALSE,glm::value_ptr(MVMatrix));
      glUniformMatrix4fv(uNormalMatrix,1,GL_FALSE,glm::value_ptr(NormalMatrix));

      glBindVertexArray(_vao);

      auto indexOffset = 0;
      int shapeIdx = 0;
      for (const auto indexCount: _scene.indexCountPerShape){
          glActiveTexture(GL_TEXTURE0);
          if(_scene.materialIDPerShape[shapeIdx]==-1)
            glBindTexture(GL_TEXTURE_2D,whiteTextureId);
          else{
            const auto& material = _scene.materials[_scene.materialIDPerShape[shapeIdx]];
            auto idTex = material.KdTextureId;
            if(idTex!=-1)
              glBindTexture(GL_TEXTURE_2D,objectsTextureId[idTex]);
            else
              glBindTexture(GL_TEXTURE_2D,0);
          }

          glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (const GLvoid*) (indexOffset * sizeof(GLuint)));
          indexOffset += indexCount;
          shapeIdx++;
      }


      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D,0);

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

  this->recupMatricesId(&shader);
  loadObjScene("/home/2ins2/ksadki01/Documents/synthese_avance/openglnoel/apps/forward-renderer_obj/assets/sponza/sponza.obj", _scene);
  shader.use();
  glEnable(GL_DEPTH_TEST);

  this->initVboVao();

  glGenTextures(1, &whiteTextureId);
        glBindTexture(GL_TEXTURE_2D, whiteTextureId);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, 1, 1);
        glm::vec4 whiteTexture(1.f, 1.f, 1.f, 1.f);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, &whiteTexture);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  for(const auto& texture : _scene.textures){
    GLuint texId = 0;
    glGenTextures(1,&texId);
    glBindTexture(GL_TEXTURE_2D, texId);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,texture.width(),texture.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,texture.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    objectsTextureId.push_back(texId);
  }

}

void Application::initVboVao(){
  //VBO
  glGenBuffers(1,&_vbo);
  glBindBuffer(GL_ARRAY_BUFFER,_vbo);
  glBufferStorage(GL_ARRAY_BUFFER,_scene.vertexBuffer.size()*sizeof(glmlv::Vertex3f3f2f),_scene.vertexBuffer.data(),GL_DYNAMIC_STORAGE_BIT);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  //IBO
  glGenBuffers(1,&_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_ibo);
  glBufferStorage(GL_ELEMENT_ARRAY_BUFFER,_scene.indexBuffer.size()*sizeof(uint32_t),_scene.indexBuffer.data(),GL_DYNAMIC_STORAGE_BIT);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
  
  //VAO
  glGenVertexArrays(1,&_vao);
  glBindVertexArray(_vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
  
  glBindVertexBuffer(0,_vbo,0,sizeof(glmlv::Vertex3f3f2f));
  
  glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
  glVertexAttribFormat(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, position));
  glVertexAttribBinding(VERTEX_ATTR_POSITION, 0);
  glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
  glVertexAttribFormat(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, normal));
  glVertexAttribBinding(VERTEX_ATTR_NORMAL, 0);
  glEnableVertexAttribArray(VERTEX_ATTR_TEXTURE);
  glVertexAttribFormat(VERTEX_ATTR_TEXTURE, 2, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, texCoords));
  glVertexAttribBinding(VERTEX_ATTR_TEXTURE, 0);
  
  glBindBuffer(GL_ARRAY_BUFFER,0);
  glBindVertexArray(0);
}

void Application::recupMatricesId(glmlv::GLProgram* shader){
  uMVPMatrix = shader->getUniformLocation("uMVPMatrix");
  uMVMatrix = shader->getUniformLocation("uMVMatrix");
  uNormalMatrix = shader->getUniformLocation("uNormalMatrix");
  
  uKdSampler = shader->getUniformLocation("uKdSampler");
  uDirectionalLightDir = shader->getUniformLocation("uDirectionalLightDir");
  uDirectionalLightIntensity = shader->getUniformLocation("uDirectionalLightIntensity");
  uPointLightPosition = shader->getUniformLocation("uPointLightPosition");
  uPointLightIntensity = shader->getUniformLocation("uPointLightIntensity");
  uKd = shader->getUniformLocation("uKd");
}