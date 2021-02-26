#include "stdafx.h"
#include "RenderManager.h"
#include "DirectXFramework.h"
#include "Scene.h"
#include "LineInstance.h"
#include "ModelFactory.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "CameraComponent.h"
#include "ModelComponent.h"
#include "InstancedModelComponent.h"
#include "MainSingleton.h"
#include "PopupTextService.h"
#include "DialogueSystem.h"

#include "Engine.h"
#include "Scene.h"

#include <algorithm>

#define USING_DEFERRED

unsigned int CRenderManager::myNumberOfDrawCallsThisFrame = 0;

CRenderManager::CRenderManager()
	: myDoFullRender(true)
	, myClearColor(0.5f, 0.5f, 0.5f, 1.0f)
{
}

CRenderManager::~CRenderManager()
{
}

bool CRenderManager::Init(CDirectXFramework* aFramework, CWindowHandler* aWindowHandler)
{
	ENGINE_ERROR_BOOL_MESSAGE(myForwardRenderer.Init(aFramework), "Failed to Init Forward Renderer.");
	ENGINE_ERROR_BOOL_MESSAGE(myDeferredRenderer.Init(aFramework), "Failed to Init Deferred Renderer.");
	ENGINE_ERROR_BOOL_MESSAGE(myFullscreenRenderer.Init(aFramework), "Failed to Init Fullscreen Renderer.");
	ENGINE_ERROR_BOOL_MESSAGE(myFullscreenTextureFactory.Init(aFramework), "Failed to Init Fullscreen Texture Factory.");
	ENGINE_ERROR_BOOL_MESSAGE(myParticleRenderer.Init(aFramework), "Failed to Init Particle Renderer.");
	ENGINE_ERROR_BOOL_MESSAGE(myRenderStateManager.Init(aFramework), "Failed to Init Render State Manager.");
	ENGINE_ERROR_BOOL_MESSAGE(myVFXRenderer.Init(aFramework), "Failed to Init VFX Renderer.");
	ENGINE_ERROR_BOOL_MESSAGE(mySpriteRenderer.Init(aFramework), "Failed to Init Sprite Renderer.");
	ENGINE_ERROR_BOOL_MESSAGE(myTextRenderer.Init(aFramework), "Failed to Init Text Renderer.");
	ENGINE_ERROR_BOOL_MESSAGE(myShadowRenderer.Init(aFramework), "Failed to Init Shadow Renderer.");
	ENGINE_ERROR_BOOL_MESSAGE(myDecalRenderer.Init(aFramework), "Failed to Init Decal Renderer.");

	ID3D11Texture2D* backbufferTexture = aFramework->GetBackbufferTexture();
	ENGINE_ERROR_BOOL_MESSAGE(backbufferTexture, "Backbuffer Texture is null.");

	myBackbuffer			 = myFullscreenTextureFactory.CreateTexture(backbufferTexture);
	myIntermediateDepth		 = myFullscreenTextureFactory.CreateDepth(aWindowHandler->GetResolution(), DXGI_FORMAT_R24G8_TYPELESS);
	myEnvironmentShadowDepth = myFullscreenTextureFactory.CreateDepth(aWindowHandler->GetResolution(), DXGI_FORMAT_R32_TYPELESS);
	myDepthCopy				 = myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution(), DXGI_FORMAT_R32_FLOAT);

	myIntermediateTexture	= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution(), DXGI_FORMAT_R8G8B8A8_UNORM);
	myLuminanceTexture		= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution(), DXGI_FORMAT_R8G8B8A8_UNORM);
	myHalfSizeTexture		= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution() / 2.0f, DXGI_FORMAT_R8G8B8A8_UNORM);
	myQuaterSizeTexture		= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution() / 4.0f, DXGI_FORMAT_R8G8B8A8_UNORM);
	myBlurTexture1			= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution(), DXGI_FORMAT_R8G8B8A8_UNORM);
	myBlurTexture2			= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution(), DXGI_FORMAT_R8G8B8A8_UNORM);
	myVignetteTexture		= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution(), DXGI_FORMAT_R8G8B8A8_UNORM);
	myDeferredTexture		= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution(), DXGI_FORMAT_R32G32B32A32_FLOAT);
	myGBuffer				= myFullscreenTextureFactory.CreateGBuffer(aWindowHandler->GetResolution());
	myGBufferCopy			= myFullscreenTextureFactory.CreateGBuffer(aWindowHandler->GetResolution());
	
	return true;
}

