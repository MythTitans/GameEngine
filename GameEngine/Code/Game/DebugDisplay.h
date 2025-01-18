#pragma once

#include <string>

#include <glm/glm.hpp>

#include "Core/Array.h"
#include "Graphics/DebugRenderer.h"
#include "Graphics/TextRenderer.h"

class RenderContext;

class DebugDisplay
{
public:
	friend class TextRenderer;

	DebugDisplay();
	~DebugDisplay();

	void NewFrame();
	void Display( const float fDeltaTime, const RenderContext& oRenderContext );
	void DisplayOverlay( const float fDeltaTime, const RenderContext& oRenderContext );

	void DisplayText( const std::string& sText, const glm::vec4& vColor = glm::vec4( 1.f, 1.f, 1.f, 1.f ) );
	void DisplayText( const std::string& sText, const float fTime, const glm::vec4& vColor = glm::vec4( 1.f, 1.f, 1.f, 1.f ) );

	void DisplayLine( const glm::vec3& vFrom, const glm::vec3& vTo, const glm::vec3& vColor );

	void DisplaySphere( const glm::vec3& vPosition, const float fRadius, const glm::vec3& vColor );

private:
	Array< Text >	m_aTexts;
	Array< Text >	m_aTimedTexts;
	Array< float >	m_aTimedTextsRemaining;

	Array< Line >	m_aLines;
	Array< Sphere > m_aSpheres;
};

extern DebugDisplay* g_pDebugDisplay;