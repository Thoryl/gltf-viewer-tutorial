#include "ViewerApplication.hpp"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#include "utils/cameras.hpp"
#include "utils/gltf.hpp"
#include "utils/images.hpp"

#include <stb_image_write.h>
#include <tiny_gltf.h>


bool ViewerApplication::loadGltfFile(tinygltf::Model & model){

  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, m_gltfFilePath.string());
  //bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

  if (!warn.empty()) {
    std::cerr << "Warn: " << warn << std::endl;
    return false;
  }

  if (!err.empty()) {
    std::cerr << "Err: " <<  err << std::endl;
    return false;
  }

  if (!ret) {
    std::cerr << "Failed to parse glTF" << std::endl;
    return false;
  }
  return true;
}

std::vector<GLuint> ViewerApplication::createBufferObjects( const tinygltf::Model &model) {
    std::vector<GLuint> bufferObjects(model.buffers.size(), 0);
    glGenBuffers(model.buffers.size(), bufferObjects.data());
    for (size_t bufferIdx = 0; bufferIdx < bufferObjects.size(); bufferIdx++)
    {
      glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[bufferIdx]);
      glBufferStorage(GL_ARRAY_BUFFER, model.buffers[bufferIdx].data.size(), model.buffers[bufferIdx].data.data(), 0);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return bufferObjects;
}

std::vector<GLuint> ViewerApplication::createVertexArrayObjects( const tinygltf::Model &model,
  const std::vector<GLuint> &bufferObjects,
  std::vector<VaoRange> &meshIndexToVaoRange) {
    std::vector<GLuint> vertexArrayObjects;

    const GLuint VERTEX_ATTRIB_POSITION_IDX = 0;
    const GLuint VERTEX_ATTRIB_NORMAL_IDX = 1;
    const GLuint VERTEX_ATTRIB_TEXCOORD0_IDX = 2;
    const GLuint VERTEX_ATTRIB_TANGENT_IDX = 3;

    for(int meshIdx = 0; meshIdx < model.meshes.size(); meshIdx++ ) {
        const int vaoOffset = vertexArrayObjects.size();
        const int primitiveSizeRange = model.meshes[meshIdx].primitives.size();
        vertexArrayObjects.resize(vaoOffset + primitiveSizeRange);
        meshIndexToVaoRange.push_back(VaoRange{vaoOffset, primitiveSizeRange});
        glGenVertexArrays(primitiveSizeRange, &vertexArrayObjects[meshIdx]);

        for(int primitiveIdx = 0; primitiveIdx < primitiveSizeRange; primitiveIdx++) {
          glBindVertexArray(vertexArrayObjects[vaoOffset + primitiveIdx]);
          {
            const auto iterator = model.meshes[meshIdx].primitives[primitiveIdx].attributes.find("POSITION");
            if (iterator != end(model.meshes[meshIdx].primitives[primitiveIdx].attributes)) {
              const auto accessorIdx = (*iterator).second;
              const auto &accessor = model.accessors[accessorIdx];
              const auto &bufferView = model.bufferViews[accessor.bufferView];
              const auto bufferIdx = bufferView.buffer;

              const auto bufferObject = bufferObjects[bufferIdx];

              glEnableVertexAttribArray(VERTEX_ATTRIB_POSITION_IDX);
              glBindBuffer(GL_ARRAY_BUFFER, bufferObject);

              const auto byteOffset = accessor.byteOffset + bufferView.byteOffset;
              glVertexAttribPointer(VERTEX_ATTRIB_POSITION_IDX, accessor.type, accessor.componentType, GL_FALSE, bufferView.byteStride, (void *)byteOffset);
            }
          }
          {
            const auto iterator = model.meshes[meshIdx].primitives[primitiveIdx].attributes.find("NORMAL");
            if (iterator != end(model.meshes[meshIdx].primitives[primitiveIdx].attributes)) {
              const auto accessorIdx = (*iterator).second;
              const auto &accessor = model.accessors[accessorIdx];
              const auto &bufferView = model.bufferViews[accessor.bufferView];
              const auto bufferIdx = bufferView.buffer;

              const auto bufferObject = bufferObjects[bufferIdx];

              glEnableVertexAttribArray(VERTEX_ATTRIB_NORMAL_IDX);
              glBindBuffer(GL_ARRAY_BUFFER, bufferObject);

              const auto byteOffset = accessor.byteOffset + bufferView.byteOffset;
              glVertexAttribPointer(VERTEX_ATTRIB_NORMAL_IDX, accessor.type, accessor.componentType, GL_FALSE, bufferView.byteStride, (void *)byteOffset);
            }
          }
          {
            const auto iterator = model.meshes[meshIdx].primitives[primitiveIdx].attributes.find("TEXCOORD_0");
            if (iterator != end(model.meshes[meshIdx].primitives[primitiveIdx].attributes)) {
              const auto accessorIdx = (*iterator).second;
              const auto &accessor = model.accessors[accessorIdx];
              const auto &bufferView = model.bufferViews[accessor.bufferView];
              const auto bufferIdx = bufferView.buffer;

              const auto bufferObject = bufferObjects[bufferIdx];

              glEnableVertexAttribArray(VERTEX_ATTRIB_TEXCOORD0_IDX);
              glBindBuffer(GL_ARRAY_BUFFER, bufferObject);

              const auto byteOffset = accessor.byteOffset + bufferView.byteOffset;
              glVertexAttribPointer(VERTEX_ATTRIB_TEXCOORD0_IDX, accessor.type, accessor.componentType, GL_FALSE, bufferView.byteStride, (void *)byteOffset);
            }
          }
          {
            const auto iterator = model.meshes[meshIdx].primitives[primitiveIdx].attributes.find("TANGENT");
            if (iterator != end(model.meshes[meshIdx].primitives[primitiveIdx].attributes)) {
              const auto accessorIdx = (*iterator).second;
              const auto &accessor = model.accessors[accessorIdx];
              const auto &bufferView = model.bufferViews[accessor.bufferView];
              const auto bufferIdx = bufferView.buffer;

              const auto bufferObject = bufferObjects[bufferIdx];

              glEnableVertexAttribArray(VERTEX_ATTRIB_TANGENT_IDX);
              glBindBuffer(GL_ARRAY_BUFFER, bufferObject);

              const auto byteOffset = accessor.byteOffset + bufferView.byteOffset;
              glVertexAttribPointer(VERTEX_ATTRIB_TANGENT_IDX, accessor.type, accessor.componentType, GL_FALSE, bufferView.byteStride, (void *)byteOffset);
            }
          }
          if(model.meshes[meshIdx].primitives[primitiveIdx].indices >= 0) {
            const auto &accessor = model.accessors[model.meshes[meshIdx].primitives[primitiveIdx].indices];
              const auto &bufferView = model.bufferViews[accessor.bufferView];
              const auto bufferIdx = bufferView.buffer;

              const auto bufferObject = bufferObjects[bufferIdx];
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObject);
          }
        }
    }
    glBindVertexArray(0);
    return vertexArrayObjects;
}

