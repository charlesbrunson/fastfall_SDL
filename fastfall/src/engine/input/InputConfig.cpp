#include "fastfall/engine/input/InputConfig.hpp"

#include "fastfall/engine/input/InputHandle.hpp"
#include "fastfall/engine/input/GamepadInput.hpp"
#include "fastfall/resource/Resources.hpp"

#include "fastfall/util/log.hpp"

#include "Gamepad.hpp"
#include <optional>

#include "SDL3/SDL.h"

#include "nlohmann/json.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

#include <vector>

namespace ff {

namespace InputConfig {

    namespace {

        short _deadzone = (SHRT_MAX / 5); //(short)(0.2 * (double)SHRT_MAX);

        Input waitingForInput = Input::None;

        std::set<InputState*> listening_states;

        // default input mapping for keyboards
        std::map<SDL_Keycode, Input> keyMap = {
            {SDLK_W,      {Input::Up}},
            {SDLK_A,      {Input::Left}},
            {SDLK_S,      {Input::Down}},
            {SDLK_D,      {Input::Right}},
            {SDLK_SPACE,  {Input::Jump}},
            {SDLK_LSHIFT, {Input::Dash}},
            {SDLK_K,      {Input::Attack}},
        };

        constexpr
        std::pair<GamepadInput, Input>
        button_map(Button button, Input input)
        {
            return { GamepadInput::makeButton(button), { input } };
        }

        constexpr
        std::pair<GamepadInput, Input>
        axis_map(JoystickAxis axis, bool positive, Input input)
        {
            return { GamepadInput::makeAxis(axis, positive), { input } };
        }

        // default input mapping for joysticks
        std::map<GamepadInput, Input> joystickMap = {
            axis_map(  SDL_GAMEPAD_AXIS_LEFTY, GamepadInput::AXIS_DIR_U, Input::Up),
            axis_map(  SDL_GAMEPAD_AXIS_LEFTX, GamepadInput::AXIS_DIR_L, Input::Left),
            axis_map(  SDL_GAMEPAD_AXIS_LEFTY, GamepadInput::AXIS_DIR_D, Input::Down),
            axis_map(  SDL_GAMEPAD_AXIS_LEFTX, GamepadInput::AXIS_DIR_R, Input::Right),
            button_map(SDL_GAMEPAD_BUTTON_DPAD_UP, Input::Up),
            button_map(SDL_GAMEPAD_BUTTON_DPAD_LEFT, Input::Left),
            button_map(SDL_GAMEPAD_BUTTON_DPAD_DOWN, Input::Down),
            button_map(SDL_GAMEPAD_BUTTON_DPAD_RIGHT, Input::Right),
            button_map(SDL_GAMEPAD_BUTTON_SOUTH, Input::Jump),
            button_map(SDL_GAMEPAD_BUTTON_EAST, Input::Dash),
            button_map(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, Input::Dash),
            button_map(SDL_GAMEPAD_BUTTON_WEST, Input::Attack),
        };

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

