#pragma once
#include <cmath>

namespace core::utility
{
	/**
	 * @brief 3D の座標・方向を表すベクトル
	 *
	 * DxLib の VECTOR に依存しない Game 層用の型。変換は Infrastructure 層が行う
	 */
	struct Vector3
	{
		float x{ 0.0f };
		float y{ 0.0f };
		float z{ 0.0f };

		constexpr Vector3 operator+(const Vector3& rhs) const noexcept
		{
			return Vector3{ x + rhs.x, y + rhs.y, z + rhs.z };
		}

		constexpr Vector3 operator-(const Vector3& rhs) const noexcept
		{
			return Vector3{ x - rhs.x, y - rhs.y, z - rhs.z };
		}

		constexpr Vector3 operator*(float scale) const noexcept
		{
			return Vector3{ x * scale, y * scale, z * scale };
		}

		constexpr Vector3& operator+=(const Vector3& rhs) noexcept
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}

		/**
		 * @brief ベクトルの長さを返す
		 * @return 長さ
		 */
		[[nodiscard]] float length() const noexcept
		{
			return std::sqrt(x * x + y * y + z * z);
		}

		/**
		 * @brief 長さを1に揃えたベクトルを返す
		 * @details 長さが0のときは零ベクトルをそのまま返す（0除算を避ける）
		 * @return 正規化したベクトル
		 */
		[[nodiscard]] Vector3 normalized() const noexcept
		{
			const float len{ length() };
			if (len <= 0.0f)
				return Vector3{};
			return Vector3{ x / len, y / len, z / len };
		}
	};
} // namespace core::utility
