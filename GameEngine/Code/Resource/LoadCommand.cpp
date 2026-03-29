#include "LoadCommand.h"

#include "Core/FileUtils.h"
#include "Core/Logger.h"
#include "Core/Serialization.h"
#include "Core/StringUtils.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "Core/stb_image.h"
#include "Core/stb_truetype.h"
#undef STB_IMAGE_IMPLEMENTATION
#undef STB_TRUETYPE_IMPLEMENTATION
#include "Graphics/MaterialManager.h"
#include "Graphics/Mesh.h"
#include "ResourceLoader.h"

template < typename T >
auto AssimpToGLM( const T& oAssimp )
{
	static_assert( false, "Not implemented" );
}

template <>
auto AssimpToGLM< aiVector3D >( const aiVector3D& vAssimp )
{
	return *reinterpret_cast< const glm::vec3* >( &vAssimp );
}

template <>
auto AssimpToGLM< aiQuaternion >( const aiQuaternion& qAssimp )
{
	return *reinterpret_cast< const glm::quat* >( &qAssimp );
}

template <>
auto AssimpToGLM< aiMatrix4x4 >( const aiMatrix4x4& mAssimp )
{
	return glm::transpose( *reinterpret_cast< const glm::mat4* >( &mAssimp ) );
}

// ////////////////////////////////////////////////////////////////////////////
// Font
// ////////////////////////////////////////////////////////////////////////////
FontLoadCommand::FontLoadCommand( const char* sFilePath, const FontResPtr& xResource )
	: LoadCommand( sFilePath, xResource )
{
}

void FontLoadCommand::Load( std::unique_lock< std::mutex >& oLock )
{
	Array< uint8 > aAtlasData( FontResource::ATLAS_WIDTH * FontResource::ATLAS_HEIGHT );
	Array< stbtt_packedchar > aPackedCharacters( FontResource::GLYPH_COUNT );

	Array< uint8 > aFontData = ReadBinaryFile( GetFilePath() );

	stbtt_pack_context oAtlasContext;
	stbtt_PackBegin( &oAtlasContext, aAtlasData.Data(), FontResource::ATLAS_WIDTH, FontResource::ATLAS_HEIGHT, 0, 1, nullptr );
	stbtt_PackFontRange( &oAtlasContext, aFontData.Data(), 0, ( float )FontResource::FONT_HEIGHT, FontResource::FIRST_GLYPH, FontResource::GLYPH_COUNT, aPackedCharacters.Data() );
	stbtt_PackEnd( &oAtlasContext );

	oLock.lock();
	m_eStatus = aAtlasData.Empty() == false ? LoadCommandStatus::LOADED : LoadCommandStatus::ERROR_READING;
	m_aAtlasData = std::move( aAtlasData );
	m_aPackedCharacters = std::move( aPackedCharacters );
	oLock.unlock();
}

void FontLoadCommand::OnFinished()
{
	switch( m_eStatus )
	{
	case LoadCommandStatus::FINISHED:
		m_xResource->m_oAtlas.Create( TextureDesc( FontResource::ATLAS_WIDTH, FontResource::ATLAS_HEIGHT, TextureFormat::R ).Data( m_aAtlasData.Data() ).GenerateMips() );
		m_xResource->m_aPackedCharacters = std::move( m_aPackedCharacters );
		m_xResource->m_eStatus = Resource::Status::LOADED;
		break;
	case LoadCommandStatus::NOT_FOUND:
	case LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}
}

void FontLoadCommand::OnDependenciesReady()
{
}

// ////////////////////////////////////////////////////////////////////////////
// Texture
// ////////////////////////////////////////////////////////////////////////////
TextureLoadCommand::TextureLoadCommand( const char* sFilePath, const TextureResPtr& xResource, const bool bSRGB, const bool bUse16Bits )
	: LoadCommand( sFilePath, xResource )
	, m_iWidth( 0 )
	, m_iHeight( 0 )
	, m_iDepth( 0 )
	, m_bSRGB( bSRGB )
	, m_bUse16Bits( bUse16Bits )
	, m_pData( nullptr )
{
}

