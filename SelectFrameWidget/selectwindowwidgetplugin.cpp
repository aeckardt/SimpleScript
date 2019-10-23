#include "selectwindowwidget.h"
#include "selectwindowwidgetplugin.h"

#include <QtPlugin>

SelectWindowWidgetPlugin::SelectWindowWidgetPlugin(QObject *parent)
    : QObject(parent)
{
    m_initialized = false;
}

void SelectWindowWidgetPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (m_initialized)
        return;

    // Add extension registrations, etc. here

    m_initialized = true;
}

bool SelectWindowWidgetPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *SelectWindowWidgetPlugin::createWidget(QWidget *parent)
{
    return new SelectWindowWidget(parent);
}

QString SelectWindowWidgetPlugin::name() const
{
    return QLatin1String("SelectWindowWidget");
}

QString SelectWindowWidgetPlugin::group() const
{
    return QLatin1String("");
}

QIcon SelectWindowWidgetPlugin::icon() const
{
    return QIcon();
}

QString SelectWindowWidgetPlugin::toolTip() const
{
    return QLatin1String("");
}

QString SelectWindowWidgetPlugin::whatsThis() const
{
    return QLatin1String("");
}

bool SelectWindowWidgetPlugin::isContainer() const
{
    return false;
}

QString SelectWindowWidgetPlugin::domXml() const
{
    return QLatin1String("<widget class=\"SelectWindowWidget\" name=\"selectWindowWidget\">\n</widget>\n");
}

QString SelectWindowWidgetPlugin::includeFile() const
{
    return QLatin1String("selectwindowwidget.h");
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(selectwindowwidgetplugin, SelectWindowWidgetPlugin)
#endif // QT_VERSION < 0x050000
