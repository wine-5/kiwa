#pragma once
#include <string>

namespace core::iface
{
	/**
	 * @brief 同梱したフォントを、このアプリの中だけで使える状態にする
	 *
	 * フォントは本来 OS に入っているものしか名前で呼べない。ファイルを持ち歩いて
	 * 使うには、起動時に登録して終了時に外す必要がある。その手順は OS ごとに
	 * まったく違うので、ここで口だけを決めて中身は platform 層へ置く。
	 *
	 * 登録を外すのはデストラクタの仕事なので、実体は使い終わるまで生かしておくこと
	 */
	class IFontInstaller
	{
	  public:
		virtual ~IFontInstaller() = default;

		/**
		 * @brief フォントを使える状態にする
		 * @param path フォントファイルのパス（実行ファイルからの相対パス、UTF-8）
		 * @return 使える状態になれば true
		 */
		virtual bool install(const std::string& path) = 0;
	};
} // namespace core::iface
