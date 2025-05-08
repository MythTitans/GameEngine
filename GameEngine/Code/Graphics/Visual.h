#pragma once

#include "Game/Component.h"
#include "Game/ResourceLoader.h"

class VisualComponent : public Component
{
public:
	explicit VisualComponent( Entity* pEntity );

	void					Setup( const char* sModelFile );
	void					Initialize() override;
	bool					IsInitialized() const override;
	void					Update( const GameContext& oGameContext ) override;

	void					DisplayInspector() override;
	void					OnPropertyChanged( const std::string& sProperty ) override;

	const Array< Mesh >&	GetMeshes() const;

private:
	PROPERTIES( VisualComponent );
	PROPERTY( "Model", m_sModelFile, std::string );

	ModelResPtr		m_xModel;
	TechniqueResPtr m_xTechnique;
};
