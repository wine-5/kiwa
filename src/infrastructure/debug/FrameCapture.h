#pragma once
#include "core/base/NonCopyable.h"
#include <string>
#include <vector>

namespace infrastructure::debug
{
	/**
	 * @brief 決めたフレームで画面を書き出し、決めたフレームで終わらせる仕掛け
	 *
	 * 動作確認のために使う。何も仕掛けなければ何もしないので、置いたままでよい
	 */
	class FrameCapture : private core::base::NonCopyable
	{
	  public:
		FrameCapture() = default;

		/**
		 * @brief 画面を書き出すフレームを足す
		 * @param frame 書き出すフレーム
		 * @param path 書き出し先のパス
		 */
		void addShot(int frame, const std::string& path);

		/**
		 * @brief 終わらせるフレームを決める
		 * @param frame このフレームを過ぎたら終わりにする
		 */
		void setQuitFrame(int frame) noexcept
		{
			m_quitFrame = frame;
		}

		/**
		 * @brief フレームの終わりに呼ぶ（書き出しと終了の判定を行う）
		 * @param width 書き出す幅
		 * @param height 書き出す高さ
		 */
		void endFrame(int width, int height);

		/**
		 * @brief 終わらせるべきかを返す
		 * @return 終わらせるならtrue
		 */
		[[nodiscard]] bool isFinished() const noexcept
		{
			return m_isFinished;
		}

	  private:
		/**
		 * @brief 書き出しの予定
		 */
		struct Shot
		{
			int frame{ 0 };
			std::string path{};
		};

		std::vector<Shot> m_shots{};

		/// @brief いま何フレーム目か
		int m_frame{ 0 };

		/// @brief 終わらせるフレーム（-1 なら終わらせない）
		int m_quitFrame{ -1 };

		/// @brief 終わらせるべきか
		bool m_isFinished{ false };
	};
} // namespace infrastructure::debug
