#include "stdafx.h"
#include "DeferredRenderer.h"
#include "DirectXFramework.h"
#include "RenderManager.h"
#include "Model.h"
#include "Camera.h"
#include "EnvironmentLight.h"
#include "PointLight.h"

#include "GameObject.h"
#include "CameraComponent.h"
#include "ModelComponent.h"
#include "InstancedModelComponent.h"
#include "MaterialHandler.h"

#include <fstream>

namespace SM = DirectX::SimpleMath;

CDeferredRenderer::CDeferredRenderer()
	: myContext(nullptr)
	, myFrameBuffer(nullptr)
	, myObjectBuffer(nullptr)
	, myLightBuffer(nullptr)
	, myPointLightBuffer(nullptr)
	, myFullscreenShader(nullptr)
	, myModelVertexShader(nullptr)
	, myInstancedModelVertexShader(nullptr)
	, myEnvironmentLightShader(nullptr)
	, myGBufferPixelShader(nullptr)
	, myPointLightShader(nullptr)
	, mySamplerState(nullptr)
	, myCurrentGBufferPixelShader(nullptr)
	, myRenderPassGBuffer(nullptr)
	, myCurrentRenderPassShader(nullptr)
	, myRenderPassIndex(9)
{}

CDeferredRenderer::~CDeferredRenderer()
{
}

