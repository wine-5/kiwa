#pragma once
#include "core/base/NonCopyable.h"
#include "core/interface/IFontInstaller.h"
#include <string>

namespace platform::windows
{
	/**
	 * @brief 同梱したフォントを Windows へ登録する
	 *
	 * 登録は他のアプリからは見えない（FR_PRIVATE）ので、環境を汚さない。
	 * 外すのはデストラクタで行うため、使い終わるまで生かしておくこと
	 */
	class WindowsFontInstaller final : public core::iface::IFontInstaller,
	                                   private core::base::NonCopyable
	{
	  public:
		WindowsFontInstaller() = default;

		/**
		 * @brief デストラクタ（登録を外す）
		 */
		~WindowsFontInstaller() override;

		bool install(const std::string& path) override;

	  private:
		/// @brief 登録したファイルのパス（解除に使う）
		std::wstring m_widePath{};

		/// @brief 登録できたか
		bool m_isRegistered{ false };
	};
} // namespace platform::windows
