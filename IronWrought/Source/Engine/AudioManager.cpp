#include "stdafx.h"
#include "AudioManager.h"
#include "Audio.h"
#include "AudioChannel.h"
#include "MainSingleton.h"
#include "PostMaster.h"
#include "JsonReader.h"
#include "RandomNumberGenerator.h"

using namespace rapidjson;

#define CAST(type) { static_cast<unsigned int>(type) }
#define AUDIO_MAX_NR_OF_SFX_FROM_COLLECTION 1

CAudioManager::CAudioManager() 
	: myWrapper() 
	, myCurrentGroundType(EGroundType::Concrete)
{
	SubscribeToMessages();

	// Init Channels
	for (unsigned int i = 0; i < static_cast<unsigned int>(EChannel::Count); ++i) {
		myChannels.push_back(myWrapper.RequestChannel(TranslateEnum(static_cast<EChannel>(i))));
	}

	// Init Sounds
	for (unsigned int i = 0; i < static_cast<unsigned int>(EMusic::Count); ++i)
	{
		myMusicAudio.push_back(myWrapper.RequestSound(GetPath(static_cast<EMusic>(i)), true));
	}

	for (unsigned int i = 0; i < static_cast<unsigned int>(EAmbience::Count); ++i)
	{
		myAmbienceAudio.push_back(myWrapper.RequestSound(GetPath(static_cast<EAmbience>(i)), true));
	}

	for (unsigned int i = 0; i < static_cast<unsigned int>(ESFX::Count); ++i)
	{
		mySFXAudio.push_back(myWrapper.RequestSound(GetPath(static_cast<ESFX>(i))));
	}

	for (unsigned int i = 0; i < static_cast<unsigned int>(ESFXCollection::Count); ++i)
	{
		FillCollection(static_cast<ESFXCollection>(i));
	}

	for (unsigned int i = 0; i < static_cast<unsigned int>(EUI::Count); ++i)
	{
		myUIAudio.push_back(myWrapper.RequestSound(GetPath(static_cast<EUI>(i))));
	}

	for (unsigned int i = 0; i < static_cast<unsigned int>(EResearcherEventVoiceLine::Count); ++i)
	{
		myResearcherEventSounds.push_back(myWrapper.RequestSound(GetPath(static_cast<EResearcherEventVoiceLine>(i))));
	}

	for (unsigned int i = 0; i < static_cast<unsigned int>(EResearcherReactionVoiceLine::Count); ++i)
	{
		FillCollection(static_cast<EResearcherReactionVoiceLine>(i));
	}

	for (unsigned int i = 0; i < static_cast<unsigned int>(ERobotVoiceLine::Count); ++i)
	{
		FillCollection(static_cast<ERobotVoiceLine>(i));
	}

	std::ifstream volumeStream("Json/Audio/AudioVolume.json");
	IStreamWrapper volumeWrapper(volumeStream);
	Document volDoc;
	volDoc.ParseStream(volumeWrapper);
	
	if (volDoc.HasParseError()) { return; }
	
	if (volDoc.HasMember("Ambience"))
	{
		float value = volDoc["Ambience"].GetFloat();
		myChannels[CAST(EChannel::Ambience)]->SetVolume(value);
	}
	if (volDoc.HasMember("Music"))
	{
		float value = volDoc["Music"].GetFloat();
		myChannels[CAST(EChannel::Music)]->SetVolume(value);
	}
	if (volDoc.HasMember("SFX"))
	{
		float value = volDoc["SFX"].GetFloat();
		myChannels[CAST(EChannel::SFX)]->SetVolume(value);
	}
	if (volDoc.HasMember("UI"))
	{
		float value = volDoc["UI"].GetFloat();
		myChannels[CAST(EChannel::UI)]->SetVolume(value);
	}
	if (volDoc.HasMember("ResearcherVoice"))
	{
		float value = volDoc["ResearcherVoice"].GetFloat();
		myChannels[CAST(EChannel::ResearcherVOX)]->SetVolume(value);
	}
	if (volDoc.HasMember("RobotVoice"))
	{
		float value = volDoc["RobotVoice"].GetFloat();
		myChannels[CAST(EChannel::RobotVOX)]->SetVolume(value);
	}
}