void TextureLoadCommand::Load( std::unique_lock< std::mutex >& oLock )
{
	int iWidth;
	int iHeight;
	int iDepth;
	uint8* pData = m_bUse16Bits ? ( uint8* )stbi_load_16( GetFilePath().c_str(), &iWidth, &iHeight, &iDepth, 0 ) : stbi_load( GetFilePath().c_str(), &iWidth, &iHeight, &iDepth, 0 );

	oLock.lock();
	m_eStatus = pData != nullptr ? LoadCommandStatus::LOADED : LoadCommandStatus::ERROR_READING;
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_iDepth = iDepth;
	m_pData = pData;
	oLock.unlock();
}

void TextureLoadCommand::OnFinished()
{
	TextureFormat eFormat = TextureFormat::RGBA;

	switch( m_iDepth )
	{
	case 1:
		eFormat = m_bUse16Bits ? TextureFormat::R16 : TextureFormat::R;
		break;
	case 3:
		eFormat = TextureFormat::RGB;
		break;
	}

	switch( m_eStatus )
	{
	case LoadCommandStatus::FINISHED:
		m_xResource->m_oTexture.Create( TextureDesc( m_iWidth, m_iHeight, eFormat ).Data( m_pData ).SRGB( m_bSRGB ).GenerateMips() );
		stbi_image_free( m_pData );
		m_xResource->m_eStatus = Resource::Status::LOADED;
		break;
	case LoadCommandStatus::NOT_FOUND:
	case LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}
}

void TextureLoadCommand::OnDependenciesReady()
{
}

// ////////////////////////////////////////////////////////////////////////////
// Model
// ////////////////////////////////////////////////////////////////////////////
ModelLoadCommand::ModelLoadCommand( const char* sFilePath, const ModelResPtr& xResource )
	: LoadCommand( sFilePath, xResource )
	, m_pScene( nullptr )
{
}

