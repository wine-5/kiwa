#include "infrastructure/graphics/Renderer3D.h"
#include "DxLib.h"

namespace
{
	/// @brief 球と柱の分割数（多いほど滑らかだが重くなる）
	constexpr int DIVISION_COUNT{ 16 };

	/**
	 * @brief Color を DxLib の色コードへ変換する
	 * @param color 変換する色
	 * @return DxLib の色コード
	 */
	unsigned int toDxColor(const core::utility::Color& color)
	{
		return GetColor(color.r, color.g, color.b);
	}

	/**
	 * @brief Vector3 を DxLib の VECTOR へ変換する
	 * @param v 変換するベクトル
	 * @return DxLib の VECTOR
	 */
	VECTOR toDxVector(const core::utility::Vector3& v)
	{
		return VGet(v.x, v.y, v.z);
	}
} // namespace

namespace infrastructure::graphics
{
	void Renderer3D::drawBox(const core::utility::Vector3& minCorner, const core::utility::Vector3& maxCorner,
	                         const core::utility::Color& color, bool isFilled)
	{
		DrawCube3D(toDxVector(minCorner), toDxVector(maxCorner), toDxColor(color), GetColor(0, 0, 0),
		           isFilled ? TRUE : FALSE);
	}

	void Renderer3D::drawCapsule(const core::utility::Vector3& from, const core::utility::Vector3& to,
	                             float radius, const core::utility::Color& color)
	{
		DrawCapsule3D(toDxVector(from), toDxVector(to), radius, DIVISION_COUNT, toDxColor(color),
		              GetColor(0, 0, 0), TRUE);
	}

	void Renderer3D::drawSphere(const core::utility::Vector3& center, float radius,
	                            const core::utility::Color& color)
	{
		DrawSphere3D(toDxVector(center), radius, DIVISION_COUNT, toDxColor(color), GetColor(0, 0, 0), TRUE);
	}

	void Renderer3D::setBlend(core::utility::BlendMode mode, float strength)
	{
		// 状態を変える前に、溜まっているぶんを今の設定で描き切る
		RenderVertex();

		const int value{ static_cast<int>(strength * 255.0f) };
		const int clamped{ value < 0 ? 0 : (value > 255 ? 255 : value) };

		switch (mode)
		{
		case core::utility::BlendMode::Alpha:
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, clamped);
			SetWriteZBuffer3D(FALSE); // 透けているものが奥行きを埋めてしまわないように
			break;

		case core::utility::BlendMode::Add:
			SetDrawBlendMode(DX_BLENDMODE_ADD, clamped);
			SetWriteZBuffer3D(FALSE);
			break;

		case core::utility::BlendMode::None:
		default:
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			SetWriteZBuffer3D(TRUE);
			break;
		}
	}

	void Renderer3D::flush()
	{
		RenderVertex();
	}
} // namespace infrastructure::graphics
