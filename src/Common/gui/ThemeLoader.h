#ifndef THEME_LOADER_H
#define THEME_LOADER_H

#include <QString>

namespace Theme {

class Loader { // TODO a namespace? and about the private functions?

public:
    static QString loadCSS(QString themeDir, QString themeName);
    static QStringList getAvailableThemes(QString themesDir);
    static bool canLoad(const QString &themesDir, const QString &themeName);

private:
    static QStringList getThemeSectionNames();
    static QString loadThemeCSSFiles(QString themeDir, QString themeName);
};

}//namespace

#endif
