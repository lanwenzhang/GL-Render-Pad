#include "perspective_camera.h"

namespace lzgl::renderer {

	PerspectiveCamera::PerspectiveCamera(float fovy, float aspect, float near, float far) {

		mFovy = fovy;
		mAspect = aspect;
		mNear = near;
		mFar = far;

	}
	PerspectiveCamera::~PerspectiveCamera() {}

	glm::mat4 PerspectiveCamera::getProjectionMatrix() {

		return glm::perspective(glm::radians(mFovy), mAspect, mNear, mFar);
	}

	void PerspectiveCamera::scale(float deltaScale) {

		auto front = glm::cross(mUp, mRight);
		mPosition += (front * deltaScale);
	}

}

