#pragma once
#include <string>

namespace core::iface
{
	class IRenderer; // 前方宣言
	class IScreen;   // 前方宣言
} // namespace core::iface

namespace game::view
{
	/**
	 * @brief 先攻を決める札の見せ方
	 *
	 * 画面の中央に札を大きく出し、引く合図で返す。返ったあとは左右へ役を振り分けて、
	 * どちらが先攻かを一目で分かるようにする。3D の中に小さく置くと読み取れないため、
	 * この場面だけは画面に重ねて見せる
	 */
	class CardDraw
	{
	  public:
		/**
		 * @brief 表示に必要なもの
		 */
		struct Content
		{
			/// @brief いま札を引く場面かどうか
			bool isActive{ false };

			/// @brief 札を返したか
			bool isRevealed{ false };

			/// @brief 引いた札が「先攻」か（偽なら「後攻」）
			bool isFirstCard{ true };

			/// @brief 左に出す役（一の手のぶん）
			std::string leftLabel{};

			/// @brief 右に出す役（二の手のぶん）
			std::string rightLabel{};
		};

		/**
		 * @brief 使うテクスチャとフォント
		 */
		struct Resources
		{
			int backTexture{ -1 };
			int firstTexture{ -1 };
			int secondTexture{ -1 };
			int headingFont{ -1 };
			int bodyFont{ -1 };
		};

		CardDraw() = default;

		/**
		 * @brief 見た目の動きを進める
		 * @param deltaTime 進める時間（秒）
		 * @param content 表示する内容
		 */
		void advance(float deltaTime, const Content& content);

		/**
		 * @brief 札と役の振り分けを描く
		 * @param renderer 2D 描画
		 * @param screen 画面サイズの取得
		 * @param resources 使うテクスチャとフォント
		 */
		void draw(core::iface::IRenderer& renderer, core::iface::IScreen& screen,
		          const Resources& resources) const;

		/**
		 * @brief いま何か見せているかを返す
		 * @return 見せているならtrue
		 */
		[[nodiscard]] bool isVisible() const noexcept;

	  private:
		Content m_content{};

		/// @brief 札が現れる進み具合（0.0〜1.0）
		float m_appear{ 0.0f };

		/// @brief 札が返る進み具合（0.0〜1.0）
		float m_flip{ 0.0f };

		/// @brief 役を振り分ける進み具合（0.0〜1.0）
		float m_announce{ 0.0f };

		/// @brief 伏せている間の揺れに使う経過時間（秒）
		float m_time{ 0.0f };
	};
} // namespace game::view
