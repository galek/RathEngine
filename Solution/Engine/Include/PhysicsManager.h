#pragma once

#include "ControllerNode.h"

#include "PxPhysicsAPI.h"
#include "extensions\PxDefaultErrorCallback.h"
#include "extensions\PxDefaultCpuDispatcher.h"
#include "extensions\PxDefaultCpuDispatcher.h"

#include "Technique.h"

#include <mutex>

namespace Rath
{
	class PhysicsManager : public physx::PxSimulationEventCallback
	{
	private:
		bool	bTermiated;
		HANDLE	hLoaderThread;
		HANDLE	hSyncEvent;
		static DWORD WINAPI WorkerFunction(LPVOID lpParam);

	protected:
		physx::PxDefaultAllocator		m_Allocator;
		physx::PxDefaultErrorCallback	m_ErrorCallback;
		physx::PxFoundation*			m_Foundation;
		physx::PxPhysics*				m_Physics;
		physx::PxDefaultCpuDispatcher*	m_Dispatcher;
		physx::PxScene*					m_Scene;
		physx::PxControllerManager*		m_ControllerManager;

#if defined(_PROFILE) | defined(_DEBUG)
		physx::PxVisualDebuggerConnection*		m_PVDConnection;
#endif

		std::mutex								m_SceneLock;

		std::vector<ControllerNode*>			m_Nodes;
		std::map<uint32, physx::PxMaterial*>	m_Materials;

		static PhysicsManager*					g_Instance;
	protected:
		void FrameMove(float fElapsedTime);
		void GetDebugMesh(std::vector<DirectX::XMFLOAT3>& vertexbuffer);

		void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count);
		void onWake(physx::PxActor** actors, physx::PxU32 count);
		void onSleep(physx::PxActor** actors, physx::PxU32 count);
		void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count);
		void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs);
	public:
		PhysicsManager();
		~PhysicsManager();

		static PhysicsManager& instance()
		{
			return *g_Instance;
		}

		operator physx::PxPhysics*()
		{
			return m_Physics;
		}

		static void setupFiltering(physx::PxRigidActor* actor, uint32 filterGroup, uint32 filterMask);
		static void setupFiltering(physx::PxShape* shape, uint32 filterGroup, uint32 filterMask);

		void AddMaterial(uint32 id, float staticFriction, float dynamicFriction, float restitution);
		physx::PxMaterial* GetMaterial(uint32 id);

		void AddActor(physx::PxActor* actor);
		void RemoveActor(physx::PxActor* actor);

		void AddNode(ControllerNode* node);
		void RemoveNode(ControllerNode* node);

		physx::PxController*	createController(const physx::PxControllerDesc& desc);
		physx::PxRigidDynamic*	createDynamic(const physx::PxTransform& transform, const physx::PxGeometry& geometry, uint32 material);
		physx::PxRigidStatic*	createStatic(const physx::PxTransform& transform);
	};
};
