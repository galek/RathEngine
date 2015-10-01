#include "pch.h"
#include "AnimationController.h"

#include <assimp/scene.h>   
#include <zlib.h>
#include <queue>

namespace Rath
{
	using namespace std;
	using namespace DirectX::SimpleMath;

	ostream& operator<< (ostream& out, const Bone* pBone)
	{
		pBone->Append(out, "", true);
		return out;
	}

	void Bone::Append(ostream& out, string prefix, bool last) const
	{
		out << prefix;
		if (last)
		{
			out << "\\-";
			prefix += "  ";
		}
		else
		{
			out << "|-";
			prefix += "| ";
		}
		out << Name << endl;

		for (size_t i = 0; i < Children.size(); i++)
			Children[i]->Append(out, prefix, i == Children.size() - 1);
	}

	void Bone::Save(std::ostream& file)
	{
		UINT nsize = (UINT)Name.size();
		file.write((char*)&nsize, sizeof(UINT));// the number of chars
		file.write(Name.c_str(), nsize);// the name of the bone
		file.write((char*)&Offset, sizeof(Offset));// the bone offsets
		file.write((char*)&OriginalLocalTransform, sizeof(OriginalLocalTransform));// original bind pose
		nsize = (UINT)Children.size();// number of children
		file.write((char*)&nsize, sizeof(UINT));// the number of children
		for (auto it : Children)// continue for all children
			it->Save(file);
	}

	template<typename T>
	void KeyList<T>::GetNeighborFrames(FLOAT time, UINT& frame, UINT& nextframe) const
	{
		while (frame < size() - 1)
		{
			if (time < data()[frame + 1].mTime)
			{
				break;
			}
			frame++;
		}
		nextframe = (frame + 1) % size();
	};

	template<typename T>
	void KeyList<T>::Save(std::ostream& file)
	{
		UINT nsize = (UINT)size();
		file.write((char*)&nsize, sizeof(UINT));
		Rath::write_compressed(file, (char*)data(), sizeof(T) * nsize);
	}

	template<typename T>
	void KeyList<T>::Load(std::istream& file)
	{
		UINT nsize = 0;
		file.read((char*)&nsize, sizeof(UINT));// the number of position keys
		resize(nsize);
		Rath::read_compressed(file, (char*)data(), sizeof(T) * nsize);
	};

	void AnimationChannel::Save(std::ostream& file)
	{
		file << Name << std::endl;
		mPositionKeys.Save(file);
		mRotationKeys.Save(file);
		mScalingKeys.Save(file);
	};

	void AnimationChannel::Load(std::istream& file)
	{
		file >> Name;
		mPositionKeys.Load(file);
		mRotationKeys.Load(file);
		mScalingKeys.Load(file);
	};

	// ------------------------------------------------------------------------------------------------
	// Constructor on a given animation. 
	Animation::Animation(const aiAnimation* pAnim) {
		mLastTime = 0.0;
		TicksPerSecond = static_cast<float>(pAnim->mTicksPerSecond != 0.0f ? pAnim->mTicksPerSecond : 30.0f);
		Duration = static_cast<float>(pAnim->mDuration);
		Name = string(pAnim->mName.data, pAnim->mName.length);
		if (Name.size() == 0) Name = "Animation" + to_string(Animation_Indexer);

		Channels.resize(pAnim->mNumChannels);
		for (unsigned int a = 0; a < pAnim->mNumChannels; a++)
		{
			aiNodeAnim* pChannel = pAnim->mChannels[a];

			Channels[a].Name = pChannel->mNodeName.data;
			Channels[a].mPositionKeys.resize(pChannel->mNumPositionKeys);
			for (UINT i = 0; i < pChannel->mNumPositionKeys; i++)
			{
				FLOAT time = (float)pChannel->mPositionKeys[i].mTime;
				Vector3 pos = Vector3((float*)&(pChannel->mPositionKeys[i].mValue));

				Channels[a].mPositionKeys[i].mTime = time;
				Channels[a].mPositionKeys[i].mValue = pos;
			}

			Channels[a].mScalingKeys.resize(pChannel->mNumScalingKeys);
			for (UINT i = 0; i < pChannel->mNumScalingKeys; i++)
			{
				FLOAT time = (float)pChannel->mScalingKeys[i].mTime;
				Vector3 scl = Vector3((float*)&(pChannel->mScalingKeys[i].mValue));

				Channels[a].mScalingKeys[i].mTime = time;
				Channels[a].mScalingKeys[i].mValue = scl;
			}

			Channels[a].mRotationKeys.resize(pChannel->mNumRotationKeys);
			for (UINT i = 0; i < pChannel->mNumRotationKeys; i++)
			{
				FLOAT time = (float)pChannel->mRotationKeys[i].mTime;
				Quaternion rot = Quaternion((float*)&(pChannel->mRotationKeys[i].mValue));

				Channels[a].mRotationKeys[i].mTime = time;
				Channels[a].mRotationKeys[i].mValue = XMVectorRotateLeft(rot, 1);
			}
		}

		array<UINT, 3> zeros = { 0, 0, 0 };
		mLastPositions.resize(pAnim->mNumChannels, zeros);
	}