CAudioManager::~CAudioManager()
{
	UnsubscribeToMessages();
	// 2021 03 08 After using std::move in Request methods sounds seem to be destroyable. Channels still can't be safely destroyed though
	//for (auto& channel : myChannels)
	//{
	//	delete channel;
	//	channel = nullptr;
	//}
	for (auto& music : myMusicAudio)
	{
		delete music;
		music = nullptr;
	}
	for (auto& ambience : myAmbienceAudio)
	{
		delete ambience;
		ambience = nullptr;
	}
	for (auto& sfx : mySFXAudio)
	{
		delete sfx;
		sfx = nullptr;
	}
	for (auto& ui : myUIAudio)
	{
		delete ui;
		ui = nullptr;
	}
	for (auto& sound : myAirVentStepSounds)
	{
		delete sound;
		sound = nullptr;
	}
	for (auto& sound : myConcreteStepSounds)
	{
		delete sound;
		sound = nullptr;
	}
	for (auto& sound : myResearcherEventSounds)
	{
		delete sound;
		sound = nullptr;
	}
	for (auto& sound : myResearcherReactionsExplosives)
	{
		delete sound;
		sound = nullptr;
	}
	for (auto& sound : myRobotAttackSounds)
	{
		delete sound;
		sound = nullptr;
	}
	for (auto& sound : myRobotDeathSounds)
	{
		delete sound;
		sound = nullptr;
	}
	for (auto& sound : myRobotIdleSounds)
	{
		delete sound;
		sound = nullptr;
	}
	for (auto& sound : myRobotPatrollingSounds)
	{
		delete sound;
		sound = nullptr;
	}
	for (auto& sound : myRobotSearchingSounds)
	{
		delete sound;
		sound = nullptr;
	}
}

