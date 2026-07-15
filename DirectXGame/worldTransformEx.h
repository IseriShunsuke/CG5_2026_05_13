#pragma once
#include "C:\Users\ponko\CG5\CG5_kamataEngine_new\External\KamataEngine\include\3d\WorldTransform.h"
#include "kamataEngine.h"

class worldTransformEx : public KamataEngine::WorldTransform 
{
public:
	void UpdateMatrix();

	KamataEngine::Matrix4x4 MakeAffineMatrix();
};
