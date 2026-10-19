#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <random>

using namespace geode::prelude;

// Returns true with the given probability (0-100).
static bool rollChance(int64_t percent) {
	static std::mt19937 rng{std::random_device{}()};
	std::uniform_int_distribution<int> dist(1, 100);
	return dist(rng) <= percent;
}

static void playJumpscare() {
	FMODAudioEngine::sharedEngine()->playEffect("jumpscare.ogg"_spr);
}

// Covers the whole screen in black for `seconds`.
// Returns false if a blackout is already showing (so it doesn't stack).
static bool showBlackout(float seconds) {
	auto scene = CCDirector::sharedDirector()->getRunningScene();
	if (!scene) return false;
	if (scene->getChildByID("blackout"_spr)) return false;

	auto overlay = CCLayerColor::create({0, 0, 0, 255});
	overlay->setID("blackout"_spr);
	scene->addChild(overlay, 99999);

	overlay->runAction(CCSequence::create(
		CCDelayTime::create(seconds),
		CCRemoveSelf::create(),
		nullptr
	));
	return true;
}

class $modify(FoxyPlayer, PlayerObject) {
	bool pushButton(PlayerButton button) {
		bool ret = PlayerObject::pushButton(button);

		if (button != PlayerButton::Jump) return ret;

		// Only in actual levels, and only once per click (ignore player 2 in dual mode)
		auto pl = PlayLayer::get();
		if (!pl || this != pl->m_player1) return ret;

		auto mod = Mod::get();
		if (!mod->getSettingValue<bool>("enabled")) return ret;

		bool soundEveryJump = mod->getSettingValue<bool>("sound-every-jump");
		if (soundEveryJump) playJumpscare();

		if (rollChance(mod->getSettingValue<int64_t>("chance"))) {
			float duration = static_cast<float>(mod->getSettingValue<double>("duration"));
			if (showBlackout(duration) && !soundEveryJump) {
				playJumpscare();
			}
		}

		return ret;
	}
};