void CAudioManager::Receive(const SMessage& aMessage) {
	switch (aMessage.myMessageType)
	{
	// UI
	case EMessageType::UIButtonPress:
	{
		if (myUIAudio.size() >= static_cast<unsigned int>(EUI::ButtonClick))
		{
			myWrapper.Play(myUIAudio[CAST(EUI::ButtonClick)], myChannels[CAST(EChannel::UI)]);
		}
	}break;

	case EMessageType::PlayStepSound:
	{
		if (myCurrentGroundType == EGroundType::Concrete)
		{
			PlayCyclicRandomSoundFromCollection(myConcreteStepSounds, EChannel::SFX, myStepSoundIndices, 1);
		}
		else if (myCurrentGroundType == EGroundType::AirVent)
		{
			PlayCyclicRandomSoundFromCollection(myAirVentStepSounds, EChannel::SFX, myStepSoundIndices, 1);
		}
	}
	break;

	case EMessageType::PlayJumpSound:
	{
		myWrapper.Play(mySFXAudio[CAST(ESFX::Jump)], myChannels[CAST(EChannel::SFX)]);
	}
	break;

	case EMessageType::PlayerTakeDamage:
	{
		myWrapper.Play(mySFXAudio[CAST(ESFX::EnemyHit)], myChannels[CAST(EChannel::SFX)]);
		//myDelayedSFX.push_back({ESFX::EnemyHit, 0.45f});
	}
	break;

	case EMessageType::PlayResearcherReactionExplosives:
	{
		PlayRandomSoundFromCollection(myResearcherReactionsExplosives, EChannel::ResearcherVOX);
	}
	break;

	case EMessageType::PlayRobotAttackSound:
	{
		PlayCyclicRandomSoundFromCollection(myRobotAttackSounds, EChannel::RobotVOX, myAttackSoundIndices, AUDIO_MAX_NR_OF_SFX_FROM_COLLECTION);
	}
	break;

	case EMessageType::PlayRobotDeathSound:
	{
		PlayCyclicRandomSoundFromCollection(myRobotDeathSounds, EChannel::RobotVOX, myDeathSoundIndices, AUDIO_MAX_NR_OF_SFX_FROM_COLLECTION);
	}
	break;

	case EMessageType::PlayRobotIdleSound:
	{
		PlayCyclicRandomSoundFromCollection(myRobotIdleSounds, EChannel::RobotVOX, myIdleSoundIndices, AUDIO_MAX_NR_OF_SFX_FROM_COLLECTION);
	}
	break;

	case EMessageType::PlayRobotPatrolling:
	{
		PlayCyclicRandomSoundFromCollection(myRobotPatrollingSounds, EChannel::RobotVOX, myPatrollingSoundIndices, AUDIO_MAX_NR_OF_SFX_FROM_COLLECTION);
	}
	break;

	case EMessageType::PlayRobotSearching:
	{
		PlayCyclicRandomSoundFromCollection(myRobotSearchingSounds, EChannel::RobotVOX, mySearchingSoundIndices, AUDIO_MAX_NR_OF_SFX_FROM_COLLECTION);
	}
	break;

	case EMessageType::EnemyTakeDamage:
	{
		PlayCyclicRandomSoundFromCollection(myRobotDeathSounds, EChannel::RobotVOX, myDeathSoundIndices, AUDIO_MAX_NR_OF_SFX_FROM_COLLECTION);
		myWrapper.Play(mySFXAudio[CAST(ESFX::EnemyHit)], myChannels[CAST(EChannel::SFX)]);
	}
	break;
	
	case EMessageType::PlayResearcherEvent:
	{
		int index = *static_cast<int*>(aMessage.data);
		myChannels[CAST(EChannel::ResearcherVOX)]->Stop();
		myWrapper.Play(myResearcherEventSounds[index], myChannels[CAST(EChannel::ResearcherVOX)]);
	}
	break;

	case EMessageType::PlaySFX:
	{
		int index = *static_cast<int*>(aMessage.data);
		myWrapper.Play(mySFXAudio[index], myChannels[CAST(EChannel::SFX)]);
	}
	break;

	case EMessageType::GravityGlovePull:
	{
		bool release = false;
		if (aMessage.data)
			release = *static_cast<bool*>(aMessage.data);
		if (release)
			myWrapper.Play(mySFXAudio[CAST(ESFX::GravityGlovePullRelease)], myChannels[CAST(EChannel::SFX)]);
		else
			myWrapper.Play(mySFXAudio[CAST(ESFX::GravityGlovePullHit)], myChannels[CAST(EChannel::SFX)]);
	}break;

	case EMessageType::GravityGlovePush:
	{
		myWrapper.Play(mySFXAudio[CAST(ESFX::GravityGlovePush)], myChannels[CAST(EChannel::SFX)]);
	}break;

	case EMessageType::EnemyAttackState:
	{
		// Todo add max 3 to be playable at the same time. Can be done inside PlayRandomSoundFromCollection
		// 
		PlayCyclicRandomSoundFromCollection(myRobotAttackSounds, EChannel::RobotVOX, myAttackSoundIndices, AUDIO_MAX_NR_OF_SFX_FROM_COLLECTION);
	}break;

	case EMessageType::EnemyPatrolState:
	{
		PlayCyclicRandomSoundFromCollection(myRobotPatrollingSounds, EChannel::RobotVOX, myPatrollingSoundIndices, AUDIO_MAX_NR_OF_SFX_FROM_COLLECTION);
	}break;

	case EMessageType::EnemySeekState:
	{
		PlayCyclicRandomSoundFromCollection(myRobotAttackSounds, EChannel::RobotVOX, myAttackSoundIndices, AUDIO_MAX_NR_OF_SFX_FROM_COLLECTION);
	}break;

	//// VOICELINES
	//case EMessageType::PlayVoiceLine:
	//{
	//	std::string message = static_cast<char*>(aMessage.data);
	//	message.append("20");
	//	//if (!myVoicelineAudio.empty()) {
	//	//	int index = *static_cast<int*>(aMessage.data);
	//	//	myChannels[CAST(EChannel::VOX)]->Stop();
	//	//	myWrapper.Play(myVoicelineAudio[index], myChannels[CAST(EChannel::VOX)]);
	//	//}
	//}break;

	//case EMessageType::StopDialogue:
	//{
	//	//if (!myVoicelineAudio.empty()) {
	//	//	myChannels[CAST(EChannel::VOX)]->Stop();
	//	//}
	//}break;

	case EMessageType::StartGame:
	{
		std::string scene = *reinterpret_cast<std::string*>(aMessage.data);
		if (strcmp(scene.c_str(), "Level_1-1") == 0)
		{
			myChannels[CAST(EChannel::Ambience)]->Stop();
			myWrapper.Play(myAmbienceAudio[CAST(EAmbience::Inside)], myChannels[CAST(EChannel::Ambience)]);
			return;
		}

		if (strcmp(scene.c_str(), "Level_1-2") == 0)
		{
			myChannels[CAST(EChannel::Ambience)]->Stop();
			myWrapper.Play(myAmbienceAudio[CAST(EAmbience::Inside)], myChannels[CAST(EChannel::Ambience)]);
			return;
		}

		if (strcmp(scene.c_str(), "Level_2-1") == 0)
		{
			myChannels[CAST(EChannel::Ambience)]->Stop();
			myWrapper.Play(myAmbienceAudio[CAST(EAmbience::Outside)], myChannels[CAST(EChannel::Ambience)]);
			return;
		}

		if (strcmp(scene.c_str(), "Level_2-2") == 0)
		{
			myChannels[CAST(EChannel::Ambience)]->Stop();
			myWrapper.Play(myAmbienceAudio[CAST(EAmbience::Outside)], myChannels[CAST(EChannel::Ambience)]);
			return;
		}
	}break;

	case EMessageType::BootUpState:
	{
		myWrapper.Play(myAmbienceAudio[CAST(EAmbience::Inside)], myChannels[CAST(EChannel::Ambience)]);
		//myWrapper.Play(myRes)
	}break;

	default: break;
	}
}