void ModelLoadCommand::Load( std::unique_lock< std::mutex >& oLock )
{
	aiScene* pSceneData = nullptr;

	const aiScene* pScene = g_pResourceLoader->m_oModelImporter.ReadFile( GetFilePath(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace );
	if( pScene != nullptr )
		pSceneData = g_pResourceLoader->m_oModelImporter.GetOrphanedScene();

	oLock.lock();
	m_eStatus = pSceneData != nullptr ? LoadCommandStatus::LOADED : LoadCommandStatus::ERROR_READING;
	m_pScene = pSceneData;
	oLock.unlock();
}

void ModelLoadCommand::OnFinished()
{
	switch( m_eStatus )
	{
	case LoadCommandStatus::FINISHED:
	{
		LoadAnimations();
		LoadSkeleton();
		LoadMaterials();
		LoadMeshes();
		aiReleaseImport( m_pScene );
		if( HasDependencies() == false )
			m_xResource->m_eStatus = Resource::Status::LOADED;
		break;
	}
	case LoadCommandStatus::NOT_FOUND:
	case LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}
}

void ModelLoadCommand::OnDependenciesReady()
{
	switch( m_eStatus )
	{
	case LoadCommandStatus::FINISHED:
		m_xResource->m_eStatus = Resource::Status::LOADED;
		break;
	case LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}

	//m_xResource->m_aTechniqueResources = std::move( m_aDependencies );
	m_aDependencies.Clear();
}

void ModelLoadCommand::LoadAnimations()
{
	m_mNodeIndices[ "SkeletonRoot" ] = 0;

	m_xResource->m_aAnimations.Resize( m_pScene->mNumAnimations );

	for( uint uAnimation = 0; uAnimation < m_pScene->mNumAnimations; ++uAnimation )
	{
		const aiAnimation* pAnimation = m_pScene->mAnimations[ uAnimation ];

		Animation& oAnimation = m_xResource->m_aAnimations[ uAnimation ];
		oAnimation.m_sName = pAnimation->mName.C_Str();
		oAnimation.m_fDuration = ( float )( pAnimation->mDuration / pAnimation->mTicksPerSecond );
		oAnimation.m_aNodeAnimations.Resize( pAnimation->mNumChannels );

		for( uint uChannel = 0; uChannel < pAnimation->mNumChannels; ++uChannel )
		{
			const aiNodeAnim* pNodeAnimation = pAnimation->mChannels[ uChannel ];

			NodeAnimation& oNodeAnimation = oAnimation.m_aNodeAnimations[ uChannel ];
			const std::string sNodeName = pNodeAnimation->mNodeName.C_Str();

			oNodeAnimation.m_oPositionCurve.m_aTimes.Resize( pNodeAnimation->mNumPositionKeys );
			oNodeAnimation.m_oPositionCurve.m_aValues.Resize( pNodeAnimation->mNumPositionKeys );
			for( uint u = 0; u < pNodeAnimation->mNumPositionKeys; ++u )
			{
				oNodeAnimation.m_oPositionCurve.m_aTimes[ u ] = ( float )( pNodeAnimation->mPositionKeys[ u ].mTime / pAnimation->mTicksPerSecond );
				oNodeAnimation.m_oPositionCurve.m_aValues[ u ] = AssimpToGLM( pNodeAnimation->mPositionKeys[ u ].mValue );
			}

			oNodeAnimation.m_oRotationCurve.m_aTimes.Resize( pNodeAnimation->mNumRotationKeys );
			oNodeAnimation.m_oRotationCurve.m_aValues.Resize( pNodeAnimation->mNumRotationKeys );
			for( uint u = 0; u < pNodeAnimation->mNumRotationKeys; ++u )
			{
				oNodeAnimation.m_oRotationCurve.m_aTimes[ u ] = ( float )( pNodeAnimation->mRotationKeys[ u ].mTime / pAnimation->mTicksPerSecond );
				glm::quat qRotation = AssimpToGLM( pNodeAnimation->mRotationKeys[ u ].mValue );
				oNodeAnimation.m_oRotationCurve.m_aValues[ u ] = glm::quat( qRotation.x, qRotation.y, qRotation.z, qRotation.w );
			}

			oNodeAnimation.m_oScaleCurve.m_aTimes.Resize( pNodeAnimation->mNumScalingKeys );
			oNodeAnimation.m_oScaleCurve.m_aValues.Resize( pNodeAnimation->mNumScalingKeys );
			for( uint u = 0; u < pNodeAnimation->mNumScalingKeys; ++u )
			{
				oNodeAnimation.m_oScaleCurve.m_aTimes[ u ] = ( float )( pNodeAnimation->mScalingKeys[ u ].mTime / pAnimation->mTicksPerSecond );
				oNodeAnimation.m_oScaleCurve.m_aValues[ u ] = AssimpToGLM( pNodeAnimation->mScalingKeys[ u ].mValue );
			}

			oNodeAnimation.m_uMatrixIndex = FetchNodeIndex( sNodeName );
		}
	}
}

void ModelLoadCommand::LoadSkeleton()
{
	m_xResource->m_oSkeleton.m_uMatrixIndex = 0;

	m_xResource->m_aPoseMatrices.Resize( ( uint )m_mNodeIndices.size(), glm::mat4( 1.f ) );
	LoadSkeleton( m_pScene->mRootNode, m_xResource->m_oSkeleton );

	m_xResource->m_aSkinMatrices.Resize( ( uint )m_mNodeIndices.size(), glm::mat4( 1.f ) );
}

void ModelLoadCommand::LoadMaterials()
{
	m_aMaterials.Resize( m_pScene->mNumMaterials );

	for( uint u = 0; u < m_pScene->mNumMaterials; ++u )
	{
		// 		TechniqueResPtr xTechniqueResource = g_pResourceLoader->LoadTechnique( "Shader/forward_opaque.tech" );
		// 		m_aDependencies.PushBack( xTechniqueResource.GetPtr() );

		const aiMaterial* pMaterial = m_pScene->mMaterials[ u ];

		aiColor3D oDiffuseColor;
		pMaterial->Get( AI_MATKEY_COLOR_DIFFUSE, oDiffuseColor );
		m_aMaterials[ u ].m_oDiffuseColor = Color( oDiffuseColor.r, oDiffuseColor.g, oDiffuseColor.b );

		aiColor3D oSpecularColor;
		pMaterial->Get( AI_MATKEY_COLOR_SPECULAR, oSpecularColor );
		m_aMaterials[ u ].m_oSpecularColor = Color( oSpecularColor.r, oSpecularColor.g, oSpecularColor.b );

		aiColor3D oEmissiveColor;
		pMaterial->Get( AI_MATKEY_COLOR_EMISSIVE, oEmissiveColor );
		m_aMaterials[ u ].m_oEmissiveColor = Color( oEmissiveColor.r, oEmissiveColor.g, oEmissiveColor.b );

		pMaterial->Get( AI_MATKEY_SHININESS, m_aMaterials[ u ].m_fShininess );

		if( pMaterial->GetTextureCount( aiTextureType_DIFFUSE ) != 0 )
		{
			aiString sFile;
			if( pMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &sFile ) == AI_SUCCESS )
			{
				TextureResPtr xTextureResource = LoadTexture( sFile.C_Str(), true );
				m_aMaterials[ u ].m_xDiffuseTextureResource = xTextureResource;
				m_aDependencies.PushBack( xTextureResource.GetPtr() );
			}
		}

		if( pMaterial->GetTextureCount( aiTextureType_NORMALS ) != 0 )
		{
			aiString sFile;
			if( pMaterial->GetTexture( aiTextureType_NORMALS, 0, &sFile ) == AI_SUCCESS )
			{
				TextureResPtr xTextureResource = LoadTexture( sFile.C_Str() );
				m_aMaterials[ u ].m_xNormalTextureResource = xTextureResource;
				m_aDependencies.PushBack( xTextureResource.GetPtr() );
			}
		}

		if( pMaterial->GetTextureCount( aiTextureType_SPECULAR ) != 0 )
		{
			aiString sFile;
			if( pMaterial->GetTexture( aiTextureType_SPECULAR, 0, &sFile ) == AI_SUCCESS )
			{
				TextureResPtr xTextureResource = LoadTexture( sFile.C_Str() );
				m_aMaterials[ u ].m_xSpecularTextureResource = xTextureResource;
				m_aDependencies.PushBack( xTextureResource.GetPtr() );
			}
		}

		if( pMaterial->GetTextureCount( aiTextureType_EMISSIVE ) != 0 )
		{
			aiString sFile;
			if( pMaterial->GetTexture( aiTextureType_EMISSIVE, 0, &sFile ) == AI_SUCCESS )
			{
				TextureResPtr xTextureResource = LoadTexture( sFile.C_Str() );
				m_aMaterials[ u ].m_xEmissiveTextureResource = xTextureResource;
				m_aDependencies.PushBack( xTextureResource.GetPtr() );
			}
		}
	}
}