bool CRenderManager::ReInit(CDirectXFramework* aFramework, CWindowHandler* aWindowHandler)
{
	ENGINE_ERROR_BOOL_MESSAGE(myRenderStateManager.Init(aFramework), "Failed to Init Render State Manager.");

	ID3D11Texture2D* backbufferTexture = aFramework->GetBackbufferTexture();
	ENGINE_ERROR_BOOL_MESSAGE(backbufferTexture, "Backbuffer Texture is null.");

	myBackbuffer			 = myFullscreenTextureFactory.CreateTexture(backbufferTexture);
	myIntermediateDepth		 = myFullscreenTextureFactory.CreateDepth(aWindowHandler->GetResolution(), DXGI_FORMAT_R24G8_TYPELESS);
	myEnvironmentShadowDepth = myFullscreenTextureFactory.CreateDepth(aWindowHandler->GetResolution(), DXGI_FORMAT_R32_TYPELESS);
	myDepthCopy				 = myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution(), DXGI_FORMAT_R32_FLOAT);
	
	myIntermediateTexture	= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution(), DXGI_FORMAT_R8G8B8A8_UNORM);
	myLuminanceTexture		= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution(), DXGI_FORMAT_R8G8B8A8_UNORM);
	myHalfSizeTexture		= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution() / 2.0f, DXGI_FORMAT_R8G8B8A8_UNORM);
	myQuaterSizeTexture		= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution() / 4.0f, DXGI_FORMAT_R8G8B8A8_UNORM);
	myBlurTexture1			= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution(), DXGI_FORMAT_R8G8B8A8_UNORM);
	myBlurTexture2			= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution(), DXGI_FORMAT_R8G8B8A8_UNORM);
	myVignetteTexture		= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution(), DXGI_FORMAT_R8G8B8A8_UNORM);
	myDeferredTexture		= myFullscreenTextureFactory.CreateTexture(aWindowHandler->GetResolution(), DXGI_FORMAT_R32G32B32A32_FLOAT);
	myGBuffer				= myFullscreenTextureFactory.CreateGBuffer(aWindowHandler->GetResolution());
	myGBufferCopy = myFullscreenTextureFactory.CreateGBuffer(aWindowHandler->GetResolution());

	return true;
}

