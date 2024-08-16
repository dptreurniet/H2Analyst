#ifndef NAMESPACE_H
#define NAMESPACE_H

#include <Qt>
#include <QColor>

namespace H2A {

inline const QString listDelimiter = "\n";

namespace SettingsKeys {
const QString PostProList = "postProList";
const QString PostProDatasetList = "postProDatasetList";
};

enum ItemRoles {
    SortRole = Qt::UserRole + 1,
    DatasetRole = Qt::UserRole + 2,
    FilterRole = Qt::UserRole + 3,
    DatasetPropertyRole = Qt::UserRole + 4,
};

enum class TreeItemType {
    File,
    System,
    Subsystem,
    Dataset
};

enum class PlotType {
    Base,
    Time
};

inline QColor getLineColor(const size_t& index) {
    std::vector<QColor> colormap = {
        QColor(1, 58, 126),
        QColor(0, 141, 255),
        QColor(255, 115, 180),
        QColor(200, 1, 255),
        QColor(79, 203, 140),
        QColor(254, 157, 58),
        QColor(250, 232, 88),
        QColor(216, 47, 53)
    };
    return colormap[index%colormap.size()];
}

enum ResamplingMode {
    fixed,
    allDatapoints
};

}


#endif // NAMESPACE_H
