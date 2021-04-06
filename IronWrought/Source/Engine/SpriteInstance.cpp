#include "stdafx.h"
#include "SpriteInstance.h"
#include "Sprite.h"
#include "Scene.h"
#include "Engine.h"
#include "WindowHandler.h"

CSpriteInstance::CSpriteInstance(CScene& aScene, bool aAddToScene)
	: mySprite(nullptr)
	, myRenderOrder(ERenderOrder::BackgroundLayer)
	, myShouldRender(true)
	, myShouldAnimate(false)
	, myAnimationTimer(0.0f)
	, myCurrentAnimationIndex(0)
	, myCurrentAnimationFrame(0)
{
	if (aAddToScene)
		aScene.AddInstance(this);
}
CSpriteInstance::CSpriteInstance()
	: mySprite(nullptr)
	, myRenderOrder(ERenderOrder::BackgroundLayer)
	, myShouldRender(true)
	, myShouldAnimate(false)
{
}

CSpriteInstance::~CSpriteInstance()
{
}

bool CSpriteInstance::Init(CSprite* aSprite, const Vector2& aScale)
{
	if (!aSprite) {
		return false;
	}

	mySprite = aSprite;
	this->SetSize(aScale);
	myRenderOrder = ERenderOrder::ForegroundLayer;

	return true;
}

bool CSpriteInstance::Init(CSprite* aSprite, const std::vector<SSpriteSheetPositionData>& someSpriteSheetPositionData, const Vector2& aScale)
{
	if (!aSprite) {
		return false;
	}

	mySprite = aSprite;
	this->SetSize(aScale);
	myRenderOrder = ERenderOrder::ForegroundLayer;

	Vector2 sheetDimensions = aSprite->GetSpriteData().myDimensions;

	for (unsigned int i = 0; i < someSpriteSheetPositionData.size(); ++i)
	{
		myAnimationData.emplace_back();
		auto& animationDataToLoad = someSpriteSheetPositionData.back();
		myAnimationData.back().myAnimationName = animationDataToLoad.myAnimationName;
		myAnimationData.back().myFramesOffset = static_cast<int>(myAnimationFrames.size());
		myAnimationData.back().myNumberOfFrames = animationDataToLoad.myNumberOfFrames;
		myAnimationData.back().myFramesPerSecond = animationDataToLoad.mySpeedInFramesPerSecond;

		for (unsigned int j = 0; j < static_cast<unsigned int>(animationDataToLoad.myNumberOfFrames); ++j)
		{
			myAnimationFrames.emplace_back();
			auto& currentUV = myAnimationFrames.back();
			currentUV.x = (animationDataToLoad.mySpriteWidth * j) / sheetDimensions.x; // Left
			currentUV.y = (animationDataToLoad.myVerticalStartingPosition) / sheetDimensions.y; // Up
			currentUV.z = (animationDataToLoad.mySpriteWidth * (j + 1)) / sheetDimensions.x; // Right
			currentUV.w = (animationDataToLoad.myVerticalStartingPosition + animationDataToLoad.mySpriteHeight) / sheetDimensions.y; // Down
		}
	}

	myShouldAnimate = !myAnimationFrames.empty();

	if (myShouldAnimate)
	{
		Vector2 frameSize = { someSpriteSheetPositionData.back().mySpriteWidth,  someSpriteSheetPositionData.back().mySpriteHeight };
		Vector2 scaleProportions = (frameSize / sheetDimensions);
		this->SetSize(aScale * scaleProportions);
	}

	return true;
}

void CSpriteInstance::SetSize(DirectX::SimpleMath::Vector2 aSize)
{
	//Sprites are assumed to be the correct native size in a 1920x1080 resolution
	CSprite::SSpriteData mySpriteData = mySprite->GetSpriteData();
	mySize = mySpriteData.myDimensions;
	mySize /= 1080.0f;
	mySize *= aSize;
	//mySize = mySpriteData.myDimensions;
	//mySize.x /= 1920.0f;
	//mySize.y /= 1080.0f;
	//mySize *= aSize;
}

void CSpriteInstance::SetShouldRender(bool aBool)
{
	myShouldRender = aBool;
}

void CSpriteInstance::Update()
{
	if (!myShouldAnimate)
		return;

	if ((myAnimationTimer += CTimer::Dt()) > (1.0f / myAnimationData[myCurrentAnimationIndex].myFramesPerSecond))
	{
		myAnimationTimer = 0.0f; // doing it properly doesn't seem to work, as CTimer is not marked at the start of this state
		myCurrentAnimationFrame++;
		if (myCurrentAnimationFrame > (myAnimationData[myCurrentAnimationIndex].myNumberOfFrames + myAnimationData[myCurrentAnimationIndex].myFramesOffset - 1))
		{
			myCurrentAnimationFrame = myAnimationData[myCurrentAnimationIndex].myFramesOffset;
		}
		this->SetUVRect(myAnimationFrames[myCurrentAnimationFrame]);
	}
}

void CSpriteInstance::SetRenderOrder(ERenderOrder aRenderOrder)
{
	myRenderOrder = aRenderOrder;
}

/// <summary>
///The y-position is flipped in this function so that we go from Shader space
///([-1, -1] in the lower left corner) to Shader space with [-1, -1] in the
/// upper left corner. I think this is more intuitive, but might as well
/// revert this.
/// </summary>
/// <param name="aPosition"></param>
void CSpriteInstance::SetPosition(DirectX::SimpleMath::Vector2 aPosition)
{
	myPosition.x = aPosition.x;
	myPosition.y = -aPosition.y;
}

void CSpriteInstance::SetNormalPosition(DirectX::SimpleMath::Vector2 aPosition)
{
	//aPosition /= 2.0f;
	//aPosition.x += 0.5f;
	//aPosition.y += 0.5f;
	myPosition.x = aPosition.x /** CEngine::GetInstance()->GetWindowHandler()->GetResolution().x*/;
	myPosition.y = aPosition.y /** CEngine::GetInstance()->GetWindowHandler()->GetResolution().y*/;
}

void CSpriteInstance::SetColor(DirectX::SimpleMath::Vector4 aColor)
{
	myColor = aColor;
}

void CSpriteInstance::SetUVRect(DirectX::SimpleMath::Vector4 aUVRect)
{
	myUVRect = aUVRect;
}
