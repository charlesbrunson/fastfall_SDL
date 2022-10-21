#include "fastfall/engine/InputConfig.hpp"

#include "fastfall/engine/input/Input.hpp"
#include "fastfall/engine/input/GamepadInput.hpp"
#include "fastfall/resource/Resources.hpp"

#include "fastfall/util/log.hpp"

#include "Gamepad.hpp"
#include <optional>

#include <SDL.h>

#include <vector>
#include <queue>

namespace ff {

namespace InputConfig {

    namespace {

        short _deadzone = (SHRT_MAX / 5); //(short)(0.2 * (double)SHRT_MAX);

        //InputMethod activeInputMethod = InputMethod::KEYBOARD;
        InputType waitingForInput = InputType::NONE;

        /*
        Vec2i mouseWindowPosition;
        Vec2f mouseWorldPosition;
        bool mouseInView = false;

        std::queue<SDL_Event> eventStack;
        constexpr size_t EVENTMAX = 1000;

        //TODO this should be set by the current state?
        InputState inputs[INPUT_COUNT] = {
            InputState(InputType::UP),
            InputState(InputType::LEFT),
            InputState(InputType::DOWN),
            InputState(InputType::RIGHT),
            InputState(InputType::JUMP),
            InputState(InputType::DASH),
            InputState(InputType::ATTACK),
            InputState(InputType::MOUSE1),
            InputState(InputType::MOUSE2)
        };
        */

        struct InputMapValue {
            constexpr InputMapValue() = default;

            constexpr InputMapValue(InputType t)
                : type(t)
            {
            }

            InputType type = InputType::NONE;
            bool active = false;
        };

        // default input mapping for keyboards
        std::map<SDL_Keycode, InputMapValue> keyMap = {
            {SDLK_w,      {InputType::UP}},
            {SDLK_a,      {InputType::LEFT}},
            {SDLK_s,      {InputType::DOWN}},
            {SDLK_d,      {InputType::RIGHT}},
            {SDLK_SPACE,  {InputType::JUMP}},
            {SDLK_LSHIFT, {InputType::DASH}},
            {SDLK_k,      {InputType::ATTACK}},
        };

        // input mapping for mouse
        std::map<MouseButton, InputMapValue> mouseMap = {
            {SDL_BUTTON_LEFT,      {InputType::MOUSE1}},
            {SDL_BUTTON_RIGHT,     {InputType::MOUSE2}},
        };


        constexpr
        std::pair<GamepadInput, InputMapValue>
        button_map(Button button, InputType input)
        {
            return { GamepadInput::makeButton(button), { input } };
        }

        constexpr
        std::pair<GamepadInput, InputMapValue>
        axis_map(JoystickAxis axis, bool positive, InputType input)
        {
            return { GamepadInput::makeAxis(axis, positive), { input } };
        }

        // default input mapping for joysticks
        std::map<GamepadInput, InputMapValue> joystickMap = {
            axis_map(SDL_CONTROLLER_AXIS_LEFTY, GamepadInput::AXIS_DIR_U, InputType::UP),
            axis_map(SDL_CONTROLLER_AXIS_LEFTX, GamepadInput::AXIS_DIR_L, InputType::LEFT),
            axis_map(SDL_CONTROLLER_AXIS_LEFTY, GamepadInput::AXIS_DIR_D, InputType::DOWN),
            axis_map(SDL_CONTROLLER_AXIS_LEFTX, GamepadInput::AXIS_DIR_R, InputType::RIGHT),
            button_map(SDL_CONTROLLER_BUTTON_DPAD_UP,      InputType::UP),
            button_map(SDL_CONTROLLER_BUTTON_DPAD_LEFT,    InputType::LEFT),
            button_map(SDL_CONTROLLER_BUTTON_DPAD_DOWN,    InputType::DOWN),
            button_map(SDL_CONTROLLER_BUTTON_DPAD_RIGHT,   InputType::RIGHT),
            button_map(SDL_CONTROLLER_BUTTON_A,            InputType::JUMP),
            button_map(SDL_CONTROLLER_BUTTON_B,            InputType::DASH),
            button_map(SDL_CONTROLLER_BUTTON_LEFTSHOULDER, InputType::DASH),
            button_map(SDL_CONTROLLER_BUTTON_X,            InputType::ATTACK),
        };