std::vector<GLuint> ViewerApplication::createTextureObjects(const tinygltf::Model &model) const {
  std::vector<GLuint> texObjects(model.textures.size());
  glGenTextures(model.textures.size(), &texObjects[0]);
  tinygltf::Sampler defaultSampler;
  defaultSampler.minFilter = GL_LINEAR;
  defaultSampler.magFilter = GL_LINEAR;
  defaultSampler.wrapS = GL_REPEAT;
  defaultSampler.wrapT = GL_REPEAT;
  defaultSampler.wrapR = GL_REPEAT;
  for(int textIdx = 0; textIdx < model.textures.size(); ++textIdx) {
    glBindTexture(GL_TEXTURE_2D, texObjects[textIdx]);

    const auto &texture = model.textures[textIdx];
    assert(texture.source >= 0);
    const auto &image = model.images[texture.source];

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0,
            GL_RGBA, image.pixel_type, image.image.data());
    const auto &sampler =
      texture.sampler >= 0 ? model.samplers[texture.sampler] : defaultSampler;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
      sampler.minFilter != -1 ? sampler.minFilter : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
      sampler.magFilter != -1 ? sampler.magFilter : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, sampler.wrapR);

    if (sampler.minFilter == GL_NEAREST_MIPMAP_NEAREST ||
        sampler.minFilter == GL_NEAREST_MIPMAP_LINEAR ||
        sampler.minFilter == GL_LINEAR_MIPMAP_NEAREST ||
        sampler.minFilter == GL_LINEAR_MIPMAP_LINEAR) {
      glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
  }
  return texObjects;
}

void keyCallback(
    GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
    glfwSetWindowShouldClose(window, 1);
  }
}

