#pragma once

namespace infrastructure::debug
{
	class FrameCapture;  // 前方宣言
	class ScriptedInput; // 前方宣言

	/**
	 * @brief 動作確認の段取りを書く場所
	 *
	 * 「何フレーム目にどのキーを押し、どこで画面を撮るか」をここへ書く。
	 * 確認用の処理が Application や入力の実装へ染み出さないよう、ここに閉じ込める。
	 * 普段は何も仕掛けない（install が空）ので、そのまま置いておいてよい
	 */
	class Scenario
	{
	  public:
		/**
		 * @brief 段取りを仕掛ける
		 * @param input キー操作の台本を入れる先
		 * @param capture 画面の書き出しを入れる先
		 */
		static void install(ScriptedInput& input, FrameCapture& capture);

		/**
		 * @brief タイトルを飛ばして対局から始めるかを返す
		 * @details 確認のときだけ true にする
		 * @return 対局から始めるならtrue
		 */
		[[nodiscard]] static bool startsInGame() noexcept;
	};
} // namespace infrastructure::debug
