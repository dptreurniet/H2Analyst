
#ifndef DATA_STORE_H
#define DATA_STORE_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QMenu>
#include <QMessageBox>

#include <vector>

#include "datafile.h"
#include "data_parser.h"
#include "dataset_operators.h"
#include "data_explorer.h"
#include "namespace.h"
#include "post_pro_dialog.h"

void updateTreeView(QStandardItem* root, const H2A::datafile_ptr& datafile);
void updateTreeView(QStandardItem* root, const H2A::datafile_vec& datafiles);
QStandardItem* createTreeItem(H2A::TreeItemType type, std::string label, H2A::dataset_ptr dataset = nullptr);

struct PostProcessor {
    H2A::DatasetProperties properties;
    H2A::ResamplingMode resampling_mode;
    QString expression;

    std::map<QString, QVariant> toMap() const;
    bool operator==(const PostProcessor& o) const;
    static postpro_ptr fromMap(const std::map<QString, QString>& map);
};

using postpro_ptr = QSharedPointer<PostProcessor>;

class DataStore : public QWidget
{
    Q_OBJECT

    QSettings mSettings;

    // ----- Widgets -----
    QVBoxLayout* layout_;
    DataExplorer* data_explorer_;

    // ----- Menus and dialogs-----
    QMenu* item_context_menu_;
    QAction* item_properties_act_;
    PostProDialog* mDialog;

    // ----- Data -----
    H2A::datafile_vec datafiles_;
    QList<postpro_ptr> mPostProcessors;
    void populateDataset(H2A::dataset_ptr& dataset);

public:
    explicit DataStore(QWidget *parent = nullptr);

    void openFile(const std::string& file);
    const H2A::dataset_vec getSelectedDatasets();

    inline const QList<postpro_ptr>& postProcessors() { return mPostProcessors; }

public slots:
    void resampleSelectedDataset();
    void dialog();
    void addPostProcessor(const postpro_ptr& pp);
    void removePostProcessor(const postpro_ptr& pp);
    void savePostProToSettings();
    void readPostProFromSettings();

signals:
    void datafileOpened(const H2A::datafile_ptr& datafile);
    void postProcessorsChanged();
};

#endif // DATA_STORE_H
