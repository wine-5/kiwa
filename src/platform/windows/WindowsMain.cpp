// 自前ヘッダを先に include する（DxLib のマクロと定数名が衝突するのを防ぐ）
#include "Bootstrap.h"
#include "DxLib.h"

/**
 * @brief Windows での入口
 *
 * 入口の形は OS ごとに違う（iOS なら別のファイルを用意して、そこから同じ
 * bootstrap::run を呼ぶ）。ここには入口の作法だけを置き、中身は持たせない
 * @return 正常終了なら0、初期化に失敗したら-1
 */
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return bootstrap::run();
}
