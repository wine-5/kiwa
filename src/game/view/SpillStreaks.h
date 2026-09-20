#pragma once
#include "core/utility/Vertex3D.h"
#include "game/view/CupGeometry.h"
#include <random>
#include <vector>

namespace core::iface
{
	class IRenderer3D; // 前方宣言
} // namespace core::iface

namespace game::view
{
	/**
	 * @brief こぼれた茶が外壁を伝って落ちる筋
	 *
	 * 縁を越えた茶は、離れて落ちるのではなく器の外側に張り付いて下りる。
	 * 何本かの筋が縁のあちこちから垂れ始め、器の形に沿って底まで走り、
	 * 台の上の染みへつながる。
	 *
	 * 見た目だけの動きなので Model も Presenter もここを知らない
	 */
	class SpillStreaks
	{
	  public:
		SpillStreaks() = default;

		/**
		 * @brief 器の寸法を伝える
		 * @param shape 器の寸法
		 */
		void setShape(const cup::Shape& shape) noexcept
		{
			m_shape = shape;
		}

		/**
		 * @brief 筋の動きを進める
		 * @param deltaTime 進める時間（秒）
		 * @param isOverflowed いまこぼれているか
		 */
		void update(float deltaTime, bool isOverflowed);

		/**
		 * @brief 筋を描く
		 * @param renderer 3D 描画
		 */
		void draw(core::iface::IRenderer3D& renderer) const;

		/**
		 * @brief 台まで届いた度合いを返す
		 * @return 0.0（まだ届かない）〜1.0（すべての筋が届いた）
		 *
		 * 台の染みは、筋が下りきってから広がる。落ちる前に染みが出ていると
		 * 「どこから来た茶なのか」が分からなくなる
		 */
		[[nodiscard]] float getReach() const noexcept;

	  private:
		/**
		 * @brief 垂れる筋一本
		 */
		struct Streak
		{
			/// @brief 器を回る向き（ラジアン）
			float angle{ 0.0f };

			/// @brief 垂れ始めるまでの待ち（秒）
			float delay{ 0.0f };

			/// @brief 下りる速さ（1秒あたりの進み具合）
			float speed{ 1.0f };

			/// @brief 筋の太さ
			float width{ 0.02f };

			/// @brief 蛇行のずらし
			float wander{ 0.0f };

			/// @brief どこまで下りたか（0.0〜1.0）
			float progress{ 0.0f };
		};

		/**
		 * @brief 筋を引き直す（こぼれた瞬間に一度だけ）
		 */
		void spawn();

		/**
		 * @brief 筋一本を組む
		 * @param streak 筋
		 */
		void buildStreak(const Streak& streak) const;

		/// @brief いま使っている器の寸法
		cup::Shape m_shape{};

		/// @brief 垂れている筋
		std::vector<Streak> m_streaks{};

		/// @brief こぼれていたか（変わり目を見つけるために持つ）
		bool m_wasOverflowed{ false };

		/// @brief 筋の形（毎フレーム組み直す）
		mutable std::vector<core::utility::Vertex3D> m_vertices{};
		mutable std::vector<unsigned short> m_indices{};

		/// @brief 筋の本数や速さを散らすための乱数
		std::mt19937 m_random{ 20260920u };
	};
} // namespace game::view
