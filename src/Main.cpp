// 自前ヘッダを先に include する（DxLib のマクロと定数名が衝突するのを防ぐ）
#include "DxLib.h"

namespace
{
	constexpr int COLOR_BIT{ 32 };
	constexpr int RENDER_WIDTH{ 1280 };
	constexpr int RENDER_HEIGHT{ 720 };
} // namespace

/**
 * @brief アプリケーションのエントリポイント
 *
 * DxLib の初期化と終了だけを受け持ち、ゲームの中身には立ち入らない。
 * 初期化に失敗したときだけ -1 を返して即座に終了する
 * @return 正常終了なら0、初期化に失敗したら-1
 */
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// ソースは UTF-8 で書くため、DxLib 側の文字コードも UTF-8 に合わせる
	// （DxLib_Init より前でしか変更できない）
	SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);

	SetGraphMode(RENDER_WIDTH, RENDER_HEIGHT, COLOR_BIT);
	ChangeWindowMode(TRUE);
	SetMainWindowText("KasaGameJam");
	SetAlwaysRunFlag(TRUE); // 非アクティブでも描画を続ける（デバッグ中に止まらないように）

	if (DxLib_Init() == -1)
		return -1;

	SetDrawScreen(DX_SCREEN_BACK);

	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		ClearDrawScreen();
		DrawString(16, 16, "KasaGameJam", GetColor(255, 255, 255));
		ScreenFlip();
	}

	DxLib_End();
	return 0;
}
