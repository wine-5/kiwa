#pragma once
#include "core/base/NonCopyable.h"
#include "core/interface/IPostEffect.h"

namespace infrastructure::graphics
{
	/**
	 * @brief DxLib の画像フィルタを使った IPostEffect の実装
	 *
	 * 明るいところだけを取り出して縮小・ぼかし、元の絵へ光を足して戻す
	 */
	class PostEffect final : public core::iface::IPostEffect, private core::base::NonCopyable
	{
	  public:
		/**
		 * @brief PostEffect のコンストラクタ（描画用の面を用意する）
		 * @param width 画面の幅
		 * @param height 画面の高さ
		 */
		PostEffect(int width, int height);

		/**
		 * @brief デストラクタ（用意した面を解放する）
		 */
		~PostEffect() override;

		void begin() override;

		void end() override;

	  private:
		int m_width{ 0 };
		int m_height{ 0 };

		/// @brief 3D を受け取る面
		int m_sceneScreen{ -1 };

		/// @brief 明るいところを滲ませるための小さい面
		int m_bloomScreen{ -1 };
	};
} // namespace infrastructure::graphics
