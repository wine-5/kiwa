#pragma once

namespace game::constant
{
	/**
	 * @brief 鳴らす音の在処
	 *
	 * 揃っているものだけを並べてある。要る音の全体像は
	 * `docs/design/audio_assets.md` にあり、まだ無いものはそこを見て足す
	 */
	namespace sound
	{
		// ---- 流し続けるもの ----

		/// @brief 茶室の間（ずっと薄く鳴らす）
		inline constexpr const char* AMBIENCE_TEAROOM{ "assets/sounds/ambience/amb_tearoom.mp3" };

		/// @brief 対局中の曲
		inline constexpr const char* BGM_DUEL_CALM{ "assets/sounds/bgm/bgm_duel_calm.mp3" };

		// ---- 札 ----

		/// @brief 伏せた二枚が現れる
		inline constexpr const char* SE_CARD_APPEAR{ "assets/sounds/se/se_card_appear.mp3" };

		/// @brief 引いた札が中央へ寄る
		inline constexpr const char* SE_CARD_DRAW{ "assets/sounds/se/se_card_draw.mp3" };

		/// @brief 札が返る
		inline constexpr const char* SE_CARD_FLIP{ "assets/sounds/se/se_card_flip.mp3" };

		// ---- 注ぐ ----

		/// @brief 注いでいる間ずっと（嵩に合わせて音程を上げる）
		inline constexpr const char* SE_POUR_LOOP{ "assets/sounds/se/se_pour_loop.mp3" };

		/// @brief 水面が縁と同じ高さになる
		inline constexpr const char* SE_SURFACE_TREMBLE{ "assets/sounds/se/se_surface_tremble.mp3" };

		/// @brief こぼれた瞬間
		inline constexpr const char* SE_SPILL{ "assets/sounds/se/se_spill.mp3" };

		// ---- 手番と決着 ----

		/// @brief 手番が移る
		inline constexpr const char* SE_TURN_CHANGE{ "assets/sounds/se/se_turn_change.mp3" };

		/// @brief 一番を落とす
		inline constexpr const char* SE_ROUND_LOSE{ "assets/sounds/se/se_round_lose.mp3" };

		/// @brief 一番を取る
		inline constexpr const char* SE_ROUND_WIN{ "assets/sounds/se/se_round_win.mp3" };
	} // namespace sound
} // namespace game::constant