void CRenderManager::Render(CScene& aScene)
{
	CRenderManager::myNumberOfDrawCallsThisFrame = 0;

	if (Input::GetInstance()->IsKeyPressed(VK_F6))	
	{
#ifdef USING_DEFERRED //Define found under #includes
		myDoFullRender = myDeferredRenderer.ToggleRenderPass();
#else
		myDoFullRender = myForwardRenderer.ToggleRenderPass();
#endif
	}

	myRenderStateManager.SetAllDefault();
	myBackbuffer.ClearTexture(myClearColor);
	myIntermediateTexture.ClearTexture(myClearColor);
	myIntermediateDepth.ClearDepth();
	myEnvironmentShadowDepth.ClearDepth();
	myGBuffer.ClearTextures(myClearColor);
	myDeferredTexture.ClearTexture();

	CEnvironmentLight* environmentlight = aScene.EnvironmentLight();
	CCameraComponent* maincamera = aScene.MainCamera();

	std::vector<CGameObject*> gameObjects = aScene.CullGameObjects(maincamera);
	std::vector<CGameObject*> instancedGameObjects;
	std::vector<CGameObject*> instancedGameObjectsWithAlpha;
	std::vector<CGameObject*> gameObjectsWithAlpha;
	std::vector<int> indicesOfOutlineModels;
	std::vector<int> indicesOfAlphaGameObjects;

#ifdef USING_DEFERRED // Define found under #includes
#pragma region Deferred
	for (unsigned int i = 0; i < gameObjects.size(); ++i)
	{
		auto instance = gameObjects[i];
		//for (auto gameObjectToOutline : aScene.ModelsToOutline()) {
		//	if (instance == gameObjectToOutline) {
		//		indicesOfOutlineModels.emplace_back(i);
		//	}
		//}

		if (instance->GetComponent<CInstancedModelComponent>()) 
		{
			if (instance->GetComponent<CInstancedModelComponent>()->RenderWithAlpha())
			{
				instancedGameObjectsWithAlpha.emplace_back(instance);
				indicesOfAlphaGameObjects.emplace_back(i);
				continue;
			}
			instancedGameObjects.emplace_back(instance);
		}
		else if (instance->GetComponent<CModelComponent>()) 
		{
			if (instance->GetComponent<CModelComponent>()->RenderWithAlpha())
			{
				gameObjectsWithAlpha.emplace_back(instance);
				indicesOfAlphaGameObjects.emplace_back(i);
				continue;
			}
		}
	}

	std::sort(indicesOfAlphaGameObjects.begin(), indicesOfAlphaGameObjects.end(), [](UINT a, UINT b) { return a > b; });
	for (auto index : indicesOfAlphaGameObjects)
	{
		std::swap(gameObjects[index], gameObjects.back());
		gameObjects.pop_back();
	}

	//std::sort(indicesOfOutlineModels.begin(), indicesOfOutlineModels.end(), [](UINT a, UINT b) { return a > b; });
	//for (auto index : indicesOfOutlineModels)
	//{
	//	std::swap(gameObjects[index], gameObjects.back());
	//	gameObjects.pop_back();
	//}

	// GBuffer
	myGBuffer.SetAsActiveTarget(&myIntermediateDepth);
	myDeferredRenderer.GenerateGBuffer(maincamera, gameObjects, instancedGameObjects);
	
	// Shadows
	myEnvironmentShadowDepth.SetAsDepthTarget();
	myShadowRenderer.Render(environmentlight, gameObjects, instancedGameObjects);

	// Decals
	myDepthCopy.SetAsActiveTarget();
	myIntermediateDepth.SetAsResourceOnSlot(0);
	myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCREENSHADER_COPYDEPTH);
	myGBufferCopy.SetAsActiveTarget();
	myGBuffer.SetAllAsResources();
	myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCREENSHADER_COPYGBUFFER);

	myRenderStateManager.SetDepthStencilState(CRenderStateManager::DepthStencilStates::DEPTHSTENCILSTATE_ONLYREAD);
	myGBuffer.SetAsActiveTarget(&myIntermediateDepth);
	myGBufferCopy.SetAllAsResources();
	myDepthCopy.SetAsResourceOnSlot(21);
	myDecalRenderer.Render(maincamera, gameObjects);

	// Lighting
	myDeferredTexture.SetAsActiveTarget();
	myGBuffer.SetAllAsResources();
	myRenderStateManager.SetBlendState(CRenderStateManager::BlendStates::BLENDSTATE_ADDITIVEBLEND);
	myEnvironmentShadowDepth.SetAsResourceOnSlot(22);
	std::vector<CPointLight*> onlyPointLights;
	onlyPointLights = aScene.CullPointLights(&maincamera->GameObject());

	myDeferredRenderer.Render(maincamera, environmentlight);
	myDeferredRenderer.Render(maincamera, onlyPointLights);

	myRenderStateManager.SetBlendState(CRenderStateManager::BlendStates::BLENDSTATE_DISABLE);
	myIntermediateTexture.SetAsActiveTarget();
	myDeferredTexture.SetAsResourceOnSlot(0);
	//myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCRENSHADER_GAMMACORRECTION);// When testing gbuffer stuff with no bloom
	if(myDoFullRender)
		myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCRENSHADER_GAMMACORRECTION);
	else
		myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCRENSHADER_GAMMACORRECTION_RENDERPASS);
	
	myIntermediateTexture.SetAsActiveTarget(&myIntermediateDepth);

