#pragma once
#include "core/utility/Vector3.h"
#include <random>
#include <vector>

namespace core::iface
{
	class IRenderer3D; // 前方宣言
} // namespace core::iface

namespace game::view
{
	/**
	 * @brief 枡の中の液体と、注がれている筋の見せ方
	 *
	 * 液面の揺れ・波紋・しぶきは見た目だけの動きなので、ゲームの状態とは切り離して
	 * ここが自前で時間を持つ。Model や Presenter はこれらを一切知らない
	 */
	class LiquidVisual
	{
	  public:
		LiquidVisual() = default;

		/**
		 * @brief 見た目の動きを進める
		 * @param deltaTime 進める時間（秒）
		 * @param isPouring いま注がれているか
		 * @param amountRatio 溜まっている嵩（0.0〜1.0）
		 */
		void advance(float deltaTime, bool isPouring, float amountRatio);

		/**
		 * @brief 液体を描く
		 * @param renderer 3D 描画
		 */
		void draw(core::iface::IRenderer3D& renderer) const;

		/**
		 * @brief いまの液面の高さを返す（揺れを含む）
		 * @return 液面の高さ
		 */
		[[nodiscard]] float getSurfaceHeight() const noexcept;

	  private:
		/**
		 * @brief 広がっていく波紋
		 */
		struct Ripple
		{
			float radius{ 0.0f };
			float life{ 1.0f }; // 1.0 から 0.0 へ落ちていく
		};

		/**
		 * @brief 跳ねたしずく
		 */
		struct Droplet
		{
			core::utility::Vector3 position{};
			core::utility::Vector3 velocity{};
			float radius{ 0.0f };
			float life{ 0.0f };
		};

		/** @brief 着水の波紋を起こす */
		void spawnRipple();

		/** @brief 着水のしぶきを飛ばす */
		void spawnSplash();

		/**
		 * @brief 範囲内の乱数を返す
		 * @param min 下限
		 * @param max 上限
		 * @return 乱数
		 */
		[[nodiscard]] float randomRange(float min, float max);

		/** @brief 溜まっている液体の本体を描く */
		void drawBody(core::iface::IRenderer3D& renderer) const;

		/** @brief 水面のひとかわと照りを描く */
		void drawSurface(core::iface::IRenderer3D& renderer) const;

		/** @brief 波紋を描く */
		void drawRipples(core::iface::IRenderer3D& renderer) const;

		/** @brief 注がれている筋を描く */
		void drawStream(core::iface::IRenderer3D& renderer) const;

		/** @brief しぶきを描く */
		void drawDroplets(core::iface::IRenderer3D& renderer) const;

		/// @brief 見た目の動きに使う経過時間（秒）
		float m_time{ 0.0f };

		/// @brief 溜まっている嵩（0.0〜1.0）
		float m_amountRatio{ 0.0f };

		/// @brief いま注がれているか
		bool m_isPouring{ false };

		/// @brief 液面の揺れの大きさ（注ぐのをやめると収まっていく）
		float m_wobble{ 0.0f };

		/// @brief 次の波紋を起こすまでの時間（秒）
		float m_rippleTimer{ 0.0f };

		std::vector<Ripple> m_ripples{};
		std::vector<Droplet> m_droplets{};

		/// @brief しぶきのばらつきに使う乱数（見た目だけなので種は固定でよい）
		std::mt19937 m_random{ 20260917 };
	};
} // namespace game::view