    std::optional<Input> is_waiting_for_bind() {
        return waitingForInput != Input::None
            ? std::make_optional(waitingForInput)
            : std::nullopt;
    }
    void bindInput(Input input, SDL_Keycode key) {
        keyMap[key] = { input };
        if (waitingForInput == input) {
            waitingForInput = Input::None;
        }
    }
    void bindInput(Input input, GamepadInput gamepad) {
        joystickMap[gamepad] = { input };
        if (waitingForInput == input) {
            waitingForInput = Input::None;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    void unbind(Input input) {
        bool unbound_any = false;
        for (auto it = keyMap.begin(); it != keyMap.end();) {
            if (it->second == input) {
                it = keyMap.erase(it);
                unbound_any = true;
            }
            else {
                it++;
            }
        }
        for (auto it = joystickMap.begin(); it != joystickMap.end();) {
            if (it->second == input) {
                it = joystickMap.erase(it);
                unbound_any = true;
            }
            else {
                it++;
            }
        }

        if (unbound_any) {
            for (auto st : listening_states) {
                st->notify_unbind(input);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////

    void updateJoystick() {
        if (!controller) {
            int count;
            auto* joystick_ids = SDL_GetJoysticks(&count);
            for (int i = 0; i < count; i++) {

                controller = Gamepad(joystick_ids[i]);

                if (controller->isConnected()) {
                    LOG_INFO("Connected game controller");
                    break;
                }
                else {
                    controller.reset();
                }
            }
        }
        else if (!controller->isConnected()) {
            LOG_INFO("Disconnected game controller");
            controller.reset();
        }
    }

    void closeJoystick() {
        controller.reset();
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    inline auto opt_find_type(const auto& map, auto key) {
        auto it = map.find(key);
        return it != map.end() ? std::make_optional(it->second) : std::nullopt;
    }

    std::optional<Input> get_type_key(SDL_Keycode key) {
        return opt_find_type(keyMap, key);
    }

    std::optional<Input> get_type_jbutton(Button jbutton) {
        return opt_find_type(joystickMap, GamepadInput::makeButton(jbutton));
    }

    std::pair<std::optional<Input>, std::optional<Input>>
    get_type_jaxis(JoystickAxis axis) {
        return {
            opt_find_type(joystickMap, GamepadInput::makeAxis(axis, false)),
            opt_find_type(joystickMap, GamepadInput::makeAxis(axis, true)),
        };
    }

    void add_listener(InputState& listen) {
        listening_states.insert(&listen);
    }

    void remove_listener(InputState& listen) {
        listening_states.erase(&listen);
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    constexpr std::string_view input_config_file = "input_config.json";

    constexpr std::string_view inputTypeToStr[INPUT_COUNT] = {
        "up",
        "left",
        "down",
        "right",
        "jump",
        "dash",
        "attack",
    };

    const std::map<std::string_view, Input> inputStrToType = {
        {"up",     Input::Up},
        {"left",   Input::Left},
        {"down",   Input::Down},
        {"right",  Input::Right},
        {"jump",   Input::Jump},
        {"dash",   Input::Dash},
        {"attack", Input::Attack},
    };

    bool configExists() {
        return std::filesystem::exists(input_config_file);
    }

    bool writeConfigFile() {
        namespace nl = nlohmann;
        nl::ordered_json config_json;
        config_json["deadzone"] = (float)getAxisDeadzone() / (float)(std::numeric_limits<short>::max)();
        config_json["keyboard"];
        config_json["controller"];
        config_json["controller"]["axis"];
        config_json["controller"]["button"];

        auto sort_by_second_first = [](const auto& lhs, const auto& rhs) {
            if (lhs.second != rhs.second) {
                return lhs.second < rhs.second;
            }
            else {
                return lhs.first < rhs.first;
            }
        };

        // keyboard mappings
        std::vector<std::pair<SDL_Keycode, Input>> sorted_keys{
            keyMap.begin(),
            keyMap.end()
        };
        std::sort(sorted_keys.begin(), sorted_keys.end(), sort_by_second_first);

        for (auto [key, val] : sorted_keys) {
            if (val == Input::None)
                continue;

            config_json["keyboard"][SDL_GetKeyName(key)] = inputTypeToStr[(int)val];
        }

        // controller mappings
        std::vector<std::pair<GamepadInput, Input>> sorted_joys{
            joystickMap.begin(),
            joystickMap.end()
        };
        std::sort(sorted_joys.begin(), sorted_joys.end(), sort_by_second_first);

        for (auto [key, val] : sorted_joys) {
            if (val == Input::None)
                continue;

            std::string name;
            std::string type;
            if (key.type == GamepadInputType::AXIS) {
                type = "axis";
                name = fmt::format("{}{}",
                                   (key.positiveSide ? "+" : "-"),
                                   SDL_GetGamepadStringForAxis((SDL_GamepadAxis)key.axis));
            }
            else if (key.type == GamepadInputType::BUTTON) {
                type = "button";
                name = SDL_GetGamepadStringForButton((SDL_GamepadButton)key.button);
            }
            else {
                continue;
            }

            config_json["controller"][type][name] = inputTypeToStr[(int)val];
        }

        // write to file
        std::filesystem::path file_path{input_config_file};
        std::ofstream file_stream{ file_path };
        file_stream << config_json.dump(2);
        return true;
    }

    bool readConfigFile() {
        namespace nl = nlohmann;
        std::filesystem::path file_path{input_config_file};
        std::ifstream file_stream{ file_path };
        nl::json config_json;
        try {
            file_stream >> config_json;
            if (config_json.empty())
                return false;

            std::vector<std::pair<Input, SDL_Keycode>> keys_to_bind;
            std::vector<std::pair<Input, GamepadInput>> joys_to_bind;
            short n_deadzone = (short)(config_json["deadzone"].get<float>() * (float)(std::numeric_limits<short>::max)());
            bool has_error = false;

            for (auto& [key, val] : config_json["keyboard"].items())
            {
                Input type;
                if (auto it = inputStrToType.find(val.get<std::string>()); it != inputStrToType.end()) {
                    type = it->second;
                }
                else {
                    LOG_ERR_("Input config parse error: unknown input type \"{}\"", val.get<std::string>());
                    has_error = true;
                    continue;
                }

                auto keycode = SDL_GetKeyFromName(key.data());
                if (keycode == SDLK_UNKNOWN) {
                    LOG_ERR_("Input config parse error: unknown key \"{}\"", key.data());
                    has_error = true;
                    continue;
                }

                keys_to_bind.emplace_back(type, keycode);
            }

            for (auto& [key, val] : config_json["controller"]["axis"].items())
            {
                char dir = key.at(0);
                std::string axis_str = key.substr(1);

                bool positive = false;
                if (dir == '+') {
                    positive = true;
                }
                else if (dir == '-') {
                    positive = false;
                }
                else {
                    LOG_ERR_("Input config parse error: unexpected axis dir \"{}\", should be + or -", dir);
                    has_error = true;
                    continue;
                }

                Input type;
                if (auto it = inputStrToType.find(val.get<std::string>()); it != inputStrToType.end()) {
                    type = it->second;
                }
                else {
                    LOG_ERR_("Input config parse error: unknown input type \"{}\"", val.get<std::string>());
                    has_error = true;
                    continue;
                }

                auto axis_enum = SDL_GetGamepadAxisFromString(axis_str.data());
                if (axis_enum == SDL_GAMEPAD_AXIS_INVALID) {
                    LOG_ERR_("Input config parse error: unknown axis \"{}\"", axis_str.data());
                    has_error = true;
                    continue;
                }

                auto axis_input = GamepadInput::makeAxis(
                    (JoystickAxis)axis_enum,
                    positive
                );


                joys_to_bind.emplace_back(type, axis_input);
                //bindInput(type, axis_input);
            }

            for (auto& [key, val] : config_json["controller"]["button"].items())
            {
                Input type;
                if (auto it = inputStrToType.find(val.get<std::string>()); it != inputStrToType.end()) {
                    type = it->second;
                }
                else {
                    LOG_ERR_("Input config parse error: unknown input type \"{}\"", val.get<std::string>());
                    has_error = true;
                    continue;
                }

                auto button_enum = SDL_GetGamepadButtonFromString(key.data());
                if (button_enum == SDL_GAMEPAD_BUTTON_INVALID) {
                    LOG_ERR_("Input config parse error: unknown button \"{}\"", key.data());
                    has_error = true;
                    continue;
                }

                auto button_input = GamepadInput::makeButton(
                    (Button)button_enum
                );
                joys_to_bind.emplace_back(type, button_input);
            }

            if (has_error) {
                LOG_ERR_("Input config loading incomplete due to errors");
                return false;
            }
            else {
                // finalize
                setAxisDeadzone(n_deadzone);
                keyMap.clear();
                joystickMap.clear();

                for (auto& v : keys_to_bind) {
                    bindInput(v.first, v.second);
                }

                for (auto& v : joys_to_bind) {
                    bindInput(v.first, v.second);
                }

                return true;
            }
        }
        catch (nl::json::parse_error& e) {
            LOG_ERR_("Input config json parse error");
            LOG_ERR_("\t               message: {}", e.what());
            LOG_ERR_("\t          exception id: {}", e.id);
            LOG_ERR_("\tbyte position of error: {}", e.byte);
        }
        return false;
    }


    //////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////
    InputObserver::InputObserver() :
        ImGuiContent(ImGuiContentType::SIDEBAR_LEFT, "Input", "System")
    {

    }

    void InputObserver::ImGui_getContent(secs deltaTime) {

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

        if (ImGui::CollapsingHeader("Bindings")) {

            if (ImGui::Button("Clear All Bindings")) {
                clearBinds();
            }
            ImGui::SameLine();
            if (ImGui::Button("Load Bindings")) {
                readConfigFile();
            }
            ImGui::SameLine();
            if (ImGui::Button("Save Bindings")) {
                writeConfigFile();
            }

            ImGui::Separator();
            for (unsigned int i = 0; i < INPUT_COUNT; i++) {
                Input in = static_cast<Input>(i);

                ImGui::Columns(3);
                ImGui::Text("%s", inputNames[i]);
                ImGui::NextColumn();

                static char bindbuf[32];
                sprintf(bindbuf, "Add Bind##%d", i);
                static char cancelbindbuf[32];
                sprintf(cancelbindbuf, "Waiting...##%d", i);

                if (waitingForInput == Input::None) {
                    if (ImGui::SmallButton(bindbuf)) {
                        waitingForInput = Input(i);
                    }
                }
                else {
                    if (ImGui::SmallButton(cancelbindbuf)) {
                        waitingForInput = Input::None;
                    }
                }

                ImGui::NextColumn();
                static char unbindbuf[32];
                sprintf(unbindbuf, "Clear Binds##%d", i);
                if (ImGui::SmallButton(unbindbuf)) {
                    unbind(Input(i));
                }
                ImGui::NextColumn();
                ImGui::Separator();

                static std::vector<const SDL_Keycode*> keys;
                static std::vector<const GamepadInput*> joys;
                keys.clear();
                joys.clear();

                for (auto& it : keyMap) {
                    if (it.second == in) {
                        keys.push_back(&it.first);
                    }
                }
                for (auto& it : joystickMap) {
                    if (it.second == in) {
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
                const bool* states = SDL_GetKeyboardState(&keycount);

                for (SDL_Keycode key = 0; key < keycount; key++) {
                    if (states[SDL_GetScancodeFromKey(key, nullptr)]) {
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

                SDL_Joystick* joystick = SDL_GetGamepadJoystick(controller->getHandle());

                int buttonCount = SDL_GetNumJoystickButtons(joystick);
                SDL_JoystickID id = SDL_GetJoystickID(joystick);
                                        
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
                    ImGui::Text("%s", SDL_GetJoystickNameForID(i)); ImGui::NextColumn();
                    ImGui::Text("Vendor ID"); ImGui::NextColumn();
                    ImGui::Text("0x%04x", SDL_GetJoystickVendor(joystick)); ImGui::NextColumn();
                    ImGui::Text("Product ID"); ImGui::NextColumn();
                    ImGui::Text("0x%04x", SDL_GetJoystickProduct(joystick)); ImGui::NextColumn();
                    ImGui::Text("Product Version"); ImGui::NextColumn();
                    ImGui::Text("0x%04x", SDL_GetJoystickProductVersion(joystick)); ImGui::NextColumn();
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
                        ImGui::Text("%s", SDL_GetJoystickButton(joystick, j) != 0 ? "PRESSED" : "");
                        ImGui::NextColumn();
                    }
                    ImGui::Columns(1);
                    ImGui::Separator();

                    ImGui::Spacing();
                        
                    int axiscount = SDL_GetNumJoystickAxes(joystick);
                    ImGui::Text("Axis Count: %d", axiscount);
                    for (int j = 0; j < axiscount; j++) {

                        static char axisbuf[32];
                        float position = 100.f * (float)SDL_GetJoystickAxis(joystick, j) / (float)(SHRT_MAX);
                        sprintf(axisbuf, "%d", (int)(position));

                        ImGui::Text("%4s: ", axisNames[j]);
                        ImGui::SameLine();
                        ImGui::ProgressBar((position + 100.f) / 200.f, ImVec2(0.f, 0.f), axisbuf);

                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    int hatcount = SDL_GetNumJoystickHats(joystick);
                    for (int j = 0; j < hatcount; j++) {
                        static std::string_view hatstr;
                        uint8_t hatpos = SDL_GetJoystickHat(joystick, j);
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
