#pragma once
#include <array>

struct ID3D11DeviceContext;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
class CDirectXFramework;
class CRenderManager;

class CFullscreenRenderer {
public:
	enum class FullscreenShader {
		FULLSCREENSHADER_COPY,
		FULLSCREENSHADER_COPYDEPTH,
		FULLSCREENSHADER_COPYGBUFFER,
		FULLSCREENSHADER_LUMINANCE,
		FULLSCREENSHADER_GAUSSIANHORIZONTAL,
		FULLSCREENSHADER_GAUSSIANVERTICAL,
		FULLSCREENSHADER_BILATERALHORIZONTAL,
		FULLSCREENSHADER_BILATERALVERTICAL,
		FULLSCREENSHADER_BLOOM,
		FULLSCREENSHADER_VIGNETTE,
		FULLSCREENSHADER_TONEMAP,
		FULLSCREENSHADER_FXAA,
		FULLSCRENSHADER_GAMMACORRECTION,
		FULLSCRENSHADER_GAMMACORRECTION_RENDERPASS,
		FULLSCREENSHADER_DOWNSAMPLE_DEPTH,
		FULLSCREENSHADER_DEPTH_AWARE_UPSAMPLING,
		FULLSCREENSHADER_COUNT
	};

	friend CRenderManager;
	
private:
	template<class T>
	void BindBuffer(ID3D11Buffer* aBuffer, T& someBufferData, std::string aBufferType)
	{
		D3D11_MAPPED_SUBRESOURCE bufferData;
		ZeroMemory(&bufferData, sizeof(D3D11_MAPPED_SUBRESOURCE));
		std::string errorMessage = aBufferType + " could not be bound.";
		ENGINE_HR_MESSAGE(myContext->Map(aBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData), errorMessage.c_str());

		memcpy(bufferData.pData, &someBufferData, sizeof(T));
		myContext->Unmap(aBuffer, 0);
	}

	struct SFullscreenData {
		Vector2 myResolution;
		Vector2 myPadding;
	} myFullscreenData;

private:
	CFullscreenRenderer();
	~CFullscreenRenderer();
	bool Init(CDirectXFramework* aFramework);
	void Render(FullscreenShader aEffect);

	ID3D11DeviceContext* myContext;
	ID3D11Buffer* myFullscreenDataBuffer;
	ID3D11VertexShader* myVertexShader;
	ID3D11SamplerState* mySampler;
	std::array<ID3D11PixelShader*, static_cast<size_t>(FullscreenShader::FULLSCREENSHADER_COUNT)> myPixelShaders;
};