#pragma once
#include "FileIO.h"
#include "SimpleMath.h"
#include "AdvancedMath.h"

#include <array>
#include <map>

struct aiAnimation;
struct aiScene;
struct aiBone;
struct aiNode;
namespace Rath
{	
	class Bone
	{
	public:
		std::string Name;
		DirectX::SimpleMath::Matrix Offset, LocalTransform, GlobalTransform, OriginalLocalTransform;
		Bone* Parent;
		std::vector<Bone*> Children;

		Bone() :Parent(0){}
		~Bone(){ for (size_t i(0); i < Children.size(); i++) delete Children[i]; }

		friend std::ostream& operator<< (std::ostream& out, const Bone* pBone);

		void Save(std::ostream& file);
	private:
		void Append(std::ostream& out, std::string prefix, bool last) const;
	};
	std::ostream& operator<< (std::ostream& out, const Bone* pBone);


	struct Key
	{
		FLOAT mTime;
	};

	struct PositionKey : Key
	{
		DirectX::SimpleMath::Vector3 mValue;
	};

	struct ScaleKey : Key
	{
		DirectX::SimpleMath::Vector3 mValue;
	};

	struct RotationKey : Key
	{
		DirectX::SimpleMath::Quaternion mValue;
	};

	template<typename T>
	class KeyList : public std::vector < T >
	{
		static_assert(std::is_base_of<Key, T>::value,
			"T must be a descendant of Key"
			);
	public:
		void GetNeighborFrames(FLOAT time, UINT& frame, UINT& nextframe) const;
		void Save(std::ostream& file);
		void Load(std::istream& file);
	};

	class AnimationChannel
	{
	public:
		std::string Name;
		KeyList<PositionKey>	mPositionKeys;
		KeyList<RotationKey>	mRotationKeys;
		KeyList<ScaleKey>		mScalingKeys;

		void Save(std::ostream& file);
		void Load(std::istream& file);
	};

	class Animation
	{
	public:
		Animation() : mLastTime(0.0f), TicksPerSecond(0.0f), Duration(0.0f), PlayAnimationForward(true), Animation_Indexer(0) {}
		Animation(const aiAnimation* pAnim);
		void Evaluate(float pTime, std::map<std::string, Bone*>& bones);
		void Save(std::ostream& file);
		void Load(std::istream& file);
		void Load(std::fstream& file, std::map<std::string, UINT>& BonesToIndex);

		DirectX::XMMATRIXLIST* GetTransforms(float dt){ return &Transforms[GetFrameIndexAt(dt)]; }
		unsigned int GetFrameIndexAt(float time);

		std::string Name;
		UINT Animation_Indexer;// this is only used if an animation has no name. I assigned it Animation + Animation_Indexer
		std::vector<AnimationChannel> Channels;
		bool PlayAnimationForward;// play forward == true, play backward == false
		float mLastTime, TicksPerSecond, Duration;
		std::vector<std::array<UINT, 3>> mLastPositions;
		std::vector<DirectX::XMMATRIXLIST> Transforms;//, QuatTransforms;/** Array to return transformations results inside. */
	};


	class AnimationController
	{
	public:
		AnimationController() : Skeleton(0), CurrentAnimIndex(-1) {}
		~AnimationController(){ Clean(); }

		void Load(const aiScene* pScene, const std::vector<aiBone*>& BoneOrderList);// this must be called to fill the SceneAnimator with valid data
		void Load(const aiScene* pScene);
		void Load(std::istream& file);
		void Load(std::fstream& file);
		void Save(std::ostream& file);

		void Clean();// frees all memory and initializes everything to a default state

