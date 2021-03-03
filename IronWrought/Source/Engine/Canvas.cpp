#include "stdafx.h"
#include "Canvas.h"
#include "MainSingleton.h"
#include "PostMaster.h"
#include "Button.h"
#include "SpriteInstance.h"
#include "TextInstance.h"
#include "TextFactory.h"
#include "AnimatedUIElement.h"
#include "InputMapper.h"
#include "Input.h"
#include "SpriteFactory.h"
#include "Sprite.h"
#include "MainSingleton.h"
#include "Engine.h"
#include "Scene.h"

#include "JsonReader.h"
//#include "rapidjson\document.h"
//#include "rapidjson\istreamwrapper.h"

using namespace rapidjson;

CCanvas::CCanvas() :
	myBackground(nullptr),
	myIsEnabled(true)
{
}

CCanvas::~CCanvas()
{
	UnsubscribeToMessages();
	myMessageTypes.clear();
	delete myBackground;
	myBackground = nullptr;

	for (size_t i = 0; i < myAnimatedUIs.size(); ++i)
	{
		delete myAnimatedUIs[i];
		myAnimatedUIs[i] = nullptr;
	}
	myAnimatedUIs.clear();

	for (size_t i = 0; i < myButtons.size(); ++i)
	{
			delete myButtons[i];
			myButtons[i] = nullptr;
	}
	myButtons.clear();

	for (size_t i = 0; i < mySprites.size(); ++i)
	{
			delete mySprites[i];
			mySprites[i] = nullptr;
	}
	mySprites.clear();

	for (size_t i = 0; i < myTexts.size(); ++i)
	{
			delete myTexts[i];
			myTexts[i] = nullptr;
	}
	myTexts.clear();
}

void CCanvas::Init(std::string aFilePath, CScene& aScene, bool addToScene)
{
	//std::ifstream inputStream(aFilePath);
	//IStreamWrapper inputWrapper(inputStream);
	Document document = CJsonReader::Get()->LoadDocument(aFilePath);
	//document.ParseStream(inputWrapper);

	if (document.HasMember("Buttons"))
	{
		auto buttonDataArray = document["Buttons"].GetArray();
		for (unsigned int i = 0; i < buttonDataArray.Size(); ++i)
		{
			SButtonData data;
			auto buttonData = buttonDataArray[i].GetObjectW();
			
			myTexts.emplace_back(new CTextInstance(aScene, addToScene));
			myTexts.back()->Init(CTextFactory::GetInstance()->GetText(buttonData["FontAndFontSize"].GetString()));
			myTexts.back()->SetText(buttonData["Text"].GetString());
			myTexts.back()->SetColor({ buttonData["Text Color R"].GetFloat(), buttonData["Text Color G"].GetFloat(), buttonData["Text Color B"].GetFloat(), 1.0f });
			myTexts.back()->SetPosition({ buttonData["Text Position X"].GetFloat(), buttonData["Text Position Y"].GetFloat() });
			myTexts.back()->SetPivot({ buttonData["Text Pivot X"].GetFloat(), buttonData["Text Pivot Y"].GetFloat() });

			data.myPosition = { buttonData["Position X"].GetFloat(), buttonData["Position Y"].GetFloat() };
			data.myDimensions = { buttonData["Pixel Width"].GetFloat(), buttonData["Pixel Height"].GetFloat() };
			data.mySpritePaths.at(0) = buttonData["Idle Sprite Path"].GetString();
			data.mySpritePaths.at(1) = buttonData["Hover Sprite Path"].GetString();
			data.mySpritePaths.at(2) = buttonData["Click Sprite Path"].GetString();

			auto messageDataArray = buttonData["Messages"].GetArray();
			data.myMessagesToSend.resize(messageDataArray.Size());

			for (unsigned int j = 0; j < messageDataArray.Size(); ++j)
			{
				data.myMessagesToSend[j] = static_cast<EMessageType>(messageDataArray[j].GetInt());
			}

			myButtons.emplace_back(new CButton(data, aScene));
		}
	}

	if (document.HasMember("Texts"))
	{
		auto textDataArray = document["Texts"].GetArray();
		for (unsigned int i = 0; i < textDataArray.Size(); ++i)
		{
			auto textData = textDataArray[i].GetObjectW();
			myTexts.emplace_back(new CTextInstance(aScene, addToScene));
			myTexts.back()->Init(CTextFactory::GetInstance()->GetText(textData["FontAndFontSize"].GetString()));
			myTexts.back()->SetText(textData["Text"].GetString());
			myTexts.back()->SetColor({ textData["Color R"].GetFloat(), textData["Color G"].GetFloat(), textData["Color B"].GetFloat(), 1.0f });
			myTexts.back()->SetPosition({ textData["Position X"].GetFloat(), textData["Position Y"].GetFloat() });
			myTexts.back()->SetPivot({ textData["Pivot X"].GetFloat(), textData["Pivot Y"].GetFloat() });
		}
	}

	if (document.HasMember("Animated UI Elements"))
	{
		auto animatedDataArray = document["Animated UI Elements"].GetArray();
		for (unsigned int i = 0; i < animatedDataArray.Size(); ++i)
		{
			myAnimatedUIs.emplace_back(new CAnimatedUIElement(animatedDataArray[i]["Path"].GetString(), aScene, addToScene));
			float x = animatedDataArray[i]["Position X"].GetFloat();
			float y = animatedDataArray[i]["Position Y"].GetFloat();

			float sx = animatedDataArray[i]["Scale X"].GetFloat();
			float sy = animatedDataArray[i]["Scale Y"].GetFloat();
			myAnimatedUIs.back()->SetPosition({ x, y });
			myAnimatedUIs.back()->SetScale({ sx, sy });//Rename scale to size for consistency.
			aScene.AddInstance(myAnimatedUIs.back());
			if (animatedDataArray[i].HasMember("Level")) {
				myAnimatedUIs.back()->Level(animatedDataArray[i]["Level"].GetFloat());
			}
		}
	}

	if (document.HasMember("Background"))
	{
		myBackground = new CSpriteInstance(aScene, addToScene);
		myBackground->Init(CSpriteFactory::GetInstance()->GetSprite(ASSETPATH(document["Background"]["Path"].GetString())));
		myBackground->SetRenderOrder(ERenderOrder::BackgroundLayer);
	}

	if (document.HasMember("Sprites"))
	{
		auto spriteDataArray = document["Sprites"].GetArray();
		for (unsigned int i = 0; i < spriteDataArray.Size(); ++i)
		{
			CSpriteInstance* spriteInstance = new CSpriteInstance(aScene, addToScene);

			Vector2 scale(1.0f, 1.0f);
			if (spriteDataArray[i].HasMember("Scale X"))
				scale.x = spriteDataArray[i]["Scale X"].GetFloat();
			if (spriteDataArray[i].HasMember("Scale Y"))
				scale.y = spriteDataArray[i]["Scale Y"].GetFloat();

			spriteInstance->Init(CSpriteFactory::GetInstance()->GetSprite(ASSETPATH(spriteDataArray[i]["Path"].GetString())), scale);
			mySprites.emplace_back(spriteInstance);
			float x = spriteDataArray[i]["Position X"].GetFloat();
			float y = spriteDataArray[i]["Position Y"].GetFloat();
			mySprites.back()->SetPosition({ x, y });
		}
	}

	if (document.HasMember("PostmasterEvents"))
	{
		auto messageDataArray = document["PostmasterEvents"]["Events"].GetArray();
		myMessageTypes.resize(messageDataArray.Size());

		for (unsigned int j = 0; j < messageDataArray.Size(); ++j)
		{
			myMessageTypes[j] = static_cast<EMessageType>(messageDataArray[j].GetInt());
		}
		SubscribeToMessages();
	}

}

