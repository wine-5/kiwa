#pragma once
#include <algorithm>

namespace core::utility
{
	/**
	 * @brief 進み具合（0.0〜1.0）をなめらかな動きに直す
	 *
	 * 見た目の動きは「一定の速さ」だと機械的に見えるため、始まりと終わりで
	 * 速さを変える。持つ状態がないので実体は作らず、すべて静的な関数にする。
	 */
	class Easing
	{
	public:
		Easing() = delete;

		/**
		 * @brief 0.0〜1.0 の範囲に収める
		 * @param t 進み具合
		 * @return 0.0〜1.0 に収めた進み具合
		 */
		static constexpr float clamp01(float t) noexcept
		{
			return std::clamp(t, 0.0f, 1.0f);
		}

		/**
		 * @brief 2つの値のあいだを進み具合で取る
		 * @param from 進み具合が 0.0 のときの値
		 * @param to 進み具合が 1.0 のときの値
		 * @param t 進み具合（0.0〜1.0）
		 * @return あいだの値
		 */
		static constexpr float lerp(float from, float to, float t) noexcept
		{
			return from + (to - from) * clamp01(t);
		}

		/**
		 * @brief ゆっくり始まって加速する
		 * @param t 進み具合（0.0〜1.0）
		 * @return なめらかにした進み具合
		 */
		static constexpr float easeIn(float t) noexcept
		{
			const float clamped{ clamp01(t) };
			return clamped * clamped * clamped;
		}

		/**
		 * @brief 勢いよく始まって静かに止まる
		 * @param t 進み具合（0.0〜1.0）
		 * @return なめらかにした進み具合
		 */
		static constexpr float easeOut(float t) noexcept
		{
			const float inverted{ 1.0f - clamp01(t) };
			return 1.0f - inverted * inverted * inverted;
		}

		/**
		 * @brief ゆっくり始まり、加速し、静かに止まる
		 * @param t 進み具合（0.0〜1.0）
		 * @return なめらかにした進み具合
		 */
		static constexpr float easeInOut(float t) noexcept
		{
			const float clamped{ clamp01(t) };
			return clamped < 0.5f ? easeIn(clamped * 2.0f) * 0.5f
			                      : 0.5f + easeOut(clamped * 2.0f - 1.0f) * 0.5f;
		}

		/**
		 * @brief 行き過ぎてから戻る
		 * @param t 進み具合（0.0〜1.0）
		 * @return 1.0 を一度越えてから 1.0 へ落ち着く進み具合
		 *
		 * 札が飛び出すような、勢いを見せたいときに使う。
		 */
		static constexpr float easeOutBack(float t) noexcept
		{
			const float inverted{ 1.0f - clamp01(t) };
			return 1.0f + inverted * inverted * (OVERSHOOT_SCALE * inverted - OVERSHOOT);
		}

		/**
		 * @brief 端でなめらかに寝る曲線
		 * @param t 進み具合（0.0〜1.0）
		 * @return なめらかにした進み具合
		 *
		 * 水面の減衰など、両端を目立たせたくないところに使う。
		 */
		static constexpr float smoothStep(float t) noexcept
		{
			const float clamped{ clamp01(t) };
			return clamped * clamped * (3.0f - 2.0f * clamped);
		}

		/**
		 * @brief いまの値を目標へ近づける
		 * @param current いまの値
		 * @param target 目標の値
		 * @param rate 近づく速さ（1秒あたりの割合）
		 * @param deltaTime 経過時間（秒）
		 * @return 近づけたあとの値
		 *
		 * 終わりの決まっていない追従に使う。刻みが粗くても行き過ぎないよう、
		 * 1回の進み幅は 1.0 で頭打ちにする。
		 */
		static constexpr float approach(float current, float target, float rate,
		                                float deltaTime) noexcept
		{
			return current + (target - current) * std::min(1.0f, rate * deltaTime);
		}

	private:
		/// @brief 行き過ぎの強さ
		static constexpr float OVERSHOOT{ 1.70158f };

		/// @brief 行き過ぎたあとの戻り具合
		static constexpr float OVERSHOOT_SCALE{ OVERSHOOT + 1.0f };
	};
} // namespace core::utility
