#include "game_camera_control.h"

namespace lzgl::renderer {

	GameCameraControl::GameCameraControl() {}
	GameCameraControl::~GameCameraControl() {}

	void GameCameraControl::onCursor(double xpos, double ypos) {

		float deltaX = (xpos - mCurrentX) * mSensitivity;
		float deltaY = (ypos - mCurrentY) * mSensitivity;

		if (mRightMouseDown) {
			pitch(-deltaY);
			yaw(-deltaX);
		}

		mCurrentX = xpos;
		mCurrentY = ypos;
	}

	void GameCameraControl::pitch(float angle) {

		mPitch += angle;
		if (mPitch > 89.0f || mPitch < -89.0f) {
			mPitch -= angle;
			return;
		}

		auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), mCamera->mRight);
		mCamera->mUp = mat * glm::vec4(mCamera->mUp, 0.0f);

	}

	void GameCameraControl::yaw(float angle) {

		auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

		mCamera->mUp = mat * glm::vec4(mCamera->mUp, 0.0f);
		mCamera->mRight = mat * glm::vec4(mCamera->mRight, 0.0f);

	}

	void GameCameraControl::update() {

		// 1 Varibales
		glm::vec3 direction(0.0f);

		auto front = glm::cross(mCamera->mUp, mCamera->mRight);
		auto right = mCamera->mRight;

		// 2 Calculate direction
		if (mKeyMap[GLFW_KEY_W]) {

			direction += front;
		}

		if (mKeyMap[GLFW_KEY_S]) {

			direction -= front;
		}

		if (mKeyMap[GLFW_KEY_A]) {

			direction -= right;
		}

		if (mKeyMap[GLFW_KEY_D]) {

			direction += right;
		}

		// 3 Normalize only if direction is not 0
		if (glm::length(direction) != 0) {

			direction = glm::normalize(direction);
			mCamera->mPosition += direction * mSpeed;
		}
	}

	void GameCameraControl::autoYaw(float deltaTime) {
		float degreesPerSecond = 20.0f;
		mAutoYawAngle += degreesPerSecond * deltaTime;

		float angleRad = glm::radians(mAutoYawAngle);
		float radius = glm::length(mCamera->mPosition);

		mCamera->mPosition.x = radius * sin(angleRad);
		mCamera->mPosition.z = radius * cos(angleRad);

		glm::vec3 target = glm::vec3(0.0f, mCamera->mPosition.y, 0.0f);
		auto front = glm::normalize(target - mCamera->mPosition);
		mCamera->mRight = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
		mCamera->mUp = glm::normalize(glm::cross(mCamera->mRight, front));
	}

}

