#pragma once

#include "Game/Component.h"
#include "Game/ResourceLoader.h"
#include "Mesh.h"
#include "Texture.h"

class RenderContext;
struct Sky;

class Skybox
{
public:
	Skybox();

	void Render( const Sky* pSky, const RenderContext& oRenderContext );

	bool OnLoading();
	void OnLoaded();

private:
	enum class SkyboxParam
	{
		VIEW_PROJECTION,
		CUBE_MAP,
		_COUNT
	};

	TechniqueResPtr				m_xSkybox;
	PARAM_SHEET( SkyboxParam )	m_oSkyboxSheet;

	Mesh						m_oMesh;
};

// TODO #eric maybe we could load the cube map only when the skybox is active (in fact the component should give all the textures to the skybox and it should generate the cube map itself)
class SkyboxComponent : public Component
{
public:
	explicit SkyboxComponent( Entity* pEntity );

	void Initialize() override;
	bool IsInitialized() const override;
	void Start() override;
	void Update( const GameContext& oGameContext ) override;
	void Stop() override;
	void Dispose() override;

	void OnPropertyChanged( const std::string& sProperty ) override;

private:
	void LoadCubeMapTextures();
	bool AreCubeMapTexturesReady() const;
	void CreateCubeMap();

	PROPERTIES( SkyboxComponent );
	PROPERTY( "Left", m_sNegativeX, std::string );
	PROPERTY( "Right", m_sPositiveX, std::string );
	PROPERTY( "Top", m_sPositiveY, std::string );
	PROPERTY( "Bottom", m_sNegativeY, std::string );
	PROPERTY( "Front", m_sPositiveZ, std::string );
	PROPERTY( "Back", m_sNegativeZ, std::string );
	PROPERTY_DEFAULT( "Active", m_bActive, bool, false );

	TextureResPtr	m_aTextures[ CubeMapDesc::_COUNT ];
	CubeMap			m_oCubeMap;

	Sky*			m_pSky;
};