bool CDeferredRenderer::Init(CDirectXFramework* aFramework)
{
	myContext = aFramework->GetContext();
	if (!myContext){
		return false; // Maybe we should fix proper error handling?? :S
	}

	ID3D11Device* device = aFramework->GetDevice();
	if (!device) {
		return false;
	}

	D3D11_BUFFER_DESC bufferDescription = { 0 };
	bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	bufferDescription.ByteWidth = sizeof(SFrameBufferData);
	ENGINE_HR_BOOL_MESSAGE(device->CreateBuffer(&bufferDescription, nullptr, &myFrameBuffer), "Frame Buffer could not be created.");

	bufferDescription.ByteWidth = sizeof(SObjectBufferData);
	ENGINE_HR_BOOL_MESSAGE(device->CreateBuffer(&bufferDescription, nullptr, &myObjectBuffer), "Object Buffer could not be created.");

	bufferDescription.ByteWidth = sizeof(SLightBufferData);
	ENGINE_HR_BOOL_MESSAGE(device->CreateBuffer(&bufferDescription, nullptr, &myLightBuffer), "Light Buffer could not be created.");

	bufferDescription.ByteWidth = sizeof(SPointLightBufferData);
	ENGINE_HR_MESSAGE(device->CreateBuffer(&bufferDescription, nullptr, &myPointLightBuffer), "Point Light Buffer could not be created.");

	std::ifstream vsFile;
	vsFile.open("Shaders/DeferredVertexShader.cso", std::ios::binary);
	std::string vsData = { std::istreambuf_iterator<char>(vsFile), std::istreambuf_iterator<char>() };
	ENGINE_HR_BOOL_MESSAGE(aFramework->GetDevice()->CreateVertexShader(vsData.data(), vsData.size(), nullptr, &myFullscreenShader), "Fullscreen Vertex Shader could not be created.");
	vsFile.close();

	vsFile.open("Shaders/DeferredModelVertexShader.cso", std::ios::binary);
	vsData = { std::istreambuf_iterator<char>(vsFile), std::istreambuf_iterator<char>() };
	ENGINE_HR_BOOL_MESSAGE(aFramework->GetDevice()->CreateVertexShader(vsData.data(), vsData.size(), nullptr, &myModelVertexShader), "Fullscreen Vertex Shader could not be created.");
	vsFile.close();

	vsFile.open("Shaders/DeferredInstancedModelVertexShader.cso", std::ios::binary);
	vsData = { std::istreambuf_iterator<char>(vsFile), std::istreambuf_iterator<char>() };
	ENGINE_HR_BOOL_MESSAGE(aFramework->GetDevice()->CreateVertexShader(vsData.data(), vsData.size(), nullptr, &myInstancedModelVertexShader), "Fullscreen Vertex Shader could not be created.");
	vsFile.close();

	std::ifstream ps1File;
	ps1File.open("Shaders/GBufferPixelShader.cso", std::ios::binary);
	std::string psData = { std::istreambuf_iterator<char>(ps1File), std::istreambuf_iterator<char>() };
	ENGINE_HR_BOOL_MESSAGE(aFramework->GetDevice()->CreatePixelShader(psData.data(), psData.size(), nullptr, &myGBufferPixelShader), "GBuffer Pixel Shader could not be createed.");
	ps1File.close();

	std::ifstream ps2File;
	ps2File.open("Shaders/DeferredLightEnvironmentShader.cso", std::ios::binary);
	psData = { std::istreambuf_iterator<char>(ps2File), std::istreambuf_iterator<char>() };
	ENGINE_HR_BOOL_MESSAGE(aFramework->GetDevice()->CreatePixelShader(psData.data(), psData.size(), nullptr, &myEnvironmentLightShader), "Environment Pixel Shader could not be createed.");
	ps2File.close();

	std::ifstream ps3File;
	ps3File.open("Shaders/DeferredLightPointShader.cso", std::ios::binary);
	psData = { std::istreambuf_iterator<char>(ps3File), std::istreambuf_iterator<char>() };
	ENGINE_HR_BOOL_MESSAGE(aFramework->GetDevice()->CreatePixelShader(psData.data(), psData.size(), nullptr, &myPointLightShader), "Point Light Pixel Shader could not be createed.");
	ps3File.close();

	LoadRenderPassPixelShaders(aFramework->GetDevice());

	CreatePixelShader("Shaders/DeferredVertexPaintPixelShader.cso", aFramework, &myVertexPaintPixelShader);
	//CreateVertexShader("Shaders/DeferredVertexPaintVertexShader.cso", aFramework, &myVertexPaintModelVertexShader);
	vsFile.open("Shaders/DeferredVertexPaintVertexShader.cso", std::ios::binary);
	vsData = { std::istreambuf_iterator<char>(vsFile), std::istreambuf_iterator<char>() };
	ENGINE_HR_BOOL_MESSAGE(aFramework->GetDevice()->CreateVertexShader(vsData.data(), vsData.size(), nullptr, &myVertexPaintModelVertexShader), "Vertex Paint Vertex Shader could not be created.");
	vsFile.close();

	// Vertex Paint input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION"	,	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL"	,	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT"	,	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BINORMAL"	,	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"UV"		,	0, DXGI_FORMAT_R32G32_FLOAT		 , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR"	,	0, DXGI_FORMAT_R32G32B32_FLOAT	 , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	ENGINE_HR_MESSAGE(aFramework->GetDevice()->CreateInputLayout(layout, sizeof(layout) / sizeof(D3D11_INPUT_ELEMENT_DESC), vsData.data(), vsData.size(), &myVertexPaintInputLayout), "Vertex Paint Input Layout could not be created.");

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = 10;
	ENGINE_HR_BOOL_MESSAGE(aFramework->GetDevice()->CreateSamplerState(&samplerDesc, &mySamplerState), "Sampler could not be created.");

	return true;
}

