#pragma once
#include "core/base/NonCopyable.h"
#include "core/interface/ICamera.h"

namespace infrastructure::graphics
{
	/**
	 * @brief DxLib のカメラを扱う ICamera の実装
	 */
	class Camera final : public core::iface::ICamera, private core::base::NonCopyable
	{
	  public:
		Camera() = default;

		void lookAt(const core::utility::Vector3& position, const core::utility::Vector3& target) override;

		void setPerspective(float fovRadian, float nearZ, float farZ) override;

		[[nodiscard]] core::utility::Vector3 getPosition() const noexcept override
		{
			return m_position;
		}

	  private:
		core::utility::Vector3 m_position{};
	};
} // namespace infrastructure::graphics
