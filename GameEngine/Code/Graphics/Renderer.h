#pragma once

#include "Bloom.h"
#include "Camera.h"
#include "Core/Types.h"
#include "DebugRenderer.h"
#include "Editor/GizmoRenderer.h"
#include "RenderTarget.h"
#include "Road.h"
#include "ShaderBuffer.h"
#include "Skinning.h"
#include "Skybox.h"
#include "Terrain.h"
#include "TextRenderer.h"
#include "VisualStructure.h"

class VisualComponent;
struct GLFWwindow;

inline constexpr uint MAX_DIRECTIONAL_LIGHTS = 4;
inline constexpr uint MAX_POINT_LIGHTS = 128;
inline constexpr uint MAX_SPOT_LIGHTS = 128;
inline constexpr uint DIRECTIONAL_SHADOW_CASCADE_COUNT = 4;

struct RenderRect
{
	RenderRect();

	uint m_uX;
	uint m_uY;
	uint m_uWidth;
	uint m_uHeight;
};

class RenderContext
{
public:
	friend class Renderer;

	float				ComputeAspectRatio() const;

	void				OnFrameBufferResized( const int iWidth, const int iHeight );

	const RenderRect&	GetRenderRect() const { return m_oRenderRect; }

private:
	RenderRect	m_oRenderRect;
};

class GPUMarker
{
public:
	GPUMarker( const char* sName );
	~GPUMarker();
};

struct GPUDirectionalLight
{
	alignas( 16 ) glm::vec3	m_vDirection;
	alignas( 4 ) float		m_fIntensity;
	alignas( 16 ) glm::vec3	m_vColor;
	alignas( 4 ) float		m_fBias;
	alignas( 16 ) glm::mat4 m_mShadowViewProjection[ DIRECTIONAL_SHADOW_CASCADE_COUNT ];
	alignas( 4 ) float		m_aShadowRange[ DIRECTIONAL_SHADOW_CASCADE_COUNT ];
};

struct GPUPointLight
{
	alignas( 16 ) glm::vec3 m_vPosition;
	alignas( 16 ) glm::vec3 m_vColor;
	alignas( 4 ) float		m_fIntensity;
	alignas( 4 ) float		m_fFalloffMinDistance;
	alignas( 4 ) float		m_fFalloffMaxDistance;
};

struct GPUSpotLight
{
	alignas( 16 ) glm::vec3 m_vPosition;
	alignas( 16 ) glm::vec3 m_vDirection;
	alignas( 16 ) glm::vec3 m_vColor;
	alignas( 4 ) float		m_fIntensity;
	alignas( 4 ) float		m_fInnerRange;
	alignas( 4 ) float		m_fOuterRange;
	alignas( 4 ) float		m_fFalloffMinDistance;
	alignas( 4 ) float		m_fFalloffMaxDistance;
};

struct GPULightingDataBlock
{
	GPUDirectionalLight m_aDirectionalLights[ MAX_DIRECTIONAL_LIGHTS ];
	GPUPointLight		m_aPointLights[ MAX_POINT_LIGHTS ];
	GPUSpotLight		m_aSpotLights[ MAX_SPOT_LIGHTS ];
	uint				m_uDirectionalLightCount;
	uint				m_uPointLightCount;
	uint				m_uSpotLightCount;
};

struct RendererStatistics
{
	RendererStatistics();

	uint64	m_uTriangleCount;
	uint64	m_uDrawCallCount;
};

class TextureSlot
{
public:
	TextureSlot();
	TextureSlot( const Texture& oTexture, const uint uSlot, const bool bArray = false );
	~TextureSlot();

	void SetSlot( const Texture& oTexture, const uint uTextureUnit, const bool bArray = false );
	void ClearSlot( const uint uTextureUnit, const bool bArray = false );

private:
	uint m_uSlot;
	bool m_bArray;
};

class CubeMapSlot
{
public:
	CubeMapSlot();
	CubeMapSlot( const CubeMap& oCubeMap, const uint uSlot );
	~CubeMapSlot();

	void SetSlot( const CubeMap& oCubeMap, const uint uTextureUnit );
	void ClearSlot( const uint uTextureUnit );

private:
	uint m_uSlot;
};

class Renderer
{
public:
	friend class Editor;

	template < bool bApplyMaterials >
	friend void DrawNodes( const Array< VisualNode* >& aVisualNodes, Technique& oTechnique, const glm::mat4& mViewProjectionMatrix );

	Renderer();
	~Renderer();

	void						Render( const RenderContext& oRenderContext );
	void						Clear();

	bool						OnLoading();
	void						OnLoaded();

	void						DisplayDebug();

	const Texture*				GetDefaultDiffuseMap() const;
	const Texture*				GetDefaultNormalMap() const;

