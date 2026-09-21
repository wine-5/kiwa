// 自前ヘッダを先に include する（DxLib のマクロと定数名が衝突するのを防ぐ）
#include "Bootstrap.h"

#if defined(__APPLE__)

/**
 * @brief iOS での入口
 *
 * DXライブラリの iOS 版は、通常の main ではなく ios_main を呼ぶ決まりになっている。
 * ここには入口の作法だけを置き、中身は Windows と同じ bootstrap::run に任せる
 * @return 正常終了なら0、初期化に失敗したら-1
 */
int ios_main(void)
{
	return bootstrap::run();
}

#endif // defined(__APPLE__)
