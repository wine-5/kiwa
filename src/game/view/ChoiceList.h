#pragma once
#include "core/utility/Vector2.h"
#include <string>
#include <vector>

namespace core::iface
{
	class IRenderer; // 前方宣言
	class IScreen;   // 前方宣言
} // namespace core::iface

namespace game::view
{
	/**
	 * @brief 画面の中ほどに並べて一つ選ばせる
	 *
	 * いま何が選ばれているかは、字の大きさ・濃さ・印の位置の三つで見せる。
	 * どれか一つだけで見せると、色の違いに気付きにくい環境で分からなくなる
	 */
	class ChoiceList
	{
	  public:
		/**
		 * @brief 表示に必要な内容
		 */
		struct Content
		{
			/// @brief いま選ばせている場面か
			bool isActive{ false };

			/// @brief 並べるもの
			std::vector<std::string> items{};

			/// @brief それぞれに添える一言
			std::vector<std::string> notes{};

			/// @brief 選ばれている番号
			int selected{ 0 };

			/// @brief 並びの中心を置く高さ（画面の高さに対する割合）
			float centerY{ 0.28f };
		};

		/**
		 * @brief 描くときに要る書体
		 */
		struct Resources
		{
			/// @brief 選ばれているものに使う書体（毛筆）
			int headingFont{ -1 };

			/// @brief 選ばれていないものと添え書きに使う書体
			int bodyFont{ -1 };

			/// @brief 選ばれているものの字の大きさ（罫を字から逃がす幅の見当に使う）
			int headingSize{ 48 };

			/// @brief 添え書きの字の大きさ
			int bodySize{ 26 };
		};

		ChoiceList() = default;

		/**
		 * @brief 選びの動きを進める
		 * @param deltaTime 進める時間（秒）
		 * @param content 表示する内容
		 */
		void update(float deltaTime, const Content& content);

		/**
		 * @brief 選ばれている字を避けて罫を引き始める位置を返す
		 * @param resources 書体
		 * @return 画面中央からの距離
		 */
		[[nodiscard]] float selectedGap(const Resources& resources) const;

		/**
		 * @brief その座標にある項目の番号を返す
		 * @param screen 画面の大きさ
		 * @param position 調べる座標（マウスの位置）
		 * @return 項目の番号。どれにも当たらなければ -1
		 */
		[[nodiscard]] int hitTest(core::iface::IScreen& screen,
		                          const core::utility::Vector2& position) const;

		/**
		 * @brief 並びを描く
		 * @param renderer 2D 描画
		 * @param screen 画面の大きさ
		 * @param resources 書体
		 */
		void draw(core::iface::IRenderer& renderer, core::iface::IScreen& screen,
		          const Resources& resources) const;

	  private:
		/// @brief いま出している内容
		Content m_content{};

		/// @brief 出てくる具合（0.0〜1.0）
		float m_appear{ 0.0f };

		/// @brief 印がいる位置（選ばれている番号を追いかける）
		float m_marker{ 0.0f };

		/// @brief 経過時間（印の明滅に使う）
		float m_time{ 0.0f };
	};
} // namespace game::view