	void						SetTechnique( const Technique& oTechnique );
	void						ClearTechnique();
	void						SetShaderBufferSlot( const ShaderBufferBase& oBuffer, const uint uBinding );
	void						SetRenderTarget( const RenderTarget& oRenderTarget );
	void						ClearRenderTarget();
	void						DrawMesh( const Mesh& oMesh );

	void						RenderQuad();
	void						BlendTextures( const Texture& oTextureA, const Texture& oTextureB );

	void						CopyRenderTarget( const RenderTarget& oSource, const RenderTarget& oDestination );
	void						CopyRenderTargetColor( const RenderTarget& oSource, const uint uSourceColorIndex, const RenderTarget& oDestination, const uint uDestinationColorIndex );
	void						CopyRenderTargetDepth( const RenderTarget& oSource, const RenderTarget& oDestination );

	const RendererStatistics&	GetStatistics() const;

private:
	enum class RenderingMode : uint8
	{
		FORWARD,
		DEFERRED,
		_COUNT
	};

	enum class MSAALevel : uint8
	{
		MSAA_NONE,
		MSAA_2X,
		MSAA_4X,
		MSAA_8X,
		_COUNT
	};

	void			RenderForward( const RenderContext& oRenderContext );
	void			RenderDeferred( const RenderContext& oRenderContext );
	void			RenderShadowMap();

#ifdef EDITOR
	void			RenderOutline( const RenderContext& oRenderContext, const VisualNode& oVisualNode );
	void			RenderGizmos( const RenderContext& oRenderContext );
#endif

	void			UpdateRenderPipeline( const RenderContext& oRenderContext );

public:
	TextRenderer		m_oTextRenderer;
	DebugRenderer		m_oDebugRenderer;
#ifdef EDITOR
	GizmoRenderer		m_oGizmoRenderer;
#endif

	Camera				m_oCamera;

	VisualStructure		m_oVisualStructure;
	GPUSkinningStorage	m_oGPUSkinningStorage;

private:
	enum class DeferredComposeParam : uint8
	{
		DIFFUSE,
		NORMAL,
		SPECULAR,
		EMISSIVE,
		MATERIAL_ID,
		DEPTH,
		VIEW_POSITION,
		INVERSE_VIEW_PROJECTION,
		_COUNT
	};

	enum class BlendParam : uint8
	{
		TEXTURE_A,
		TEXTURE_B,
		_COUNT
	};

	enum class OutlineParam : uint8
	{
		MODEL,
		MODEL_VIEW_PROJECTION,
		DISPLACEMENT,
		CAMERA_POSITION,
		COLOR,
		SKINNING_OFFSET,
		_COUNT
	};

	enum class GizmoParam : uint8
	{
		MODEL_VIEW_PROJECTION,
		COLOR,
		_COUNT
	};

	RenderTarget							m_oFramebuffer;
	RenderTarget							m_oForwardMSAATarget;
	RenderTarget							m_oForwardTarget;
	RenderTarget							m_oPostProcessTarget;
	RenderTarget							m_oDeferredMapsTarget;
	RenderTarget							m_oDeferredComposeTarget;
	RenderTarget							m_oShadowMapTarget;

	Mesh									m_oRenderMesh;

	ShaderBuffer< GPUSkinningDataBlock >	m_oSkinningBuffer;
	ShaderBuffer< GPUMaterialDataBlock >	m_oMaterialBuffer;
	ShaderBuffer< GPULightingDataBlock >	m_oLightingBuffer;

	TextureResPtr							m_xDefaultDiffuseMap;
	TextureResPtr							m_xDefaultNormalMap;

	TechniqueResPtr							m_xDeferredMaps;
	TechniqueResPtr							m_xDeferredCompose;
	PARAM_SHEET( DeferredComposeParam )		m_oDeferredComposeSheet;
	TechniqueResPtr							m_xBlend;
	PARAM_SHEET( BlendParam )				m_oBlendSheet;
	TechniqueResPtr							m_xOutline;
	PARAM_SHEET( OutlineParam )				m_oOutlineSheet;
	TechniqueResPtr							m_xGizmo;
	PARAM_SHEET( GizmoParam )				m_oGizmoSheet;
	TechniqueResPtr							m_xShadowMap; // TODO #eric use a param sheet at some point ?

	Skybox									m_oSkybox;
	Terrain									m_oTerrain;
	Road									m_oRoad;
	Bloom									m_oBloom;

	RenderingMode							m_eRenderingMode;
	MSAALevel								m_eMSAALevel;
	bool									m_bSRGB;
	bool									m_bEnableFrustumCulling;

	bool									m_bUpdateRenderPipeline;

	bool									m_bDisplayDebug;

	RendererStatistics						m_oStatistics;
};

extern Renderer* g_pRenderer;