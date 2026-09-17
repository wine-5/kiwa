#pragma once

namespace core::constant
{
	/// @brief 描画解像度の幅（ウィンドウの実サイズが変わってもこの値で描く）
	inline constexpr int RENDER_WIDTH{ 1280 };

	/// @brief 描画解像度の高さ
	inline constexpr int RENDER_HEIGHT{ 720 };

	/// @brief 画面のカラービット数
	inline constexpr int COLOR_BIT{ 32 };

	/// @brief ゲーム内時間を進める刻み幅（秒）。更新はモニタの速さに依らずこの固定幅で回す
	inline constexpr float FIXED_TIME_STEP{ 1.0f / 60.0f };
} // namespace core::constant
