#include "infrastructure/debug/FrameCapture.h"
#include "DxLib.h"

namespace infrastructure::debug
{
	void FrameCapture::addShot(int frame, const std::string& path)
	{
		m_shots.push_back(Shot{ frame, path });
	}

	void FrameCapture::endFrame(int width, int height)
	{
		++m_frame;

		for (const Shot& shot : m_shots)
		{
			if (shot.frame == m_frame)
				SaveDrawScreenToPNG(0, 0, width, height, shot.path.c_str());
		}

		if (m_quitFrame >= 0 && m_frame > m_quitFrame)
			m_isFinished = true;
	}
} // namespace infrastructure::debug
