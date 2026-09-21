#include "platform/windows/WindowsFontInstaller.h"
#include <windows.h>

namespace
{
	/**
	 * @brief UTF-8 の文字列を Windows の広い文字へ直す
	 * @param text 変換する文字列
	 * @return 変換した文字列
	 */
	std::wstring toWide(const std::string& text)
	{
		if (text.empty())
			return {};

		const int length{ MultiByteToWideChar(CP_UTF8, 0, text.c_str(),
		                                      static_cast<int>(text.size()), nullptr, 0) };
		if (length <= 0)
			return {};

		std::wstring result(static_cast<std::size_t>(length), L'\0');
		MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), result.data(),
		                    length);
		return result;
	}
} // namespace

namespace platform::windows
{
	bool WindowsFontInstaller::install(const std::string& path)
	{
		m_widePath = toWide(path);
		if (m_widePath.empty())
			return false;

		// FR_PRIVATE を付けると、このアプリの中だけで使える状態になる
		m_isRegistered = AddFontResourceExW(m_widePath.c_str(), FR_PRIVATE, nullptr) > 0;
		return m_isRegistered;
	}

	WindowsFontInstaller::~WindowsFontInstaller()
	{
		if (m_isRegistered)
			RemoveFontResourceExW(m_widePath.c_str(), FR_PRIVATE, nullptr);
	}
} // namespace platform::windows