void CAudioManager::Receive(const SStringMessage& aMessage)
{
	if (strcmp(aMessage.myMessageType, "Level_1-1") == 0)
	{
		myChannels[CAST(EChannel::Ambience)]->Stop();
		myWrapper.Play(myAmbienceAudio[CAST(EAmbience::Inside)], myChannels[CAST(EChannel::Ambience)]);
		return;
	}
	
	if (strcmp(aMessage.myMessageType, "Level_1-2") == 0)
	{
		myChannels[CAST(EChannel::Ambience)]->Stop();
		myWrapper.Play(myAmbienceAudio[CAST(EAmbience::Inside)], myChannels[CAST(EChannel::Ambience)]);
		return;
	}
	
	if (strcmp(aMessage.myMessageType, "Level_2-1") == 0)
	{
		myChannels[CAST(EChannel::Ambience)]->Stop();
		myWrapper.Play(myAmbienceAudio[CAST(EAmbience::Outside)], myChannels[CAST(EChannel::Ambience)]);
		return;
	}

	if (strcmp(aMessage.myMessageType, "Level_2-2") == 0)
	{
		myChannels[CAST(EChannel::Ambience)]->Stop();
		myWrapper.Play(myAmbienceAudio[CAST(EAmbience::Outside)], myChannels[CAST(EChannel::Ambience)]);
		return;
	}
}

void CAudioManager::Update()
{
	if (myDelayedSFX.size() > 0)
	{
		const float dt = CTimer::Dt();

		for (auto it = myDelayedSFX.begin(); it != myDelayedSFX.end();)
		{
			it->myTimer -= dt;
			if (it->myTimer <= 0.0f)
			{
				myWrapper.Play(mySFXAudio[CAST(it->mySFX)], myChannels[CAST(EChannel::SFX)]);
				it = myDelayedSFX.erase(it);
				continue;
			}
			++it;
		}
	}
}

void CAudioManager::SubscribeToMessages()
{
	CMainSingleton::PostMaster().Subscribe(EMessageType::UIButtonPress, this);
	CMainSingleton::PostMaster().Subscribe(EMessageType::PlayResearcherReactionExplosives, this);
	CMainSingleton::PostMaster().Subscribe(EMessageType::PlayRobotAttackSound, this);
	CMainSingleton::PostMaster().Subscribe(EMessageType::PlayRobotDeathSound, this);
	CMainSingleton::PostMaster().Subscribe(EMessageType::PlayRobotIdleSound, this);
	CMainSingleton::PostMaster().Subscribe(EMessageType::PlayRobotPatrolling, this);
	CMainSingleton::PostMaster().Subscribe(EMessageType::PlayRobotSearching, this);
	CMainSingleton::PostMaster().Subscribe(EMessageType::PlayResearcherEvent, this);
	CMainSingleton::PostMaster().Subscribe(EMessageType::PlaySFX, this);

	// Player & Gravity Glove 
	CMainSingleton::PostMaster().Subscribe(EMessageType::PlayStepSound, this);
	CMainSingleton::PostMaster().Subscribe(EMessageType::PlayJumpSound, this);
	CMainSingleton::PostMaster().Subscribe(EMessageType::PlayerTakeDamage, this);
	CMainSingleton::PostMaster().Subscribe(EMessageType::GravityGlovePull, this);
	CMainSingleton::PostMaster().Subscribe(EMessageType::GravityGlovePush, this);

	// Enemy
	CMainSingleton::PostMaster().Subscribe(EMessageType::EnemyPatrolState, this);
	CMainSingleton::PostMaster().Subscribe(EMessageType::EnemySeekState, this);
	CMainSingleton::PostMaster().Subscribe(EMessageType::EnemyAttackState, this);
	CMainSingleton::PostMaster().Subscribe(EMessageType::EnemyTakeDamage, this);

	//CMainSingleton::PostMaster().Subscribe(EMessageType::PlayVoiceLine, this);
	//CMainSingleton::PostMaster().Subscribe(EMessageType::StopDialogue, this);
	
	// Ambience
	CMainSingleton::PostMaster().Subscribe(EMessageType::StartGame, this);
	CMainSingleton::PostMaster().Subscribe("Level_1-1", this);
	CMainSingleton::PostMaster().Subscribe("Level_1-2", this);
	CMainSingleton::PostMaster().Subscribe("Level_2-1", this);
	CMainSingleton::PostMaster().Subscribe("Level_2-2", this);

	CMainSingleton::PostMaster().Subscribe(EMessageType::BootUpState, this);
}

