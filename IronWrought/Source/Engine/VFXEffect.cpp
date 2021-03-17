#include "stdafx.h"
#include "VFXEffect.h"
#include "RandomNumberGenerator.h"

void SVFXEffect::SpawnParticles(unsigned int anIndex, DirectX::SimpleMath::Vector3& aCameraPosition, CParticleEmitter::SParticleData& someParticleData, const Vector3& aTranslation, const float aScale)
{
	myEmitterTimers[anIndex] += CTimer::Dt();
	if (myEmitterTimers[anIndex] > (1.0f / someParticleData.mySpawnRate) && (myParticleVertices[anIndex].size() < someParticleData.myNumberOfParticles)) {
		myParticleVertices[anIndex].emplace_back(myParticlePools[anIndex].front());
		myParticlePools[anIndex].pop();
		myParticleVertices[anIndex].back().myLifeTime = /*someParticleData.myParticleLifetime*/0.0f + Random(someParticleData.myLifetimeLowerBound, someParticleData.myLifetimeUpperBound);
		myParticleVertices[anIndex].back().myPosition =
		{ ((Random(someParticleData.myOffsetLowerBound.x, someParticleData.myOffsetUpperBound.x)) * (1.0f))
			, ((Random(someParticleData.myOffsetLowerBound.y, someParticleData.myOffsetUpperBound.y)) * (1.0f))
			, ((Random(someParticleData.myOffsetLowerBound.z, someParticleData.myOffsetUpperBound.z)) * (1.0f))
			, 1.0f
		};
		myParticleVertices[anIndex].back().myMovement = someParticleData.myParticleStartDirection;
		myParticleVertices[anIndex].back().myStartMovement = someParticleData.myParticleStartDirection + Random(someParticleData.myDirectionLowerBound, someParticleData.myDirectionUpperBound, 0.0f);
		myParticleVertices[anIndex].back().myEndMovement = someParticleData.myParticleEndDirection + Random(someParticleData.myDirectionLowerBound, someParticleData.myDirectionUpperBound, 0.0f);
		myParticleVertices[anIndex].back().myColor = someParticleData.myParticleStartColor;


		myParticleVertices[anIndex].back().mySize = { someParticleData.myParticleStartSize * aScale, someParticleData.myParticleStartSize * aScale };
		myParticleVertices[anIndex].back().mySquaredDistanceToCamera = Vector3::DistanceSquared({ aTranslation.x, aTranslation.y, aTranslation.z }, aCameraPosition);
		myEmitterTimers[anIndex] -= (1.0f / someParticleData.mySpawnRate);
	}
}

void SVFXEffect::UpdateParticles(unsigned int anIndex, DirectX::SimpleMath::Vector3& aCameraPosition, CParticleEmitter::SParticleData& particleData, const float aScale)
{
	std::vector<unsigned int> indicesOfParticlesToRemove;
	for (UINT i = 0; i < myParticleVertices[anIndex].size(); ++i)
	{

		float quotient = myParticleVertices[anIndex][i].myLifeTime / particleData.myParticleLifetime;
		std::cout << myParticleVertices[anIndex][i].myLifeTime << std::endl;
		myParticleVertices[anIndex][i].myColor = Vector4::Lerp
		(
			particleData.myParticleEndColor,
			particleData.myParticleStartColor,
			quotient
		);

		Vector2 p1 = { 0.0f, 0.0f };//myParticleSizeCurves[anIndex][0];
		Vector2 p2 = { 0.75f, 0.5f };//myParticleSizeCurves[anIndex][1];
		Vector2 p3 = { 0.85f, 1.0f };//myParticleSizeCurves[anIndex][2];
		Vector2 p4 = { 1.0f, 0.5f };//myParticleSizeCurves[anIndex][3];

		float evalutedQuotientOnCatmullRomCurve = Vector2::CatmullRom(p1, p2, p3, p4, quotient).x;
		myParticleVertices[anIndex][i].mySize = Vector2::Lerp
		(
			{ particleData.myParticleStartSize * aScale, particleData.myParticleStartSize * aScale },
			{ particleData.myParticleEndSize * aScale, particleData.myParticleEndSize * aScale },
			evalutedQuotientOnCatmullRomCurve
		);

		myParticleVertices[anIndex][i].myMovement = Vector4::Lerp
		(
			particleData.myParticleEndDirection,
			particleData.myParticleStartDirection,
			quotient
		);

		Vector4 direction = Vector4::Lerp
		(
			myParticleVertices[anIndex][i].myEndMovement,
			myParticleVertices[anIndex][i].myStartMovement,
			quotient
		);

		myParticleVertices[anIndex][i].mySquaredDistanceToCamera = Vector3::DistanceSquared
		(
			{ myParticleVertices[anIndex][i].myPosition.x,
			myParticleVertices[anIndex][i].myPosition.y,
			myParticleVertices[anIndex][i].myPosition.z },
			aCameraPosition
		);

		Vector4 newPosition = direction * particleData.myParticleSpeed * CTimer::Dt() + myParticleVertices[anIndex][i].myPosition;
		myParticleVertices[anIndex][i].myPosition = newPosition;

		if ((myParticleVertices[anIndex][i].myLifeTime += CTimer::Dt()) > particleData.myParticleLifetime) {
			indicesOfParticlesToRemove.emplace_back(i);
		}


	}

	std::sort(indicesOfParticlesToRemove.begin(), indicesOfParticlesToRemove.end(), [](UINT a, UINT b) { return a > b; });
	for (UINT i = 0; i < indicesOfParticlesToRemove.size(); ++i) {
		std::swap(myParticleVertices[anIndex][indicesOfParticlesToRemove[i]], myParticleVertices[anIndex].back());
		myParticlePools[anIndex].push(myParticleVertices[anIndex].back());
		myParticleVertices[anIndex].pop_back();
	}
}

void SVFXEffect::ResetParticles()
{
	for (unsigned int i = 0; i < myParticleEmitters.size(); ++i) {
		myEmitterDelays[i] = myEmitterBaseDelays[i];
		myEmitterDurations[i] = myEmitterBaseDurations[i];
	}
	for (unsigned int i = 0; i < myParticleEmitters.size(); ++i) {
		size_t currentSize = myParticleVertices[i].size();
		for (unsigned int k = 0; k < currentSize; ++k) {
			myParticlePools[i].push(myParticleVertices[i].back());
			myParticleVertices[i].pop_back();
		}
	}
}

void SVFXEffect::Enable()
{
	myIsEnabled = true;

	for (unsigned int i = 0; i < myVFXBases.size(); ++i) {
		myVFXDelays[i] = myVFXBaseDelays[i];
		myVFXDurations[i] = myVFXBaseDurations[i];
	}

	for (unsigned int i = 0; i < myParticleEmitters.size(); ++i) {
		myEmitterDelays[i] = myEmitterBaseDelays[i];
		myEmitterDurations[i] = myEmitterBaseDurations[i];
	}
}

void SVFXEffect::Disable()
{
	myIsEnabled = false;

	for (unsigned int i = 0; i < myVFXBases.size(); ++i) {
		myVFXIsActive[i] = false;
	}

	for (unsigned int i = 0; i < myParticleEmitters.size(); ++i) {
		size_t currentSize = myParticleVertices[i].size();
		for (unsigned int j = 0; j < currentSize; ++j) {
			myParticlePools[i].push(myParticleVertices[i].back());
			myParticleVertices[i].pop_back();
		}
		myEmitterDelays[i] = 0.0f;
		myEmitterDurations[i] = 0.0f;
	}
}
