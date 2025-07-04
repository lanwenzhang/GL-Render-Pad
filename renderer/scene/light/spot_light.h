#pragma once

#include "light.h"
#include "../object.h"

namespace lzgl::renderer {
	class SpotLight :public Light, public Object {

	public:
		SpotLight();
		~SpotLight();
	public:
		float mInnerAngle{ 0.0f };
		float mOuterAngle{ 0.0f };
	};
}
