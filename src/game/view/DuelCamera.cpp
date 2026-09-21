#include "game/view/DuelCamera.h"
#include "core/interface/ICamera.h"
#include "core/utility/Easing.h"
#include <algorithm>

namespace
{
	using core::utility::Vector3;

	/// @brief ふだんの視点
	constexpr Vector3 CAMERA_POSITION{ 0.4f, 1.62f, -1.9f };

	/// @brief ふだん見ている先
	constexpr Vector3 CAMERA_TARGET{ -0.12f, 0.52f, 0.0f };

	/// @brief ふだんの画角
	constexpr float CAMERA_FOV{ 0.85f };

	/// @brief 近くの見切り
	constexpr float CAMERA_NEAR{ 0.1f };

	/// @brief 遠くの見切り
	constexpr float CAMERA_FAR{ 100.0f };

	/// @brief 寄り切ったときの視点
	constexpr Vector3 CAMERA_CLOSE{ 0.28f, 1.26f, -1.40f };

	/// @brief 寄り切ったときの画角（狭いほど寄って見える）
	constexpr float CAMERA_CLOSE_FOV{ 0.76f };

	/// @brief じわりと寄り始める嵩
	constexpr float CREEP_START{ 0.5f };

	/// @brief 注いでいる間の寄りの上限（寄り切りは注ぎ終わりのために残す）
	constexpr float CREEP_MAX{ 0.45f };

	/// @brief この嵩を越えて手を離したら、縁へ寄って一拍置く
	constexpr float BRINK_AMOUNT{ 0.78f };

	/// @brief 寄ったまま止まっている時間（秒）
	constexpr float LINGER_TIME{ 1.25f };

	/// @brief 寄る速さ（息を詰める場面なので素早く）
	constexpr float ZOOM_IN_RATE{ 4.2f };

	/// @brief 引く速さ（ゆっくり戻して余韻を残す）
	constexpr float ZOOM_OUT_RATE{ 1.3f };

	/**
	 * @brief 2点のあいだを取る
	 * @param from 進み具合が 0.0 のときの点
	 * @param to 進み具合が 1.0 のときの点
	 * @param t 進み具合（0.0〜1.0）
	 * @return あいだの点
	 */
	constexpr Vector3 lerp(const Vector3& from, const Vector3& to, float t) noexcept
	{
		return from + (to - from) * t;
	}
} // namespace

namespace game::view
{
	DuelCamera::DuelCamera(core::iface::ICamera& camera) : m_camera{ camera }
	{
	}

	Vector3 DuelCamera::brinkTarget() const
	{
		// 見るのは器の縁。器が変われば高さも変わる
		return Vector3{ 0.0f, m_focus.rimHeight, 0.0f };
	}

	void DuelCamera::update(float deltaTime, const Focus& focus)
	{
		m_focus = focus;

		// 注いでいるあいだだけ、嵩が上がるほどじわりと寄る。
		// 手を離したら引いて元の画へ戻す。毎回そこから寄り直すほうが、
		// 寄ったことがはっきり分かる
		const float creep{ focus.isPouring
			                   ? std::clamp((focus.amountRatio - CREEP_START) / (1.0f - CREEP_START),
			                                0.0f, 1.0f) *
			                         CREEP_MAX
			                   : 0.0f };

		// 注いでいた手が離れた瞬間、際が近ければ寄って一拍止まる。
		// 「越えたか」を見せる時間を作るのがこの寄りの要
		if (m_wasPouring && !focus.isPouring && focus.amountRatio >= BRINK_AMOUNT)
			m_lingerTime = LINGER_TIME;

		// こぼしたときも、伝い落ちるところを見せたいので寄ったままにする
		if (focus.isOverflowed)
			m_lingerTime = LINGER_TIME;

		m_wasPouring = focus.isPouring;
		m_lingerTime = std::max(0.0f, m_lingerTime - deltaTime);

		const float target{ m_lingerTime > 0.0f ? 1.0f : creep };
		const float rate{ target > m_closeUp ? ZOOM_IN_RATE : ZOOM_OUT_RATE };
		m_closeUp = core::utility::Easing::approach(m_closeUp, target, rate, deltaTime);
	}

	Vector3 DuelCamera::getEye() const
	{
		return lerp(CAMERA_POSITION, CAMERA_CLOSE, core::utility::Easing::easeOut(m_closeUp));
	}

	void DuelCamera::apply() const
	{
		// 際が近いほど器へ寄る。画角も狭めて、見えるものを器だけにする
		const float closeUp{ core::utility::Easing::easeOut(m_closeUp) };

		m_camera.setPerspective(CAMERA_FOV + (CAMERA_CLOSE_FOV - CAMERA_FOV) * closeUp, CAMERA_NEAR,
		                        CAMERA_FAR);
		m_camera.lookAt(getEye(), lerp(CAMERA_TARGET, brinkTarget(), closeUp));
	}
} // namespace game::view