int ViewerApplication::run()
{
  // Loader shaders
  const auto glslProgram =
      compileProgram({m_ShadersRootPath / m_AppName / m_vertexShader,
          m_ShadersRootPath / m_AppName / m_fragmentShader});

  const auto modelViewProjMatrixLocation =
      glGetUniformLocation(glslProgram.glId(), "uModelViewProjMatrix");
  const auto modelViewMatrixLocation =
      glGetUniformLocation(glslProgram.glId(), "uModelViewMatrix");
  const auto modelMatrixLocation =
      glGetUniformLocation(glslProgram.glId(), "uModelMatrix");
  const auto normalMatrixLocation =
      glGetUniformLocation(glslProgram.glId(), "uNormalMatrix");
  const auto lightDirectionLocation =
      glGetUniformLocation(glslProgram.glId(), "uLightDirection");
  const auto lightIntensityLocation =
      glGetUniformLocation(glslProgram.glId(), "uLightIntensity");
  const auto baseColorTextureLocation =
      glGetUniformLocation(glslProgram.glId(), "uBaseColorTexture");
  const auto baseColorFactorLocation =
      glGetUniformLocation(glslProgram.glId(), "uBaseColorFactor");
  const auto metallicFactorLocation =
      glGetUniformLocation(glslProgram.glId(), "uMetallicFactor");
  const auto roughnessFactorLocation =
      glGetUniformLocation(glslProgram.glId(), "uRoughnessFactor");
  const auto metallicRoughnessTextureLocation =
      glGetUniformLocation(glslProgram.glId(), "uMetallicRoughnessTexture");
  const auto emissiveTextureLocation =
      glGetUniformLocation(glslProgram.glId(), "uEmissiveTexture");
  const auto emissiveFactorLocation =
      glGetUniformLocation(glslProgram.glId(), "uEmissiveFactor");
  const auto occlusionTextureLocation =
      glGetUniformLocation(glslProgram.glId(), "uOcclusionTexture");
  const auto occlusionStrengthLocation =
      glGetUniformLocation(glslProgram.glId(), "uOcclusionStrength");
  const auto normalTextureLocation =
      glGetUniformLocation(glslProgram.glId(), "uNormalMapTexture");
  const auto normalScaleLocation =
      glGetUniformLocation(glslProgram.glId(), "uNormalMapScale");
  const auto uHasTangent =
      glGetUniformLocation(glslProgram.glId(), "uHasTangent");
  const auto uHasNormalMap =
      glGetUniformLocation(glslProgram.glId(), "uHasNormalMap");


  tinygltf::Model model;
  if(!loadGltfFile(model)) {
    return EXIT_FAILURE;
  };


  glm::vec3 bboxMin, bboxMax;
  computeSceneBounds(model, bboxMin, bboxMax);
  const glm::vec3 center = (bboxMax + bboxMin) * 0.5f;
  const glm::vec3 diagonalVector = bboxMax - bboxMin;
  const glm::vec3 up(0, 1, 0);
  const glm::vec3 eye = diagonalVector.z > 0 ? center + diagonalVector :
                                              center + 2.f * glm::cross(diagonalVector, up);
  // Build projection matrix
  auto maxDistance = glm::length(diagonalVector);
  maxDistance = maxDistance > 0.f ? maxDistance : 100.f;
  const auto projMatrix =
      glm::perspective(70.f, float(m_nWindowWidth) / m_nWindowHeight,
          0.001f * maxDistance, 1.5f * maxDistance);

  std::unique_ptr<CameraController> cameraController
    = std::make_unique<TrackballCameraController>(m_GLFWHandle.window(), 3.f * maxDistance);
  if (m_hasUserCamera) {
    cameraController->setCamera(m_userCamera);
  } else {
    cameraController->setCamera(
        Camera{eye, center, up});
  }

  std::vector<GLuint> texObjects = createTextureObjects(model);
  GLuint whiteTexture;
  float white[] = {1., 1., 1., 1.};
  glGenTextures(1, &whiteTexture);
  glBindTexture(GL_TEXTURE_2D, whiteTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0,
        GL_RGBA, GL_FLOAT, white);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

  glBindTexture(GL_TEXTURE_2D, 0);

  std::vector<GLuint> bufferObjects = createBufferObjects(model);

  std::vector<VaoRange> meshIndexToVaoRange;
  std::vector<GLuint> vertexArrayObjects = createVertexArrayObjects(model, bufferObjects, meshIndexToVaoRange);
  //std::cout << vertexArrayObjects.size() << std::endl;

  glm::vec3 lightDirection(1.f,1.f,1.f);
  glm::vec3 lightIntensity(1.f,1.f,1.f);

  bool lightFromCamera = false;


  // Setup OpenGL state for rendering
  glEnable(GL_DEPTH_TEST);
  glslProgram.use();

  const auto bindMaterial = [&](const auto materialIndex) {
    if(materialIndex >= 0) {
      const auto &material = model.materials[materialIndex];
      const auto &pbrMetallicRoughness = material.pbrMetallicRoughness;
      if(pbrMetallicRoughness.baseColorTexture.index >= 0) {
        const auto &texture = model.textures[pbrMetallicRoughness.baseColorTexture.index];
        glActiveTexture(GL_TEXTURE0);
        assert(texture.source >= 0);
        glBindTexture(GL_TEXTURE_2D, texObjects[texture.source]);
        glUniform1i(baseColorTextureLocation, 0);
        glUniform4f(baseColorFactorLocation,
          (float)pbrMetallicRoughness.baseColorFactor[0],
          (float)pbrMetallicRoughness.baseColorFactor[1],
          (float)pbrMetallicRoughness.baseColorFactor[2],
          (float)pbrMetallicRoughness.baseColorFactor[3]);

      }
      else {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
        glUniform1i(baseColorTextureLocation, 0);
        glUniform4f(baseColorFactorLocation,
          white[0],
          white[1],
          white[2],
          white[3]);
      }
      if(pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
        const auto &texture = model.textures[pbrMetallicRoughness.metallicRoughnessTexture.index];
        glActiveTexture(GL_TEXTURE1);
        assert(texture.source >= 0);
        glBindTexture(GL_TEXTURE_2D, texObjects[texture.source]);
        glUniform1i(metallicRoughnessTextureLocation, 1);
        glUniform1f(metallicFactorLocation,
          (float)pbrMetallicRoughness.metallicFactor);
        glUniform1f(roughnessFactorLocation,
          (float)pbrMetallicRoughness.roughnessFactor);
      }
      else {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1i(metallicRoughnessTextureLocation, 1);
        glUniform1f(metallicFactorLocation,
          0);
        glUniform1f(roughnessFactorLocation,
          0);
      }
      if(material.emissiveTexture.index >= 0) {
        const auto &texture = model.textures[material.emissiveTexture.index];
        glActiveTexture(GL_TEXTURE2);
        assert(texture.source >= 0);
        glBindTexture(GL_TEXTURE_2D, texObjects[texture.source]);
        glUniform1i(emissiveTextureLocation, 2);
        glUniform3f(emissiveFactorLocation,
          (float)material.emissiveFactor[0],
          (float)material.emissiveFactor[1],
          (float)material.emissiveFactor[2]);
      }
      else {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1i(emissiveTextureLocation, 2);
        glUniform3f(emissiveFactorLocation,
          0,
          0,
          0);
      }
      if(material.occlusionTexture.index >= 0) {
        const auto &texture = model.textures[material.occlusionTexture.index];
        glActiveTexture(GL_TEXTURE3);
        assert(texture.source >= 0);
        glBindTexture(GL_TEXTURE_2D, texObjects[texture.source]);
        glUniform1i(occlusionTextureLocation, 3);
        glUniform1f(occlusionStrengthLocation,
          (float)material.occlusionTexture.strength);
      }
      else {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1i(occlusionTextureLocation, 3);
        glUniform1f(occlusionStrengthLocation,
          0);
      }
      if(material.normalTexture.index >= 0) {
        const auto &texture = model.textures[material.normalTexture.index];
        glActiveTexture(GL_TEXTURE4);
        assert(texture.source >= 0);
        glBindTexture(GL_TEXTURE_2D, texObjects[texture.source]);
        glUniform1i(normalTextureLocation, 4);
        glUniform1f(normalScaleLocation,
          (float)material.normalTexture.scale);
        glUniform1i(uHasNormalMap, 1);

      }
      else {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1i(normalTextureLocation, 4);
        glUniform1f(normalScaleLocation,
          1);
        glUniform1i(uHasNormalMap, 0);
      }
    }
  };

  // Lambda function to draw the scene
  const auto drawScene = [&](const Camera &camera) {
    glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto viewMatrix = camera.getViewMatrix();

    // The recursive function that should draw a node
    // We use a std::function because a simple lambda cannot be recursive
    const std::function<void(int, const glm::mat4 &)> drawNode =
        [&](int nodeIdx, const glm::mat4 &parentMatrix) {
          tinygltf::Node node = model.nodes[nodeIdx];
          glm::mat4 modelMatrix = getLocalToWorldMatrix(node, parentMatrix);

          if(lightIntensityLocation >= 0) {
            glUniform3f(lightIntensityLocation, lightIntensity[0], lightIntensity[1], lightIntensity[2]);
          }
          if(lightDirectionLocation >= 0) {
            if(lightFromCamera) {
              glUniform3f(lightDirectionLocation, 0, 0, 1);
            }
            else {
              const glm::vec3 normalizedLightDirectionViewSpace =
                  glm::normalize(glm::vec3(viewMatrix * glm::vec4(lightDirection, 0.)));
              glUniform3f(lightDirectionLocation,
                  normalizedLightDirectionViewSpace[0],
                  normalizedLightDirectionViewSpace[1],
                  normalizedLightDirectionViewSpace[2]);
            }
          }

          if(node.mesh >= 0){
            const glm::mat4 modelViewMatrix = viewMatrix * modelMatrix ;
            const glm::mat4 modelViewProjectionMatrix = projMatrix * modelViewMatrix;
            const glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));

            glUniformMatrix4fv(modelViewMatrixLocation, 1, GL_FALSE, value_ptr(modelViewMatrix));
            glUniformMatrix4fv(modelViewProjMatrixLocation, 1, GL_FALSE, value_ptr(modelViewProjectionMatrix));
            glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, value_ptr(modelMatrix));
            glUniformMatrix4fv(normalMatrixLocation, 1, GL_FALSE, value_ptr(normalMatrix));

            const tinygltf::Mesh &mesh = model.meshes[node.mesh];
            const auto &vaoRangeMesh = meshIndexToVaoRange[node.mesh];
            for(size_t primIdx = 0; primIdx < mesh.primitives.size(); ++primIdx) {
              const auto vaoPrimitive = vertexArrayObjects[vaoRangeMesh.begin + primIdx];
              const auto &currentPrimitive = mesh.primitives[primIdx];
              bindMaterial(currentPrimitive.material);
              glBindVertexArray(vaoPrimitive);
              if(currentPrimitive.indices >= 0) {
                const auto &accessor = model.accessors[currentPrimitive.indices];
                const auto &bufferView = model.bufferViews[accessor.bufferView];
                const auto byteOffset = accessor.byteOffset + bufferView.byteOffset;
                glDrawElements(currentPrimitive.mode, GLsizei(accessor.count),
                    accessor.componentType, (const GLvoid *)byteOffset);
              } else {
                const auto accessorIdx = (*begin(currentPrimitive.attributes)).second;
                const auto &accessor = model.accessors[accessorIdx];
                glDrawArrays(currentPrimitive.mode, 0, GLsizei(accessor.count));
              }
            }
          }
          for(const auto childNode : node.children) {
            drawNode(childNode, modelMatrix);
          }
        };

    // Draw the scene referenced by gltf file
    if (model.defaultScene >= 0) {
      for(int nodeIdx : model.scenes[model.defaultScene].nodes) {
        drawNode(model.scenes[model.defaultScene].nodes[nodeIdx], glm::mat4(1));
      }
    }
  };

  if(!m_OutputPath.empty()) {
    size_t numCoponents = 3;
    std::vector<unsigned char> pixels(numCoponents * m_nWindowWidth * m_nWindowHeight);
    renderToImage(m_nWindowWidth, m_nWindowHeight, numCoponents, pixels.data(), [&]() {
      drawScene(cameraController->getCamera());
    });
    flipImageYAxis(m_nWindowWidth, m_nWindowHeight, numCoponents, pixels.data());
    const auto strPath = m_OutputPath.string();
    stbi_write_png(
    strPath.c_str(), m_nWindowWidth, m_nWindowHeight, numCoponents, pixels.data(), 0);
    return 0;
  }

  // Loop until the user closes the window
  for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose();
       ++iterationCount) {
    const auto seconds = glfwGetTime();

    const auto camera = cameraController->getCamera();
    drawScene(camera);

    // GUI code:
    imguiNewFrame();

    {
      ImGui::Begin("GUI");
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
          1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("eye: %.3f %.3f %.3f", camera.eye().x, camera.eye().y,
            camera.eye().z);
        ImGui::Text("center: %.3f %.3f %.3f", camera.center().x,
            camera.center().y, camera.center().z);
        ImGui::Text(
            "up: %.3f %.3f %.3f", camera.up().x, camera.up().y, camera.up().z);

        ImGui::Text("front: %.3f %.3f %.3f", camera.front().x, camera.front().y,
            camera.front().z);
        ImGui::Text("left: %.3f %.3f %.3f", camera.left().x, camera.left().y,
            camera.left().z);

        if (ImGui::Button("CLI camera args to clipboard")) {
          std::stringstream ss;
          ss << "--lookat " << camera.eye().x << "," << camera.eye().y << ","
             << camera.eye().z << "," << camera.center().x << ","
             << camera.center().y << "," << camera.center().z << ","
             << camera.up().x << "," << camera.up().y << "," << camera.up().z;
          const auto str = ss.str();
          glfwSetClipboardString(m_GLFWHandle.window(), str.c_str());
        }
        static int cameraControllerType = 0;
        const auto cameraControllerTypeChanged =
            ImGui::RadioButton("Trackball", &cameraControllerType, 0) ||
            ImGui::RadioButton("First Person", &cameraControllerType, 1);
        if (cameraControllerTypeChanged) {
          const auto currentCamera = cameraController->getCamera();
          if (cameraControllerType == 0) {
            cameraController = std::make_unique<TrackballCameraController>(
                m_GLFWHandle.window(), 3.f * maxDistance);
          } else {
            cameraController = std::make_unique<FirstPersonCameraController>(
                m_GLFWHandle.window(), 30.f * maxDistance);
          }
          cameraController->setCamera(currentCamera);
        }
      }
      if(ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
        static float angleTheta = 0.;
        static float anglePhi = 0.;
        static float numberPI = 3.14;
        if(ImGui::SliderFloat("Theta Angle", &angleTheta, 0, numberPI) ||
          ImGui::SliderFloat("Phi Angle", &anglePhi, 0, 2. * numberPI)) {
          const auto sinTheta = glm::sin(angleTheta);
          const auto cosTheta = glm::cos(angleTheta);
          const auto sinPhi = glm::sin(anglePhi);
          const auto cosPhi = glm::cos(anglePhi);
          lightDirection = glm::vec3(sinTheta * cosPhi, cosTheta, sinTheta * sinPhi);
        }
        static glm::vec3 color(1.f, 1.f, 1.f);
        static float intensityFactor = 1.f;
        if(ImGui::ColorEdit3("Color",(float *)&color) ||
          ImGui::InputFloat("Intensity Factor", &intensityFactor)) {
            lightIntensity = color * intensityFactor;
        }
        ImGui::Checkbox("Light from camera", &lightFromCamera);
      }
      ImGui::End();
    }

    imguiRenderFrame();

    glfwPollEvents(); // Poll for and process events

    auto ellapsedTime = glfwGetTime() - seconds;
    auto guiHasFocus =
        ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
    if (!guiHasFocus) {
      cameraController->update(float(ellapsedTime));
    }

    m_GLFWHandle.swapBuffers(); // Swap front and back buffers
  }

  // TODO clean up allocated GL data

  return 0;
}

