#include <string>
#include <map>
#include <vector>

// assimp include files. These three are usually needed.
#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/postprocess.h"
#include "assimp/LogStream.hpp"
#include <fstream>

#include "AnimMathFunc.h"
#include "ModelMath.h"
#include "IronMath.h"

#include "Animator.h"

//#define ANIMATION_DURATION_IN_MILLISECONDS// AS of 2021 02 23 is not used

#define NUM_BONES_PER_VERTEX 4
#define TEMP_FRAMES_PER_SECOND 24.0f//30.0f // Original was 25.0f

#define ANIMATED_AT_FRAMES_PER_SECOND 30.0f//30.0f // Original was 25.0f

#ifdef _DEBUG
#define ANIMATION_SAFE_MODE
#endif
struct VertexBoneDataAnim
{
	uint myIDs[NUM_BONES_PER_VERTEX];
	float myWeights[NUM_BONES_PER_VERTEX];

	VertexBoneDataAnim()
	{
		Reset();
	};

	void Reset()
	{
		memset(myIDs, 0, sizeof(myIDs));
		memset(myWeights, 0, sizeof(myWeights));
	}

	void AddBoneData(uint BoneID, float Weight)
	{
		for (uint i = 0; i < NUM_BONES_PER_VERTEX; i++)
		{
			if (myWeights[i] == 0.0f)
			{
				myIDs[i] = BoneID;
				myWeights[i] = Weight;
				return;
			}
		}
		// Should never get here - more bones than we have space for
		assert(false);
	}
};

struct BoneInfoAnim
{
	aiMatrix4x4 myBoneOffset;
	aiMatrix4x4 myFinalTransformation;
};

#define INVALID_MATERIAL 0xFFFFFFFF;

struct MeshEntry
{
	MeshEntry()
	{
		myNumIndices = 0;
		myBaseVertex = 0;
		myBaseIndex = 0;
		myMaterialIndex = INVALID_MATERIAL;
	}

	unsigned int myNumIndices;
	unsigned int myBaseVertex;
	unsigned int myBaseIndex;
	unsigned int myMaterialIndex;
};


class CAnimationController
{

	friend class IronWroughtImGui::CAnimator;
public:
	CAnimationController();
	~CAnimationController();

	//Create Init Function?

	bool ImportRig(const std::string& anFBXFilePath = "");// Todo: handle in factory
	bool ImportAnimation(const std::string& fileName);
	bool InitFromScene(const aiScene* pScene);
	void LoadBones(uint aMeshIndex, const aiMesh* aMesh);

	// Update functions
	void ReadNodeHeirarchy(
		  const aiScene* aScene
		, float anAnimationTime
		, const aiNode* aNode
		, const aiMatrix4x4& aParentTransform);

	void ReadNodeHeirarchy(
		  const aiScene* aFromScene
		, const aiScene* aToScene
		, float aTickFrom
		, float aTickTo
		, const aiNode* aNodeFrom
		, const aiNode* aNodeTo
		, const aiMatrix4x4& aParentTransform);

	void SetBoneTransforms(std::array<aiMatrix4x4, 64>& aTransformsVector);
	void UpdateAnimationTimes(std::array<SlimMatrix44, 64>& someBones);
	
	uint AnimationCount();

	void Animation0Index(int anIndex) { myAnimIndex0 = anIndex; }
	void Animation1Index(int anIndex) { myAnimIndex1 = anIndex; }
	const uint Animation0Index() { return myAnimIndex0; }
	const uint Animation1Index() { return myAnimIndex1; }

	const float AnimationDurationInSeconds(uint anIndex);


private:
	bool AnimationIndexWithinRange(uint anIndex);
	void UpdateAnimationTimeFrames();

private:
	float myTicks0;
	float myTicks1;

	// Used to set the time it should take to blend from myAnimation1 to myAnimation0
	float myBlendingTime;
	float myBlendingTimeMul;
	float myPlayTime;