void CDeferredRenderer::GenerateGBuffer(CCameraComponent* aCamera, std::vector<CGameObject*>& aGameObjectList, std::vector<CGameObject*>& aInstancedGameObjectList)
{
	SM::Matrix& cameraMatrix = aCamera->GameObject().myTransform->Transform();
	myFrameBufferData.myCameraPosition = SM::Vector4{ cameraMatrix._41, cameraMatrix._42, cameraMatrix._43, 1.f };
	myFrameBufferData.myToCamera = cameraMatrix.Invert();
	myFrameBufferData.myToProjection = aCamera->GetProjection();

	BindBuffer(myFrameBuffer, myFrameBufferData, "Frame Buffer");

	myContext->VSSetConstantBuffers(0, 1, &myFrameBuffer);
	myContext->PSSetConstantBuffers(0, 1, &myFrameBuffer);

	// Toggling render passes
	if (myCurrentRenderPassShader == nullptr)
		myCurrentGBufferPixelShader = myGBufferPixelShader;
	else
		myCurrentGBufferPixelShader = myRenderPassGBuffer;

	for (auto& gameObject : aGameObjectList)
	{
		CModelComponent* modelComponent = gameObject->GetComponent<CModelComponent>();
		if (modelComponent == nullptr)
			continue;

		if (modelComponent->GetMyModel() == nullptr)
			continue;

		CModel* model = modelComponent->GetMyModel();
		CModel::SModelData modelData = model->GetModelData();

		myObjectBufferData.myToWorld = gameObject->myTransform->GetWorldMatrix();
		int dnCounter = 0;
		for (auto detailNormal : model->GetModelData().myDetailNormals)
		{
			if (detailNormal)
				++dnCounter;
		}
		myObjectBufferData.myNumberOfDetailNormals = dnCounter;

		BindBuffer(myObjectBuffer, myObjectBufferData, "Object Buffer");

		myContext->IASetPrimitiveTopology(modelData.myPrimitiveTopology);
		myContext->IASetInputLayout(modelData.myInputLayout);

		myContext->VSSetConstantBuffers(1, 1, &myObjectBuffer);
		myContext->VSSetShader(myModelVertexShader, nullptr, 0);

		myContext->PSSetConstantBuffers(1, 1, &myObjectBuffer);
		myContext->PSSetShaderResources(8, dnCounter, &modelData.myDetailNormals[0]);

		myContext->PSSetShader(myCurrentGBufferPixelShader, nullptr, 0);
		myContext->PSSetSamplers(0, 1, &modelData.mySamplerState);

		// Vertex Paint
		unsigned int vertexColorID = modelComponent->VertexPaintColorID();

		// Render all meshes
		for (unsigned int i = 0; i < modelData.myMeshes.size(); ++i)
		{
			if (vertexColorID > 0)
			{
				auto vertexPaintMaterials = CMainSingleton::MaterialHandler().GetVertexPaintMaterials(modelComponent->VertexPaintMaterialNames());
				ID3D11Buffer* vertexColorBuffer = CMainSingleton::MaterialHandler().GetVertexColorBuffer(vertexColorID);
				UINT stride = sizeof(DirectX::SimpleMath::Vector3);
				ID3D11Buffer* bufferPointers[2] = { modelData.myMeshes[i].myVertexBuffer, vertexColorBuffer };
				UINT strides[2] = { modelData.myMeshes[i].myStride, stride };
				UINT offsets[2] = { 0, 0 };
				myContext->IASetInputLayout(myVertexPaintInputLayout);
				myContext->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
				myContext->VSSetShader(myVertexPaintModelVertexShader, nullptr, 0);
				myContext->PSSetShader(myVertexPaintPixelShader, nullptr, 0);
				myContext->PSSetShaderResources(12, 9, &vertexPaintMaterials[0]/*&myVertexPaintMaterials[0]*/);
			}
			else {
				myContext->IASetVertexBuffers(0, 1, &modelData.myMeshes[i].myVertexBuffer, &modelData.myMeshes[i].myStride, &modelData.myMeshes[i].myOffset);
			}
			myContext->IASetIndexBuffer(modelData.myMeshes[i].myIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			myContext->PSSetShaderResources(5, 3, &modelData.myMaterials[modelData.myMeshes[i].myMaterialIndex][0]);
			myContext->PSSetShaderResources(8, 4, &modelData.myDetailNormals[0]);
			myContext->DrawIndexed(modelData.myMeshes[i].myNumberOfIndices, 0, 0);
			CRenderManager::myNumberOfDrawCallsThisFrame++;
		}
	}

	ID3D11ShaderResourceView* nullView = NULL;
	myContext->PSSetShaderResources(5,  1, &nullView);// Albedo
	myContext->PSSetShaderResources(6, 1, &nullView);// Normal
	myContext->PSSetShaderResources(7, 1, &nullView);// Material
	myContext->PSSetShaderResources(8, 1, &nullView);// DN1
	myContext->PSSetShaderResources(9, 1, &nullView);// DN2
	myContext->PSSetShaderResources(10, 1, &nullView);// DN3
	myContext->PSSetShaderResources(11, 1, &nullView);// DN4
	myObjectBufferData.myNumberOfDetailNormals = 0;// Making sure to reset it!
	
	for (auto& gameobject : aInstancedGameObjectList)
	{
		CInstancedModelComponent* instanceComponent = gameobject->GetComponent<CInstancedModelComponent>();
		if (instanceComponent == nullptr)
			continue;

		CModel* model = instanceComponent->GetModel();
		CModel::SModelInstanceData modelData = model->GetModelInstanceData();

		int dnCounter = 0;
		for (auto detailNormal : model->GetModelInstanceData().myDetailNormals)
		{
			if (detailNormal)
				++dnCounter;
		}
		myObjectBufferData.myNumberOfDetailNormals = dnCounter;

		BindBuffer(myObjectBuffer, myObjectBufferData, "Object Buffer");

		{
			D3D11_MAPPED_SUBRESOURCE instanceBuffer;
			ZeroMemory(&instanceBuffer, sizeof(D3D11_MAPPED_SUBRESOURCE));
			ENGINE_HR_MESSAGE(myContext->Map(modelData.myInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &instanceBuffer), "Instanced Buffer Could not be mapped in ForwardRenderer.");
			memcpy(instanceBuffer.pData, &instanceComponent->GetInstancedTransforms()[0], sizeof(DirectX::SimpleMath::Matrix) * instanceComponent->GetInstancedTransforms().size());
			myContext->Unmap(modelData.myInstanceBuffer, 0);
		}

		myContext->IASetPrimitiveTopology(modelData.myPrimitiveTopology);
		myContext->IASetInputLayout(modelData.myInputLayout);

		//ID3D11Buffer* bufferPointers[2] = {modelData.myVertexBuffer, modelData.myInstanceBuffer};
		//myContext->IASetVertexBuffers(0, 2, bufferPointers, modelData.myStride, modelData.myOffset);
		//myContext->IASetIndexBuffer(modelData.myIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		myContext->VSSetConstantBuffers(1, 1, &myObjectBuffer);
		myContext->VSSetShader(myInstancedModelVertexShader, nullptr, 0);

		myContext->PSSetConstantBuffers(1, 1, &myObjectBuffer);
		//myContext->PSSetShaderResources(5, 3, &modelData.myTexture[0]);
		//myContext->PSSetShaderResources(9, 3, &modelData.myTexture[0]);
		myContext->PSSetShaderResources(8, 4, &modelData.myDetailNormals[0]);

		myContext->PSSetSamplers(0, 1, &modelData.mySamplerState);

		//myContext->PSSetShader(myGBufferPixelShader, nullptr, 0);
		myContext->PSSetShader(myCurrentGBufferPixelShader, nullptr, 0);

		// Render all meshes
		for (unsigned int i = 0; i < modelData.myMeshes.size(); ++i)
		{
			ID3D11Buffer* bufferPointers[2] = { modelData.myMeshes[i].myVertexBuffer, modelData.myInstanceBuffer };
			myContext->IASetVertexBuffers(0, 2, bufferPointers, modelData.myMeshes[i].myStride, modelData.myMeshes[i].myOffset);
			myContext->IASetIndexBuffer(modelData.myMeshes[i].myIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			myContext->PSSetShaderResources(5, 3, &modelData.myMaterials[modelData.myMeshes[i].myMaterialIndex][0]);
			myContext->DrawIndexedInstanced(modelData.myMeshes[i].myNumberOfIndices, model->InstanceCount(), 0, 0, 0);
			CRenderManager::myNumberOfDrawCallsThisFrame++;
		}
	}

	myContext->PSSetShaderResources(5,  1, &nullView);// Albedo
	myContext->PSSetShaderResources(6, 1, &nullView);// Normal
	myContext->PSSetShaderResources(7, 1, &nullView);// Material
	myContext->PSSetShaderResources(8, 1, &nullView);// DN1
	myContext->PSSetShaderResources(9, 1, &nullView);// DN2
	myContext->PSSetShaderResources(10, 1, &nullView);// DN3
	myContext->PSSetShaderResources(11, 1, &nullView);// DN4
	myObjectBufferData.myNumberOfDetailNormals = 0;// Making sure to reset it!
}

void CDeferredRenderer::Render(CCameraComponent* aCamera, CEnvironmentLight* anEnvironmentLight)
{

	SM::Matrix& cameraMatrix = aCamera->GameObject().myTransform->Transform();
	myFrameBufferData.myCameraPosition = SM::Vector4{ cameraMatrix._41, cameraMatrix._42, cameraMatrix._43, 1.f };
	myFrameBufferData.myToProjection = aCamera->GetProjection();
	BindBuffer(myFrameBuffer, myFrameBufferData, "Frame Buffer");

	myContext->PSSetConstantBuffers(0, 1, &myFrameBuffer);
	ID3D11ShaderResourceView* environmentLightShaderResource = *anEnvironmentLight->GetCubeMap();
	myContext->PSSetShaderResources(0, 1, &environmentLightShaderResource);

	// Update lightbufferdata and fill lightbuffer
	myLightBufferData.myDirectionalLightDirection = anEnvironmentLight->GetDirection();
	myLightBufferData.myDirectionalLightColor = anEnvironmentLight->GetColor();
	BindBuffer(myLightBuffer, myLightBufferData, "Light Buffer");
	myContext->PSSetConstantBuffers(2, 1, &myLightBuffer);

	myContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	myContext->IASetInputLayout(nullptr);
	myContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	myContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

	myContext->GSSetShader(nullptr, nullptr, 0);

	myContext->VSSetShader(myFullscreenShader, nullptr, 0);

	if(myCurrentRenderPassShader)
		myContext->PSSetShader(myRenderPassShaders[myRenderPassIndex], nullptr, 0);
	else
		myContext->PSSetShader(myEnvironmentLightShader, nullptr, 0);

	myContext->PSSetSamplers(0, 1, &mySamplerState);

	myContext->Draw(3, 0);
	CRenderManager::myNumberOfDrawCallsThisFrame++;
}

void CDeferredRenderer::Render(CCameraComponent* aCamera, std::vector<CPointLight*>& aPointLightList)
{
	if (myCurrentRenderPassShader)
		return;

	SM::Matrix& cameraMatrix = aCamera->GameObject().myTransform->Transform();
	myFrameBufferData.myCameraPosition = SM::Vector4{ cameraMatrix._41, cameraMatrix._42, cameraMatrix._43, 1.f };
	myFrameBufferData.myToProjection = aCamera->GetProjection();
	BindBuffer(myFrameBuffer, myFrameBufferData, "Frame Buffer");
	myContext->PSSetConstantBuffers(0, 1, &myFrameBuffer);

	for (CPointLight* currentInstance : aPointLightList) {
		//Update pointlightbufferdata and fill pointlightbuffer
		SM::Vector3 position = currentInstance->GetPosition();
		SM::Vector3 color = currentInstance->GetColor();
		myPointLightBufferData.myPositionAndRange = { position.x, position.y, position.z, currentInstance->GetRange() };
		myPointLightBufferData.myColorAndIntensity = { color.x, color.y, color.z, currentInstance->GetIntensity() };

		BindBuffer(myPointLightBuffer, myPointLightBufferData, "Point Light Buffer");
		myContext->PSSetConstantBuffers(3, 1, &myPointLightBuffer);

		myContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		myContext->IASetInputLayout(nullptr);
		myContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		myContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

		myContext->GSSetShader(nullptr, nullptr, 0);

		myContext->VSSetShader(myFullscreenShader, nullptr, 0);
		myContext->PSSetShader(myPointLightShader, nullptr, 0);
		myContext->PSSetSamplers(0, 1, &mySamplerState);

		myContext->Draw(3, 0);
		CRenderManager::myNumberOfDrawCallsThisFrame++;
	}

}

bool CDeferredRenderer::CreateVertexShader(std::string aFilepath, CDirectXFramework* aFramework, ID3D11VertexShader** outVertexShader)
{
	std::ifstream vsFile;
	vsFile.open(aFilepath, std::ios::binary);
	std::string vsData = { std::istreambuf_iterator<char>(vsFile), std::istreambuf_iterator<char>() };
	ENGINE_HR_BOOL_MESSAGE(aFramework->GetDevice()->CreateVertexShader(vsData.data(), vsData.size(), nullptr, outVertexShader), "Vertex Shader could not be created.");
	vsFile.close();
	return true;
}

bool CDeferredRenderer::CreatePixelShader(std::string aFilepath, CDirectXFramework* aFramework, ID3D11PixelShader** outPixelShader)
{
	std::ifstream psFile;
	psFile.open(aFilepath, std::ios::binary);
	std::string psData = { std::istreambuf_iterator<char>(psFile), std::istreambuf_iterator<char>() };
	ENGINE_HR_BOOL_MESSAGE(aFramework->GetDevice()->CreatePixelShader(psData.data(), psData.size(), nullptr, outPixelShader), "Pixel Shader could not be created.");
	psFile.close();
	return true;
}

bool CDeferredRenderer::LoadRenderPassPixelShaders(ID3D11Device* aDevice)
{
	// Render pass shaders
	std::ifstream psFile;
	psFile.open("Shaders/DeferredRenderPassShader_Albedo.cso", std::ios::binary);
	std::string psData = {std::istreambuf_iterator<char>(psFile), std::istreambuf_iterator<char>()};
	psFile.close();
	myRenderPassShaders.emplace_back();
	ENGINE_HR_MESSAGE(aDevice->CreatePixelShader(psData.data(), psData.size(), nullptr, &myRenderPassShaders[0]), "Color Pixel Shader could not be created.");

	// ===============
	psFile.open("Shaders/DeferredRenderPassShader_Normal.cso", std::ios::binary);
	psData = {std::istreambuf_iterator<char>(psFile), std::istreambuf_iterator<char>()};
	psFile.close();
	myRenderPassShaders.emplace_back();
	ENGINE_HR_MESSAGE(aDevice->CreatePixelShader(psData.data(), psData.size(), nullptr, &myRenderPassShaders[1]), "Normal Pixel Shader could not be created.");

	// ===============
	psFile.open("Shaders/DeferredRenderPassShader_Roughness.cso", std::ios::binary);
	psData = {std::istreambuf_iterator<char>(psFile), std::istreambuf_iterator<char>()};
	psFile.close();
	myRenderPassShaders.emplace_back();
	ENGINE_HR_MESSAGE(aDevice->CreatePixelShader(psData.data(), psData.size(), nullptr, &myRenderPassShaders[2]), "Roughness Pixel Shader could not be created.");

	// ===============
	psFile.open("Shaders/DeferredRenderPassShader_Metalness.cso", std::ios::binary);
	psData = {std::istreambuf_iterator<char>(psFile), std::istreambuf_iterator<char>()};
	psFile.close();
	myRenderPassShaders.emplace_back();
	ENGINE_HR_MESSAGE(aDevice->CreatePixelShader(psData.data(), psData.size(), nullptr, &myRenderPassShaders[3]), "Metalness Pixel Shader could not be created.");

	// ===============
	psFile.open("Shaders/DeferredRenderPassShader_AO.cso", std::ios::binary);
	psData = {std::istreambuf_iterator<char>(psFile), std::istreambuf_iterator<char>()};
	psFile.close();
	myRenderPassShaders.emplace_back();
	ENGINE_HR_MESSAGE(aDevice->CreatePixelShader(psData.data(), psData.size(), nullptr, &myRenderPassShaders[4]), "Ambient Occlusion Pixel Shader could not be created.");

	// ===============
	psFile.open("Shaders/DeferredRenderPassShader_Emissive.cso", std::ios::binary);
	psData = {std::istreambuf_iterator<char>(psFile), std::istreambuf_iterator<char>()};
	psFile.close();
	myRenderPassShaders.emplace_back();
	ENGINE_HR_MESSAGE(aDevice->CreatePixelShader(psData.data(), psData.size(), nullptr, &myRenderPassShaders[5]), "Emissive Pixel Shader could not be created.");

	// ===============
	psFile.open("Shaders/DeferredRenderPassGBufferPixelShader.cso", std::ios::binary);
	psData = {std::istreambuf_iterator<char>(psFile), std::istreambuf_iterator<char>()};
	psFile.close();
	ENGINE_HR_MESSAGE(aDevice->CreatePixelShader(psData.data(), psData.size(), nullptr, &myRenderPassGBuffer), "Renderpass GBuffer could not be created.");

	return true;
}

bool CDeferredRenderer::ToggleRenderPass()
{
	++myRenderPassIndex;
	if (myRenderPassIndex == myRenderPassShaders.size())
	{
		myCurrentRenderPassShader = nullptr;
		return true;
	} else if (myRenderPassIndex > myRenderPassShaders.size())
	{
		myRenderPassIndex = 0;
	}
	myCurrentRenderPassShader = myRenderPassShaders[myRenderPassIndex];
	return false;
}
