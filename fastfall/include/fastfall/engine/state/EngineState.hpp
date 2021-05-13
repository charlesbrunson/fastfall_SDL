#pragma once

#include "fastfall/util/math.hpp"

#include "fastfall/render/Drawable.hpp"

#include "fastfall/engine/time/time.hpp"

//#include "util/Updatable.hpp"

#include <assert.h>
#include <type_traits>
#include <memory>

// TODO
//#include "game/InstanceID.hpp"

//#include <SFML/Graphics.hpp>
//#include <SFML/System.hpp>


// just for putting ImGui tags on widget labels
#define APP_IGID(X) (appendImGuiID(X).c_str())

namespace ff {

class EngineStateHandler;

enum class EngineStateID {
	NO_STATE = 0,
	TEST_STATE
};

enum class EngineStateAction {
	CONTINUE,  // continue running this state

	// nextState must be initialized
	GOTO_NEXT, // notify state handler of nextState, nextState becomes active 
	SWAP_NEXT, // exit and free this state, nextState becomes active

	EXIT_PREV, // exit and free this state, revert to prev state, closes game if last state
	CLOSE_ALL  // exit and free all states, engine will finish running
};

class EngineState : public Drawable {

	friend EngineStateHandler;
public:

	EngineState();
	virtual ~EngineState() = default;

	// no copy
	EngineState(const EngineState&) = delete;
	EngineState& operator=(const EngineState&) = delete;

	// no move
	EngineState(EngineState&&) = delete;
	EngineState& operator=(EngineState&&) = delete;

	template<typename T>
	void initImGuiHeader() {
		ImGuiHeader = typeid(T).name();
		ImGuiHeader = ImGuiHeader.substr(6); // remove "class "
		ImGuiHeader += " #";
		ImGuiHeader += std::to_string(stateIGID);
	}

	virtual void update(secs deltaTime) = 0;
	virtual void predraw(secs deltaTime) = 0;

	virtual void updateImGUI() {};

	// what instance ID this state is associated with, if any (zero)
	// TODO
	//inline InstanceID getInstanceID() { return instanceID; };

	// used by rendered to update view
	inline Vec2f getViewPos() const noexcept { return viewPos; };
	inline float getViewZoom() const noexcept { return viewZoom; };

	// used by renderer
	inline const Color& getClearColor() const noexcept { return clearColor; };

	inline EngineState* getPrevState() const noexcept { return prevState; };
	inline EngineState* getNextState() const noexcept { return nextState.get(); };

	inline const EngineStateAction& getEngineAction() noexcept { return eAct; };

	template<
		typename T,
		typename... Ts,
		typename = typename std::enable_if<
		std::is_base_of<EngineState, std::decay_t<T>>::value &&
		!std::is_same<EngineState, std::decay_t<T>>::value
		>::type
	>
		void createState(Ts&&... params) {

		// protect states down chain from getting dropped
		assert(!nextState.get());

		nextState = std::make_unique<T>(std::forward<Ts>(params)...);
		nextState.get()->prevState = this;
	}

protected:

	// TODO
	//InstanceID instanceID;

	Color clearColor;

	Vec2f viewPos;
	float viewZoom = 1.f;

	EngineStateID stateID = EngineStateID::NO_STATE;
	EngineStateAction eAct = EngineStateAction::CONTINUE;

	inline bool isActiveState() { return active; };

	const std::string& getImGuiHeader();
	std::string appendImGuiID(std::string&& str) const {
		return (str + "##" + ImGuiHeader);
	}

private:

	std::unique_ptr<EngineState> nextState;
	EngineState* prevState = nullptr;

	bool active = false;

	std::string ImGuiHeader;

	int stateIGID;
	static int stateIGIDCounter;
};

}