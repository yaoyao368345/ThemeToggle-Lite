#pragma once

enum class StartupStatus {
    Disabled,
    Registered,
    StalePath,
    Error,
};

StartupStatus GetStartupStatus();

bool SetStartupEnabled(bool enabled);
