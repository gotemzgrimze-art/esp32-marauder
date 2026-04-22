#pragma once

#include "app_state.h"

namespace settings_store {

void begin();
AppSettings load();
bool save(const AppSettings &settings);

}  // namespace settings_store
