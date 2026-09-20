#pragma once

namespace game::constant
{
	/**
	 * @brief 作品全体で使う書体
	 *
	 * 書体名は、同梱した ttf を読み込んで初めて使えるようになる名前で、
	 * ファイルと名前が食い違うと黙って別の書体で描かれてしまう。
	 * 取り違えを防ぐため、ファイルの場所と書体名は必ず隣に並べて置く。
	 *
	 * 複数の画面で使うものだけを置き、ひとつの View でしか使わない大きさは
	 * その View に置く（配色と同じ考え方）
	 */
	namespace font
	{
		// ========== 書体 ==========

		/// @brief 見出しに使う毛筆（同梱の ttf を読み込んで使う）
		inline constexpr const char* HEADING_FAMILY{ "KouzanBrushFont" };

		/// @brief 見出しの毛筆の在処
		inline constexpr const char* HEADING_FILE{ "assets/fonts/KouzanMouhitu.ttf" };

		/// @brief 本文に使う書体（細かい字は明朝のほうが読みやすい）
		inline constexpr const char* BODY_FAMILY{ "Noto Serif JP" };


		// ========== 大きさ ==========

		/// @brief 見出しの大きさ
		inline constexpr int HEADING_SIZE{ 48 };

		/// @brief 本文の大きさ
		inline constexpr int BODY_SIZE{ 26 };
	} // namespace font
} // namespace game::constant