	unsigned int Animation::GetFrameIndexAt(float ptime){
		// get a [0.f ... 1.f) value by allowing the percent to wrap around 1
		ptime *= TicksPerSecond;

		float time = 0.0f;
		if (Duration > 0.0)
			time = fmod(ptime, Duration);

		float percent = time / Duration;
		if (!PlayAnimationForward) percent = (percent - 1.0f)*-1.0f;// this will invert the percent so the animation plays backwards
		return static_cast<unsigned int>((static_cast<float>(Transforms.size()) * percent));
	}

	void Animation::Save(std::ostream& file)
	{
		UINT nsize = 0;
		file << Name << std::endl;// the size of the animation name
		file.write((char*)&Duration, sizeof(Duration));// the duration
		file.write((char*)&TicksPerSecond, sizeof(TicksPerSecond));// the number of ticks per second
		nsize = (UINT)Channels.size();// number of animation channels,
		file.write((char*)&nsize, sizeof(UINT));// the number animation channels
		for (size_t j(0); j < Channels.size(); j++)
		{// for each channel
			Channels[j].Save(file);
		}
	}

	void Animation::Load(std::istream& file){
		UINT nsize = 0;
		file >> Name;
		file.read((char*)&Duration, sizeof(Duration));// the duration
		file.read((char*)&TicksPerSecond, sizeof(TicksPerSecond));// the number of ticks per second
		file.read((char*)&nsize, sizeof(UINT));// the number animation channels
		Channels.resize(nsize);
		for (size_t j(0); j < Channels.size(); j++)
		{// for each channel
			Channels[j].Load(file);
		}

		array<UINT, 3> zeros = { 0, 0, 0 };
		mLastPositions.resize(Channels.size(), zeros);
	}

	// ------------------------------------------------------------------------------------------------
	// Evaluates the animation tracks for a given time stamp. 
	void Animation::Evaluate(float pTime, map<string, Bone*>& bones) {

		pTime *= TicksPerSecond;

		float time = 0.0f;
		if (Duration > 0.0)
			time = fmod(pTime, Duration);

		// calculate the transformations for each animation channel
		for (unsigned int a = 0; a < Channels.size(); a++){
			const AnimationChannel* channel = &Channels[a];
			auto bonenode = bones.find(channel->Name);

			if (bonenode == bones.end()) {
				continue;
			}

			UINT frame[3];
			UINT nextFrame[3];

			for (size_t i = 0; i < 3; i++)
				frame[i] = (time >= mLastTime) ? mLastPositions[a][i] : 0;

			XMVECTOR presentPosition = XMVectorSet(0, 0, 0, 1);
			XMVECTOR presentRotation = XMVectorSet(0, 0, 0, 1);
			XMVECTOR presentScaling = XMVectorSet(1, 1, 1, 1);

			if (channel->mPositionKeys.size() > 0) {
				channel->mPositionKeys.GetNeighborFrames(time, frame[0], nextFrame[0]);

				const PositionKey& key = channel->mPositionKeys[frame[0]];
				const PositionKey& nextKey = channel->mPositionKeys[nextFrame[0]];

				double diffTime = nextKey.mTime - key.mTime;
				if (diffTime < 0.0)
					diffTime += Duration;
				if (diffTime > 0) {
					float factor = float((time - key.mTime) / diffTime);

					XMVECTOR pos = XMLoadFloat3(&key.mValue);
					XMVECTOR nextpos = XMLoadFloat3(&nextKey.mValue);
					presentPosition = XMVectorLerp(pos, nextpos, factor);
				}
				else {
					presentPosition = XMLoadFloat3(&key.mValue);
				}
			}

			if (channel->mRotationKeys.size() > 0) {
				channel->mRotationKeys.GetNeighborFrames(time, frame[1], nextFrame[1]);

				const RotationKey& key = channel->mRotationKeys[frame[1]];
				const RotationKey& nextKey = channel->mRotationKeys[nextFrame[1]];

				double diffTime = nextKey.mTime - key.mTime;
				if (diffTime < 0.0) diffTime += Duration;
				if (diffTime > 0) {
					float factor = float((time - key.mTime) / diffTime);

					XMVECTOR rot = XMLoadFloat4(&key.mValue);
					XMVECTOR nextrot = XMLoadFloat4(&nextKey.mValue);
					presentRotation = XMQuaternionSlerp(rot, nextrot, factor);
				}
				else
				{
					presentRotation = XMLoadFloat4(&key.mValue);
				}
			}

			if (channel->mScalingKeys.size() > 0) {
				channel->mScalingKeys.GetNeighborFrames(time, frame[2], nextFrame[2]);

				const ScaleKey& key = channel->mScalingKeys[frame[2]];
				const ScaleKey& nextKey = channel->mScalingKeys[nextFrame[2]];

				presentScaling = XMLoadFloat3(&key.mValue);
			}

			for (size_t i = 0; i < 3; i++)
				mLastPositions[a][i] = frame[i];

			XMMATRIX mat = XMMatrixRotationQuaternion(presentRotation);
			mat *= XMMatrixScalingFromVector(presentScaling);
			mat *= XMMatrixTranslationFromVector(presentPosition);
			bonenode->second->LocalTransform = mat;
		}
		mLastTime = time;
	}

