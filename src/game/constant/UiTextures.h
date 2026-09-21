#pragma once

namespace game::constant
{
	/**
	 * @brief 画面に重ねる UI の画像
	 *
	 * 下絵から切り出したもので、`tools/cut_ui_assets.py` が作り直す。
	 * 置き場所を変えるときはここだけを直せばよい
	 */
	namespace ui
	{
		/// @brief 手番を出す短冊
		inline constexpr const char* TURN_PLATE{ "assets/textures/ui/turn_plate.png" };

		/// @brief 勝ち星を出す短冊
		inline constexpr const char* SCORE_PLATE{ "assets/textures/ui/score_plate.png" };

		/// @brief 一の手の紋
		inline constexpr const char* EMBLEM_ONE{ "assets/textures/ui/emblem_one.png" };

		/// @brief 二の手の紋
		inline constexpr const char* EMBLEM_TWO{ "assets/textures/ui/emblem_two.png" };

		/// @brief スペースキーの絵
		inline constexpr const char* KEY_CAP_SPACE{ "assets/textures/ui/key_cap_space.png" };

		/// @brief Enter キーの絵
		inline constexpr const char* KEY_CAP_ENTER{ "assets/textures/ui/key_cap_enter.png" };

		/// @brief 勝ちの落款
		inline constexpr const char* SEAL_VICTORY{ "assets/textures/ui/seal_victory.png" };

		/// @brief 掛軸（リザルトで使う）
		inline constexpr const char* RESULT_SCROLL{ "assets/textures/ui/result_scroll.png" };
	} // namespace ui
} // namespace game::constant
