#pragma once

#include <glm/fwd.hpp>

class TextureResource;

void ColorEdit( const char* sLabel, glm::vec3& vColor );
void TexturePreview( const char* sLabel, const TextureResource* pTexture );