	void AnimationController::Clean(){// this should clean everything up 
		CurrentAnimIndex = -1;
		Animations.clear();// clear all animations
		delete Skeleton;// This node will delete all children recursivly
		Skeleton = nullptr;// make sure to zero it out
	}

	void AnimationController::Load(const aiScene* pScene, const vector<aiBone*>& BoneOrderList)
	{// this will build the skeleton based on the scene passed to it and CLEAR EVERYTHING
		Clean();

		if (pScene->mRootNode)
			Skeleton = CreateBoneTree(pScene->mRootNode, nullptr);

#if defined(CONSOLE)
		cout << Skeleton << endl;
#endif

		for (auto bone : BoneOrderList)
		{
			auto found = BonesByName.find(bone->mName.data);
			if (found != BonesByName.end())
			{
				Bones.push_back(found->second);
				found->second->Offset = XMMatrixTranspose(XMLoadFloat4x4((XMFLOAT4X4*)&bone->mOffsetMatrix));
				BonesToIndex[found->first] = (UINT)Bones.size() - 1;
			}
			else
			{
				assert(0);
			}
		}
		Transforms.resize(Bones.size());

		if (!pScene->HasAnimations()) return;

		ExtractAnimations(pScene);

		float timestep = 1.0f / 30.0f;// 30 per second
		for (size_t i(0); i < Animations.size(); i++){// pre calculate the animations
			SetAnimIndex((unsigned int)i);
			float dt = 0;
			for (float ticks = 0; ticks < Animations[i].Duration; ticks += Animations[i].TicksPerSecond / 30.0f){
				dt += timestep;
				Calculate(dt);
				Animations[i].Transforms.push_back(XMMATRIXLIST());
				XMMATRIXLIST & trans = Animations[i].Transforms.back();
				for (size_t a = 0; a < Transforms.size(); ++a){
					XMMATRIX rotationmat = Bones[a]->Offset * Bones[a]->GlobalTransform;
					trans.push_back(rotationmat);
				}
			}
		}
		//ResetUpdateTransforms(Skeleton);
	}

	void AnimationController::Load(const aiScene* pScene)
	{
		size_t Anims = Animations.size();
		ExtractAnimations(pScene);

		float timestep = 1.0f / 30.0f;// 30 per second
		for (size_t i(Anims - 1); i < Animations.size(); i++){// pre calculate the animations
			SetAnimIndex((unsigned int)i);
			float dt = 0;
			for (float ticks = 0; ticks < Animations[i].Duration; ticks += Animations[i].TicksPerSecond / 30.0f){
				dt += timestep;
				Calculate(dt);
				Animations[i].Transforms.push_back(XMMATRIXLIST());
				XMMATRIXLIST & trans = Animations[i].Transforms.back();
				for (size_t a = 0; a < Transforms.size(); ++a){
					XMMATRIX rotationmat = Bones[a]->Offset * Bones[a]->GlobalTransform;
					trans.push_back(rotationmat);
				}
			}
		}
	}

	void AnimationController::Save(std::ostream& file)
	{
		if (Skeleton)
			Skeleton->Save(file);

		UINT nsize = (UINT)Animations.size();
		file.write((char*)&nsize, sizeof(UINT));// the number of animations	
		for (UINT i(0); i < nsize; i++){
			Animations[i].Save(file);
		}

		nsize = (UINT)Bones.size();
		file.write((char*)&nsize, sizeof(UINT));// the number of bones
		for (UINT i(0); i < Bones.size(); i++){
			file << Bones[i]->Name << std::endl;// the name of the bone
		}
	}

	void AnimationController::Load(std::istream& file){
		Clean();// make sure to clear this before writing new data
		Skeleton = LoadSkeleton(file, NULL);
#if defined(CONSOLE)
		cout << Skeleton << endl;
#endif
		UINT nsize = 0;
		file.read((char*)&nsize, sizeof(UINT));// the number of animations
		Animations.resize(nsize);
		for (UINT i(0); i < nsize; i++){
			Animations[i].Load(file);
		}
		for (UINT i(0); i < Animations.size(); i++)
		{// get all the animation names so I can reference them by name and get the correct id
			AnimationNameToId.insert(map<string, UINT>::value_type(Animations[i].Name, i));
		}
		if (Animations.size() >0) CurrentAnimIndex = 0;// set it to the first animation if there are any
		file.read((char*)&nsize, sizeof(UINT));// the number of bones
		Bones.resize(nsize);
		for (UINT i(0); i < Bones.size(); i++){
			string bname;
			file >> bname;
			auto found = BonesByName.find(bname);
			BonesToIndex[found->first] = i;
			Bone* tep = found->second;
			Bones[i] = tep;
		}

		Transforms.resize(Bones.size());
		float timestep = 1.0f / 30.0f;// 30 per second
		for (size_t i(0); i < Animations.size(); i++){// pre calculate the animations
			SetAnimIndex((unsigned int)i);
			float dt = 0;
			for (float ticks = 0; ticks < Animations[i].Duration; ticks += Animations[i].TicksPerSecond / 30.0f){
				dt += timestep;
				Calculate(dt);
				Animations[i].Transforms.push_back(XMMATRIXLIST());
				XMMATRIXLIST & trans = Animations[i].Transforms.back();
				for (size_t a = 0; a < Transforms.size(); ++a){
					XMMATRIX rotationmat = Bones[a]->Offset * Bones[a]->GlobalTransform;
					trans.push_back(rotationmat);
				}
			}
		}
	}

