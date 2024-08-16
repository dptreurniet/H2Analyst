
#ifndef POST_PRO_DIALOG_H
#define POST_PRO_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSyntaxHighlighter>
#include <QListView>
#include <QLabel>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QSettings>
#include <QFileDialog>
#include <QMetaType>

#include "dataset.h"
#include "namespace.h"
#include "data_explorer.h"
#include "post_pro_expression_edit.h"
#include "data_parser.h"

class DataStore;
struct PostProcessor;

using postpro_ptr = QSharedPointer<PostProcessor>;

class ExpressionHighlighter : public QSyntaxHighlighter
{
    QRegularExpression datasetExpression;

public:
    ExpressionHighlighter(QObject *parent = nullptr);
    virtual void highlightBlock(const QString& text) override;
};

class PostProDialog : public QDialog
{
    Q_OBJECT

    DataStore* mDataStore;
    QTextCharFormat createDatasetTextObject(H2A::DatasetProperties& properties);
    QSettings mSettings;
    QList<H2A::DatasetProperties> mDatasetList;

    QVBoxLayout* mVLayout;
    QHBoxLayout* mHLayout;
    QDialogButtonBox* mButtonBox;

    // ----- Left column
    QVBoxLayout* mLeftLayout;
    QListView* postProcessorListView;
    QStandardItemModel* postProcessorListModel;

    QFormLayout* formLayout;
    QLineEdit* nameEdit;
    QComboBox* resampleEdit;
    PostProExpressionEdit* expressionEdit;
    ExpressionHighlighter* highlighter;

    QPushButton* mSavePostProcessorButton;
    QPushButton* mDeletePostProcessorButton;
    QDialogButtonBox* centerButtonBox;

    // ----- Right column -----
    QVBoxLayout* mRightLayout;
    DataExplorer* mDataExplorer;


    bool datasetListInSettings() { return mSettings.contains(H2A::SettingsKeys::PostProDatasetList + "/size"); }
    postpro_ptr selectedPostProcessor();
    const postpro_ptr postProcessorFromIndex(const QModelIndex& index);

public:
    PostProDialog(DataStore* parent = nullptr);

private slots:
    void loadDatasetsFromFile();
    void createPostProcessor();
    void deletePostProcessor();
    void clickedPostProcessor(const QModelIndex& index);
    void openPostProcessor(const postpro_ptr& pp);
    void loadDatasetListFromSettings();
    void closeWithSave();
    void closeWithoutSave();

public slots:
    virtual int exec() override;
    void updatePostProcessorList();
    void updateDatasetList();

};

#endif // POST_PRO_DIALOG_H
