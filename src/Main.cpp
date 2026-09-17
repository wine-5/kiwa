// 自前ヘッダを先に include する（DxLib のマクロと定数名が衝突するのを防ぐ）
#include "Application.h"
#include "core/constant/ScreenConstants.h"
#include "DxLib.h"

/**
 * @brief アプリケーションのエントリポイント
 *
 * DxLib の初期化・終了だけを受け持ち、ゲームの中身には立ち入らない。
 * 初期化より後の一切は Application が引き受ける
 * @return 正常終了なら0、初期化に失敗したら-1
 */
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// ソースは UTF-8 で書くため、DxLib 側の文字コードも UTF-8 に合わせる
	// （DxLib_Init より前でしか変更できない）
	SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);

	SetGraphMode(core::constant::RENDER_WIDTH, core::constant::RENDER_HEIGHT, core::constant::COLOR_BIT);
	ChangeWindowMode(TRUE);
	SetMainWindowText("KasaGameJam");
	SetAlwaysRunFlag(TRUE); // 非アクティブでも描画を続ける（デバッグ中に止まらないように）

	if (DxLib_Init() == -1)
		return -1;

	SetDrawScreen(DX_SCREEN_BACK);

	// 3D 描画の下ごしらえ。Zバッファが無いと奥のものが手前に描かれてしまう
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);
	SetUseBackCulling(TRUE);
	// ライティングは使わない。面ごとに指定した色がそのまま出るので、
	// 木の陰影は色の選び方で作る（光源に頼ると枡の内側が真っ黒に潰れる）
	SetUseLighting(FALSE);

	// 和風の見た目に寄せる。明朝体が無い環境では既定のフォントのままになる
	ChangeFont("游明朝");
	SetFontSize(26);

	{
		// リソースの解放を DxLib_End より先に済ませるため、スコープで囲む
		Application app{ core::constant::RENDER_WIDTH, core::constant::RENDER_HEIGHT };
		app.run();
	}

	DxLib_End();
	return 0;
}