	void AnimationController::Load(fstream& file)
	{
		UINT id = (UINT)Animations.size();
		Animations.resize(id + 1);
		Animations.back().Load(file, BonesToIndex);
		AnimationNameToId.insert(map<string, UINT>::value_type(Animations.back().Name, id));

		float timestep = 1.0f / 30.0f;// 30 per second
		float dt = 0;
		SetAnimIndex(id);
		for (float ticks = 0; ticks < Animations[id].Duration; ticks += Animations[id].TicksPerSecond / 30.0f){
			dt += timestep;
			Calculate(dt);
			Animations[id].Transforms.push_back(XMMATRIXLIST());
			XMMATRIXLIST & trans = Animations[id].Transforms.back();
			for (size_t a = 0; a < Transforms.size(); ++a){
				XMMATRIX rotationmat = Bones[a]->Offset * Bones[a]->GlobalTransform;
				trans.push_back(rotationmat);
			}
		}
	}

	void AnimationController::ExtractAnimations(const aiScene* pScene){
		for (size_t i(0); i < pScene->mNumAnimations; i++){
			Animations.push_back(Animation(pScene->mAnimations[i]));// add the animations
		}
		for (UINT i(0); i < Animations.size(); i++){// get all the animation names so I can reference them by name and get the correct id
			AnimationNameToId.insert(map<string, UINT>::value_type(Animations[i].Name, i));
		}
		CurrentAnimIndex = 0;
		SetAnimation("Idle");
	}

	bool AnimationController::SetAnimation(const string& name){
		auto itr = AnimationNameToId.find(name);
		INT oldindex = CurrentAnimIndex;
		if (itr != AnimationNameToId.end()) CurrentAnimIndex = itr->second;
		return oldindex != CurrentAnimIndex;
	}

	bool AnimationController::SetAnimIndex(INT pAnimIndex){
		if ((size_t)pAnimIndex >= Animations.size()) return false;// no change, or the animations data is out of bounds
		INT oldindex = CurrentAnimIndex;
		CurrentAnimIndex = pAnimIndex;// only set this after the checks for good data and the object was actually inserted
		return oldindex != CurrentAnimIndex;
	}

	// ------------------------------------------------------------------------------------------------
	// Calculates the node transformations for the scene. 
	void AnimationController::Calculate(float pTime){
		if ((CurrentAnimIndex < 0) | ((size_t)CurrentAnimIndex >= Animations.size())) return;// invalid animation

		Animations[CurrentAnimIndex].Evaluate(pTime, BonesByName);
		UpdateTransforms(Skeleton);
	}

	// ------------------------------------------------------------------------------------------------
	// Calculates the bone matrices for the given mesh. 
	void AnimationController::CalcBoneMatrices(){
		for (size_t a = 0; a < Transforms.size(); ++a){
			Transforms[a] = Bones[a]->Offset * Bones[a]->GlobalTransform;
		}
	}

	// ------------------------------------------------------------------------------------------------
	// Recursively creates an internal node structure matching the current scene and animation.
	Bone* AnimationController::CreateBoneTree(aiNode* pNode, Bone* pParent){
		Bone* internalNode = new Bone();// create a node
		internalNode->Name = pNode->mName.data;// get the name of the bone
		internalNode->Parent = pParent; //set the parent, in the case this is theroot node, it will be null

		BonesByName[internalNode->Name] = internalNode;// use the name as a key
		internalNode->LocalTransform = XMMatrixTranspose(XMLoadFloat4x4((XMFLOAT4X4*)&pNode->mTransformation));
		internalNode->OriginalLocalTransform = internalNode->LocalTransform;// a copy saved
		CalculateBoneToWorldTransform(internalNode);

		// continue for all child nodes and assign the created internal nodes as our children
		for (unsigned int a = 0; a < pNode->mNumChildren; a++){// recursivly call this function on all children
			internalNode->Children.push_back(CreateBoneTree(pNode->mChildren[a], internalNode));
		}
		return internalNode;
	}

	// ------------------------------------------------------------------------------------------------
	// Recursively updates the internal node transformations from the given matrix array
	void AnimationController::UpdateTransforms(Bone* pNode) {
		CalculateBoneToWorldTransform(pNode);// update global transform as well
		for (auto it = pNode->Children.begin(); it != pNode->Children.end(); ++it)// continue for all children
			UpdateTransforms(*it);
	}

