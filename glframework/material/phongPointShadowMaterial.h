#pragma once

#include "material.h"
#include "../texture.h"


class PhongPointShadowMaterial :public Material {

public:
	PhongPointShadowMaterial();
	~PhongPointShadowMaterial();

public:
	Texture* mDiffuse{ nullptr };
	Texture* mSpecularMask{ nullptr };
	float mShiness{ 1.0f };
	float mBias{ 0.0f };
	float mDiskTightness{ 1.0f };
	float mPcfRadius{ 0.0f };
};