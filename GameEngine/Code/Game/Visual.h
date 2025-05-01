#pragma once

#include "Component.h"
#include "ResourceLoader.h"

class VisualComponent : public Component
{
public:
	explicit VisualComponent( Entity* pEntity );

	void					Setup( const char* sModelFile );
	void					Initialize() override;
	bool					IsInitialized() const override;
	void					Update( const float fDeltaTime ) override;

	void					DisplayInspector() override;

	const Array< Mesh >&	GetMeshes() const;

private:
	PROPERTIES( VisualComponent );
	PROPERTY( "Model", m_sModelFile, std::string );

	ModelResPtr		m_xModel;
	TechniqueResPtr m_xTechnique;
};
