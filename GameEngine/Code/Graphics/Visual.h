#pragma once

#include "Game/Component.h"
#include "Game/ResourceLoader.h"

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

#ifdef EDITOR
	void					DisplayInspector() override;
	void					OnPropertyChanged( const std::string& sProperty ) override;
#endif

	const Array< Mesh >&	GetMeshes() const;

private:
	PROPERTIES( VisualComponent );
	PROPERTY( "Model", m_sModelFile, std::string );

	ModelResPtr			m_xModel;
	TechniqueResPtr		m_xTechnique;

	VisualNode*			m_pVisualNode;
};
