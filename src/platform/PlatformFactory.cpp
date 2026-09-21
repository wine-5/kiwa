#include "platform/PlatformFactory.h"

#if defined(_WIN32)
#include "platform/windows/WindowsFontInstaller.h"
#endif

namespace
{
	/**
	 * @brief 何もしないフォントの登録
	 *
	 * OS 側で同梱フォントが最初から使える場合（iOS のように、束ねたファイルを
	 * アプリの持ち物として扱う仕組みがある場合）は、登録の手当てが要らない。
	 * そのときはこれを渡して、呼ぶ側の書き方を変えずに済ませる
	 */
	class NullFontInstaller final : public core::iface::IFontInstaller
	{
	  public:
		bool install([[maybe_unused]] const std::string& path) override
		{
			return true;
		}
	};
} // namespace

namespace platform
{
	std::unique_ptr<core::iface::IFontInstaller> PlatformFactory::createFontInstaller()
	{
#if defined(_WIN32)
		return std::make_unique<windows::WindowsFontInstaller>();
#else
		return std::make_unique<NullFontInstaller>();
#endif
	}
} // namespace platform
