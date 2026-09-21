#include "game/view/Plate.h"
#include "core/interface/IRenderer.h"
#include <algorithm>

namespace
{
	using core::utility::Vector2;
} // namespace

namespace game::view
{
	void Plate::draw(core::iface::IRenderer& renderer, int texture, const Vector2& center,
	                 float width, float alpha, float scale)
	{
		const Vector2 source{ renderer.getTextureSize(texture) };
		if (source.x <= 0.0f || source.y <= 0.0f)
			return;

		const float height{ source.y * scale };
		const float cap{ source.x * CAP_RATIO * scale };
		const float top{ center.y - height * 0.5f };
		const float left{ center.x - width * 0.5f };

		// 端だけで幅が埋まってしまうときは、素直に全体を縮める
		if (width <= cap * 2.0f)
		{
			renderer.drawTexturePart(texture, Vector2{ 0.0f, 0.0f }, source, Vector2{ left, top },
			                         Vector2{ width, height }, alpha);
			return;
		}

		const float sourceCap{ source.x * CAP_RATIO };
		const float sourceMiddle{ source.x - sourceCap * 2.0f };

		// 左の端
		renderer.drawTexturePart(texture, Vector2{ 0.0f, 0.0f }, Vector2{ sourceCap, source.y },
		                         Vector2{ left, top }, Vector2{ cap, height }, alpha);

		// 中ほど（ここだけを伸ばす）
		renderer.drawTexturePart(texture, Vector2{ sourceCap, 0.0f },
		                         Vector2{ sourceMiddle, source.y }, Vector2{ left + cap, top },
		                         Vector2{ width - cap * 2.0f, height }, alpha);

		// 右の端
		renderer.drawTexturePart(texture, Vector2{ source.x - sourceCap, 0.0f },
		                         Vector2{ sourceCap, source.y },
		                         Vector2{ left + width - cap, top }, Vector2{ cap, height }, alpha);
	}

	float Plate::widthFor(const std::string& text, int fontSize, float padding)
	{
		// 書体の幅を測る手だてが無いので、字数から見当をつける。
		// 全角はほぼ文字の大きさぶん、半角はその半分として数える
		float count{ 0.0f };
		for (const char character : text)
		{
			const auto byte{ static_cast<unsigned char>(character) };

			// UTF-8 の続きのバイト（10xxxxxx）は数えない
			if ((byte & 0xC0) == 0x80)
				continue;

			count += byte < 0x80 ? 0.5f : 1.0f;
		}

		return count * static_cast<float>(fontSize) + padding * 2.0f;
	}
} // namespace game::view
