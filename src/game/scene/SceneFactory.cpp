#include "game/scene/SceneFactory.h"
#include "game/scene/InGameScene.h"
#include "game/scene/ResultScene.h"
#include "game/scene/TitleScene.h"

namespace game::scene
{
	std::unique_ptr<IScene> SceneFactory::create(SceneType sceneType, const SceneContext& context)
	{
		switch (sceneType)
		{
		case SceneType::InGame:
			return std::make_unique<InGameScene>(context);
		case SceneType::Result:
			return std::make_unique<ResultScene>(context);
		case SceneType::Title:
		default:
			return std::make_unique<TitleScene>(context);
		}
	}
} // namespace game::scene
