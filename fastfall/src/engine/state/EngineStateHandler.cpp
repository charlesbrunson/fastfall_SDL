#include <utility>

#include "fastfall/engine/state/EngineStateHandler.hpp"
#include "fastfall/engine/state/EngineState.hpp"

namespace ff {

EngineStateHandler::EngineStateHandler() {

}
EngineStateHandler::EngineStateHandler(std::unique_ptr<EngineState> st) {
	root = std::move(st);
	stateSize = 1u;
}

void EngineStateHandler::update() {

	auto st = getActiveState();
	while (st && st->eAct != EngineStateAction::CONTINUE) {

		switch (st->eAct) {
		case EngineStateAction::GOTO_NEXT:
			if (st->nextState) {
				st->active = false;
				st->nextState->active = true;
			}
			st->eAct = EngineStateAction::CONTINUE;
			break;

		case EngineStateAction::SWAP_NEXT:
			{
				auto next = st->nextState.get();
				auto prev = st->prevState;
				if (!next) {
					st->eAct = EngineStateAction::EXIT_PREV;
					break;
				}
				next->prevState = st->prevState;
				st->active = false;
				std::unique_ptr<EngineState> store = std::move(st->nextState);
				store->active = true;
				if (prev) {
					prev->nextState.reset();
					prev->nextState = std::move(store);
				}
				else {
					root.reset();
					root = std::move(store);
				}
			}
			break;

		case EngineStateAction::EXIT_PREV:
			if (st->prevState) {
				// ask prev to free us
				st->prevState->nextState.reset();
			}
			else {
				// we're root
				root.reset();
			}
			break;

		case EngineStateAction::CLOSE_ALL:
			clear();
			break;
		default: 
			break;
		}
		st = getActiveState();
	}
	if (st)
		st->active = true;
}

bool EngineStateHandler::empty() const {
	return root.get() == nullptr;
};

void EngineStateHandler::clear() {
	root.reset();
};


Color EngineStateHandler::getClearColor() {
	if (auto state = getActiveState()) {
		return state->clearColor;
	}
	return Color::Black;

}

EngineState* const EngineStateHandler::getActiveState() {
	stateSize = 0u;
	EngineState* st = root.get();
	while (st) {
		stateSize++;
		if (st->active)
			break;

		if (st->nextState.get()) {
			st = st->nextState.get();
		}
		else {
			break;
		}
	}
	if (st) st->active = true;
	return st;

};

}