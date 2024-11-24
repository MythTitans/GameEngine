#pragma once

#include <GL/glew.h>

#include "Core/Array.h"
#include "Core/Types.h"

enum class TextureFormat
{
	RGBA
};

class Texture
{
public:
	Texture();

	void Create( const int iWidth, const int iHeight, const TextureFormat eTextureFormat, const uint8* pData );
	void Destroy();

private:
	GLuint m_uTextureID;
};