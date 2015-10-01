#include "pch.h"
#include "PhysicsManager.h"

#include "Assetlibrary.h"

using namespace physx;

namespace Rath
{
	DWORD WINAPI PhysicsManager::WorkerFunction(LPVOID lpParam)
	{
		PhysicsManager* pParent = (PhysicsManager*)lpParam;

		while (!pParent->bTermiated)
		{
			DWORD dwWaitResult = WaitForSingleObject(pParent->hSyncEvent, INFINITE);
		}

		return 0;
	}


		void PhysicsManager::onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) 
		{ 
			PX_UNUSED(constraints); 
			PX_UNUSED(count); 
		}

		void PhysicsManager::onWake(PxActor** actors, PxU32 count) 
		{ 
			PX_UNUSED(actors); 
			PX_UNUSED(count); 
		}

		void PhysicsManager::onSleep(PxActor** actors, PxU32 count) 
		{ 
			PX_UNUSED(actors); 
			PX_UNUSED(count); 
		}

		void PhysicsManager::onTrigger(PxTriggerPair* pairs, PxU32 count) 
		{ 
			PX_UNUSED(pairs); 
			PX_UNUSED(count); 
		}

		void PhysicsManager::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
		{
			PX_UNUSED((pairHeader));
			//if ((pairHeader.actors[0]->userData != nullptr) && 
			//	(pairHeader.actors[1]->userData != nullptr))
			//	std::cout << '1';

			const PxU32 bufferSize = 64;
			PxContactPairPoint contacts[bufferSize];
			for (PxU32 i = 0; i<nbPairs; i++)
			{
				const PxContactPair& cp = pairs[i];
				
				//PxU32 nbContacts = pairs[i].extractContacts(contacts, bufferSize);
				//for (PxU32 j = 0; j < nbContacts; j++)
				//{
				//	PxVec3 point = contacts[j].position;
				//	PxVec3 impulse = contacts[j].impulse;
				//	PxU32 internalFaceIndex0 = contacts[j].internalFaceIndex0;
				//	PxU32 internalFaceIndex1 = contacts[j].internalFaceIndex1;
				//	//...
				//}
			}
		}


	PxFilterFlags ContactReportFilterShader(
		PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		// let triggers through
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlag::eDEFAULT;
		}
		// generate contacts for all that were not filtered above
		pairFlags = PxPairFlag::eCONTACT_DEFAULT;

		// trigger the contact callback for pairs (A,B) where 
		// the filtermask of A contains the ID of B and vice versa.
		if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;

		return PxFilterFlag::eDEFAULT;
	}

	PhysicsManager*	PhysicsManager::g_Instance = nullptr;
	PhysicsManager::PhysicsManager()
	{
		m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);
		m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(), true, nullptr);
		m_Dispatcher = PxDefaultCpuDispatcherCreate(1);

		PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
		sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
		sceneDesc.cpuDispatcher = m_Dispatcher;
		sceneDesc.filterShader = ContactReportFilterShader;// PxDefaultSimulationFilterShader;
		sceneDesc.simulationEventCallback = this;
		sceneDesc.flags |= PxSceneFlag::eREQUIRE_RW_LOCK;

		m_Scene = m_Physics->createScene(sceneDesc);
		m_ControllerManager = PxCreateControllerManager(*m_Scene);

#if defined(_PROFILE) | defined(_DEBUG)
		if (m_Physics->getPvdConnectionManager())
		{
			m_Physics->getVisualDebugger()->setVisualizeConstraints(true);
			m_Physics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_CONTACTS, true);
			m_Physics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES, true);
			m_PVDConnection = PxVisualDebuggerExt::createConnection(m_Physics->getPvdConnectionManager(), "127.0.0.1" , 5425, 10);
		}
		m_Scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
