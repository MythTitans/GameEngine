#pragma once

#include <GL/glew.h>

#include "Core/Array.h"

enum class TextureFormat : uint8
{
	R,
	R16,
	RGB,
	RGBA,
	RGB16,
	NORMAL,
	DEPTH,
	ID
};

enum class TextureWrapping : uint8
{
	REPEAT,
	REPEAT_MIRROR,
	CLAMP,
	CLAMP_MIRROR,
	//BORDER // TODO #eric add support for this
};

constexpr uint GetFormatBytes( const TextureFormat eFormat );

struct TextureDesc
{
	TextureDesc( const int iWidth, const int iHeight, const TextureFormat eFormat );

	TextureDesc& Data( const uint8* pData );
	TextureDesc& Multisample( int8 iSamples );
	TextureDesc& Wrapping( const TextureWrapping eWrapping );
	TextureDesc& HorizontalWrapping( const TextureWrapping eWrapping );
	TextureDesc& VerticalWrapping( const TextureWrapping eWrapping );
	TextureDesc& SRGB( const bool bSRGB = true );
	TextureDesc& GenerateMips( const bool bGenerateMips = true );

	int				m_iWidth;
	int				m_iHeight;
	const uint8*	m_pData;
	TextureFormat	m_eFormat;
	TextureWrapping	m_eHorizontalWrapping;
	TextureWrapping	m_eVerticalWrapping;
	int8			m_iSamples;
	bool			m_bSRGB;
	bool			m_bGenerateMips;
};

class Texture
{
public:
	friend class Renderer;
	friend class TextRenderer;
	friend class RenderTarget;

	Texture();

	void			Create( const TextureDesc& oDesc );
	void			Destroy();

	GLuint			GetID() const;
	int				GetWidth() const;
	int				GetHeight() const;
	TextureFormat	GetFormat() const;

	void			FetchData( Array< uint8 >& aData, const bool bSRGB = false ) const;

private:
	GLuint			m_uTextureID;
	int				m_iWidth;
	int				m_iHeight;
	TextureFormat	m_eFormat;
};

struct CubeMapDesc
{
	enum Side
	{
		POSITIVE_X,
		NEGATIVE_X,
		POSITIVE_Y,
		NEGATIVE_Y,
		POSITIVE_Z,
		NEGATIVE_Z,
		_COUNT
	};

	CubeMapDesc( const int iWidth, const int iHeight, const TextureFormat eFormat );

	CubeMapDesc& Data( const uint8* pData, const Side eSide );
	CubeMapDesc& Wrapping( const TextureWrapping eWrapping );
	CubeMapDesc& HorizontalWrapping( const TextureWrapping eWrapping );
	CubeMapDesc& VerticalWrapping( const TextureWrapping eWrapping );
	CubeMapDesc& DepthWrapping( const TextureWrapping eWrapping );
	CubeMapDesc& SRGB( const bool bSRGB = true );
	CubeMapDesc& GenerateMips( const bool bGenerateMips = true );

	int				m_iWidth;
	int				m_iHeight;
	const uint8*	m_pData[ Side::_COUNT ];
	TextureFormat	m_eFormat;
	TextureWrapping	m_eHorizontalWrapping;
	TextureWrapping	m_eVerticalWrapping;
	TextureWrapping	m_eDepthWrapping;
	bool			m_bSRGB;
	bool			m_bGenerateMips;
};

class CubeMap
{
public:
	friend class Renderer;

	CubeMap();

	void			Create( const CubeMapDesc& oDesc );
	void			Destroy();

	GLuint			GetID() const;
	int				GetWidth() const;
	int				GetHeight() const;
	TextureFormat	GetFormat() const;

private:
	GLuint			m_uTextureID;
	int				m_iWidth;
	int				m_iHeight;
	TextureFormat	m_eFormat;
};