	// ------------------------------------------------------------------------------------------------
	// Calculates the global transformation matrix for the given internal node
	void AnimationController::CalculateBoneToWorldTransform(Bone* child){
		child->GlobalTransform = child->LocalTransform;
		Bone* parent = child->Parent;
		while (parent){// this will climb the nodes up along through the parents concentating all the matrices to get the Object to World transform, or in this case, the Bone To World transform
			child->GlobalTransform *= parent->LocalTransform;
			parent = parent->Parent;// get the parent of the bone we are working on 
		}
	}

	void AnimationController::ResetUpdateTransforms(Bone* pNode) {
		ResetCalculateBoneToWorldTransform(pNode);// update global transform as well
		for (auto it = pNode->Children.begin(); it != pNode->Children.end(); ++it)// continue for all children
			ResetUpdateTransforms(*it);
	}

	// ------------------------------------------------------------------------------------------------
	// Calculates the global transformation matrix for the given internal node
	void AnimationController::ResetCalculateBoneToWorldTransform(Bone* child){
		child->GlobalTransform = child->OriginalLocalTransform;
		Bone* parent = child->Parent;
		while (parent){// this will climb the nodes up along through the parents concentating all the matrices to get the Object to World transform, or in this case, the Bone To World transform
			child->GlobalTransform *= parent->OriginalLocalTransform;
			parent = parent->Parent;// get the parent of the bone we are working on 
		}
	}


	void AnimationController::SaveSkeleton(std::ostream& file, Bone* parent){
		file << parent->Name;// the name of the bone
		file.write((char*)&parent->Offset, sizeof(parent->Offset));// the bone offsets
		file.write((char*)&parent->OriginalLocalTransform, sizeof(parent->OriginalLocalTransform));// original bind pose
		UINT nsize = (UINT)parent->Children.size();// number of children
		file.write((char*)&nsize, sizeof(UINT));// the number of children
		for (auto it : parent->Children)// continue for all children
			SaveSkeleton(file, it);
	}

	Bone* AnimationController::LoadSkeleton(std::istream& file, Bone* parent){
		Bone* internalNode = new Bone();// create a node
		internalNode->Parent = parent; //set the parent, in the case this is theroot node, it will be null
		UINT nsize = 0;
		file >> internalNode->Name;// the name of the bone
		BonesByName[internalNode->Name] = internalNode;// use the name as a key
		file.read((char*)&internalNode->Offset, sizeof(internalNode->Offset));// the bone offsets
		file.read((char*)&internalNode->OriginalLocalTransform, sizeof(internalNode->OriginalLocalTransform));// original bind pose

		internalNode->LocalTransform = internalNode->OriginalLocalTransform;// a copy saved
		CalculateBoneToWorldTransform(internalNode);

		file.read((char*)&nsize, sizeof(UINT));// the number of children

		// continue for all child nodes and assign the created internal nodes as our children
		for (unsigned int a = 0; a < nsize; a++){// recursivly call this function on all children
			internalNode->Children.push_back(LoadSkeleton(file, internalNode));
		}
		return internalNode;

	}


	string trim(const string& str, const string& whitespace = " \t")
	{
		const auto strBegin = str.find_first_not_of(whitespace);
		if (strBegin == string::npos)
			return ""; // no content

		const auto strEnd = str.find_last_not_of(whitespace);
		const auto strRange = strEnd - strBegin + 1;

		return str.substr(strBegin, strRange);
	}

	queue<string> split(const string &s, char delim)
	{
		queue<std::string> elems;
		string item;

		// use stdlib to tokenize the string
		stringstream ss(s);
		while (getline(ss, item, delim))
			if (!item.empty())
				elems.push(item);

		return elems;
	}

