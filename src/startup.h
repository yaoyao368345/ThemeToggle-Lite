#pragma once

enum class StartupStatus {
    Disabled,
    Enabled,
    StalePath,
    Error,
};

StartupStatus GetStartupStatus();

bool SetStartupEnabled(bool enabled);