void CAudioManager::UnsubscribeToMessages()
{
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::UIButtonPress, this);
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::PlayResearcherReactionExplosives, this);
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::PlayRobotAttackSound, this);
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::PlayRobotDeathSound, this);
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::PlayRobotIdleSound, this);
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::PlayRobotPatrolling, this);
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::PlayRobotSearching, this);
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::PlayResearcherEvent, this);
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::PlaySFX, this);

	// Player & Gravity Glove 
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::PlayStepSound, this);
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::PlayJumpSound, this);
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::PlayerTakeDamage, this);
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::GravityGlovePull, this);
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::GravityGlovePush, this);

	// Enemy
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::EnemyPatrolState, this);
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::EnemySeekState, this);
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::EnemyAttackState, this);
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::EnemyTakeDamage, this);

	//CMainSingleton::PostMaster().Unsubscribe(EMessageType::PlayVoiceLine, this);
	//CMainSingleton::PostMaster().Unsubscribe(EMessageType::StopDialogue, this);

	// Ambience
	CMainSingleton::PostMaster().Unsubscribe(EMessageType::StartGame, this);
	CMainSingleton::PostMaster().Unsubscribe("Level_1-1", this);
	CMainSingleton::PostMaster().Unsubscribe("Level_1-2", this);
	CMainSingleton::PostMaster().Unsubscribe("Level_2-1", this);
	CMainSingleton::PostMaster().Unsubscribe("Level_2-2", this);

	CMainSingleton::PostMaster().Unsubscribe(EMessageType::BootUpState, this);
}

std::string CAudioManager::GetPath(EMusic type) const
{
	std::string path = myMusicPath;
	path.append(TranslateEnum(type));
	path.append(".mp3");
	return path;
}

std::string CAudioManager::GetPath(EAmbience type) const
{
	std::string path = myAmbiencePath;
	path.append(TranslateEnum(type));
	path.append(".mp3");
	return path;
}

std::string CAudioManager::GetPath(ESFX type) const
{
	std::string path = mySFXPath;
	path.append(TranslateEnum(type));
	path.append(".mp3");
	return path;
}

std::string CAudioManager::GetPath(EUI type) const
{
	std::string path = myUIPath;
	path.append(TranslateEnum(type));
	path.append(".mp3");
	return path;
}

std::string CAudioManager::GetPath(EResearcherEventVoiceLine type) const
{
	std::string path = myVoxPath;
	path.append(TranslateEnum(type));
	path.append(".mp3");
	return path;
}

//std::string CAudioManager::GetPath(EResearcherReactionVoiceLine type) const
//{
//	std::string path = myVoxPath;
//	path.append(TranslateEnum(type));
//	path.append(".mp3");
//	return path;
//}


//std::string CAudioManager::GetPath(ERobotVoiceLine type) const
//{
//	std::string path = myVoxPath;
//	path.append(TranslateEnum(type));
//	path.append(".mp3");
//	return path;
//}

