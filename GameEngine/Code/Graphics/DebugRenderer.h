#pragma once

#include <GL/glew.h>

#include "Game/ResourceTypes.h"
#include "Mesh.h"
#include "Technique.h"

class RenderContext;

struct Line
{
	Line( const glm::vec3& vFrom, const glm::vec3& vTo, const glm::vec3& vColor )
		: m_vFrom( vFrom )
		, m_vTo( vTo )
		, m_vColor( vColor )
	{
	}

	glm::vec3 m_vFrom;
	glm::vec3 m_vTo;
	glm::vec3 m_vColor;
};

struct Sphere
{
	Sphere( const glm::vec3& vPosition, const float fRadius, const glm::vec3& vColor )
		: m_vPosition( vPosition )
		, m_fRadius( fRadius )
		, m_vColor( vColor )
	{
	}

	glm::vec3	m_vPosition;
	float		m_fRadius;
	glm::vec3	m_vColor;
};

struct Cylinder
{
	Cylinder( const glm::vec3& vFrom, const glm::vec3& vTo, const float fRadius, const glm::vec3& vColor )
		: m_vFrom( vFrom )
		, m_vTo( vTo )
		, m_fRadius( fRadius )
		, m_vColor( vColor )
	{
	}

	glm::vec3	m_vFrom;
	glm::vec3	m_vTo;
	float		m_fRadius;
	glm::vec3	m_vColor;
};

struct Box
{
	Box( const glm::vec3& vCenter, const glm::vec3& vHalfSize, const glm::mat3& mAxes, const glm::vec3& vColor )
		: m_vCenter( vCenter )
		, m_vHalfSize( vHalfSize )
		, m_mAxes( mAxes )
		, m_vColor( vColor )
	{
	}

	glm::vec3 m_vCenter;
	glm::vec3 m_vHalfSize;
	glm::mat3 m_mAxes;
	glm::vec3 m_vColor;
};

struct WireMesh
{
	WireMesh( const Mesh& oMesh, const glm::mat4x3& mMatrix, const glm::vec3& vColor )
		: m_oMesh( oMesh )
		, m_mMatrix( mMatrix )
		, m_vColor( vColor )
	{
	}

	Mesh		m_oMesh;
	glm::mat4x3	m_mMatrix;
	glm::vec3	m_vColor;
};

class DebugRenderer
{
public:
	DebugRenderer();
	~DebugRenderer();

	void RenderLines( const Array< Line >& aLines, const RenderContext& oRenderContext );
	void RenderSpheres( const Array< Sphere >& aSpheres, const RenderContext& oRenderContext );
	void RenderWireSpheres( const Array< Sphere >& aSpheres, const RenderContext& oRenderContext );
	void RenderWireCylinders( const Array< Cylinder >& aCylinders, const RenderContext& oRenderContext );
	void RenderWireCones( const Array< Cylinder >& aCylinders, const RenderContext& oRenderContext );
	void RenderWireBoxes( const Array< Box >& aBoxes, const RenderContext& oRenderContext );
	void RenderWireMeshes( const Array< WireMesh >& aMeshes, const RenderContext& oRenderContext );

	bool OnLoading();
	void OnLoaded();

private:
	enum class LineParam : uint8
	{
		VIEW_PROJECTION,
		COLOR,
		_COUNT
	};

	enum class SphereParam : uint8
	{
		VIEW_PROJECTION,
		COLOR,
		POSITION,
		RADIUS,
		_COUNT
	};

	enum class UnlitParam : uint
	{
		MODEL_VIEW_PROJECTION,
		DIFFUSE_COLOR,
		DIFFUSE_MAP,
		_COUNT
	};

	Array< GLfloat > GenerateSphereEquator();
	Array< GLfloat > GenerateSphereMeridians();
	Array< GLfloat > GenerateCylinderEquator( const glm::vec3& vNormal, const float fRadius );

	TechniqueResPtr				m_xLine;
	PARAM_SHEET( LineParam )	m_oLineSheet;
	TechniqueResPtr				m_xSphere;
	PARAM_SHEET( SphereParam )	m_oSphereSheet;
	TechniqueResPtr				m_xUnlit;
	PARAM_SHEET( UnlitParam )	m_oUnlitSheet;

	GLuint						m_uVertexArrayID;
	GLuint						m_uVertexBufferID;
};