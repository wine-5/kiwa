#pragma once

namespace game::view
{
	/**
	 * @brief 画面に出す器の見た目
	 *
	 * View は Model を見ないので、器の種類はここで別に定義する。
	 * Model の VesselType からの読み替えは Presenter が受け持つ
	 */
	enum class VesselLook
	{
		Guinomi,   ///< ぐい呑
		Yunomi,    ///< 湯呑
		Sobachoko, ///< そば猪口
		Chawan,    ///< 茶碗
	};
} // namespace game::view
