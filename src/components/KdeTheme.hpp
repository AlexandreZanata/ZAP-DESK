#pragma once

#include "AppTheme.hpp"

class QApplication;
class QPalette;

namespace components {

class KdeTheme {
public:
    static void install(QApplication& app);
    static void apply(AppThemeMode mode);
    static QPalette paletteFor(AppThemeMode mode);
};

}  // namespace components