ViewerApplication::ViewerApplication(const fs::path &appPath, uint32_t width,
    uint32_t height, const fs::path &gltfFile,
    const std::vector<float> &lookatArgs, const std::string &vertexShader,
    const std::string &fragmentShader, const fs::path &output) :
    m_nWindowWidth(width),
    m_nWindowHeight(height),
    m_AppPath{appPath},
    m_AppName{m_AppPath.stem().string()},
    m_ImGuiIniFilename{m_AppName + ".imgui.ini"},
    m_ShadersRootPath{m_AppPath.parent_path() / "shaders"},
    m_gltfFilePath{gltfFile},
    m_OutputPath{output}
{
  if (!lookatArgs.empty()) {
    m_hasUserCamera = true;
    m_userCamera =
        Camera{glm::vec3(lookatArgs[0], lookatArgs[1], lookatArgs[2]),
            glm::vec3(lookatArgs[3], lookatArgs[4], lookatArgs[5]),
            glm::vec3(lookatArgs[6], lookatArgs[7], lookatArgs[8])};
  }

  if (!vertexShader.empty()) {
    m_vertexShader = vertexShader;
  }

  if (!fragmentShader.empty()) {
    m_fragmentShader = fragmentShader;
  }

  ImGui::GetIO().IniFilename =
      m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows
                                  // positions in this file

  glfwSetKeyCallback(m_GLFWHandle.window(), keyCallback);

  printGLVersion();
}