#pragma once

#include "Bloom.h"
#include "Camera.h"
#include "Core/Common.h"
#include "DebugRenderer.h"
#include "Game/ResourceLoader.h"
#include "GizmoRenderer.h"
#include "RenderTarget.h"
#include "TextRenderer.h"
#include "VisualStructure.h"

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

	void			Render( const RenderContext& oRenderContext );
	void			Clear();

	bool			OnLoading();

	void			DisplayDebug();

	const Texture*	GetDefaultDiffuseMap() const;
	const Texture*	GetDefaultNormalMap() const;

	void			SetTechnique( const Technique& oTechnique );
	void			ClearTechnique();
	void			SetTextureSlot( const Texture& oTexture, const uint uTextureUnit );
	void			ClearTextureSlot( const uint uTextureUnit );
	void			SetRenderTarget( const RenderTarget& oRenderTarget );
	void			ClearRenderTarget();
	void			CopyDepthToBackBuffer( const RenderTarget& oRenderTarget, const RenderRect& oRect );
	void			DrawMesh( const Mesh& oMesh );

	void			RenderScreen();
	void			PresentTexture( const Texture& oTexture );
	void			BlendTextures( const Texture& oTextureA, const Texture& oTextureB );

private:
	enum RenderingMode : uint8
	{
		FORWARD,
		DEFERRED,
		_COUNT
	};

	void			RenderForward( const RenderContext& oRenderContext );
	void			RenderDeferred( const RenderContext& oRenderContext );
	uint64			RenderPicking( const RenderContext& oRenderContext, const int iCursorX, const int iCursorY, const bool bAllowGizmos );
	void			RenderOutline( const RenderContext& oRenderContext, const VisualNode& oVisualNode );
	void			RenderGizmos( const RenderContext& oRenderContext );

public:
	TextRenderer	m_oTextRenderer;
	DebugRenderer	m_oDebugRenderer;
	GizmoRenderer	m_oGizmoRenderer;

	Camera			m_oCamera;

	VisualStructure	m_oVisualStructure;

private:
	RenderTarget	m_oForwardTarget;
	RenderTarget	m_oDeferredTarget;
	RenderTarget	m_oPickingTarget;

	Mesh			m_oRenderMesh;

	TextureResPtr	m_xDefaultDiffuseMap;
	TextureResPtr	m_xDefaultNormalMap;
	TechniqueResPtr	m_xDeferredMaps;
	TechniqueResPtr	m_xDeferredCompose;
	TechniqueResPtr	m_xPresentation;
	TechniqueResPtr	m_xBlend;
	TechniqueResPtr	m_xPicking;
	TechniqueResPtr	m_xOutline;
	TechniqueResPtr	m_xGizmo;

	Bloom			m_oBloom;

	RenderingMode	m_eRenderingMode;
	bool			m_bMSAA;
	bool			m_bSRGB;

	bool			m_bDisplayDebug;
};

extern Renderer* g_pRenderer;