#pragma endregion ! Deferred
#else
#pragma region Forward
	std::vector<LightPair> pointlights;
	std::vector<LightPair> pointLightsInstanced;

	myIntermediateTexture.SetAsActiveTarget(&myIntermediateDepth);

	for (unsigned int i = 0; i < gameObjects.size(); ++i)
	{
		auto instance = gameObjects[i];
		for (auto gameObjectToOutline : aScene.ModelsToOutline()) {
			if (instance == gameObjectToOutline) {
				indicesOfOutlineModels.emplace_back(i);
			}
		}

		if (instance->GetComponent<CModelComponent>()) {
			pointlights.emplace_back(aScene.CullLights(instance));
		}
		else if (instance->GetComponent<CInstancedModelComponent>()) {

			pointLightsInstanced.emplace_back(aScene.CullLightInstanced(instance->GetComponent<CInstancedModelComponent>()));

			if (instance->GetComponent<CInstancedModelComponent>()->RenderWithAlpha())
			{
				instancedGameObjectsWithAlpha.emplace_back(instance);
				continue;
			}
			instancedGameObjects.emplace_back(instance);
		}
	}

	std::sort(indicesOfOutlineModels.begin(), indicesOfOutlineModels.end(), [](UINT a, UINT b) { return a > b; });

	for (auto index : indicesOfOutlineModels)
	{
		std::swap(gameObjects[index], gameObjects.back());
		gameObjects.pop_back();
	}

	myForwardRenderer.Render(environmentlight, pointlights, maincamera, gameObjects);
	myForwardRenderer.InstancedRender(environmentlight, pointLightsInstanced, maincamera, instancedGameObjects);
#pragma endregion ! Forward
#endif // USING_DEFERRED

