#pragma once
#include "ForwardRenderer.h"
#include "DeferredRenderer.h"
#include "FullscreenRenderer.h"
#include "FullscreenTexture.h"
#include "FullscreenTextureFactory.h"
#include "ParticleRenderer.h"
#include "VFXRenderer.h"
#include "RenderStateManager.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "GBuffer.h"

class CDirectXFramework;
class CWindowHandler;
class CScene;

class CRenderManager {
public:
	CRenderManager();
	~CRenderManager();
	bool Init(CDirectXFramework* aFramework, CWindowHandler* aWindowHandler);
	bool ReInit(CDirectXFramework* aFramework, CWindowHandler* aWindowHandler);
	void Render(CScene& aScene);

	void Release();

private:
	void Clear(DirectX::SimpleMath::Vector4 aClearColor);

private:
	void RenderBloom();
	void RenderWithoutBloom();

private:
	CRenderStateManager myRenderStateManager;
	CForwardRenderer myForwardRenderer;
	CDeferredRenderer myDeferredRenderer;
	CFullscreenRenderer myFullscreenRenderer;
	CParticleRenderer myParticleRenderer;
	CVFXRenderer myVFXRenderer;
	CSpriteRenderer mySpriteRenderer;
	CTextRenderer myTextRenderer;

	CFullscreenTextureFactory myFullscreenTextureFactory;
	CFullscreenTexture myBackbuffer;
	CFullscreenTexture myIntermediateTexture;
	CFullscreenTexture myIntermediateDepth;
	CFullscreenTexture myLuminanceTexture;
	CFullscreenTexture myHalfSizeTexture;
	CFullscreenTexture myQuaterSizeTexture;
	CFullscreenTexture myBlurTexture1;
	CFullscreenTexture myBlurTexture2;
	CFullscreenTexture myVignetteTexture;
	CFullscreenTexture myDeferredTexture;
	CGBuffer myGBuffer;


	DirectX::SimpleMath::Vector4 myClearColor;

	bool myUseBloom;
	//int myFrameCounter;// Used for a hack solution, can probably be removed
};