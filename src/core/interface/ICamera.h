#pragma once
#include "core/utility/Vector3.h"

namespace core::iface
{
	/**
	 * @brief 3D カメラのインターフェース
	 */
	class ICamera
	{
	  public:
		virtual ~ICamera() = default;

		/**
		 * @brief 視点と注視点を設定する（上方向はY軸から導出される）
		 * @param position 視点の座標
		 * @param target 注視点の座標
		 */
		virtual void lookAt(const core::utility::Vector3& position,
		                    const core::utility::Vector3& target) = 0;

		/**
		 * @brief 透視投影を設定する
		 * @param fovRadian 垂直方向の画角（ラジアン）
		 * @param nearZ 手前のクリップ面までの距離
		 * @param farZ 奥のクリップ面までの距離
		 */
		virtual void setPerspective(float fovRadian, float nearZ, float farZ) = 0;

		/**
		 * @brief 現在の視点の座標を返す
		 * @return 視点の座標
		 */
		[[nodiscard]] virtual core::utility::Vector3 getPosition() const noexcept = 0;
	};
} // namespace core::iface
