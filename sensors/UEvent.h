
/*
 * Copyright (C) 2024 Paranoid Android
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <unordered_map>
#include <sstream>
#include <string>

namespace android {
namespace hardware {
namespace sensors {

class UEvent {
  public:
    UEvent(const char *data) : msg() {
        while (*data) {
            std::string kv(data);
            data += kv.length() + 1;
            auto pos = kv.find("=");
            if (pos != std::string::npos) {
                std::string key = kv.substr(0, pos);
                std::string value = kv.substr(pos + 1, kv.length());
                msg.insert({key, value});
            }
        }
    }

    bool contains(std::string match) {
        auto pos = match.find("=");
        if (pos != std::string::npos) {
            std::string key = match.substr(0, pos);
            std::string value = match.substr(pos + 1, match.length());
            return get(key, "") == value;
        }
        return match == "" || get(match, "") != "";
    }

    std::string get(std::string key, std::string defaultValue) {
        auto result = msg.find(key);
        return result == msg.end() ? defaultValue : result->second;
    }

    operator std::string() {
        std::ostringstream oss;
        for (const auto& [key, value] : msg) {
            oss << key << " = " << value << "\n";
        }
        return oss.str();
    }

  private:
    std::unordered_map<std::string, std::string> msg;
};

}  // namespace sensors
}  // namespace hardware
}  // namespace android