        /*
        InputState* getKeyInput(SDL_Keycode key) {
            auto t = keyMap.find(key);
            if (t != keyMap.end()) {
                unsigned ndx = static_cast<unsigned>(t->second.type);
                return &inputs[ndx];
            }
            return nullptr;
        }
        InputState* getMouseInput(MouseButton button) {
            auto t = mouseMap.find(button);
            if (t != mouseMap.end()) {
                unsigned ndx = static_cast<unsigned>(t->second.type);
                return &inputs[ndx];
            }
            return nullptr;
        }
        InputState* getJoyInput(Button button) {
            auto t = joystickMap.find(GamepadInput::makeButton(button));
            if (t != joystickMap.end()) {
                unsigned ndx = static_cast<unsigned>(t->second.type);
                return &inputs[ndx];
            }
            return nullptr;
        }
        std::pair<InputState*, const GamepadInput*> getAxisInput(JoystickAxis axis, bool side) {
            auto t = joystickMap.find(GamepadInput::makeAxis(axis, side));
            if (t != joystickMap.end()) {
                unsigned ndx = static_cast<unsigned>(t->second.type);
                return std::make_pair(&inputs[ndx], &t->first);
            }
            return std::make_pair(nullptr, nullptr);
        }
        */

        const GamepadInput* getGamepadInput(JoystickAxis axis, bool side) {
            auto t = joystickMap.find(GamepadInput::makeAxis(axis, side));
            if (t != joystickMap.end()) {
                return &t->first;
            }
            return nullptr;
        }

        void clearBinds() {
            keyMap.clear();
            joystickMap.clear();
        }

        std::optional<Gamepad> controller;

    } // anonymous namespace

    //////////////////////////////////////////////////////////////////////////////////////////
    void setAxisDeadzone(short deadzone) {
        _deadzone = deadzone;
    }
    short getAxisDeadzone() {
        return _deadzone;
    }

    //////////////////////////////////////////////////////////////////////////////////////////

    /*
    Vec2i getMouseWindowPosition() {
        return mouseWindowPosition;
    }
    Vec2f getMouseWorldPosition() {
        return mouseWorldPosition;
    }
    void setMouseWorldPosition(Vec2f pos) {
        mouseWorldPosition = pos;
    }

    void setMouseInView(bool in_view) {
        mouseInView = in_view;
    }
    bool getMouseInView() {
        return mouseInView;
    }
    */

    //////////////////////////////////////////////////////////////////////////////////////////

    /*
    void pushEvent(const SDL_Event& e) {

        eventStack.push(e);
        while (eventStack.size() > EVENTMAX) {
            eventStack.pop();
        }
    }
    */