#endif
		m_Materials.emplace(0, m_Physics->createMaterial(0.5f, 0.5f, 0.6f));

		g_Instance = this;
	}


	PhysicsManager::~PhysicsManager()
	{
#if defined(_PROFILE) | defined(_DEBUG)
		if(m_PVDConnection)
			m_PVDConnection->release();
#endif
		for (auto it : m_Materials)
		{
			it.second->release();
		}

		m_ControllerManager->release();
		m_Scene->release();
		m_Dispatcher->release();
		m_Physics->release();
		m_Foundation->release();
	}

	void PhysicsManager::setupFiltering(PxRigidActor* actor, uint32 filterGroup, uint32 filterMask)
	{
		PxFilterData filterData;
		filterData.word0 = (PxU32)filterGroup; // word0 = own ID
		filterData.word1 = (PxU32)filterMask;	// word1 = ID mask to filter pairs that trigger a contact callback;
		const PxU32 numShapes = actor->getNbShapes();
		PxShape** shapes = (PxShape**)malloc(sizeof(PxShape*)*numShapes);
		actor->getShapes(shapes, numShapes);
		for (PxU32 i = 0; i < numShapes; i++)
		{
			PxShape* shape = shapes[i];
			shape->setSimulationFilterData(filterData);
		}
		free(shapes);
	}

	void PhysicsManager::setupFiltering(physx::PxShape* shape, uint32 filterGroup, uint32 filterMask)
	{
		PxFilterData filterData;
		filterData.word0 = (PxU32)filterGroup; // word0 = own ID
		filterData.word1 = (PxU32)filterMask;	// word1 = ID mask to filter pairs that trigger a contact callback;
		shape->setSimulationFilterData(filterData);
	}

	void PhysicsManager::FrameMove(float fElapsedTime)
	{
		fElapsedTime = min(0.1f, fElapsedTime);

		m_SceneLock.lock();
		for (auto it : m_Nodes)
		{
			it->FrameMove(fElapsedTime);
		}
		m_Scene->simulate(fElapsedTime);
		m_Scene->fetchResults(true);
		m_SceneLock.unlock();
	}

	void PhysicsManager::GetDebugMesh(std::vector<XMFLOAT3>& vertexbuffer)
	{
#if defined(_PROFILE) | defined(_DEBUG)
		m_SceneLock.lock();
		const PxRenderBuffer& rb = m_Scene->getRenderBuffer();
		vertexbuffer.reserve(rb.getNbLines() * 2);
		for (PxU32 i = 0; i < rb.getNbLines(); i++)
		{
			const PxDebugLine& line = rb.getLines()[i];
			vertexbuffer.emplace_back(XMFLOAT3(line.pos0.x, line.pos0.y, line.pos0.z));
			vertexbuffer.emplace_back(XMFLOAT3(line.pos1.x, line.pos1.y, line.pos1.z));
		}
		m_SceneLock.unlock();
#endif
	}

	void PhysicsManager::AddMaterial(uint32 id, float staticFriction, float dynamicFriction, float restitution)
	{
		auto it = m_Materials.find(id);
		if (it == m_Materials.end())
		{
			physx::PxMaterial* material = m_Physics->createMaterial(staticFriction, dynamicFriction, restitution);
			m_Materials.emplace(id, material);
		}
	}

	physx::PxMaterial* PhysicsManager::GetMaterial(uint32 id)
	{
		physx::PxMaterial* material = nullptr;
		auto it = m_Materials.find(id);
		if (it != m_Materials.end())
		{
			material = it->second;
		}
		return material;
	}

	void PhysicsManager::AddActor(PxActor* actor)
	{
		m_SceneLock.lock();
		m_Scene->addActor(*actor);
		m_SceneLock.unlock();
	}

	void PhysicsManager::RemoveActor(PxActor* actor)
	{
		m_SceneLock.lock();
		m_Scene->removeActor(*actor);
		m_SceneLock.unlock();
	}

	PxController* PhysicsManager::createController(const PxControllerDesc& desc)
	{
		m_SceneLock.lock();
		PxController* c = m_ControllerManager->createController(desc);
		m_SceneLock.unlock();
		return c;
	}

	PxRigidDynamic* PhysicsManager::createDynamic(const PxTransform& transform, const PxGeometry& geometry, uint32 material)
	{
		physx::PxMaterial* mat = nullptr;
		auto it = m_Materials.find(material);
		if (it != m_Materials.end())
		{
			mat = it->second;
		}
		else
		{
			return nullptr;
		}
		m_SceneLock.lock();
		PxRigidDynamic* d = PxCreateDynamic(*m_Physics, transform, geometry, *mat, 10.0f);
		m_SceneLock.unlock();
		return d;
	}

	physx::PxRigidStatic* PhysicsManager::createStatic(const physx::PxTransform& transform)
	{
		return m_Physics->createRigidStatic(transform);
	}

	void PhysicsManager::AddNode(ControllerNode* node)
	{
		m_Nodes.push_back(node);
	}

	void PhysicsManager::RemoveNode(ControllerNode* node)
	{
		std::remove(m_Nodes.begin(), m_Nodes.end(), node);
	}
}