void ModelLoadCommand::LoadMeshes()
{
	aiNode* pRoot = m_pScene->mRootNode;

	const uint uMeshCount = CountMeshes( pRoot );
	m_xResource->m_aMeshes.Reserve( uMeshCount );
	LoadMeshes( pRoot );
}

uint ModelLoadCommand::CountMeshes( aiNode* pNode )
{
	uint uCount = pNode->mNumMeshes;

	for( uint u = 0; u < pNode->mNumChildren; ++u )
		uCount += CountMeshes( pNode->mChildren[ u ] );

	return uCount;
}

void ModelLoadCommand::LoadMeshes( aiNode* pNode )
{
	for( uint u = 0; u < pNode->mNumMeshes; ++u )
		LoadMesh( m_pScene->mMeshes[ pNode->mMeshes[ u ] ] );

	for( uint u = 0; u < pNode->mNumChildren; ++u )
		LoadMeshes( pNode->mChildren[ u ] );
}

void ModelLoadCommand::LoadMesh( aiMesh* pMesh )
{
	const uint uVertexCount = pMesh->mNumVertices;

	Array< glm::vec3 > aVertices( uVertexCount );
	Array< glm::vec3 > aNormals( uVertexCount );
	Array< glm::vec3 > aTangents( uVertexCount );
	Array< glm::vec2 > aUVs( uVertexCount );

	for( uint u = 0; u < uVertexCount; ++u )
	{
		aVertices[ u ] = glm::vec3( pMesh->mVertices[ u ].x, pMesh->mVertices[ u ].y, pMesh->mVertices[ u ].z );
		aNormals[ u ] = glm::vec3( pMesh->mNormals[ u ].x, pMesh->mNormals[ u ].y, pMesh->mNormals[ u ].z );
		aTangents[ u ] = glm::vec3( pMesh->mTangents[ u ].x, pMesh->mTangents[ u ].y, pMesh->mTangents[ u ].z );

		if( pMesh->mTextureCoords[ 0 ] != nullptr )
			aUVs[ u ] = glm::vec2( pMesh->mTextureCoords[ 0 ][ u ].x, pMesh->mTextureCoords[ 0 ][ u ].y );
	}

	uint uIndexCount = 0;
	for( uint u = 0; u < pMesh->mNumFaces; ++u )
		uIndexCount += pMesh->mFaces[ u ].mNumIndices;

	Array< GLuint > aIndices;
	aIndices.Reserve( uIndexCount );

	for( uint uFace = 0; uFace < pMesh->mNumFaces; ++uFace )
	{
		const aiFace& oFace = pMesh->mFaces[ uFace ];
		for( uint uIndex = 0; uIndex < oFace.mNumIndices; ++uIndex )
			aIndices.PushBack( oFace.mIndices[ uIndex ] );
	}

	Array< uint > aBonesCount( uVertexCount, 0 );
	Array< SkinData > aSkinData( uVertexCount );

	for( uint uBone = 0; uBone < pMesh->mNumBones; ++uBone )
	{
		const aiBone* pBone = pMesh->mBones[ uBone ];

		uint uBoneIndex = 0;
		const auto it = m_mNodeIndices.find( pBone->mName.C_Str() );
		ASSERT( it != m_mNodeIndices.cend() );
		if( it != m_mNodeIndices.cend() )
			uBoneIndex = it->second;

		m_xResource->m_aSkinMatrices[ uBoneIndex ] = AssimpToGLM( pBone->mOffsetMatrix );

		for( uint uWeight = 0; uWeight < pBone->mNumWeights; ++uWeight )
		{
			const aiVertexWeight& oVertexWeight = pBone->mWeights[ uWeight ];

			const uint uVertexIndex = oVertexWeight.mVertexId;
			const float fBoneWeight = ( float )oVertexWeight.mWeight;

			uint& uVertexBoneIndex = aBonesCount[ uVertexIndex ];
			ASSERT( uVertexBoneIndex < MAX_VERTEX_BONE_COUNT );
			aSkinData[ uVertexIndex ].m_aBones[ uVertexBoneIndex ] = uBoneIndex;
			aSkinData[ uVertexIndex ].m_aWeights[ uVertexBoneIndex ] = fBoneWeight;
			++uVertexBoneIndex;
		}
	}

	FitAxisAlignedBox( m_xResource->m_oAABB, aVertices );

	MeshBuilder oMeshBuilder = MeshBuilder( std::move( aVertices ), std::move( aIndices ) )
		.WithUVs( std::move( aUVs ) )
		.WithNormals( std::move( aNormals ) )
		.WithTangents( std::move( aTangents ) )
		.WithSkinData( std::move( aSkinData ) );

	if( pMesh->mMaterialIndex >= 0 && pMesh->mMaterialIndex < m_aMaterials.Count() )
	{
		const MaterialReference oMaterial = g_pMaterialManager->CreateMaterial( m_aMaterials[ pMesh->mMaterialIndex ] );
		oMeshBuilder.WithMaterial( oMaterial );
	}

	m_xResource->m_aMeshes.PushBack( oMeshBuilder.Build() );
}

