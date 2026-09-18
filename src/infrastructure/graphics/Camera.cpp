#include "infrastructure/graphics/Camera.h"
#include "DxLib.h"

namespace infrastructure::graphics
{
	void Camera::lookAt(const core::utility::Vector3& position, const core::utility::Vector3& target)
	{
		m_position = position;
		SetCameraPositionAndTarget_UpVecY(VGet(position.x, position.y, position.z),
		                                  VGet(target.x, target.y, target.z));
	}

	void Camera::setPerspective(float fovRadian, float nearZ, float farZ)
	{
		// Near/Far はカメラのセットアップより先に決めておく必要がある
		SetCameraNearFar(nearZ, farZ);
		SetupCamera_Perspective(fovRadian);
	}
} // namespace infrastructure::graphics
