#include "fastfall/engine/InputConfig.hpp"

#include "fastfall/engine/input/Input.hpp"
#include "fastfall/engine/input/GamepadInput.hpp"
#include "fastfall/resource/Resources.hpp"

#include "fastfall/util/log.hpp"

#include "input/Gamepad.hpp"
#include <optional>

#include "SDL.h"

#include "nlohmann/json.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

#include <vector>

namespace ff {

namespace InputConfig {

    namespace {

        short _deadzone = (SHRT_MAX / 5); //(short)(0.2 * (double)SHRT_MAX);

        InputType waitingForInput = InputType::NONE;

        std::set<InputState*> listening_states;

        // default input mapping for keyboards
        std::map<SDL_Keycode, InputType> keyMap = {
            {SDLK_w,      {InputType::UP}},
            {SDLK_a,      {InputType::LEFT}},
            {SDLK_s,      {InputType::DOWN}},
            {SDLK_d,      {InputType::RIGHT}},
            {SDLK_SPACE,  {InputType::JUMP}},
            {SDLK_LSHIFT, {InputType::DASH}},
            {SDLK_k,      {InputType::ATTACK}},
        };

        constexpr
        std::pair<GamepadInput, InputType>
        button_map(Button button, InputType input)
        {
            return { GamepadInput::makeButton(button), { input } };
        }

        constexpr
        std::pair<GamepadInput, InputType>
        axis_map(JoystickAxis axis, bool positive, InputType input)
        {
            return { GamepadInput::makeAxis(axis, positive), { input } };
        }

        // default input mapping for joysticks
        std::map<GamepadInput, InputType> joystickMap = {
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

    std::optional<InputType> is_waiting_for_bind() {
        return waitingForInput != InputType::NONE
            ? std::make_optional(waitingForInput)
            : std::nullopt;
    }
    void bindInput(InputType input, SDL_Keycode key) {
        keyMap[key] = { input };
        if (waitingForInput == input) {
            waitingForInput = InputType::NONE;
        }
    }
    void bindInput(InputType input, GamepadInput gamepad) {
        joystickMap[gamepad] = { input };
        if (waitingForInput == input) {
            waitingForInput = InputType::NONE;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    void unbind(InputType input) {
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
            int joycount = SDL_NumJoysticks();
            for (int i = 0; i < joycount; i++) {

                controller = Gamepad(i);

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

    std::optional<InputType> get_type_key(SDL_Keycode key) {
        return opt_find_type(keyMap, key);
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

    void add_listener(InputState& listen) {
        listening_states.insert(&listen);
    }

    void remove_listener(InputState& listen) {
        listening_states.erase(&listen);
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    constexpr std::string_view input_config_file = "input_config.json";

    constexpr std::string_view inputTypeStr[INPUT_COUNT] = {
        "up",
        "left",
        "down",
        "right",
        "jump",
        "dash",
        "attack",
    };

    bool writeConfigFile() {
        namespace nl = nlohmann;
        nl::ordered_json config_json;
        config_json["deadzone"] = fmt::format("{:1.3f}", (float)getAxisDeadzone() / (float)std::numeric_limits<short>::max());
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
        std::vector<std::pair<SDL_Keycode, InputType>> sorted_keys{
            keyMap.begin(),
            keyMap.end()
        };
        std::sort(sorted_keys.begin(), sorted_keys.end(), sort_by_second_first);

        for (auto [key, val] : sorted_keys) {
            if (val == InputType::NONE)
                continue;

            config_json["keyboard"][SDL_GetKeyName(key)] = inputTypeStr[(int)val];
        }

        // controller mappings
        std::vector<std::pair<GamepadInput, InputType>> sorted_joys{
            joystickMap.begin(),
            joystickMap.end()
        };
        std::sort(sorted_joys.begin(), sorted_joys.end(), sort_by_second_first);

        for (auto [key, val] : sorted_joys) {
            if (val == InputType::NONE)
                continue;

            std::string name;
            std::string type;
            if (key.type == GamepadInputType::AXIS) {
                type = "axis";
                name = fmt::format("{}{}",
                                   (key.positiveSide ? "+" : "-"),
                                   SDL_GameControllerGetStringForAxis((SDL_GameControllerAxis)key.axis));
            }
            else if (key.type == GamepadInputType::BUTTON) {
                type = "button";
                name = SDL_GameControllerGetStringForButton((SDL_GameControllerButton)key.button);
            }
            else {
                continue;
            }

            config_json["controller"][type][name] = inputTypeStr[(int)val];
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



            return true;
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
