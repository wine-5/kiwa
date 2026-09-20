#pragma once
#include "core/base/NonCopyable.h"
#include <string>

namespace platform::font
{
	/**
	 * @brief 同梱したフォントファイルを、このアプリの中だけで使える状態にする
	 *
	 * フォントは本来 OS に入っているものしか名前で呼べない。ファイルを持ち歩いて
	 * 使うには、起動時に登録して終了時に外す必要がある。その面倒を引き受ける。
	 * 登録は他のアプリからは見えない（FR_PRIVATE）ので、環境を汚さない
	 */
	class FontFile : private core::base::NonCopyable
	{
	  public:
		/**
		 * @brief FontFile のコンストラクタ（フォントを登録する）
		 * @param path フォントファイルのパス（実行ファイルからの相対パス、UTF-8）
		 */
		explicit FontFile(const std::string& path);

		/**
		 * @brief デストラクタ（登録を外す）
		 */
		~FontFile();

		/**
		 * @brief 登録できたかを返す
		 * @return 登録できていれば true
		 */
		[[nodiscard]] bool isRegistered() const noexcept
		{
			return m_isRegistered;
		}

	  private:
		/// @brief 登録したファイルのパス（解除に使う）
		std::wstring m_widePath{};

		/// @brief 登録できたか
		bool m_isRegistered{ false };
	};
} // namespace platform::font
