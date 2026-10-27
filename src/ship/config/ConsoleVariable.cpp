#include "ship/config/ConsoleVariable.h"

#include <functional>
#include "ship/utils/filesystemtools/DiskFile.h"
#include "ship/utils/Utils.h"
#include "ship/config/Config.h"
#include "ship/Context.h"

#ifdef _MSC_VER
#define strdup _strdup
#endif

namespace Ship {

ConsoleVariable::ConsoleVariable() : ConsoleVariable(Context::GetRawInstance()->GetHostConfig()) {
}

ConsoleVariable::ConsoleVariable(std::shared_ptr<Config> config) : mConfig(std::move(config)) {
    Load();
}

ConsoleVariable::~ConsoleVariable() {
    SPDLOG_TRACE("destruct console variables");
}

void ConsoleVariable::DelegateToHost(std::shared_ptr<ConsoleVariable> host, std::vector<std::string> keys) {
    mHost = std::move(host);
    mHostKeys = std::move(keys);
}

const std::vector<std::string>& ConsoleVariable::GetEngineKeys() {
    static const std::vector<std::string> sKeys = {
        CVAR_VSYNC_ENABLED,
        CVAR_Z_FIGHTING_MODE,
        CVAR_INTERNAL_RESOLUTION,
        CVAR_MSAA_VALUE,
        CVAR_SDL_WINDOWED_FULLSCREEN,
        CVAR_TEXTURE_FILTER,
        CVAR_IMGUI_CONTROLLER_NAV,
        CVAR_CONSOLE_WINDOW_OPEN,
        CVAR_CONTROLLER_CONFIGURATION_WINDOW_OPEN,
        CVAR_CONTROLLER_DISCONNECTED_WINDOW_OPEN,
        CVAR_CONTROLLER_REORDERING_WINDOW_OPEN,
        CVAR_GFX_DEBUGGER_WINDOW_OPEN,
        CVAR_STATS_WINDOW_OPEN,
        CVAR_ENABLE_MULTI_VIEWPORTS,
        CVAR_LOW_RES_MODE,
        CVAR_SIMULATED_INPUT_LAG,
        CVAR_GAME_OVERLAY_FONT,
        CVAR_MENU_BAR_OPEN,
        CVAR_PREFIX_CONTROLLERS,
        CVAR_PREFIX_ADVANCED_RESOLUTION,
        CVAR_AUDIO_CHANNELS_SETTING,
        CVAR_ALLOW_BACKGROUND_INPUTS,
        CVAR_SCRIPT_SAFE_LEVEL,
    };
    return sKeys;
}

ConsoleVariable* ConsoleVariable::GetHostFor(std::string_view name) const {
    if (mHost == nullptr) {
        return nullptr;
    }
    for (const std::string& key : mHostKeys) {
        if (!name.starts_with(key)) {
            continue;
        }
        if (name.size() == key.size() || name[key.size()] == '.') {
            return mHost.get();
        }
    }
    return nullptr;
}

std::shared_ptr<CVar> ConsoleVariable::Get(const char* name) {
    if (auto host = GetHostFor(name)) {
        return host->Get(name);
    }
    auto it = mVariables.find(name);
    return it != mVariables.end() ? it->second : nullptr;
}

int32_t ConsoleVariable::GetInteger(const char* name, int32_t defaultValue) {
    auto variable = Get(name);

    if (variable != nullptr && variable->Type == ConsoleVariableType::Integer) {
        return variable->Integer;
    }

    return defaultValue;
}

float ConsoleVariable::GetFloat(const char* name, float defaultValue) {
    auto variable = Get(name);

    if (variable != nullptr && variable->Type == ConsoleVariableType::Float) {
        return variable->Float;
    }

    return defaultValue;
}

const char* ConsoleVariable::GetString(const char* name, const char* defaultValue) {
    auto variable = Get(name);

    if (variable != nullptr && variable->Type == ConsoleVariableType::String) {
        return variable->String;
    }

    return defaultValue;
}

Color_RGBA8 ConsoleVariable::GetColor(const char* name, Color_RGBA8 defaultValue) {
    auto variable = Get(name);

    if (variable != nullptr && variable->Type == ConsoleVariableType::Color) {
        return variable->Color;
    } else if (variable != nullptr && variable->Type == ConsoleVariableType::Color24) {
        Color_RGBA8 temp;
        temp.r = variable->Color24.r;
        temp.g = variable->Color24.g;
        temp.b = variable->Color24.b;
        temp.a = 255;
        return temp;
    }

    return defaultValue;
}

Color_RGB8 ConsoleVariable::GetColor24(const char* name, Color_RGB8 defaultValue) {
    auto variable = Get(name);

    if (variable != nullptr && variable->Type == ConsoleVariableType::Color24) {
        return variable->Color24;
    } else if (variable != nullptr && variable->Type == ConsoleVariableType::Color) {
        Color_RGB8 temp;
        temp.r = variable->Color.r;
        temp.g = variable->Color.g;
        temp.b = variable->Color.b;
        return temp;
    }

    return defaultValue;
}

void ConsoleVariable::SetInteger(const char* name, int32_t value) {
    if (auto host = GetHostFor(name)) {
        host->SetInteger(name, value);
        return;
    }
    auto& variable = mVariables[name];
    if (variable == nullptr) {
        variable = std::make_shared<CVar>();
    }

    variable->Type = ConsoleVariableType::Integer;
    variable->Integer = value;
}

void ConsoleVariable::SetFloat(const char* name, float value) {
    if (auto host = GetHostFor(name)) {
        host->SetFloat(name, value);
        return;
    }
    auto& variable = mVariables[name];
    if (variable == nullptr) {
        variable = std::make_shared<CVar>();
    }

    variable->Type = ConsoleVariableType::Float;
    variable->Float = value;
}

void ConsoleVariable::SetString(const char* name, const char* value) {
    if (auto host = GetHostFor(name)) {
        host->SetString(name, value);
        return;
    }
    auto& variable = mVariables[name];
    if (variable == nullptr) {
        variable = std::make_shared<CVar>();
    }

    variable->Type = ConsoleVariableType::String;
    if (variable->String != nullptr) {
        free(variable->String);
    }
    variable->String = strdup(value);
}

void ConsoleVariable::SetColor(const char* name, Color_RGBA8 value) {
    if (auto host = GetHostFor(name)) {
        host->SetColor(name, value);
        return;
    }
    auto& variable = mVariables[name];
    if (!variable) {
        variable = std::make_shared<CVar>();
    }

    variable->Type = ConsoleVariableType::Color;
    variable->Color = value;
}

void ConsoleVariable::SetColor24(const char* name, Color_RGB8 value) {
    if (auto host = GetHostFor(name)) {
        host->SetColor24(name, value);
        return;
    }
    auto& variable = mVariables[name];
    if (!variable) {
        variable = std::make_shared<CVar>();
    }

    variable->Type = ConsoleVariableType::Color24;
    variable->Color24 = value;
}

void ConsoleVariable::RegisterInteger(const char* name, int32_t defaultValue) {
    if (Get(name) == nullptr) {
        SetInteger(name, defaultValue);
    }
}

void ConsoleVariable::RegisterFloat(const char* name, float defaultValue) {
    if (Get(name) == nullptr) {
        SetFloat(name, defaultValue);
    }
}

void ConsoleVariable::RegisterString(const char* name, const char* defaultValue) {
    if (Get(name) == nullptr) {
        SetString(name, defaultValue);
    }
}

void ConsoleVariable::RegisterColor(const char* name, Color_RGBA8 defaultValue) {
    if (Get(name) == nullptr) {
        SetColor(name, defaultValue);
    }
}

void ConsoleVariable::RegisterColor24(const char* name, Color_RGB8 defaultValue) {
    if (Get(name) == nullptr) {
        SetColor24(name, defaultValue);
    }
}

void ConsoleVariable::ClearVariable(const char* name) {
    if (auto host = GetHostFor(name)) {
        host->ClearVariable(name);
        return;
    }
    std::shared_ptr<Config> conf = mConfig;
    auto var = Get(name);
    if (var != nullptr) {
        bool color = var->Type == ConsoleVariableType::Color || var->Type == ConsoleVariableType::Color24;
        if (color) {
            std::string a = StringHelper::Sprintf("%s.%s", name, "A");
            std::string b = StringHelper::Sprintf("%s.%s", name, "B");
            std::string g = StringHelper::Sprintf("%s.%s", name, "G");
            std::string r = StringHelper::Sprintf("%s.%s", name, "R");
            std::string t = StringHelper::Sprintf("%s.%s", name, "Type");
            mVariables.erase(a);
            mVariables.erase(b);
            mVariables.erase(g);
            mVariables.erase(r);
            mVariables.erase(t);
            conf->Erase(std::string("CVars.") + a);
            conf->Erase(std::string("CVars.") + b);
            conf->Erase(std::string("CVars.") + g);
            conf->Erase(std::string("CVars.") + r);
            conf->Erase(std::string("CVars.") + t);
        } else if (var->Type == ConsoleVariableType::String) {
            free(var->String);
            var->String = nullptr;
        }
    }
    mVariables.erase(name);
    conf->Erase(StringHelper::Sprintf("CVars.%s", name));
}

void ConsoleVariable::ClearBlock(const char* name) {
    if (auto host = GetHostFor(name)) {
        host->ClearBlock(name);
        return;
    }
    mConfig->EraseBlock(StringHelper::Sprintf("CVars.%s", name));
    Load();
}

// Goes through the typed accessors so a copy between this store and the host lands in the right one.
void ConsoleVariable::CopyVariable(const char* from, const char* to) {
    auto variableFrom = Get(from);
    if (variableFrom == nullptr || std::string_view(from) == to) {
        return;
    }

    switch (variableFrom->Type) {
        case ConsoleVariableType::Integer:
            SetInteger(to, variableFrom->Integer);
            break;
        case ConsoleVariableType::Float:
            SetFloat(to, variableFrom->Float);
            break;
        case ConsoleVariableType::String:
            SetString(to, variableFrom->String);
            break;
        case ConsoleVariableType::Color:
            SetColor(to, variableFrom->Color);
            break;
        case ConsoleVariableType::Color24:
            SetColor24(to, variableFrom->Color24);
            break;
        default:
            SPDLOG_ERROR("CopyVariable: unknown type {} for {}", static_cast<int>(variableFrom->Type), from);
            break;
    }
}

void ConsoleVariable::Save() {
    if (mHost != nullptr) {
        mHost->Save();
    }
    std::shared_ptr<Config> conf = mConfig;

    for (const auto& variable : mVariables) {
        const std::string key = StringHelper::Sprintf("CVars.%s", variable.first.c_str());

        if (variable.second->Type == ConsoleVariableType::String && variable.second != nullptr) {
            conf->SetString(key, variable.second->String);
        } else if (variable.second->Type == ConsoleVariableType::Integer) {
            conf->SetInt(key, variable.second->Integer);
        } else if (variable.second->Type == ConsoleVariableType::Float) {
            conf->SetFloat(key, variable.second->Float);
        } else if (variable.second->Type == ConsoleVariableType::Color ||
                   variable.second->Type == ConsoleVariableType::Color24) {
            auto keyStr = key.c_str();
            conf->SetUInt(StringHelper::Sprintf("%s.R", keyStr), variable.second->Type == ConsoleVariableType::Color
                                                                     ? variable.second->Color.r
                                                                     : variable.second->Color24.r);
            conf->SetUInt(StringHelper::Sprintf("%s.G", keyStr), variable.second->Type == ConsoleVariableType::Color
                                                                     ? variable.second->Color.g
                                                                     : variable.second->Color24.g);
            conf->SetUInt(StringHelper::Sprintf("%s.B", keyStr), variable.second->Type == ConsoleVariableType::Color
                                                                     ? variable.second->Color.b
                                                                     : variable.second->Color24.b);
            if (variable.second->Type == ConsoleVariableType::Color) {
                conf->SetUInt(StringHelper::Sprintf("%s.A", keyStr), variable.second->Color.a);
                conf->SetString(StringHelper::Sprintf("%s.Type", keyStr), "RGBA");
            } else {
                conf->SetString(StringHelper::Sprintf("%s.Type", keyStr), "RGB");
            }
        }
    }

    conf->Save();
}

void ConsoleVariable::Load() {
    std::shared_ptr<Config> conf = mConfig;
    conf->Reload();
    if (!mVariables.empty()) {
        mVariables.clear();
    }

    LoadFromPath("", conf->GetNestedJson()["CVars"].items());

    LoadLegacy();
}

void ConsoleVariable::LoadFromPath(
    std::string path, nlohmann::detail::iteration_proxy<nlohmann::detail::iter_impl<nlohmann::json>> items) {
    if (!path.empty()) {
        path += ".";
    }

    for (const auto& item : items) {
        std::string itemPath = path + item.key();
        auto value = item.value();
        switch (value.type()) {
            case nlohmann::detail::value_t::array:
                break;
            case nlohmann::detail::value_t::object:
                if (value.contains("Type") && value["Type"].get<std::string>() == "RGBA") {
                    Color_RGBA8 clr;
                    clr.r = value["R"].get<uint8_t>();
                    clr.g = value["G"].get<uint8_t>();
                    clr.b = value["B"].get<uint8_t>();
                    clr.a = value["A"].get<uint8_t>();
                    SetColor(itemPath.c_str(), clr);
                } else if (value.contains("Type") && value["Type"].get<std::string>() == "RGB") {
                    Color_RGB8 clr;
                    clr.r = value["R"].get<uint8_t>();
                    clr.g = value["G"].get<uint8_t>();
                    clr.b = value["B"].get<uint8_t>();
                    SetColor24(itemPath.c_str(), clr);
                } else {
                    LoadFromPath(itemPath, value.items());
                }

                break;
            case nlohmann::detail::value_t::string:
                SetString(itemPath.c_str(), value.get<std::string>().c_str());
                break;
            case nlohmann::detail::value_t::boolean:
                SetInteger(itemPath.c_str(), value.get<bool>());
                break;
            case nlohmann::detail::value_t::number_unsigned:
            case nlohmann::detail::value_t::number_integer:
                SetInteger(itemPath.c_str(), value.get<int>());
                break;
            case nlohmann::detail::value_t::number_float:
                SetFloat(itemPath.c_str(), value.get<float>());
                break;
            default:;
        }
    }
}
void ConsoleVariable::LoadLegacy() {
    auto conf = Context::GetPathRelativeToAppDirectory("cvars.cfg");
    if (DiskFile::Exists(conf)) {
        const auto lines = DiskFile::ReadAllLines(conf);

        for (const std::string& line : lines) {
            std::vector<std::string> cfg = StringHelper::Split(line, " = ");
            if (line.empty()) {
                continue;
            }
            if (cfg.size() < 2) {
                continue;
            }

            if (cfg[1].find("\"") == std::string::npos && (cfg[1].find("#") != std::string::npos)) {
                std::string value(cfg[1]);
                value.erase(std::remove_if(value.begin(), value.end(), [](char c) { return c == '#'; }), value.end());
                auto splitTest = StringHelper::Split(value, "\r")[0];

                uint32_t val = std::stoul(splitTest, nullptr, 16);
                Color_RGBA8 clr;
                clr.r = val >> 24;
                clr.g = val >> 16;
                clr.b = val >> 8;
                clr.a = val & 0xFF;
                SetColor(cfg[0].c_str(), clr);
            }

            if (cfg[1].find("\"") != std::string::npos) {
                std::string value(cfg[1]);
                value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());
                SetString(cfg[0].c_str(), strdup(value.c_str()));
            }
            if (Math::IsNumber<float>(cfg[1])) {
                SetFloat(cfg[0].c_str(), std::stof(cfg[1]));
            }
            if (Math::IsNumber<int>(cfg[1])) {
                SetInteger(cfg[0].c_str(), std::stoi(cfg[1]));
            }
        }

        fs::remove(conf);
    }
}
} // namespace Ship
