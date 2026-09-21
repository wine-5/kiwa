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

		/// @brief タイトルの曲
		inline constexpr const char* BGM_TITLE{ "assets/sounds/bgm/bgm_title.mp3" };

		/// @brief 対局中の曲
		inline constexpr const char* BGM_DUEL_CALM{ "assets/sounds/bgm/bgm_duel_calm.mp3" };

		/// @brief 一番の決着（短い締め。繰り返さない）
		inline constexpr const char* BGM_RESULT_ROUND{ "assets/sounds/bgm/bgm_result_round.mp3" };

		/// @brief リザルトの曲
		inline constexpr const char* BGM_RESULT_MATCH{ "assets/sounds/bgm/bgm_result_match.mp3" };

		// ---- 画面まわり ----

		/// @brief 指しているものが変わる
		inline constexpr const char* SE_CURSOR{ "assets/sounds/se/se_cursor.mp3" };

		/// @brief 決める
		inline constexpr const char* SE_DECIDE{ "assets/sounds/se/se_decide.mp3" };

		/// @brief 一つ前へ戻る
		inline constexpr const char* SE_BACK{ "assets/sounds/se/se_back.mp3" };

		/// @brief 襖が閉まる（場面の切り替わり）
		inline constexpr const char* SE_SCENE_CHANGE{ "assets/sounds/se/se_scene_change.mp3" };

		// ---- 札 ----

		/// @brief 伏せた二枚が現れる
		inline constexpr const char* SE_CARD_APPEAR{ "assets/sounds/se/se_card_appear.mp3" };

		/// @brief 引いた札が中央へ寄る
		inline constexpr const char* SE_CARD_DRAW{ "assets/sounds/se/se_card_draw.mp3" };

		/// @brief 札が返る
		inline constexpr const char* SE_CARD_FLIP{ "assets/sounds/se/se_card_flip.mp3" };

		// ---- 注ぐ ----

		/// @brief 土瓶を持ち上げる
		inline constexpr const char* SE_POT_LIFT{ "assets/sounds/se/se_pot_lift.mp3" };

		/// @brief 土瓶を置く（手番の終わりの区切り）
		inline constexpr const char* SE_POT_PLACE{ "assets/sounds/se/se_pot_place.mp3" };

		/// @brief 注いでいる間ずっと（嵩に合わせて音程を上げる）
		inline constexpr const char* SE_POUR_LOOP{ "assets/sounds/se/se_pour_loop.mp3" };

		/// @brief 水面が縁と同じ高さになる
		inline constexpr const char* SE_SURFACE_TREMBLE{ "assets/sounds/se/se_surface_tremble.mp3" };

		/// @brief こぼれた瞬間
		inline constexpr const char* SE_SPILL{ "assets/sounds/se/se_spill.mp3" };

		/// @brief 茶が外壁を伝って落ちる
		inline constexpr const char* SE_SPILL_RUN{ "assets/sounds/se/se_spill_run.mp3" };

		// ---- 手番と決着 ----

		/// @brief 手番が移る
		inline constexpr const char* SE_TURN_CHANGE{ "assets/sounds/se/se_turn_change.mp3" };

		/// @brief 一番を落とす
		inline constexpr const char* SE_ROUND_LOSE{ "assets/sounds/se/se_round_lose.mp3" };

		/// @brief 一番を取る
		inline constexpr const char* SE_ROUND_WIN{ "assets/sounds/se/se_round_win.mp3" };

		/// @brief 掛軸が下りる
		inline constexpr const char* SE_SCROLL_OPEN{ "assets/sounds/se/se_scroll_open.mp3" };

		/// @brief 勝の落款が押される
		inline constexpr const char* SE_SEAL_STAMP{ "assets/sounds/se/se_seal_stamp.mp3" };

		/// @brief 対局の決着
		inline constexpr const char* SE_MATCH_WIN{ "assets/sounds/se/se_match_win.mp3" };
	} // namespace sound
} // namespace game::constant