    /*
    void processEvents() {

        constexpr short JOY_AXIS_MAP_THRESHOLD = 25000;

        SDL_Event e;
        while (!eventStack.empty()) {
            e = eventStack.front();
            eventStack.pop();

            int i = 0;
            switch (e.type) {
            case SDL_KEYDOWN:
                if (e.key.repeat != 0) {
                    // disregard repeats
                    break;
                }
                if (waitingForInput != InputType::NONE) {
                    bindInput(waitingForInput, e.key.keysym.sym);
                    waitingForInput = InputType::NONE;
                }
                else if (auto ptr = getKeyInput(e.key.keysym.sym)) {
                    auto& key = keyMap.at(e.key.keysym.sym);

                    // guard against input sticking
                    if (!key.active) {
                        ptr->activate();
                    }
                    key.active = true;
                }
                break;
            case SDL_KEYUP:
                if (auto ptr = getKeyInput(e.key.keysym.sym)) {
                    auto& key = keyMap.at(e.key.keysym.sym);
                    ptr->deactivate();
                    key.active = false;
                }
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                if (waitingForInput != InputType::NONE) {
                    bindInput(waitingForInput, GamepadInput::makeButton(e.cbutton.button));
                    waitingForInput = InputType::NONE;
                }
                else if (auto ptr = getJoyInput(e.cbutton.button)) {
                    auto& button = joystickMap.at(Input::GamepadInput::makeButton(e.cbutton.button));
                    if (!button.active) {
                        ptr->activate();
                    }
                    button.active = true;
                }
                break;
            case SDL_CONTROLLERBUTTONUP:
                if (auto ptr = getJoyInput(e.cbutton.button)) {
                    auto& button = joystickMap.at(Input::GamepadInput::makeButton(e.cbutton.button));
                    if (button.active) {
                        ptr->deactivate();
                        button.active = false;
                    }
                }
                break;
            //case SDL_CONTROLLER:
            //    break;
            case SDL_CONTROLLERAXISMOTION:

                if (waitingForInput != InputType::NONE && abs(e.caxis.value) > JOY_AXIS_MAP_THRESHOLD) {
                    bindInput(waitingForInput, GamepadInput::makeAxis(e.caxis.axis, e.caxis.value > JOY_AXIS_MAP_THRESHOLD));
                    waitingForInput = InputType::NONE;
                }
                else {
                    static auto inRange = [](bool side, float position) -> bool {
                        if (side) {
                            return position >= _deadzone;
                        }
                        else {
                            return position <= -_deadzone;
                        }
                    };

                    auto checkAxisSide = [&e](bool side) {
                        auto r = getAxisInput(e.caxis.axis, side);

                        if (r.first && r.second) {
                            bool inrangeCur = inRange(r.second->positiveSide, e.caxis.value);
                            bool inrangePrev = inRange(r.second->positiveSide, r.first->axis_prev_pos);

                            if (inrangeCur != inrangePrev) {
                                inrangeCur ? r.first->activate() : r.first->deactivate();
                            }
                            r.first->axis_prev_pos = e.caxis.value;
                        }
                    };

                    checkAxisSide(true);
                    checkAxisSide(false);

                }
                break;
            case SDL_MOUSEMOTION:
                //updateMouse = true;
                mouseWindowPosition.x = e.motion.x;
                mouseWindowPosition.y = e.motion.y;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (auto ptr = getMouseInput(e.button.button)) {
                    auto& mouse = mouseMap.at(e.button.button);

                    if (!mouse.active) {
                        ptr->activate();
                        ptr->mouse_press_pos = Vec2i{ e.button.x, e.button.y };
                    }
                    mouse.active = true;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (auto ptr = getMouseInput(e.button.button)) {
                    auto& mouse = mouseMap.at(e.button.button);
                    ptr->deactivate();
                    ptr->mouse_release_pos = Vec2i{ e.button.x, e.button.y };
                    mouse.active = false;
                }
                break;
            case SDL_JOYDEVICEADDED:
            case SDL_JOYDEVICEREMOVED:
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
                Input::updateJoystick();
                break;
            }
        }
    }
    */

    //////////////////////////////////////////////////////////////////////////////////////////

    /*
    void update(secs deltaTime) {
        if (deltaTime > 0.0) {
            processEvents();

            for (InputState& in : inputs) {
                in.update(deltaTime);
            }
        }
    }
    */

