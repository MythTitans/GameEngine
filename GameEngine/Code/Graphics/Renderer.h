#pragma once

#include "Camera.h"
#include "Core/Common.h"
#include "Game/ResourceLoader.h"
#include "RenderTarget.h"
#include "TechniqueDefinition.h"
#include "TextRenderer.h"

class VisualComponent;
struct GLFWwindow;

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

class Renderer
{
public:
	friend class Editor;

	Renderer();
	~Renderer();

	void				Render( const RenderContext& oRenderContext );

	Camera&				GetCamera();
	const Camera&		GetCamera() const;

	TextRenderer&		GetTextRenderer();
	const TextRenderer& GetTextRenderer() const;

	bool				OnLoading();

	void				DisplayDebug();

private:
	enum RenderingMode : uint8
	{
		FORWARD,
		DEFERRED,
		_COUNT
	};

	void				RenderForward( const RenderContext& oRenderContext );
	void				RenderDeferred( const RenderContext& oRenderContext );
	uint64				RenderPicking( const RenderContext& oRenderContext, const int iCursorX, const int iCursorY );
	void				RenderOutline( const RenderContext& oRenderContext, const VisualComponent& oObject );

	void				SetTechnique( const Technique& oTechnique );
	void				ClearTechnique();
	void				SetTextureSlot( const Texture& oTexture, const uint uTextureUnit );
	void				ClearTextureSlot( const uint uTextureUnit );
	void				SetRenderTarget( const RenderTarget& oRenderTarget );
	void				ClearRenderTarget();
	void				DrawMesh( const Mesh& oMesh );

	TextRenderer				m_oTextRenderer;
	Camera						m_oCamera;

	RenderTarget				m_oRenderTarget;
	RenderTarget				m_oPickingTarget;

	Mesh						m_oRenderMesh;

	TextureResPtr				m_xDefaultDiffuseMap;
	TextureResPtr				m_xDefaultNormalMap;
	TechniqueResPtr				m_xForwardOpaque;
	TechniqueResPtr				m_xDeferredMaps;
	TechniqueResPtr				m_xDeferredCompose;
	TechniqueResPtr				m_xPicking;
	TechniqueResPtr				m_xOutline;
	ForwardOpaqueDefinition		m_oForwardOpaque;
	DeferredMapsDefinition		m_oDeferredMaps;
	DeferredComposeDefinition	m_oDeferredCompose;
	PickingDefinition			m_oPicking;
	OutlineDefinition			m_oOutline;

	RenderingMode				m_eRenderingMode;

	bool						m_bDisplayDebug;
};

extern Renderer* g_pRenderer;