void ModelLoadCommand::LoadSkeleton( aiNode* pNode, Skeleton& oParent )
{
	const std::string sNodeName = pNode->mName.C_Str();
	if( Contains( sNodeName, "$_Translation" ) || Contains( sNodeName, "$_PreRotation" ) || Contains( sNodeName, "$_Rotation" ) )
	{
		oParent.m_aChildren.Reserve( pNode->mNumChildren );
		for( uint u = 0; u < pNode->mNumChildren; ++u )
			LoadSkeleton( pNode->mChildren[ u ], oParent );
	}
	else
	{
		oParent.m_aChildren.PushBack( Skeleton() );

		Skeleton& oSkeleton = oParent.m_aChildren.Back();
		oSkeleton.m_aChildren.Reserve( pNode->mNumChildren );
		for( uint u = 0; u < pNode->mNumChildren; ++u )
			LoadSkeleton( pNode->mChildren[ u ], oSkeleton );

		const uint uNodeIndex = FetchNodeIndex( sNodeName );
		oSkeleton.m_uMatrixIndex = uNodeIndex;

		if( uNodeIndex >= m_xResource->m_aPoseMatrices.Count() )
			m_xResource->m_aPoseMatrices.Resize( uNodeIndex + 1 );

		m_xResource->m_aPoseMatrices[ uNodeIndex ] = AssimpToGLM( pNode->mTransformation );
	}
}