std::string CAudioManager::TranslateEnum(EChannel enumerator) const
{
	switch (enumerator)
	{
	case EChannel::Music:
		return "Music";
	case EChannel::Ambience:
		return "Ambience";
	case EChannel::SFX:
		return "SFX";
	case EChannel::UI:
		return "UI";
	case EChannel::ResearcherVOX:
		return "ResearcherVOX";
	case EChannel::RobotVOX:
		return "RobotVOX";
	default:
		return "";
	}
}

std::string CAudioManager::TranslateEnum(EMusic /*enumerator*/) const
{
	//switch (enumerator) {
	//default:
	//	return "";
	//}
	return "DE";
}

std::string CAudioManager::TranslateEnum(EAmbience enumerator) const {
	switch (enumerator)
	{
	case EAmbience::AirVent:
		return "AirVent";
	case EAmbience::Inside:
		return "Inside";
	case EAmbience::Outside:
		return "Outside";
	default:
		return "";
	}
}
std::string CAudioManager::TranslateEnum(ESFX enumerator) const {
	switch (enumerator)
	{
	case ESFX::GravityGlovePullBuildup:
		return "GravityGlovePullBuildup";
	case ESFX::GravityGlovePullHit:
		return "GravityGlovePullHit";
	case ESFX::GravityGlovePush:
		return "GravityGlovePush";
	case ESFX::GravityGlovePullRelease:
		return "GravityGlovePullRelease";
	case ESFX::Jump:
		return "Jump";
	case ESFX::EnemyHit:
		return "EnemyHit";
	case ESFX::SwitchPress:
		return "SwitchPress";
	case ESFX::PickupGravityGlove:
		return "PickupGravityGlove";
	case ESFX::PickupHeal:
		return "PickupHeal";
	default:
		return "";
	}
}
std::string CAudioManager::TranslateEnum(ESFXCollection enumerator) const
{
	switch (enumerator)
	{
	case ESFXCollection::StepAirVent:
		return "StepAirVent";
	case ESFXCollection::StepConcrete:
		return "StepConcrete";
	default:
		return "";
	}
}
std::string CAudioManager::TranslateEnum(EUI enumerator) const {
	switch (enumerator)
	{
	case EUI::ButtonClick:
		return "ButtonClick";
	case EUI::PlayClick:
		return "PlayClick";
	default:
		return "";
	}
}
std::string CAudioManager::TranslateEnum(EResearcherEventVoiceLine enumerator) const
{
	switch (enumerator)
	{
	case EResearcherEventVoiceLine::ResearcherDoorEventVerticalSlice:
		return "ResearcherDoorEventVerticalSlice";
	case EResearcherEventVoiceLine::ResearcherIntroVerticalSlice:
		return "ResearcherIntroVerticalSlice";
	default:
		return "";
	}
}
std::string CAudioManager::TranslateEnum(EResearcherReactionVoiceLine enumerator) const
{
	switch (enumerator) 
	{
	case EResearcherReactionVoiceLine::ResearcherReactionExplosives:
		return "ResearcherReactionExplosives";
	default:
		return "";	
	}
}
std::string CAudioManager::TranslateEnum(ERobotVoiceLine enumerator) const
{
	switch (enumerator)
	{
	case ERobotVoiceLine::RobotAttack:
		return "RobotAttack";
	case ERobotVoiceLine::RobotDeath:
		return "RobotDeath";
	case ERobotVoiceLine::RobotIdle:
		return "RobotIdle";
	case ERobotVoiceLine::RobotPatrolling:
		return "RobotPatrolling";
	case ERobotVoiceLine::RobotSearching:
		return "RobotSearching";
	default:
		return "";
	}
}

void CAudioManager::FillCollection(ESFXCollection enumerator)
{
	unsigned int counter = 0;

	switch (enumerator)
	{
	case ESFXCollection::StepAirVent:
	{
		CAudio* sound = myWrapper.TryGetSound(mySFXPath + GetCollectionPath(enumerator, ++counter));

		while (sound != nullptr)
		{
			myAirVentStepSounds.push_back(sound);
			sound = myWrapper.TryGetSound(mySFXPath + GetCollectionPath(enumerator, ++counter));
		}
	}
		break;
	case ESFXCollection::StepConcrete:
	{
		CAudio* sound = myWrapper.TryGetSound(mySFXPath + GetCollectionPath(enumerator, ++counter));

		while (sound != nullptr)
		{
			myConcreteStepSounds.push_back(sound);
			sound = myWrapper.TryGetSound(mySFXPath + GetCollectionPath(enumerator, ++counter));
		}
	}
		break;
	default:
		break;
	}
}

