#pragma once

namespace bootstrap
{
	/**
	 * @brief DxLib を起こしてゲームを動かし、後片付けまで行う
	 *
	 * ここには OS ごとの違いを持ち込まない。入口（WinMain など）は OS ごとに
	 * 別のファイルへ置き、そこからこれを呼ぶ
	 * @return 正常終了なら0、初期化に失敗したら-1
	 */
	int run();
} // namespace bootstrap