TextureResPtr ModelLoadCommand::LoadTexture( const std::string& sFileName, const bool bSRGB /*= false*/ )
{
	int uTextureIndex = -1;
	for( uint u = 0; u < m_pScene->mNumTextures; ++u )
	{
		if( sFileName == m_pScene->mTextures[ u ]->mFilename.C_Str() )
		{
			uTextureIndex = ( int )u;
			break;
		}
	}

	if( uTextureIndex != -1 )
	{
		aiTexture* pTexture = m_pScene->mTextures[ uTextureIndex ];

		std::string sInternalFileName = sFileName;
		const uint64 uOffset = sFileName.find( ".fbm/" );
		if( uOffset != std::string::npos )
			Replace( sInternalFileName, sInternalFileName.substr( 0, uOffset + 5 ), GetFilePath() + "@" );

		return g_pResourceLoader->LoadTexture( sInternalFileName.c_str(), ( uint8* )pTexture->pcData, pTexture->mWidth, bSRGB );
	}
	else
	{
		return g_pResourceLoader->LoadTexture( sFileName.c_str(), bSRGB );
	}
}

uint ModelLoadCommand::FetchNodeIndex( const std::string& sName )
{
	const auto it = m_mNodeIndices.find( sName );
	if( it != m_mNodeIndices.cend() )
	{
		return it->second;
	}

	const uint uIndex = ( uint )m_mNodeIndices.size();
	m_mNodeIndices[ sName ] = uIndex;
	return uIndex;
}

// ////////////////////////////////////////////////////////////////////////////
// Shader
// ////////////////////////////////////////////////////////////////////////////
ShaderLoadCommand::ShaderLoadCommand( const char* sFilePath, const ShaderResPtr& xResource, Array< std::string >&& aFlags )
	: LoadCommand( sFilePath, xResource )
	, m_eShaderType( ShaderType::UNDEFINED )
	, m_aFlags( aFlags )
{
	const std::filesystem::path oFilePath = GetFilePath();
	if( oFilePath.extension() == ".vs" )
		m_eShaderType = ShaderType::VERTEX_SHADER;
	else if( oFilePath.extension() == ".ps" )
		m_eShaderType = ShaderType::PIXEL_SHADER;
	else if( oFilePath.extension() == ".cs" )
		m_eShaderType = ShaderType::COMPUTE_SHADER;
}

void ShaderLoadCommand::Load( std::unique_lock< std::mutex >& oLock )
{
	std::string sContent = ReadTextFile( GetFilePath() );

	oLock.lock();
	m_eStatus = LoadCommandStatus::LOADED;
	m_sShaderCode = std::move( sContent );
	oLock.unlock();
}

void ShaderLoadCommand::OnFinished()
{
	switch( m_eStatus )
	{
	case LoadCommandStatus::FINISHED:
		m_xResource->m_oShader.Create( m_sShaderCode, m_eShaderType, m_aFlags );
		m_xResource->m_eStatus = Resource::Status::LOADED;
		break;
	case LoadCommandStatus::NOT_FOUND:
	case LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}
}

void ShaderLoadCommand::OnDependenciesReady()
{
}

// ////////////////////////////////////////////////////////////////////////////
// Technique
// ////////////////////////////////////////////////////////////////////////////
TechniqueLoadCommand::TechniqueLoadCommand( const char* sFilePath, const TechniqueResPtr& xResource )
	: LoadCommand( sFilePath, xResource )
{
}