	// With a myBlendingTime of 0 myAnimIndex0 is played. Using lerp use: 0.0f, to play.
	uint myAnimIndex0;
	// With a myBlendingTime of > 0 myAnimIndex1 is played. Using lerp use: 1.0f, to play.
	uint myAnimIndex1;

	uint myNumOfBones;
	bool myUpdateBoth;
	bool myTemporary;

	aiVector3D myRotation;

	// Holds the animations that we play. Each animation modifies bonetransforms depending on animation time.
	std::vector<const aiScene*>			myScenes;// was std::vector<const aiScene*>
	aiMatrix4x4							myGlobalInverseTransform;
	std::map<std::string, uint>			myBoneMapping;
	std::vector<MeshEntry>				myEntries;
	std::vector<BoneInfoAnim>			myBoneInfo;
	std::vector<VertexBoneDataAnim>		myMass;
	std::vector<std::string>			myAnimationClipNames;


public:

	uint GetCurrentAnimationIndex0()
	{
		return myAnimIndex0;
	}

	std::vector<std::string>& GetAnimationClipNames() 
	{
		return myAnimationClipNames;
	}

	struct SerializedObject {
		SerializedObject(CAnimationController& data)
		{
			myAnimationTime0 = &data.myTicks0;
			myAnimationTime1 = &data.myTicks1;
			myBlendingTime = &data.myBlendingTime;
			myBlendingTimeMul = &data.myBlendingTimeMul;;
			myPlayTime = &data.myPlayTime;
			myAnim0Index = &data.myAnimIndex0;
			myAnim1Index = &data.myAnimIndex1;
			myNumOfBones = &data.myNumOfBones;
			myUpdateBoth = &data.myUpdateBoth;
			myTemporary = &data.myTemporary;
			myRotation = data.myRotation;
			myAnimations = data.myScenes;
			myGlobalInverseTransform = data.myGlobalInverseTransform;
			myBoneMapping = data.myBoneMapping;
			myEntries = data.myEntries;
			myBoneInfo = data.myBoneInfo;
			myMass = data.myMass;
		}

		float* myAnimationTime0;
		float* myAnimationTime1;
		float* myBlendingTime;
		float* myBlendingTimeMul;
		float* myPlayTime;
		uint* myAnim0Index;
		uint* myAnim1Index;
		uint* myNumOfBones;
		bool* myUpdateBoth;
		bool* myTemporary;
		aiVector3D							myRotation;
		std::vector<const aiScene*>			myAnimations;
		aiMatrix4x4							myGlobalInverseTransform;
		std::map<std::string, uint>			myBoneMapping;
		std::vector<MeshEntry>				myEntries;
		std::vector<BoneInfoAnim>			myBoneInfo;
		std::vector<VertexBoneDataAnim>		myMass;
	};

private:
	/// <summary>
	/// Interpolate for aiQuatKey, aiVectorKey 
	/// </summary>
	/// <param name="time">the current time * ticksPerSecond</param>
	template<class T>
	void Interpolate(T& out, const float time, const uint keyStart, const uint keyEnd, const T* someKeys)
	{
		//const float percent = InvLerp(someKeys[keyStart].mTime, someKeys[keyEnd].mTime, time);
		const float percent = Remap(someKeys[keyStart].mTime, someKeys[keyEnd].mTime, 0.0f, 1.0f, time);
		Assimp::Interpolator<T>()(out, someKeys[keyStart], someKeys[keyEnd], time);
	}


	float Lerp(float a, float b, float t)
	{
		return (1.0f - t) * a + b * t;
	}

	float InvLerp(float a, float b, float v)
	{
		return (v - a) / (b - a);
	}

	float Remap(float inMin, float inMax, float outMin, float outMax, float v)
	{
		float t = InvLerp(inMin, inMax, v);
		return Lerp(outMin, outMax, t);
	}
};