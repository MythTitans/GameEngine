#pragma once

#include "Game/Component.h"
#include "Resource/ResourceLoader.h"

class AnimatorComponent;

class VisualComponent : public Component
{
public:
	explicit VisualComponent( Entity* pEntity );

	void					Setup( const char* sModelFile );
	void					Initialize() override;
	bool					IsInitialized() const override;
	void					Start() override;
	void					Update( const GameContext& oGameContext ) override;
	void					Stop() override;
	void					Dispose() override;

	void					DisplayGizmos( const bool bSelected ) override;
#ifdef EDITOR
	bool					DisplayInspector() override;
	void					OnPropertyChanged( const std::string& sProperty ) override;
#endif

	const Array< Mesh >&	GetMeshes() const;

private:
	void					UpdateModel();
	void					UpdateMaterial();

	PROPERTIES( VisualComponent );
	PROPERTY( "Model", m_sModelFile, std::string );
	PROPERTY( "Material", m_sMaterialFile, std::string );

	ModelResPtr			m_xModel;
	MaterialResPtr		m_xMaterial;
	TechniqueResPtr		m_xTechnique;

	VisualNode*			m_pVisualNode;
	AxisAlignedBox		m_oModelAABB;

	bool				m_bModelDirty;
	bool				m_bMaterialDirty;
};