void TechniqueLoadCommand::Load( std::unique_lock< std::mutex >& oLock )
{
	std::string sContent = ReadTextFile( GetFilePath() );

	std::string sVertexShader;
	std::string sPixelShader;
	std::string sComputeShader;
	Array< std::string > aParameters;
	Array< std::pair< std::string, uint > > aArrayParameters;
	Array< std::string > aFlags;

	bool bSuccess = true;

	try
	{
		const nlohmann::json oJsonContent = nlohmann::json::parse( sContent );

		if( oJsonContent.contains( "computeShader" ) )
		{
			sComputeShader = oJsonContent.at( "computeShader" );
		}
		else
		{
			sVertexShader = oJsonContent.at( "vertexShader" );
			sPixelShader = oJsonContent.at( "pixelShader" );
		}

		if( oJsonContent.contains( "parameters" ) )
		{
			const nlohmann::json& oParameters = oJsonContent[ "parameters" ];
			aParameters.Reserve( ( uint )oParameters.size() );
			for( const std::string& sParameter : oParameters )
				aParameters.PushBack( sParameter );
		}

		if( oJsonContent.contains( "arrays" ) )
		{
			const nlohmann::json& oArrays = oJsonContent[ "arrays" ];
			aArrayParameters.Reserve( ( uint )oArrays.size() );
			for( const nlohmann::json& oArray : oArrays )
			{
				const uint uCount = oArray.at( "count" );
				if( oArray.contains( "parameter" ) )
					aArrayParameters.PushBack( std::pair( oArray[ "parameter" ], uCount ) );

				if( oArray.contains( "parameters" ) )
				{
					aArrayParameters.Reserve( aArrayParameters.Count() + ( uint )oArray[ "parameters" ].size() );
					for( const std::string& sParameter : oArray[ "parameters" ] )
						aArrayParameters.PushBack( std::pair( sParameter, uCount ) );
				}
			}
		}
	}
	catch( const std::exception& oException )
	{
		LOG_ERROR( "{} : {}", GetFilePath(), oException.what() );
		bSuccess = false;
	}

	oLock.lock();
	if( bSuccess )
	{
		m_eStatus = LoadCommandStatus::LOADED;
		m_aParameters = std::move( aParameters );
		m_aArrayParameters = std::move( aArrayParameters );

		if( sComputeShader.empty() == false )
		{
			m_aShaders.PushBack( sComputeShader );
		}
		else
		{
			m_aShaders.PushBack( sVertexShader );
			m_aShaders.PushBack( sPixelShader );
		}
	}
	else
	{
		m_eStatus = LoadCommandStatus::ERROR_READING;
	}
	oLock.unlock();
}

void TechniqueLoadCommand::OnFinished()
{
	switch( m_eStatus )
	{
	case LoadCommandStatus::FINISHED:
		for( const std::string& sShader : m_aShaders )
			m_aDependencies.PushBack( g_pResourceLoader->LoadShader( sShader.c_str() ).GetPtr() );
		break;
	case LoadCommandStatus::NOT_FOUND:
	case LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}
}

void TechniqueLoadCommand::OnDependenciesReady()
{
	switch( m_eStatus )
	{
	case LoadCommandStatus::FINISHED:
	{
		Array< const Shader* > aShaders;
		aShaders.Reserve( m_aDependencies.Count() );
		for( const StrongPtr< Resource >& pDependency : m_aDependencies )
		{
			const ShaderResource* pShaderResource = static_cast< const ShaderResource* >( pDependency.GetPtr() );
			ASSERT( pShaderResource->IsLoaded() );
			aShaders.PushBack( &pShaderResource->m_oShader );
		}
		m_xResource->m_oTechnique.Create( aShaders, m_aParameters, m_aArrayParameters );

		m_xResource->m_eStatus = Resource::Status::LOADED;

		break;
	}
	case LoadCommandStatus::NOT_FOUND:
	case LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}

	m_xResource->m_aShaderResources = std::move( m_aDependencies );
}

