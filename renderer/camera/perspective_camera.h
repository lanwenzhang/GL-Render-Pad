#pragma once

#include "camera.h"

namespace lzgl::renderer {

	class PerspectiveCamera : public Camera {
	public:
		PerspectiveCamera(float fovy, float aspect, float near, float far);
		~PerspectiveCamera();

		glm::mat4 getProjectionMatrix()override;

		void scale(float deltaScale)override;

	public:
		float mFovy = 0.0f;
		float mAspect = 0.0f;
	};

}
