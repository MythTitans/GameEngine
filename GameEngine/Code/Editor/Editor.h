#pragma once

#ifdef EDITOR

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

#include "Core/Array.h"
#include "Core/Types.h"
#include "Math/MathUtils.h"
#include "PickingTool.h"
#include "TrenchTool.h"

class Entity;
class GizmoComponent;
class InputContext;
class RenderContext;
class TextureResource;
struct GameContext;

class SnapshotStore
{
public:
	explicit SnapshotStore( const uint uCapacity );

	void			Push( const bool bResetForward = true );
	void			Pop();

	nlohmann::json&	Back();

	uint			BackwardCount() const;
	uint			ForwardCount() const;

private:
	Array< nlohmann::json > m_aSnapshots;

	uint					m_uStart;
	uint					m_uEnd;
	uint					m_uCurrentEnd;
};

class Editor
{
public:
	Editor();
	~Editor();

	bool		OnLoading();
	void		OnLoaded();

	// TODO #eric do this better, only there to take the first snapshot for undo
	void		OnSceneLoaded();

	void		Update( const InputContext& oInputContext, const RenderContext& oRenderContext );
	void		Render( const RenderContext& oRenderContext );

	PickingTool		m_oPickingTool;
	TrenchTool		m_oTrenchTool;

private:
	RayUtil		ComputeCursorViewRay( const InputContext& oInputContext, const RenderContext& oRenderContext ) const;
	glm::vec3	ProjectOnGizmo( const RayUtil& oRay, const GizmoComponent& oGizmo ) const;
	Entity*		DuplicateEntity( const Entity* pEntity, const std::string& sNameSuffix = "_Duplicate", Entity* pForcedParent = nullptr );

	bool		DisplayHierarchy( Entity* pEntity, int iImGuiID );

	void		StoreSnapshot();
	void		RestoreSnapshotBackward();
	void		RestoreSnapshotForward();

	uint64			m_uSelectedEntityID;
	uint64			m_uGizmoEntityID;

	glm::vec3		m_vInitialEntityPosition;
	glm::quat		m_qInitialEntityRotation;

	glm::vec3		m_vMoveStartPosition;
	glm::vec3		m_vRotationAxis;

	nlohmann::json	m_oSceneJson;
	SnapshotStore	m_oSnapshotStore;

	bool			m_bDisplayEditor;

	bool			m_bStoreSnapshot;
};

extern Editor* g_pEditor;

#endif