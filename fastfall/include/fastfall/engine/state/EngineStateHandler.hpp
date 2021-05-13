#pragma once

#include <stack>
#include <memory>

#include "fastfall/render/Color.hpp"

#include "EngineState.hpp"

//using EngineQueue = std::vector<std::unique_ptr<EngineState>>;

namespace ff {

class EngineStateHandler {
public:

	EngineStateHandler();
	EngineStateHandler(std::unique_ptr<EngineState> st);

	void update();

	bool empty() const;

	void clear();

	template<
		typename T,
		typename... Ts,
		typename = typename std::enable_if<
		std::is_base_of<EngineState, std::decay_t<T>>::value &&
		!std::is_same<EngineState, std::decay_t<T>>::value
		>::type
	>
		void createState(Ts&& ... params) {
		auto aState = getActiveState();

		if (aState) {
			aState->createState<T>(std::forward<Ts>(params)...);
			aState->active = false;
			aState->nextState->active = true;
		}
		else {
			root = std::make_unique<T>(std::forward<Ts>(params)...);
			root->active = true;
		}
		stateSize++;
	}

	void pushState(std::unique_ptr<EngineState> st) {
		auto aState = getActiveState();
		if (aState) {
			aState->nextState.swap(st);
			aState->active = false;
			aState->nextState->active = true;
		}
		else {
			root->nextState.swap(st);
			root->active = true;
		}
		stateSize++;
	}

	Color getClearColor();

	EngineState* const getActiveState();

	size_t size() { return stateSize; };

protected:

	size_t stateSize = 0u;
	std::unique_ptr<EngineState> root;

};

}