	void Animation::Load(fstream& file, map<string, UINT>& BonesToIndex)
	{
		string line;

		int status = -1;
		const char* stat[] = {
			"translateX",
			"translateY",
			"translateZ",
			"rotateX",
			"rotateY",
			"rotateZ",
			"scaleX",
			"scaleY",
			"scaleZ"
		};
		string bone;
		bool keys = false;
		UINT framecount = 0;

		map<string, AnimationChannel*> channels;

		while (getline(file, line))
		{
			line = trim(line);
			queue<string> slitline = split(trim(line), ' ');
			if (slitline.size() > 0)
			{
				if (slitline.front().compare("endTime") == 0)
				{
					slitline.pop();
					framecount = atoi(slitline.front().c_str());
				}
				else if (slitline.front().compare("anim") == 0)
				{
					slitline.pop();
					slitline.pop();
					status = -1;
					keys = false;
					for (int i = 0; i < 9; i++)
					{
						if (slitline.front().compare(stat[i]) == 0)
						{
							status = i;
							break;
						}
					}
					slitline.pop();
					bone = slitline.front();
					replace(bone.begin(), bone.end(), '-', '_');

					auto it = channels.find(bone);
					if (it == channels.end())
					{
						auto chan = new AnimationChannel();
						chan->Name = bone;
						chan->mPositionKeys.resize(framecount);
						chan->mRotationKeys.resize(framecount);
						chan->mScalingKeys.resize(framecount);
						for (size_t i = 0; i < framecount; i++)
						{
							chan->mPositionKeys[i].mTime = i * 30.0f;
							chan->mPositionKeys[i].mValue = { NAN, NAN, NAN };
							chan->mRotationKeys[i].mTime = i * 30.0f;
							chan->mRotationKeys[i].mValue = { NAN, NAN, NAN, NAN };
							chan->mScalingKeys[i].mTime = i * 30.0f;
							chan->mScalingKeys[i].mValue = { NAN, NAN, NAN };
						}
						channels.insert(map<string, AnimationChannel*>::value_type(bone, chan));
					}
				}
				else if (slitline.front().compare("keys") == 0)
				{
					keys = true;
				}
				else if (keys)
				{
					if (slitline.front().compare("}") == 0)
					{
						keys = false;
					}
					else
					{
						float frame = atoi(slitline.front().c_str()) * 30.0f;

						slitline.pop();
						float value = (float)atof(slitline.front().c_str());

						auto it = channels.find(bone);
						if (it != channels.end())
						{
							auto chan = it->second;
							switch (status)
							{
							case 0:
								for (auto& it : chan->mPositionKeys)
									if (it.mTime == frame)
									{
										it.mValue.x = value;
										break;
									}
								break;
							case 1:
								for (auto& it : chan->mPositionKeys)
									if (it.mTime == frame)
									{
										it.mValue.y = value;
										break;
									}
								break;
							case 2:
								for (auto& it : chan->mPositionKeys)
									if (it.mTime == frame)
									{
										it.mValue.z = value;
										break;
									}
								break;
							case 3:
								for (auto& it : chan->mRotationKeys)
									if (it.mTime == frame)
									{
										it.mValue.x = value;
										break;
									}
								break;
							case 4:
								for (auto& it : chan->mRotationKeys)
									if (it.mTime == frame)
									{
										it.mValue.y = value;
										break;
									}
								break;
							case 5:
								for (auto& it : chan->mRotationKeys)
									if (it.mTime == frame)
									{
										it.mValue.z = value;
										break;
									}
								break;
							case 6:
								for (auto& it : chan->mScalingKeys)
									if (it.mTime == frame)
									{
										it.mValue.x = value;
										break;
									}
								break;
							case 7:
								for (auto& it : chan->mScalingKeys)
									if (it.mTime == frame)
									{
										it.mValue.y = value;
										break;
									}
								break;
							case 8:
								for (auto& it : chan->mScalingKeys)
									if (it.mTime == frame)
									{
										it.mValue.z = value;
										break;
									}
								break;
							}
						}
					}
				}
			}
		}

		TicksPerSecond = 30.0f;
		Duration = framecount * TicksPerSecond;
		Name = "Animation" + to_string(Animation_Indexer);

		Channels.resize(BonesToIndex.size());
		array<UINT, 3> zeros = { 0, 0, 0 };
		mLastPositions.resize(BonesToIndex.size(), zeros);

		for (auto it : BonesToIndex)
		{
			size_t index = it.second;
			Channels[index].Name = it.first;
		}

		for (auto it : channels)
		{
			auto chan = it.second;

			for (auto it = chan->mPositionKeys.begin(); it != chan->mPositionKeys.end();) {
				if ((isnan(it->mValue.x)) &&
					(isnan(it->mValue.y)) &&
					(isnan(it->mValue.z)))
				{
					it = chan->mPositionKeys.erase(it);
				}
				else
				{
					++it;
				}
			}

			float last = 0.f;
			float lasttime = 0.f;
			for (size_t i = 0; i < chan->mPositionKeys.size(); i++)
			{
				if (isnan(chan->mPositionKeys[i].mValue.x))
				{
					size_t j = 0;
					for (j = i + 1; j < chan->mPositionKeys.size(); j++)
					{
						if (!isnan(chan->mPositionKeys[j].mValue.x))
						{
							float diffTime = chan->mPositionKeys[j].mTime - lasttime;
							float factor = float((chan->mPositionKeys[i].mTime - lasttime) / diffTime);
							chan->mPositionKeys[i].mValue.x = (1 - factor)*last + factor*chan->mPositionKeys[j].mValue.x;
							break;
						}
					}
					if (j == chan->mPositionKeys.size())
					{
						for (j = i; j < chan->mPositionKeys.size(); j++)
						{
							chan->mPositionKeys[j].mValue.x = last;
						}
						break;
					}
				}
				else
				{
					last = chan->mPositionKeys[i].mValue.x;
					lasttime = chan->mPositionKeys[i].mTime;
				}
			}

			last = 0.f;
			lasttime = 0.f;
			for (size_t i = 0; i < chan->mPositionKeys.size(); i++)
			{
				if (isnan(chan->mPositionKeys[i].mValue.y))
				{
					size_t j = 0;
					for (j = i + 1; j < chan->mPositionKeys.size(); j++)
					{
						if (!isnan(chan->mPositionKeys[j].mValue.y))
						{
							float diffTime = chan->mPositionKeys[j].mTime - lasttime;
							float factor = float((chan->mPositionKeys[i].mTime - lasttime) / diffTime);
							chan->mPositionKeys[i].mValue.y = (1 - factor)*last + factor*chan->mPositionKeys[j].mValue.y;
							break;
						}
					}
					if (j == chan->mPositionKeys.size())
					{
						for (j = i; j < chan->mPositionKeys.size(); j++)
						{
							chan->mPositionKeys[j].mValue.y = last;
						}
						break;
					}
				}
				else
				{
					last = chan->mPositionKeys[i].mValue.y;
					lasttime = chan->mPositionKeys[i].mTime;
				}
			}

			last = 0.f;
			lasttime = 0.f;
			for (size_t i = 0; i < chan->mPositionKeys.size(); i++)
			{
				if (isnan(chan->mPositionKeys[i].mValue.z))
				{
					size_t j = 0;
					for (j = i + 1; j < chan->mPositionKeys.size(); j++)
					{
						if (!isnan(chan->mPositionKeys[j].mValue.z))
						{
							float diffTime = chan->mPositionKeys[j].mTime - lasttime;
							float factor = float((chan->mPositionKeys[i].mTime - lasttime) / diffTime);
							chan->mPositionKeys[i].mValue.z = (1 - factor)*last + factor*chan->mPositionKeys[j].mValue.z;
							break;
						}
					}
					if (j == chan->mPositionKeys.size())
					{
						for (j = i; j < chan->mPositionKeys.size(); j++)
						{
							chan->mPositionKeys[j].mValue.z = last;
						}
						break;
					}
				}
				else
				{
					last = chan->mPositionKeys[i].mValue.z;
					lasttime = chan->mPositionKeys[i].mTime;
				}
			}

			for (auto it = chan->mRotationKeys.begin(); it != chan->mRotationKeys.end();) {
				if ((isnan(it->mValue.x)) &&
					(isnan(it->mValue.y)) &&
					(isnan(it->mValue.z)))
				{
					it = chan->mRotationKeys.erase(it);
				}
				else
				{
					++it;
				}
			}

			last = 0.f;
			lasttime = 0.f;
			for (size_t i = 0; i < chan->mRotationKeys.size(); i++)
			{
				if (isnan(chan->mRotationKeys[i].mValue.x))
				{
					size_t j = 0;
					for (j = i + 1; j < chan->mRotationKeys.size(); j++)
					{
						if (!isnan(chan->mRotationKeys[j].mValue.x))
						{
							float diffTime = chan->mRotationKeys[j].mTime - lasttime;
							float factor = float((chan->mRotationKeys[i].mTime - lasttime) / diffTime);
							chan->mRotationKeys[i].mValue.x = (1 - factor)*last + factor*chan->mRotationKeys[j].mValue.x;
							break;
						}
					}
					if (j == chan->mRotationKeys.size())
					{
						for (j = i; j < chan->mRotationKeys.size(); j++)
						{
							chan->mRotationKeys[j].mValue.x = last;
						}
						break;
					}
				}
				else
				{
					last = chan->mRotationKeys[i].mValue.x;
					lasttime = chan->mRotationKeys[i].mTime;
				}
			}

			last = 0.f;
			lasttime = 0.f;
			for (size_t i = 0; i < chan->mRotationKeys.size(); i++)
			{
				if (isnan(chan->mRotationKeys[i].mValue.y))
				{
					size_t j = 0;
					for (j = i + 1; j < chan->mRotationKeys.size(); j++)
					{
						if (!isnan(chan->mRotationKeys[j].mValue.y))
						{
							float diffTime = chan->mRotationKeys[j].mTime - lasttime;
							float factor = float((chan->mRotationKeys[i].mTime - lasttime) / diffTime);
							chan->mRotationKeys[i].mValue.y = (1 - factor)*last + factor*chan->mRotationKeys[j].mValue.y;
							break;
						}
					}
					if (j == chan->mRotationKeys.size())
					{
						for (j = i; j < chan->mRotationKeys.size(); j++)
						{
							chan->mRotationKeys[j].mValue.y = last;
						}
						break;
					}
				}
				else
				{
					last = chan->mRotationKeys[i].mValue.y;
					lasttime = chan->mRotationKeys[i].mTime;
				}
			}

			last = 0.f;
			lasttime = 0.f;
			for (size_t i = 0; i < chan->mRotationKeys.size(); i++)
			{
				if (isnan(chan->mRotationKeys[i].mValue.z))
				{
					size_t j = 0;
					for (j = i + 1; j < chan->mRotationKeys.size(); j++)
					{
						if (!isnan(chan->mRotationKeys[j].mValue.z))
						{
							float diffTime = chan->mRotationKeys[j].mTime - lasttime;
							float factor = float((chan->mRotationKeys[i].mTime - lasttime) / diffTime);
							chan->mRotationKeys[i].mValue.z = (1 - factor)*last + factor*chan->mRotationKeys[j].mValue.z;
							break;
						}
					}
					if (j == chan->mRotationKeys.size())
					{
						for (j = i; j < chan->mRotationKeys.size(); j++)
						{
							chan->mRotationKeys[j].mValue.z = last;
						}
						break;
					}
				}
				else
				{
					last = chan->mRotationKeys[i].mValue.z;
					lasttime = chan->mRotationKeys[i].mTime;
				}
			}

			for (size_t i = 0; i < chan->mRotationKeys.size(); i++)
			{
				XMVECTOR vec = XMLoadFloat4(&chan->mRotationKeys[i].mValue);
				vec = XMVectorSwizzle(vec, 2, 1, 0, 3);
				XMVECTOR qt = XMQuaternionRotationRollPitchYawFromVector(vec);
				XMStoreFloat4(&chan->mRotationKeys[i].mValue, qt);
			}

			for (auto it = chan->mScalingKeys.begin(); it != chan->mScalingKeys.end();) {
				if ((isnan(it->mValue.x)) &&
					(isnan(it->mValue.y)) &&
					(isnan(it->mValue.z)))
				{
					it = chan->mScalingKeys.erase(it);
				}
				else
				{
					++it;
				}
			}

			last = 0.f;
			lasttime = 0.f;
			for (size_t i = 0; i < chan->mScalingKeys.size(); i++)
			{
				if (isnan(chan->mScalingKeys[i].mValue.x))
				{
					size_t j = 0;
					for (j = i + 1; j < chan->mScalingKeys.size(); j++)
					{
						if (!isnan(chan->mScalingKeys[j].mValue.x))
						{
							float diffTime = chan->mScalingKeys[j].mTime - lasttime;
							float factor = float((chan->mScalingKeys[i].mTime - lasttime) / diffTime);
							chan->mScalingKeys[i].mValue.x = (1 - factor)*last + factor*chan->mScalingKeys[j].mValue.x;
							break;
						}
					}
					if (j == chan->mScalingKeys.size())
					{
						for (j = i; j < chan->mScalingKeys.size(); j++)
						{
							chan->mScalingKeys[j].mValue.x = last;
						}
						break;
					}
				}
				else
				{
					last = chan->mScalingKeys[i].mValue.x;
					lasttime = chan->mScalingKeys[i].mTime;
				}
			}

			last = 0.f;
			lasttime = 0.f;
			for (size_t i = 0; i < chan->mScalingKeys.size(); i++)
			{
				if (isnan(chan->mScalingKeys[i].mValue.y))
				{
					size_t j = 0;
					for (j = i + 1; j < chan->mScalingKeys.size(); j++)
					{
						if (!isnan(chan->mScalingKeys[j].mValue.y))
						{
							float diffTime = chan->mScalingKeys[j].mTime - lasttime;
							float factor = float((chan->mScalingKeys[i].mTime - lasttime) / diffTime);
							chan->mScalingKeys[i].mValue.y = (1 - factor)*last + factor*chan->mScalingKeys[j].mValue.y;
							break;
						}
					}
					if (j == chan->mScalingKeys.size())
					{
						for (j = i; j < chan->mScalingKeys.size(); j++)
						{
							chan->mScalingKeys[j].mValue.y = last;
						}
						break;
					}
				}
				else
				{
					last = chan->mScalingKeys[i].mValue.y;
					lasttime = chan->mScalingKeys[i].mTime;
				}
			}

			last = 0.f;
			lasttime = 0.f;
			for (size_t i = 0; i < chan->mScalingKeys.size(); i++)
			{
				if (isnan(chan->mScalingKeys[i].mValue.z))
				{
					size_t j = 0;
					for (j = i + 1; j < chan->mScalingKeys.size(); j++)
					{
						if (!isnan(chan->mScalingKeys[j].mValue.z))
						{
							float diffTime = chan->mScalingKeys[j].mTime - lasttime;
							float factor = float((chan->mScalingKeys[i].mTime - lasttime) / diffTime);
							chan->mScalingKeys[i].mValue.z = (1 - factor)*last + factor*chan->mScalingKeys[j].mValue.z;
							break;
						}
					}
					if (j == chan->mScalingKeys.size())
					{
						for (j = i; j < chan->mScalingKeys.size(); j++)
						{
							chan->mScalingKeys[j].mValue.z = last;
						}
						break;
					}
				}
				else
				{
					last = chan->mScalingKeys[i].mValue.z;
					lasttime = chan->mScalingKeys[i].mTime;
				}
			}
			auto found = BonesToIndex.find(chan->Name);
			if (found != BonesToIndex.end())
			{
				size_t index = found->second;
				Channels[index].mPositionKeys.swap(chan->mPositionKeys);
				Channels[index].mRotationKeys.swap(chan->mRotationKeys);
				Channels[index].mScalingKeys.swap(chan->mScalingKeys);
			}
			delete chan;
		}
	}
}