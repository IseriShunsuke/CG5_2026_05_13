#pragma once

#include "kamataEngine.h"

class worldTransformEx : public KamataEngine::WorldTransform 
{
public:
	void UpdateMatrix();

	KamataEngine::Matrix4x4 MakeAffineMatrix();
};
