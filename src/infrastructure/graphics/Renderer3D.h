#pragma once
#include "core/base/NonCopyable.h"
#include "core/interface/IRenderer3D.h"

namespace infrastructure::graphics
{
	/**
	 * @brief DxLib を使った IRenderer3D の実装
	 */
	class Renderer3D final : public core::iface::IRenderer3D, private core::base::NonCopyable
	{
	  public:
		Renderer3D() = default;

		void drawBox(const core::utility::Vector3& minCorner, const core::utility::Vector3& maxCorner,
		             const core::utility::Color& color, bool isFilled = true) override;

		void drawCapsule(const core::utility::Vector3& from, const core::utility::Vector3& to, float radius,
		                 const core::utility::Color& color) override;

		void drawSphere(const core::utility::Vector3& center, float radius,
		                const core::utility::Color& color) override;

		void flush() override;
	};
} // namespace infrastructure::graphics
