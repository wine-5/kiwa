#pragma once

namespace game::constant
{
	/**
	 * @brief 操作の案内文と、OS ごとに変わる操作の事情
	 *
	 * Windows はキーボードとマウス、iOS は画面に触れるだけで遊ぶ。
	 * 触れた指は、注ぐ（スペース）と決める（Enter）の両方として渡るので、
	 * 画面のどこを触れても進められる。そのぶん案内の言い回しだけが変わる
	 */
	namespace input_guide
	{
#if defined(__APPLE__)
		/// @brief 遊びの途中でアプリを閉じる項目を出すか（iOS は自分で終わらせない作法）
		inline constexpr bool CAN_QUIT{ false };

		/// @brief 注ぐ合図にキーの絵を出すか（触れて遊ぶので文字で伝える）
		inline constexpr bool SHOWS_KEY_CAP{ false };

		/// @brief 注ぐ合図の文言（キーの絵を出さないとき）
		inline constexpr const char* POUR_HINT{ "押している間、注ぐ" };

		/// @brief 札を引く場面の案内
		inline constexpr const char* DRAW_CARD{ "札を触れて引く" };

		/// @brief 一番が終わって次へ進む案内
		inline constexpr const char* NEXT_ROUND{ "画面を触れて次の勝負へ" };

		/// @brief 試合が終わって結果へ進む案内
		inline constexpr const char* TO_RESULT{ "画面を触れて結果へ" };

		/// @brief 試合が終わってもう一番指す案内
		inline constexpr const char* REMATCH{ "画面を触れてもう一番" };

		/// @brief 結果の画面からタイトルへ戻る案内
		inline constexpr const char* TO_TITLE{ "画面を触れてタイトルへ" };

		/// @brief 休みへ入る札の文字
		inline constexpr const char* PAUSE_LABEL{ "休み" };
#else
		inline constexpr bool CAN_QUIT{ true };
		inline constexpr bool SHOWS_KEY_CAP{ true };
		inline constexpr const char* POUR_HINT{ "" };
		inline constexpr const char* DRAW_CARD{ "←→ で選び、Enter で引く　　札を押してもよい" };
		inline constexpr const char* NEXT_ROUND{ "どちらかのキーで次の勝負へ" };
		inline constexpr const char* TO_RESULT{ "どちらかのキーで結果へ" };
		inline constexpr const char* REMATCH{ "どちらかのキーでもう一番" };
		inline constexpr const char* TO_TITLE{ "Enter でタイトルへ" };
		inline constexpr const char* PAUSE_LABEL{ "休み（Esc）" };
#endif
	} // namespace input_guide
} // namespace game::constant
