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
		inline constexpr const char* AMBIENCE_TEAROOM{ "assets/sounds/ambience/amb_tearoom.opus" };

		/// @brief タイトルの曲
		inline constexpr const char* BGM_TITLE{ "assets/sounds/bgm/bgm_title.opus" };

		/// @brief 対局中の曲
		inline constexpr const char* BGM_DUEL_CALM{ "assets/sounds/bgm/bgm_duel_calm.opus" };

		/// @brief 一番の決着（短い締め。繰り返さない）
		inline constexpr const char* BGM_RESULT_ROUND{ "assets/sounds/bgm/bgm_result_round.opus" };

		/// @brief リザルトの曲
		inline constexpr const char* BGM_RESULT_MATCH{ "assets/sounds/bgm/bgm_result_match.opus" };

		// ---- 画面まわり ----

		/// @brief 指しているものが変わる
		inline constexpr const char* SE_CURSOR{ "assets/sounds/se/se_cursor.opus" };

		/// @brief 決める
		inline constexpr const char* SE_DECIDE{ "assets/sounds/se/se_decide.opus" };

		/// @brief 一つ前へ戻る
		inline constexpr const char* SE_BACK{ "assets/sounds/se/se_back.opus" };

		/// @brief 襖が閉まる（場面の切り替わり）
		inline constexpr const char* SE_SCENE_CHANGE{ "assets/sounds/se/se_scene_change.opus" };

		// ---- 札 ----

		/// @brief 伏せた二枚が現れる
		inline constexpr const char* SE_CARD_APPEAR{ "assets/sounds/se/se_card_appear.opus" };

		/// @brief 引いた札が中央へ寄る
		inline constexpr const char* SE_CARD_DRAW{ "assets/sounds/se/se_card_draw.opus" };

		/// @brief 札が返る
		inline constexpr const char* SE_CARD_FLIP{ "assets/sounds/se/se_card_flip.opus" };

		// ---- 注ぐ ----

		/// @brief 土瓶を持ち上げる
		inline constexpr const char* SE_POT_LIFT{ "assets/sounds/se/se_pot_lift.opus" };

		/// @brief 土瓶を置く（手番の終わりの区切り）
		inline constexpr const char* SE_POT_PLACE{ "assets/sounds/se/se_pot_place.opus" };

		/// @brief 注いでいる間ずっと（嵩に合わせて音程を上げる）
		inline constexpr const char* SE_POUR_LOOP{ "assets/sounds/se/se_pour_loop.opus" };

		/// @brief 水面が縁と同じ高さになる
		inline constexpr const char* SE_SURFACE_TREMBLE{ "assets/sounds/se/se_surface_tremble.opus" };

		/// @brief こぼれた瞬間
		inline constexpr const char* SE_SPILL{ "assets/sounds/se/se_spill.opus" };

		/// @brief 茶が外壁を伝って落ちる
		inline constexpr const char* SE_SPILL_RUN{ "assets/sounds/se/se_spill_run.opus" };

		// ---- 手番と決着 ----

		/// @brief 手番が移る
		inline constexpr const char* SE_TURN_CHANGE{ "assets/sounds/se/se_turn_change.opus" };

		/// @brief 一番を落とす
		inline constexpr const char* SE_ROUND_LOSE{ "assets/sounds/se/se_round_lose.opus" };

		/// @brief 一番を取る
		inline constexpr const char* SE_ROUND_WIN{ "assets/sounds/se/se_round_win.opus" };

		/// @brief 掛軸が下りる
		inline constexpr const char* SE_SCROLL_OPEN{ "assets/sounds/se/se_scroll_open.opus" };

		/// @brief 勝の落款が押される
		inline constexpr const char* SE_SEAL_STAMP{ "assets/sounds/se/se_seal_stamp.opus" };

		/// @brief 対局の決着
		inline constexpr const char* SE_MATCH_WIN{ "assets/sounds/se/se_match_win.opus" };
	} // namespace sound
} // namespace game::constant