    //////////////////////////////////////////////////////////////////////////////////////////
    void bindInput(InputType input, SDL_Keycode key) {
        keyMap[key] = InputMapValue{ input };
    }
    void bindInput(InputType input, GamepadInput gamepad) {
        joystickMap[gamepad] = InputMapValue{ input };
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    void unbind(InputType input) {
        for (auto it = keyMap.begin(); it != keyMap.end();) {
            if (it->second.type == input) {
                it = keyMap.erase(it);
            }
            else {
                it++;
            }
        }
        for (auto it = joystickMap.begin(); it != joystickMap.end();) {
            if (it->second.type == input) {
                it = joystickMap.erase(it);
            }
            else {
                it++;
            }
        }
        unsigned ndx = static_cast<unsigned>(input);
        //inputs[ndx].reset();
    }

    //////////////////////////////////////////////////////////////////////////////////////////

    /*
    void resetState() {
        //updateMouse = false;
        mouseInView = false;
        for (auto& it : inputs) {
            it.reset();
        }
    }
    */

    //////////////////////////////////////////////////////////////////////////////////////////

    /*
    bool isPressed(InputType input, secs bufferWindow) {
        unsigned ndx = static_cast<unsigned>(input);
        return inputs[ndx].is_pressed(bufferWindow);
    }
    bool isHeld(InputType input) {
        unsigned ndx = static_cast<unsigned>(input);
        return inputs[ndx].is_held();
    }
    void confirmPress(InputType input) {
        unsigned ndx = static_cast<unsigned>(input);
        inputs[ndx].confirm_press();
    }
    */

    //////////////////////////////////////////////////////////////////////////////////////////

    void updateJoystick() {
        if (!controller) {
            int joycount = SDL_NumJoysticks();
            for (int i = 0; i < joycount; i++) {

                controller = Gamepad(i);

                if (controller->isConnected()) {
                    LOG_INFO("Connected game controller");
                    break;
                }
                else {
                    controller = std::nullopt;
                }
            }
        }
        else if (!controller->isConnected()) {
            LOG_INFO("Disconnected game controller");
            controller = std::nullopt;
        }
    }

    void closeJoystick() {
        controller = std::nullopt;
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    inline auto opt_find_type(const auto& map, auto key) {
        auto it = map.find(key);
        return it != map.end() ? std::make_optional(it->second.type) : std::nullopt;
    }

    std::optional<InputType> get_type_key(SDL_Keycode key) {
        return opt_find_type(keyMap, key);
    }

    std::optional<InputType> get_type_mbutton(MouseButton mbutton) {
        return opt_find_type(mouseMap, mbutton);
    }

    std::optional<InputType> get_type_jbutton(Button jbutton) {
        return opt_find_type(joystickMap, GamepadInput::makeButton(jbutton));
    }

    std::pair<std::optional<InputType>, std::optional<InputType>>
    get_type_jaxis(JoystickAxis axis) {
        return {
            opt_find_type(joystickMap, GamepadInput::makeAxis(axis, false)),
            opt_find_type(joystickMap, GamepadInput::makeAxis(axis, true)),
        };
    }

    const GamepadInput* getGamepadInput(JoystickAxis axis, bool side) {
        auto it = joystickMap.find(GamepadInput::makeAxis(axis, side));
        return it != joystickMap.end() ? &it->first : nullptr;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////
    InputObserver::InputObserver() :
        ImGuiContent(ImGuiContentType::SIDEBAR_LEFT, "Input", "System")
    {

    }

    void InputObserver::ImGui_getContent() {

        static const char* inputNames[] = {
            "Up",
            "Left",
            "Down",
            "Right",
            "Jump",
            "Dash",
            "Attack",
            "Mouse1",
            "Mouse2"
        };

        ImGui::Columns(5, "inputs");
        ImGui::Separator();
        ImGui::Text("Input"); ImGui::NextColumn();
        ImGui::Text("Active"); ImGui::NextColumn();
        ImGui::Text("Counter"); ImGui::NextColumn();
        ImGui::Text("Confirmed"); ImGui::NextColumn();
        //ImGui::Text("Enabled"); ImGui::NextColumn();
        ImGui::Text("Duration"); ImGui::NextColumn();

        /*
        ImGui::Separator();
        int i = 0;
        for (auto& in : inputs) {
            ImGui::Text("%s", inputNames[i]); ImGui::NextColumn();
            ImGui::Text("%d", in.is_active()); ImGui::NextColumn();
            ImGui::Text("%d", in.num_activators()); ImGui::NextColumn();

            if (in.is_confirmable()) {
                static char confirmbuf[32];
                sprintf(confirmbuf, "Confirm##%d", i);
                if (ImGui::SmallButton(confirmbuf)) {
                    confirmPress(in.type);
                }
            }
            ImGui::NextColumn();

            //ImGui::Text("%d", in.is_enabled()); ImGui::NextColumn();
            ImGui::Text("%.2f", in.get_last_pressed()); ImGui::NextColumn();
            i++;
        };
        */


        ImGui::Columns(1);
        ImGui::Separator();


        if (ImGui::CollapsingHeader("Bindings")) {

            if (ImGui::Button("Clear All Bindings")) {
                clearBinds();
            }

            ImGui::Separator();
            for (unsigned int i = 0; i < INPUT_COUNT; i++) {
                InputType in = static_cast<InputType>(i);

                ImGui::Columns(3);
                ImGui::Text("%s", inputNames[i]);
                ImGui::NextColumn();

                static char bindbuf[32];
                sprintf(bindbuf, "Add Bind##%d", i);
                static char cancelbindbuf[32];
                sprintf(cancelbindbuf, "Waiting...##%d", i);

                if (waitingForInput == InputType::NONE) {
                    if (ImGui::SmallButton(bindbuf)) {
                        waitingForInput = InputType(i);
                    }
                }
                else {
                    if (ImGui::SmallButton(cancelbindbuf)) {
                        waitingForInput = InputType::NONE;
                    }
                }

                ImGui::NextColumn();
                static char unbindbuf[32];
                sprintf(unbindbuf, "Clear Binds##%d", i);
                if (ImGui::SmallButton(unbindbuf)) {
                    unbind(InputType(i));
                }
                ImGui::NextColumn();
                ImGui::Separator();

                static std::vector<const SDL_Keycode*> keys;
                static std::vector<const GamepadInput*> joys;
                keys.clear();
                joys.clear();

                for (auto& it : keyMap) {
                    if (it.second.type == in) {
                        keys.push_back(&it.first);
                    }
                }
                for (auto& it : joystickMap) {
                    if (it.second.type == in) {
                        joys.push_back(&it.first);
                    }
                }
                for (unsigned i = 0; i < keys.size() || i < joys.size(); i++) {
                    ImGui::NextColumn();
                    if (i < keys.size()) {
                        ImGui::Text("%s", SDL_GetKeyName(*keys.at(i)));
                    }
                    ImGui::NextColumn();
                    if (i < joys.size()) {
                        ImGui::Text("%s", joys.at(i)->toString().c_str());
                    }
                    ImGui::NextColumn();
                }
                ImGui::Columns(1);
                ImGui::Separator();
            }
        }

        if (ImGui::CollapsingHeader("Devices")) {
            ImGui::Spacing();
            if (ImGui::TreeNode("Keyboard")) {
                ImGui::Text("Held Keys");
                ImGui::Separator();
                static std::vector<SDL_Keycode> heldKeys(32);
                static std::vector<std::string> keynames(32);
                heldKeys.clear();

                int keycount = 1;
                const Uint8* state = SDL_GetKeyboardState(&keycount);

                for (SDL_Keycode key = 0; key < keycount; key++) {
                    if (state[SDL_GetScancodeFromKey(key)]) {
                        if (heldKeys.size() < 32) {
                            heldKeys.push_back(key);
                        }
                        else {
                            break;
                        }
                    }
                }
                for (unsigned i = 0u; i < 32u; i++) {
                    if (i < heldKeys.size()) {
                        keynames[i] = SDL_GetKeyName(heldKeys.at(i));
                        ImGui::Text("[%s]", keynames[i].c_str());
                    }
                    else {
                        if (i == 0u) {
							ImGui::NewLine();
                        }
                        else {
                            break;
                        }
                    }
                    ImGui::SameLine();
                }
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::TreePop();
            }

            //display gamepads
            
            static const char* axisNames[] = {
                "X",
                "Y",
                "Z",
                "R",
                "U",
                "V",
                "PovX",
                "PovY"
            };
            

            if (controller && controller->isConnected()) {
                int i = 0;

                SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller->getHandle());

                int buttonCount = SDL_JoystickNumButtons(joystick);
                SDL_JoystickID id = SDL_JoystickInstanceID(joystick);
                                        
                static char controllerbuf[32];
                sprintf(controllerbuf, "Controller %1d", i);

                if (ImGui::TreeNode(controllerbuf)) {

                    ImGui::Text("Controller Information");

                    static char controllerbuf[32];
                    sprintf(controllerbuf, "%d", i);
                    ImGui::Columns(2, controllerbuf);
                    ImGui::SetColumnWidth(0, 120.f);
                    ImGui::Separator();
                    ImGui::Text("Controller Name"); ImGui::NextColumn();
                    ImGui::Text("%s", SDL_JoystickNameForIndex(i)); ImGui::NextColumn();
                    ImGui::Text("Vendor ID"); ImGui::NextColumn();
                    ImGui::Text("0x%04x", SDL_JoystickGetVendor(joystick)); ImGui::NextColumn();
                    ImGui::Text("Product ID"); ImGui::NextColumn();
                    ImGui::Text("0x%04x", SDL_JoystickGetProduct(joystick)); ImGui::NextColumn();
                    ImGui::Text("Product Version"); ImGui::NextColumn();
                    ImGui::Text("0x%04x", SDL_JoystickGetProductVersion(joystick)); ImGui::NextColumn();
                    ImGui::Columns(1);
                    ImGui::Separator();

                    ImGui::Spacing();

                    static char buttonbuf[32];
                    sprintf(buttonbuf, "%d", i);
                    ImGui::Text("Button Count: %d", buttonCount);
                    ImGui::Columns(2, buttonbuf);
                    ImGui::Separator();
                    for (int j = 0; j < buttonCount; j++) {

                        ImGui::Text("Button %2d", j);
                        ImGui::NextColumn();
                        ImGui::Text("%s", SDL_JoystickGetButton(joystick, j) != 0 ? "PRESSED" : "");
                        ImGui::NextColumn();
                    }
                    ImGui::Columns(1);
                    ImGui::Separator();

                    ImGui::Spacing();
                        
                    int axiscount = SDL_JoystickNumAxes(joystick);
                    ImGui::Text("Axis Count: %d", axiscount);
                    for (int j = 0; j < axiscount; j++) {

                        static char axisbuf[32];
                        float position = 100.f * (float)SDL_JoystickGetAxis(joystick, j) / (float)(SHRT_MAX);
                        sprintf(axisbuf, "%d", (int)(position));

                        ImGui::Text("%4s: ", axisNames[j]);
                        ImGui::SameLine();
                        ImGui::ProgressBar((position + 100.f) / 200.f, ImVec2(0.f, 0.f), axisbuf);

                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    int hatcount = SDL_JoystickNumHats(joystick);
                    for (int j = 0; j < hatcount; j++) {
                        static std::string_view hatstr;
                        uint8_t hatpos = SDL_JoystickGetHat(joystick, j);
                        switch (hatpos) {
                            case SDL_HAT_CENTERED: 
                                hatstr = "SDL_HAT_CENTERED";
                                break;
                            case SDL_HAT_UP:
                                hatstr = "SDL_HAT_UP";
                                break;
                            case SDL_HAT_RIGHT:
                                hatstr = "SDL_HAT_RIGHT";
                                break;
                            case SDL_HAT_DOWN:
                                hatstr = "SDL_HAT_DOWN";
                                break;
                            case SDL_HAT_LEFT:
                                hatstr = "SDL_HAT_LEFT";
                                break;
                            case SDL_HAT_RIGHTUP:
                                hatstr = "SDL_HAT_RIGHTUP";
                                break;
                            case SDL_HAT_RIGHTDOWN:
                                hatstr = "SDL_HAT_RIGHTDOWN";
                                break;
                            case SDL_HAT_LEFTUP:
                                hatstr = "SDL_HAT_LEFTUP";
                                break;
                            case SDL_HAT_LEFTDOWN:
                                hatstr = "SDL_HAT_LEFTDOWN";
                                break;
                        }
                        ImGui::Text("Hat %2d: %s", j, hatstr.data());
                    }

                    ImGui::TreePop();
                }
            } 
        }
    }

} // namespace Input

}
