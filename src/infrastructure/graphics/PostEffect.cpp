#include "infrastructure/graphics/PostEffect.h"
#include "DxLib.h"

namespace
{
	/// @brief 滲ませる面の縮小率（小さいほど広く滲む）
	constexpr int BLOOM_DIVISOR{ 4 };

	/// @brief この明るさより暗いところは滲ませない
	constexpr int BLOOM_THRESHOLD{ 186 };

	/// @brief ぼかしの幅
	constexpr int BLUR_PIXEL_WIDTH{ 16 };

	/// @brief ぼかしの強さ
	constexpr int BLUR_PARAM{ 900 };

	/// @brief 足し戻す光の強さ
	constexpr int BLOOM_BLEND{ 150 };
} // namespace

namespace infrastructure::graphics
{
	PostEffect::PostEffect(int width, int height) : m_width{ width }, m_height{ height }
	{
		// 3D を描く面には奥行きが要る
		SetDrawValidGraphCreateZBufferFlag(TRUE);
		m_sceneScreen = MakeScreen(width, height, TRUE);

		// 滲ませる面は小さくてよい（そのぶん広くぼける）
		SetDrawValidGraphCreateZBufferFlag(FALSE);
		m_bloomScreen = MakeScreen(width / BLOOM_DIVISOR, height / BLOOM_DIVISOR, TRUE);
	}

	PostEffect::~PostEffect()
	{
		if (m_sceneScreen >= 0)
			DeleteGraph(m_sceneScreen);
		if (m_bloomScreen >= 0)
			DeleteGraph(m_bloomScreen);
	}

	void PostEffect::begin()
	{
		SetDrawScreen(m_sceneScreen);
		ClearDrawScreen();

		// 奥行きの判定は既定で切れている。3D を描いている間は入れておかないと、
		// あとから描いたものが器を突き抜けて手前に出てしまう
		SetUseZBufferFlag(TRUE);
		SetWriteZBufferFlag(TRUE);
	}

	void PostEffect::end()
	{
		// このあとの 2D は奥行きと関係ないので戻しておく
		SetWriteZBufferFlag(FALSE);
		SetUseZBufferFlag(FALSE);

		// 明るいところだけを残して小さい面へ写す
		GraphFilterBlt(m_sceneScreen, m_bloomScreen, DX_GRAPH_FILTER_BRIGHT_CLIP, DX_CMP_LESS,
		               BLOOM_THRESHOLD, TRUE, GetColor(0, 0, 0), 0);
		GraphFilter(m_bloomScreen, DX_GRAPH_FILTER_GAUSS, BLUR_PIXEL_WIDTH, BLUR_PARAM);

		SetDrawScreen(DX_SCREEN_BACK);
		ClearDrawScreen();

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		DrawGraph(0, 0, m_sceneScreen, FALSE);

		// 滲ませた光を足し戻す
		SetDrawBlendMode(DX_BLENDMODE_ADD, BLOOM_BLEND);
		DrawExtendGraph(0, 0, m_width, m_height, m_bloomScreen, FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
} // namespace infrastructure::graphics
