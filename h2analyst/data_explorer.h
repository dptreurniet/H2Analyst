#ifndef DATA_EXPLORER_H
#define DATA_EXPLORER_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QItemSelectionModel>
#include <QRegularExpression>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QPushButton>

#include <unordered_set>
#include <boost/algorithm/string.hpp>

#include "datafile.h"
#include "dataset.h"
#include "namespace.h"

class DataExplorer;

class DatasetTreeView : public QTreeView
{
    Q_OBJECT

    QStandardItemModel* mItemModel;
    QSortFilterProxyModel* mFilterModel;
    QItemSelectionModel* mTreeSelectionModel;

    QStandardItem* createTreeItem(H2A::TreeItemType type, QString label, H2A::dataset_ptr dataset = nullptr);

    // ----- Options ------
    struct TreeViewOptions {
        bool hideSingleFileRow;
    } mOptions;

protected:
    virtual void startDrag(Qt::DropActions supportedActions) override;

public:
    DatasetTreeView(QWidget* parent = nullptr);
    const H2A::dataset_vec getSelectedDatasets();
    inline QSortFilterProxyModel* filterModel() { return mFilterModel; }
    inline TreeViewOptions& options() { return mOptions; }

public slots:
    void update(const H2A::datafile_vec& datafiles);
    void update(const QList<H2A::DatasetProperties>& properties);

signals:
    void updated();

};

class DataExplorer : public QWidget
{
    Q_OBJECT

    QVBoxLayout* mLayout;
    QLineEdit* mFilterInput;
    DatasetTreeView* mTreeView;
    QPushButton* mPlaceholderButton;

public:
    explicit DataExplorer(QWidget *parent = nullptr);
    inline const H2A::dataset_vec getSelectedDatasets() { return mTreeView->getSelectedDatasets(); }
    inline void setHideSingleFileRow(bool hide) { mTreeView->options().hideSingleFileRow = hide; }
    void updateTreeView(const H2A::datafile_vec& datafiles);
    void updateTreeView(const QList<H2A::DatasetProperties>& properties);
    void setPlaceholderText(const QString& text);

private slots:
    void filterChanged();
    void expandAll();
    void updatePlaceholder();

signals:
    void placeholderClicked();
};

#endif // DATA_EXPLORER_H