		bool HasSkeleton() const { return !Bones.empty(); }// lets the caller know if there is a skeleton present
		// the set animation returns whether the animation changed or is still the same. 
		bool SetAnimIndex(INT pAnimIndex);// this takes an index to set the current animation to
		bool SetAnimation(const std::string& name);// this takes a string to set the animation to, i.e. SetAnimation("Idle");
		// the next two functions are good if you want to change the direction of the current animation. You could use a forward walking animation and reverse it to get a walking backwards
		void PlayAnimationForward() { Animations[CurrentAnimIndex].PlayAnimationForward = true; }
		void PlayAnimationBackward() { Animations[CurrentAnimIndex].PlayAnimationForward = false; }
		//this function will adjust the current animations speed by a percentage. So, passing 100, would do nothing, passing 50, would decrease the speed by half, and 150 increase it by 50%
		void AdjustAnimationSpeedBy(float percent) { Animations[CurrentAnimIndex].TicksPerSecond *= percent / 100.0f; }
		//This will set the animation speed
		void AdjustAnimationSpeedTo(float tickspersecond) { Animations[CurrentAnimIndex].TicksPerSecond = tickspersecond; }
		// get the animationspeed... in ticks per second
		float GetAnimationSpeed() const { return Animations[CurrentAnimIndex].TicksPerSecond; }
		// get the transforms needed to pass to the vertex shader. This will wrap the dt value passed, so it is safe to pass 50000000 as a valid number
		DirectX::XMMATRIXLIST* GetTransforms(float dt){ return Animations[CurrentAnimIndex].GetTransforms(dt); }

		INT GetAnimationIndex() const { return CurrentAnimIndex; }
		std::string GetAnimationName() const { return Animations[CurrentAnimIndex].Name; }
		//GetBoneIndex will return the index of the bone given its name. The index can be used to index directly into the vector returned from GetTransform
		int GetBoneIndex(const std::string& bname){ std::map<std::string, unsigned int>::iterator found = BonesToIndex.find(bname); if (found != BonesToIndex.end()) return found->second; else return -1; }
		//GetBoneTransform will return the matrix of the bone given its name and the time. be careful with this to make sure and send the correct dt. If the dt is different from what the model is currently at, the transform will be off
		DirectX::XMMATRIX GetBoneTransform(float dt, const std::string& bname) { int bindex = GetBoneIndex(bname); if (bindex == -1) return DirectX::XMMatrixIdentity(); return Animations[CurrentAnimIndex].GetTransforms(dt)->data()[bindex]; }
		// same as above, except takes the index
		DirectX::XMMATRIX GetBoneTransform(float dt, unsigned int bindex) { return Animations[CurrentAnimIndex].GetTransforms(dt)->data()[bindex]; }

		Bone* GetBone(const std::string& bname) { return BonesByName.find(bname)->second; };

		std::vector<Animation> Animations;// a vector that holds each animation 
		INT CurrentAnimIndex;/** Current animation index */
		void Calculate(float pTime);
	protected:

		Bone* Skeleton;/** Root node of the internal scene structure */
		std::map<std::string, Bone*> BonesByName;/** Name to node map to quickly find nodes by their name */
		std::map<std::string, UINT> BonesToIndex;/** Name to node map to quickly find nodes by their name */
		std::map<std::string, UINT> AnimationNameToId;// find animations quickly
		std::vector<Bone*> Bones;// DO NOT DELETE THESE when the destructor runs... THEY ARE JUST COPIES!!
		DirectX::XMMATRIXLIST Transforms;// temp array of transfrorms

		void SaveSkeleton(std::ostream& file, Bone* pNode);
		Bone* LoadSkeleton(std::istream& file, Bone* pNode);

		void UpdateTransforms(Bone* pNode);
		void CalculateBoneToWorldTransform(Bone* pInternalNode);/** Calculates the global transformation matrix for the given internal node */

		void ResetUpdateTransforms(Bone* pNode);
		void ResetCalculateBoneToWorldTransform(Bone* pInternalNode);/** Calculates the global transformation matrix for the given internal node */


		void CalcBoneMatrices();

		void ExtractAnimations(const aiScene* pScene);
		Bone* CreateBoneTree(aiNode* pNode, Bone* pParent);// Recursively creates an internal node structure matching the current scene and animation. 

	};
}