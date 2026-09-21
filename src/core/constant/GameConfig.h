#pragma once

namespace core::constant
{
	/**
	 * @brief ビルド構成ごとの振る舞いをまとめたもの
	 *
	 * 「いま Debug か Release か」を判断する場所をここ一箇所に閉じ込める。
	 * 各所で構成を直接見に行くと、確認用の仕掛けを製品版へ持ち出してしまったり、
	 * 逆に外し忘れたりする。ここの旗を `if constexpr` で見て分ければ、
	 * 使わない側は機械語にも残らない。
	 *
	 * 持つ状態がないので実体は作らず、すべて静的な定数にする
	 */
	class GameConfig
	{
	public:
		GameConfig() = delete;

#if defined(_DEBUG)
		/// @brief 開発中の構成か（Visual Studio の Debug 構成で true）
		static constexpr bool IS_DEBUG{ true };
#else
		static constexpr bool IS_DEBUG{ false };
#endif

		/// @brief 動作確認の台本（Scenario）を差し込むか
		static constexpr bool USES_SCENARIO{ IS_DEBUG };

		/// @brief 画面を画像として書き出せるようにするか
		static constexpr bool ALLOWS_FRAME_CAPTURE{ IS_DEBUG };

		/// @brief 当たり判定や数値など、開発中だけ見たいものを画面に出すか
		static constexpr bool SHOWS_DEBUG_INFO{ IS_DEBUG };

		/// @brief 窓の外へ出ても動かし続けるか（開発中は止まると困る）
		static constexpr bool RUNS_WHILE_INACTIVE{ IS_DEBUG };

		/// @brief 全画面で始めるか
		///
		/// 窓で始める。渡した先の環境によっては全画面での起動が通らないことがあり、
		/// そのとき何も出ないまま終わってしまう。全画面は F1 でいつでも切り替えられる
		static constexpr bool STARTS_FULLSCREEN{ false };

		/// @brief 窓の見出し
		static constexpr const char* WINDOW_TITLE{ IS_DEBUG ? "際 -KIWA- [Debug]"
		                                                    : "際 -KIWA-" };
	};
} // namespace core::constant