//#pragma region MODEL OUTLINES
//	for (auto modelToOutline : aScene.ModelsToOutline()) {
//		std::vector<CGameObject*> interimVector;
//		if (modelToOutline) {
//			pointlights.emplace_back(aScene.CullLights(modelToOutline));
//			interimVector.emplace_back(modelToOutline);
//			myRenderStateManager.SetDepthStencilState(CRenderStateManager::DepthStencilStates::DEPTHSTENCILSTATE_STENCILWRITE, 0xFF);
//
//			myForwardRenderer.Render(environmentlight, pointlights, maincamera, interimVector);
//
//			if (modelToOutline != aScene.GetPlayer()) {
//				modelToOutline->GetComponent<CTransformComponent>()->SetOutlineScale();
//			}
//
//			myRenderStateManager.SetDepthStencilState(CRenderStateManager::DepthStencilStates::DEPTHSTENCILSTATE_STENCILMASK, 0xFF);
//
//			if (modelToOutline != aScene.GetPlayer()) {
//				myForwardRenderer.RenderOutline(maincamera, modelToOutline, CModelFactory::GetInstance()->GetOutlineModelSubset(), { 1.0f, 0.0f, 0.0f, 1.0f });
//				modelToOutline->GetComponent<CTransformComponent>()->ResetScale();
//			}
//			else {
//				myForwardRenderer.RenderOutline(maincamera, modelToOutline, CModelFactory::GetInstance()->GetOutlineModelSubset(), { 25.0f / 255.0f, 200.0f / 255.0f, 208.0f / 255.0f, 1.0f });
//			}
//		}
//	}
//#pragma endregion ! MODEL OUTLINES

	const std::vector<CLineInstance*>& lineInstances = aScene.CullLineInstances();
	const std::vector<SLineTime>& lines = aScene.CullLines();
	myForwardRenderer.RenderLines(maincamera, lines);
	myForwardRenderer.RenderLineInstances(maincamera, lineInstances);

	// Alpha stage for objects in World 3D space
	myRenderStateManager.SetBlendState(CRenderStateManager::BlendStates::BLENDSTATE_ALPHABLEND);
	myRenderStateManager.SetDepthStencilState(CRenderStateManager::DepthStencilStates::DEPTHSTENCILSTATE_ONLYREAD);

	std::vector<LightPair> pointlights;
	std::vector<LightPair> pointLightsInstanced;

	myIntermediateTexture.SetAsActiveTarget(&myIntermediateDepth);

	for (unsigned int i = 0; i < instancedGameObjectsWithAlpha.size(); ++i)
	{
		pointLightsInstanced.emplace_back(aScene.CullLightInstanced(instancedGameObjectsWithAlpha[i]->GetComponent<CInstancedModelComponent>()));
	}
	for (unsigned int i = 0; i < gameObjectsWithAlpha.size(); ++i)
	{
		pointlights.emplace_back(aScene.CullLights(gameObjectsWithAlpha[i]));
	}

	myForwardRenderer.InstancedRender(environmentlight, pointLightsInstanced, maincamera, instancedGameObjectsWithAlpha);
	myForwardRenderer.Render(environmentlight, pointlights, maincamera, gameObjectsWithAlpha);

	//VFX
	myRenderStateManager.SetRasterizerState(CRenderStateManager::RasterizerStates::RASTERIZERSTATE_NOFACECULLING);
	myVFXRenderer.Render(maincamera, gameObjects);
	myRenderStateManager.SetRasterizerState(CRenderStateManager::RasterizerStates::RASTERIZERSTATE_DEFAULT);

	myParticleRenderer.Render(maincamera, gameObjects);
	// ! VFX
	// ! Alpha stage for objects in World 3D space

	//std::vector<CSpriteInstance*> sprites = myScene.CullSprites();
	//mySpriteRenderer.Render(sprites);

	//std::vector<CAnimatedUIElement*> animatedUIElements = myScene.CullAnimatedUI();
	//mySpriteRenderer.Render(animatedUIElements);

	myRenderStateManager.SetBlendState(CRenderStateManager::BlendStates::BLENDSTATE_DISABLE);
	myRenderStateManager.SetDepthStencilState(CRenderStateManager::DepthStencilStates::DEPTHSTENCILSTATE_DEFAULT);

	// Hope this works! IT DOES :D
	myDoFullRender ? RenderBloom() : RenderWithoutBloom();

	myRenderStateManager.SetBlendState(CRenderStateManager::BlendStates::BLENDSTATE_ALPHABLEND);
	myRenderStateManager.SetDepthStencilState(CRenderStateManager::DepthStencilStates::DEPTHSTENCILSTATE_ONLYREAD);

	std::vector<CSpriteInstance*> sprites = aScene.CullSprites();
	CMainSingleton::PopupTextService().EmplaceSprites(sprites);
	CMainSingleton::DialogueSystem().EmplaceSprites(sprites);
	mySpriteRenderer.Render(sprites);

	std::vector<CSpriteInstance*> animatedUIFrames;
	std::vector<CAnimatedUIElement*> animatedUIElements = aScene.CullAnimatedUI(animatedUIFrames);
	CEngine::GetInstance()->GetActiveScene().MainCamera()->EmplaceSprites(animatedUIFrames);
	mySpriteRenderer.Render(animatedUIElements);
	mySpriteRenderer.Render(animatedUIFrames);

	myRenderStateManager.SetBlendState(CRenderStateManager::BlendStates::BLENDSTATE_DISABLE);
	myRenderStateManager.SetDepthStencilState(CRenderStateManager::DepthStencilStates::DEPTHSTENCILSTATE_DEFAULT);

	std::vector<CTextInstance*> textsToRender = aScene.Texts();
	CMainSingleton::PopupTextService().EmplaceTexts(textsToRender);
	CMainSingleton::DialogueSystem().EmplaceTexts(textsToRender);
	myTextRenderer.Render(textsToRender);
}