// ////////////////////////////////////////////////////////////////////////////
// Material
// ////////////////////////////////////////////////////////////////////////////
MaterialLoadCommand::MaterialLoadCommand( const char* sFilePath, const MaterialResPtr& xResource )
	: LoadCommand( sFilePath, xResource )
{
}

void MaterialLoadCommand::Load( std::unique_lock< std::mutex >& oLock )
{
	std::string sContent = ReadTextFile( GetFilePath() );

	std::string sType;
	nlohmann::json oJsonData;

	bool bSuccess = true;

	try
	{
		const nlohmann::json oJsonContent = nlohmann::json::parse( sContent );

		sType = oJsonContent[ "type" ];
		oJsonData = oJsonContent[ "data" ];
	}
	catch( const std::exception& oException )
	{
		LOG_ERROR( "{} : {}", GetFilePath(), oException.what() );
		bSuccess = false;
	}

	oLock.lock();
	if( bSuccess )
	{
		m_eStatus = LoadCommandStatus::LOADED;
		m_sType = std::move( sType );
		m_oJsonData = std::move( oJsonData );
	}
	else
	{
		m_eStatus = LoadCommandStatus::ERROR_READING;
	}
	oLock.unlock();
}

void MaterialLoadCommand::OnFinished()
{
	switch( m_eStatus )
	{
	case LoadCommandStatus::FINISHED:
		LoadMaterial();
		break;
	case LoadCommandStatus::NOT_FOUND:
	case LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}
}

void MaterialLoadCommand::OnDependenciesReady()
{
	switch( m_eStatus )
	{
	case LoadCommandStatus::FINISHED:
		m_xResource->m_eStatus = Resource::Status::LOADED;
		break;
	case LoadCommandStatus::NOT_FOUND:
	case LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}

	m_xResource->m_aTextureResources = std::move( m_aDependencies );
}

void MaterialLoadCommand::LoadMaterial()
{
	if( m_sType == "Lit" )
	{
		LitMaterialData oMaterialData;
		oMaterialData.m_oDiffuseColor = m_oJsonData[ "diffuseColor" ];
		oMaterialData.m_oSpecularColor = m_oJsonData[ "specularColor" ];
		oMaterialData.m_oEmissiveColor = m_oJsonData[ "emissiveColor" ];
		oMaterialData.m_fShininess = m_oJsonData[ "shininess" ];
		oMaterialData.m_xDiffuseTextureResource = g_pResourceLoader->LoadTexture( std::string( m_oJsonData[ "diffuseTexture" ] ).c_str() );
		oMaterialData.m_xNormalTextureResource = g_pResourceLoader->LoadTexture( std::string( m_oJsonData[ "normalTexture" ] ).c_str() );
		oMaterialData.m_xSpecularTextureResource = g_pResourceLoader->LoadTexture( std::string( m_oJsonData[ "specularTexture" ] ).c_str() );
		oMaterialData.m_xEmissiveTextureResource = g_pResourceLoader->LoadTexture( std::string( m_oJsonData[ "emissiveTexture" ] ).c_str() );
		m_xResource->m_oMaterial = g_pMaterialManager->CreateMaterial< LitMaterialData >(oMaterialData);

		m_aDependencies.PushBack( oMaterialData.m_xDiffuseTextureResource.GetPtr() );
		m_aDependencies.PushBack( oMaterialData.m_xNormalTextureResource.GetPtr() );
		m_aDependencies.PushBack( oMaterialData.m_xSpecularTextureResource.GetPtr() );
		m_aDependencies.PushBack( oMaterialData.m_xEmissiveTextureResource.GetPtr() );
	}
	else if( m_sType == "Unlit" )
	{
		UnlitMaterialData oMaterialData;
		oMaterialData.m_oDiffuseColor = m_oJsonData[ "diffuseColor" ];
		oMaterialData.m_xDiffuseTextureResource = g_pResourceLoader->LoadTexture( std::string( m_oJsonData[ "diffuseTexture" ] ).c_str() );
		m_xResource->m_oMaterial = g_pMaterialManager->CreateMaterial< UnlitMaterialData >( oMaterialData );

		m_aDependencies.PushBack( oMaterialData.m_xDiffuseTextureResource.GetPtr() );
	}
}