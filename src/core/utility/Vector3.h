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
		 * @brief 内積を返す
		 * @details 掛け算の記号は拡大縮小に使っているため、演算子ではなく名前で持つ
		 * @param rhs もう一方のベクトル
		 * @return 内積
		 */
		[[nodiscard]] constexpr float dot(const Vector3& rhs) const noexcept
		{
			return x * rhs.x + y * rhs.y + z * rhs.z;
		}

		/**
		 * @brief 外積を返す
		 * @param rhs もう一方のベクトル
		 * @return 外積
		 */
		[[nodiscard]] constexpr Vector3 cross(const Vector3& rhs) const noexcept
		{
			return Vector3{ y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x };
		}

		/**
		 * @brief 面で跳ね返したベクトルを返す
		 * @param normal 面の向き（正規化済みであること）
		 * @return 跳ね返したベクトル
		 */
		[[nodiscard]] constexpr Vector3 reflected(const Vector3& normal) const noexcept
		{
			return normal * (2.0f * dot(normal)) - *this;
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
