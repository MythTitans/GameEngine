#pragma once

#include <string>

#include <glm/glm.hpp>

#include "Core/Array.h"
#include "Graphics/DebugRenderer.h"
#include "Graphics/TextRenderer.h"

class RenderContext;

template < typename Shape >
struct DebugShapeHolder
{
	void Add( const Shape& oShape, const bool bDepth )
	{
		if( bDepth )
			m_aShapes.PushBack( oShape );
		else
			m_aShapesNoDepth.PushBack( oShape );
	}

	void Clear()
	{
		m_aShapes.Clear();
		m_aShapesNoDepth.Clear();
	}

	Array< Shape > m_aShapes;
	Array< Shape > m_aShapesNoDepth;
};

class DebugDisplay
{
public:
	friend class TextRenderer;

	DebugDisplay();
	~DebugDisplay();

	void NewFrame();
	void Display( const RenderContext& oRenderContext );
	void DisplayOverlay( const float fDeltaTime, const RenderContext& oRenderContext );

	void DisplayText( const std::string& sText, const glm::vec4& vColor = glm::vec4( 1.f, 1.f, 1.f, 1.f ) );
	void DisplayText( const std::string& sText, const float fTime, const glm::vec4& vColor = glm::vec4( 1.f, 1.f, 1.f, 1.f ) );

	void DisplayLine( const glm::vec3& vFrom, const glm::vec3& vTo, const glm::vec3& vColor, const bool bDepth = true );

	void DisplaySphere( const glm::vec3& vPosition, const float fRadius, const glm::vec3& vColor, const bool bDepth = true );

	void DisplayWireSphere( const glm::vec3& vPosition, const float fRadius, const glm::vec3& vColor, const bool bDepth = true );
	void DisplayWireCylinder( const glm::vec3& vFrom, const glm::vec3& vTo, const float fRadius, const glm::vec3& vColor, const bool bDepth = true );
	void DisplayWireCone( const glm::vec3& vFrom, const glm::vec3& vTo, const float fRadius, const glm::vec3& vColor, const bool bDepth = true );
	void DisplayWireBox( const glm::vec3& vCenter, const glm::vec3& vHalfSize, const glm::mat3& mAxes, const glm::vec3& vColor, const bool bDepth = true );
	void DisplayWireMesh( const Mesh& oMesh, const glm::mat4x3& mMatrix, const glm::vec3& vColor, const bool bDepth = true );

private:
	Array< Text >		m_aTexts;
	Array< Text >		m_aTimedTexts;
	Array< float >		m_aTimedTextsRemaining;
						
	DebugShapeHolder< Line >		m_aLines;
	DebugShapeHolder< Sphere >		m_aSpheres;
	DebugShapeHolder< Sphere >		m_aWireSpheres;
	DebugShapeHolder< Cylinder >	m_aWireCylinders;
	DebugShapeHolder< Cylinder >	m_aWireCones;
	DebugShapeHolder< Box >			m_aWireBoxes;
	DebugShapeHolder< WireMesh >	m_aWireMeshes;
};

extern DebugDisplay* g_pDebugDisplay;