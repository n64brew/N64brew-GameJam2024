/*
===============================================================================
AF_RENDERER_H

Definition for functions for rendering 
===============================================================================
*/
#ifndef AF_RENDERER_H
#define AF_RENDERER_H
#include "AF_Window.h"
#include "ECS/Components/AF_CCamera.h"
#include "AF_MeshData.h"
#include "ECS/Entities/AF_ECS.h"
#include "ECS/Components/AF_CTransform3D.h"
#include "ECS/Components/AF_CSprite.h"
#include "AF_Debug.h"
#include "AF_Time.h"

#ifdef __cplusplus
extern "C" {
#endif

void CheckGLError(const char * _message);

// Init
void AF_Renderer_Init(AF_ECS* _ecs, Vec2 _screenSize);
void AF_Renderer_LateStart(AF_ECS* _ecs);
void AF_Renderer_Update(AF_ECS* _ecs, AF_Time* _time);
void AF_Renderer_Finish(void);
void AF_Renderer_Shutdown(AF_ECS* _ecs);
void AF_Renderer_PlayAnimation(AF_CSkeletalAnimation* _animation);
//void AF_Renderer_Debug(void);
uint32_t AF_LIB_InitRenderer(AF_Window* _window);
uint32_t AF_LoadTexture(const char* _texturePath);


void AF_LIB_InitMeshBuffers(AF_Entity* _entities, uint32_t _entityCount);

//void Init(GLFWwindow* _window, std::vector<Entity*>& _entities);
//void InitRenderingData(std::vector<Entity*>& _entities);
//static AF_MeshBuffers InitBuffers(const AF_MeshBuffers& _bufferObject);
// vulkan things
//void CreateSurface(GLFWwindow* _window);
//void RecreateSwapChain(GLFWwindow* _window);
//VkDevice& GetDevice();
//VkPhysicalDevice& GetPhysicalDevice();
//VkInstance& GetInstance();
//VkQueue& GetQueue();
//VkRenderPass& GetRenderPass();
//VkSampleCountFlagBits& GetMSAASamples();
////void InitWindow(AF_AppData& _appData);
//void InitVulkan(GLFWwindow* _window, std::vector<Entity*>& _entities);
//void CreateInstance();
        
//void CreateLogicalDevice();
//void CreateSwapChain(GLFWwindow* _window);
//void CreateSurface();
//void CreateImageViews();
// EditorImageViews
//void CreateOffscreenImageResources();
//void CreateRenderPass();
//void CreateFrameBuffers();
//void CreateCommandPool();
//void CreateDepthResources();


//void CreateTextureImage(const char* _texturePath, std::vector<VkImage>& _textureImages, std::vector<VkDeviceMemory>& _textureImagesMemory, const uint32_t _levels, VkImageCreateFlagBits _flags, VkImageLayout _imageLayout);
//void CreateTextureImage(const char* _texturePath, VkImage& _textureImage, VkDeviceMemory& _textureImageMemory);
//void CreateTextureImageView(VkImage& _textureImage, VkImageView& _textureImageView, uint32_t _mipLevels);
//void CreateTextureImageView(std::vector<VkImage>& _textureImages, std::vector<VkImageView>& _textureImageViews, uint32_t _mipLevels, uint32_t _layers, VkImageViewType _imageViewType);
//void CreateTextureSampler(VkDevice& _device, VkPhysicalDevice& _physicalDevice, VkSampler& _textureSampler);
//void CreateTextureSampler(VkDevice& _device, VkPhysicalDevice& _physicalDevice, std::vector<VkImage>& _textureImages, std::vector<VkSampler>& _textureSamplers);


//void CreateVertexBuffer(VkDevice& _device, std::vector<AF_Vertex>& _vertices);
//void CreateIndexBuffer(VkDevice& _device, std::vector<uint32_t>& indices);
//void CreateUniformBuffers(VkDevice& _device, std::vector<VkBuffer>& _uniformBuffers, std::vector<VkDeviceMemory>& _uniformBuffersMemory, std::vector<void*>& _uniformBuffersMapped, const VkDeviceSize _uniformBufferSize);


//void CreateDescriptorPool();
//void CreateTextureDescriptorSets(VkDevice& _device, std::vector<VkImageView>& _modelTextureImagesView, std::vector<VkSampler>& _modelTextureSamplers );

//void CreateDescriptorSets();
//void CreateDescriptorSetLayout();
//void CreateGraphicsPipeline(std::vector<Entity*>& _entities);
//void CreateCommandBuffers();
//void CreateSyncObjects();
//void CreateColorResources();
//void CreateIMGUIRenderPass();
//void CreateOffscreenFramebuffer();

// Create Material
//AF_Material& CreateMaterial(const AF_Vec2 _screenDimensions, const std::string& _vertShaderPath, const std::string& _fragShaderPath, const std::string& _diffuseTexturePath) override;
//AF_Mesh& CreateMesh(const AF_Vec2& _cameraSize, const std::string& _vertShaderPath, const std::string& _fragShaderPath, const std::string& _texturePath)  override;



// Draw
// TODO: don't like passing in the camera or debug mesh
void AF_LIB_DisplayRenderer(AF_Window* _window, AF_Entity* _cameraEntity, AF_ECS* _ecs, uint32_t shaderID);
 
//void DrawFrame(GLFWwindow* _window, Entity& _cameraEntity, std::vector<Entity*>& _entities);
//static void RenderMesh(const AF_Mesh& _mesh, const AF_Camera& _camera);


// Destroy
void AF_LIB_DestroyRenderer(AF_ECS* _ecs);
// Cleanup
//static void CleanUpMesh(const unsigned int _shaderID);
//void CloseWindow();
//void CleanUp();
//void CleanupSwapChain();


// Error checking
// Util
//static void CheckGLError(std::string _message);
//bool CheckValidationLayerSupport();
//void SetupDebugMessager();
//void PickPhysicalDevice();

// Load Models
//void LoadModel(CModel& _model, std::vector<AF_Vertex>& _vertices, std::vector<uint32_t>& _indices);


// Getters and setters
//VkSampler& GetTextureSampler();
//std::vector<VkImageView>& GetImageViews();
//VkImageView& GetCurrentImageView();
//VkImageView& GetCurrentEditorImageView();
//void SetEditorMode(const bool _state);
//void SetViewportEditor(const bool _state);
//uint32_t GetDrawCalls() const;
//VkExtent2D& GetSwapChainExtent() const;
//bool GetFramebufferResized() const;
//void SetFramebufferResized(const bool _state);
//static glm::vec3 CalculateFront(const CTransform3D& _transform);
//std::vector<AF_Mesh>& getMeshes() const override;



// Textures

uint32_t AF_Renderer_LoadTexture(char const * path);
void AF_Renderer_SetTexture(const uint32_t _shaderID, const char* _shaderVarName, uint32_t _textureID);
//static unsigned int LoadTexture(char const * path);
//static void SetDiffuseTexture(const unsigned int _shaderID);
//static void SetSpecularTexture(const unsigned int _shaderID);
//static void SetEmissionTexture(const unsigned int _shaderID);
//static void SetEmissionMaskTexture(const unsigned int _shaderID);



#ifdef __cplusplus
}
#endif

#endif // AF_RENDERER_H

