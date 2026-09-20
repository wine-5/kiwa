#pragma once
#include "game/view/VesselLook.h"
#include <algorithm>
#include <cmath>

namespace game::view::cup
{
	/**
	 * @brief 器の内側の寸法
	 *
	 * 原点は器の底の中心。モデル（tools/make_models.py）の内側の形に合わせてある。
	 * 液体を描くものがここを見る。数値を変えるときは make_models.py の断面も
	 * 一緒に直すこと（合っていないと、液体が内壁から浮いたり壁を突き抜けたりする）
	 */
	struct Shape
	{
		/// @brief 内側の底の高さ
		float floorTop{ 0.075f };

		/// @brief 口の高さ
		float rimHeight{ 0.60f };

		/// @brief 内側の底の半径
		float floorRadius{ 0.175f };

		/// @brief 口のところの内半径
		float rimRadius{ 0.408f };

		/// @brief 器の壁の厚み（外側の面までの距離）
		float wall{ 0.022f };

		/**
		 * @brief 液体が入る高さを返す
		 * @return 底から口までの高さ
		 */
		[[nodiscard]] constexpr float getInnerHeight() const noexcept
		{
			return rimHeight - floorTop;
		}

		/**
		 * @brief その高さでの内側の半径を返す
		 *
		 * 器は口へ向かって開いているが、広がり方は一定ではなく腰のあたりで急に開く。
		 * 直線で近似するとモデルの内壁との間に隙間ができ、器が空に見えてしまうので、
		 * モデルの断面に合わせて平方根で近似する
		 * @param height 高さ
		 * @return 内側の半径
		 */
		[[nodiscard]] float radiusAt(float height) const noexcept
		{
			const float t{ std::clamp((height - floorTop) / getInnerHeight(), 0.0f, 1.0f) };
			return floorRadius + (rimRadius - floorRadius) * std::sqrt(t);
		}

		/**
		 * @brief その高さでの外側の半径を返す
		 *
		 * こぼれた茶が外壁を伝って落ちるところに使う。腰から下は器の形が
		 * すぼまっていくが、伝う筋はそこに沿うので、底までまっすぐには下りない
		 * @param height 高さ
		 * @return 外側の半径
		 */
		[[nodiscard]] float outerRadiusAt(float height) const noexcept
		{
			// 腰より下（液体の入らないところ）は、高台へ向けてすぼめる
			if (height < floorTop)
			{
				const float t{ std::clamp(height / floorTop, 0.0f, 1.0f) };
				return (floorRadius + wall) * (0.86f + 0.14f * t);
			}
			return radiusAt(height) + wall;
		}

		/**
		 * @brief 嵩の割合から液面の高さを求める
		 * @param ratio 嵩（0.0〜1.0）
		 * @return 液面の高さ
		 */
		[[nodiscard]] constexpr float surfaceHeight(float ratio) const noexcept
		{
			return floorTop + getInnerHeight() * ratio;
		}
	};

	/**
	 * @brief 器の種類から寸法を引く
	 * @param look 器の見た目
	 * @return その器の寸法
	 */
	[[nodiscard]] inline constexpr Shape shapeOf(VesselLook look) noexcept
	{
		switch (look)
		{
		case VesselLook::Guinomi:
			return Shape{ 0.050f, 0.30f, 0.120f, 0.300f, 0.018f };
		case VesselLook::Sobachoko:
			return Shape{ 0.060f, 0.62f, 0.240f, 0.300f, 0.020f };
		case VesselLook::Chawan:
			return Shape{ 0.080f, 0.50f, 0.260f, 0.620f, 0.026f };
		case VesselLook::Yunomi:
		default:
			return Shape{ 0.075f, 0.60f, 0.175f, 0.408f, 0.022f };
		}
	}
} // namespace game::view::cup
