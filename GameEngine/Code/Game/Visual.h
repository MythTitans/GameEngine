#pragma once

#include "Component.h"

class VisualComponent : public Component
{
public:
	explicit VisualComponent( Entity* pEntity );

	void					Setup( const char* sModelFile );
	void					Initialize() override;
	bool					IsInitialized() override;
	void					Update( const float fDeltaTime ) override;

	const Array< Mesh >&	GetMeshes() const;

private:
	std::string		m_sModelFile;
	ModelResPtr		m_xModel;
	TechniqueResPtr m_xTechnique;
};
