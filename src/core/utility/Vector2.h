#pragma once
#include <cmath>

namespace core::utility
{
	/**
	 * @brief 2D の座標・速度を表すベクトル
	 *
	 * DxLib の VECTOR に依存しない Game 層用の型。座標も速度もこれ一つで扱う
	 */
	struct Vector2
	{
		float x{ 0.0f };
		float y{ 0.0f };

		constexpr Vector2 operator+(const Vector2& rhs) const noexcept
		{
			return Vector2{ x + rhs.x, y + rhs.y };
		}

		constexpr Vector2 operator-(const Vector2& rhs) const noexcept
		{
			return Vector2{ x - rhs.x, y - rhs.y };
		}

		constexpr Vector2 operator*(float scale) const noexcept
		{
			return Vector2{ x * scale, y * scale };
		}

		constexpr Vector2& operator+=(const Vector2& rhs) noexcept
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}

		constexpr Vector2& operator-=(const Vector2& rhs) noexcept
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}

		constexpr Vector2& operator*=(float scale) noexcept
		{
			x *= scale;
			y *= scale;
			return *this;
		}

		/**
		 * @brief ベクトルの長さを返す
		 * @return 長さ
		 */
		[[nodiscard]] float length() const noexcept
		{
			return std::sqrt(x * x + y * y);
		}

		/**
		 * @brief 長さの2乗を返す（比較だけなら平方根を避けられる）
		 * @return 長さの2乗
		 */
		[[nodiscard]] constexpr float lengthSquared() const noexcept
		{
			return x * x + y * y;
		}

		/**
		 * @brief 長さを1に揃えたベクトルを返す
		 * @details 長さが0のときは零ベクトルをそのまま返す（0除算を避ける）
		 * @return 正規化したベクトル
		 */
		[[nodiscard]] Vector2 normalized() const noexcept
		{
			const float len{ length() };
			if (len <= 0.0f)
				return Vector2{};
			return Vector2{ x / len, y / len };
		}
	};
} // namespace core::utility