void CCanvas::Update()
{
	if (myButtons.size() <= 0)
		return;

	DirectX::SimpleMath::Vector2 mousePos = { static_cast<float>(Input::GetInstance()->MouseX()), static_cast<float>(Input::GetInstance()->MouseY()) };
	for (unsigned int i = 0; i < myButtons.size(); ++i)
	{
		myButtons[i]->CheckMouseCollision(mousePos);
	}

	if (Input::GetInstance()->IsMousePressed(Input::EMouseButton::Left))
	{
		for (unsigned int i = 0; i < myButtons.size(); ++i)
		{
			myButtons[i]->Click(true, nullptr);
		}
	}

	if (Input::GetInstance()->IsMouseReleased(Input::EMouseButton::Left))
	{
		for (unsigned int i = 0; i < myButtons.size(); ++i)
		{
			myButtons[i]->Click(false, nullptr);
		}
	}
}

void CCanvas::Receive(const SMessage& /*aMessage*/)
{
	//switch (aMessage.myMessageType)
	//{
	//default:
	//	break;
	//}
}

void CCanvas::SubscribeToMessages()
{
	for (auto messageType : myMessageTypes)
	{
		CMainSingleton::PostMaster().Subscribe(messageType, this);
	}
}

void CCanvas::UnsubscribeToMessages()
{
	for (auto messageType : myMessageTypes)
	{
		CMainSingleton::PostMaster().Unsubscribe(messageType, this);
	}
}

bool CCanvas::GetEnabled()
{
	return myIsEnabled;
}

void CCanvas::SetEnabled(bool isEnabled)
{
	if (myIsEnabled != isEnabled)
	{
		myIsEnabled = isEnabled;

		for (auto button : myButtons)
		{
			button->myEnabled = myIsEnabled;
		}

		for (auto sprite : mySprites)
		{
			sprite->SetShouldRender(myIsEnabled);
		}
	}
}
