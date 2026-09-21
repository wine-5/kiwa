#pragma once
#include "core/interface/IFontInstaller.h"
#include <memory>

namespace platform
{
	/**
	 * @brief 動かす OS ごとの実体を作る
	 *
	 * OS によって中身が変わるものは、ここを通してしか作らない。
	 * こうしておけば「どの OS か」で分かれる場所がこの一つだけになり、
	 * 別の OS へ持っていくときに探し回らずに済む
	 */
	class PlatformFactory
	{
	  public:
		PlatformFactory() = delete;

		/**
		 * @brief 同梱フォントを使える状態にするものを作る
		 * @return その OS に合った実体（手当てが要らない OS では何もしないもの）
		 */
		[[nodiscard]] static std::unique_ptr<core::iface::IFontInstaller> createFontInstaller();
	};
} // namespace platform
