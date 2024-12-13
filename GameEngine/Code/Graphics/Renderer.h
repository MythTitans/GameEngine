#pragma once

#include "Camera.h"
#include "Core/Common.h"
#include "Core/ResourceLoader.h"
#include "RenderTarget.h"
#include "TechniqueDefinition.h"
#include "TextRenderer.h"

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
	Renderer();
	~Renderer();

	void				Render( const RenderContext& oRenderContext );

	Camera&				GetCamera();
	const Camera&		GetCamera() const;

	TextRenderer&		GetTextRenderer();
	const TextRenderer& GetTextRenderer() const;

	bool				OnLoading();

private:
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
	Mesh						m_oRenderMesh;

	TechniqueResPtr				m_xDeferredMaps;
	TechniqueResPtr				m_xDeferredCompose;
	DeferredMapsDefinition		m_oDeferredMaps;
	DeferredComposeDefinition	m_oDeferredCompose;
};

extern Renderer* g_pRenderer;