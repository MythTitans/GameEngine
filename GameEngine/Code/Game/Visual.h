#pragma once

#include "Component.h"
#include "ResourceLoader.h"

class VisualComponent : public Component
{
public:
	explicit VisualComponent( Entity* pEntity );

	void					Setup( const char* sModelFile );
	void					Initialize() override;
	bool					IsInitialized() override;
	void					Update( const float fDeltaTime ) override;

	void					DisplayInspector() override;

	const Array< Mesh >&	GetMeshes() const;

private:
	std::string		m_sModelFile;
	ModelResPtr		m_xModel;
	TechniqueResPtr m_xTechnique;
};