void CRenderManager::Release()
{
	Clear(myClearColor);
	CEngine::GetInstance()->myFramework->GetContext()->OMSetRenderTargets(0, 0, 0);
	CEngine::GetInstance()->myFramework->GetContext()->OMGetDepthStencilState(0, 0);
	CEngine::GetInstance()->myFramework->GetContext()->ClearState();

	myBackbuffer.ReleaseTexture();
	myIntermediateTexture.ReleaseTexture();
	myIntermediateDepth.ReleaseDepth();
	myLuminanceTexture.ReleaseTexture();
	myHalfSizeTexture.ReleaseTexture();
	myQuaterSizeTexture.ReleaseTexture();
	myBlurTexture1.ReleaseTexture();
	myBlurTexture2.ReleaseTexture();
	myVignetteTexture.ReleaseTexture();
	myDeferredTexture.ReleaseTexture();
	//myGBuffer // Should something be released for the GBuffer?
}

void CRenderManager::Clear(DirectX::SimpleMath::Vector4 aClearColor)
{
	myBackbuffer.ClearTexture(aClearColor);
	myIntermediateDepth.ClearDepth();
}

void CRenderManager::RenderBloom()
{
	myLuminanceTexture.SetAsActiveTarget();
	myIntermediateTexture.SetAsResourceOnSlot(0);
	myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCREENSHADER_LUMINANCE);

	myHalfSizeTexture.SetAsActiveTarget();
	myLuminanceTexture.SetAsResourceOnSlot(0);
	myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCREENSHADER_COPY);

	myQuaterSizeTexture.SetAsActiveTarget();
	myHalfSizeTexture.SetAsResourceOnSlot(0);
	myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCREENSHADER_COPY);

	myBlurTexture1.SetAsActiveTarget();
	myQuaterSizeTexture.SetAsResourceOnSlot(0);
	myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCREENSHADER_COPY);

	myBlurTexture2.SetAsActiveTarget();
	myBlurTexture1.SetAsResourceOnSlot(0);
	myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCREENSHADER_GAUSSIANHORIZONTAL);

	myBlurTexture1.SetAsActiveTarget();
	myBlurTexture2.SetAsResourceOnSlot(0);
	myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCREENSHADER_GAUSSIANVERTICAL);

	myBlurTexture2.SetAsActiveTarget();
	myBlurTexture1.SetAsResourceOnSlot(0);
	myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCREENSHADER_GAUSSIANHORIZONTAL);

	myBlurTexture1.SetAsActiveTarget();
	myBlurTexture2.SetAsResourceOnSlot(0);
	myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCREENSHADER_GAUSSIANVERTICAL);

	myQuaterSizeTexture.SetAsActiveTarget();
	myBlurTexture1.SetAsResourceOnSlot(0);
	myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCREENSHADER_COPY);

	myHalfSizeTexture.SetAsActiveTarget();
	myQuaterSizeTexture.SetAsResourceOnSlot(0);
	myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCREENSHADER_COPY);

	myVignetteTexture.SetAsActiveTarget();
	myIntermediateTexture.SetAsResourceOnSlot(0);
	myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCREENSHADER_VIGNETTE);

	myBackbuffer.SetAsActiveTarget();
	myVignetteTexture.SetAsResourceOnSlot(0);
	myHalfSizeTexture.SetAsResourceOnSlot(1);
	myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCREENSHADER_BLOOM);
}

void CRenderManager::RenderWithoutBloom()
{
	myBackbuffer.SetAsActiveTarget();
	myIntermediateTexture.SetAsResourceOnSlot(0);
	myFullscreenRenderer.Render(CFullscreenRenderer::FullscreenShader::FULLSCREENSHADER_VIGNETTE);
}