void CAudioManager::FillCollection(EResearcherReactionVoiceLine enumerator)
{
	unsigned int counter = 0;

	switch (enumerator)
	{
	case EResearcherReactionVoiceLine::ResearcherReactionExplosives:
	{
		CAudio* sound = myWrapper.TryGetSound(myVoxPath + GetCollectionPath(enumerator, ++counter));
		
		while (sound != nullptr)
		{
			myResearcherReactionsExplosives.push_back(sound);
			sound = myWrapper.TryGetSound(myVoxPath + GetCollectionPath(enumerator, ++counter));
		}
	}
		break;
	default:
		break;
	}
}

void CAudioManager::FillCollection(ERobotVoiceLine enumerator)
{
	unsigned int counter = 0;

	switch (enumerator)
	{
	case ERobotVoiceLine::RobotAttack:
	{
		CAudio* sound = myWrapper.TryGetSound(myVoxPath + GetCollectionPath(enumerator, ++counter));
		
		while (sound != nullptr)
		{
			myRobotAttackSounds.push_back(sound);
			sound = myWrapper.TryGetSound(myVoxPath + GetCollectionPath(enumerator, ++counter));
		}
	}
	break;

	case ERobotVoiceLine::RobotDeath:
	{
		CAudio* sound = myWrapper.TryGetSound(myVoxPath + GetCollectionPath(enumerator, ++counter));

		while (sound != nullptr)
		{
			myRobotDeathSounds.push_back(sound);
			sound = myWrapper.TryGetSound(myVoxPath + GetCollectionPath(enumerator, ++counter));
		}
	}
	break;

	case ERobotVoiceLine::RobotIdle:
	{
		CAudio* sound = myWrapper.TryGetSound(myVoxPath + GetCollectionPath(enumerator, ++counter));

		while (sound != nullptr)
		{
			myRobotIdleSounds.push_back(sound);
			sound = myWrapper.TryGetSound(myVoxPath + GetCollectionPath(enumerator, ++counter));
		}
	}
	break;

	case ERobotVoiceLine::RobotPatrolling:
	{
		CAudio* sound = myWrapper.TryGetSound(myVoxPath + GetCollectionPath(enumerator, ++counter));

		while (sound != nullptr)
		{
			myRobotPatrollingSounds.push_back(sound);
			sound = myWrapper.TryGetSound(myVoxPath + GetCollectionPath(enumerator, ++counter));
		}
	}
	break;

	case ERobotVoiceLine::RobotSearching:
	{
		CAudio* sound = myWrapper.TryGetSound(myVoxPath + GetCollectionPath(enumerator, ++counter));

		while (sound != nullptr)
		{
			myRobotSearchingSounds.push_back(sound);
			sound = myWrapper.TryGetSound(myVoxPath + GetCollectionPath(enumerator, ++counter));
		}
	}
	break;

	default:
		break;
	}
}

void CAudioManager::PlayRandomSoundFromCollection(const std::vector<CAudio*>& aCollection, const EChannel& aChannel, const int& aMaxNrOfChannelsActive)
{
	if (aCollection.empty())
		return;

	//std::cout << __FUNCTION__ << " " << myChannels[CAST(aChannel)]->PlayCount() << std::endl;
	if(myChannels[CAST(aChannel)]->PlayCount() > aMaxNrOfChannelsActive)
	   return;

	unsigned int randomIndex = Random(0, static_cast<int>(aCollection.size()) - 1);
	myWrapper.Play(aCollection[randomIndex], myChannels[CAST(aChannel)]);
}

void CAudioManager::PlayCyclicRandomSoundFromCollection(const std::vector<CAudio*>& aCollection, const EChannel& aChannel, std::vector<int>& someCollectionIndices, const int& aMaxNrOfChannelsActive)
{
	if (aCollection.empty())
		return;

	if (myChannels[CAST(aChannel)]->PlayCount() > aMaxNrOfChannelsActive)
		return;

	unsigned int randomIndex = Random(0, static_cast<int>(aCollection.size()) - 1, someCollectionIndices);
	myWrapper.Play(aCollection[randomIndex], myChannels[CAST(aChannel)]);
}
