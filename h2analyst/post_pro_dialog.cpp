#include "post_pro_dialog.h"
#include "data_store.h"

ExpressionHighlighter::ExpressionHighlighter(QObject *parent) : QSyntaxHighlighter(parent),
    datasetExpression("<\\d{1,}>")
{
}

void ExpressionHighlighter::highlightBlock(const QString& text) {
    QTextCharFormat myClassFormat;
    myClassFormat.setFontWeight(QFont::Bold);
    myClassFormat.setForeground(Qt::darkMagenta);

    QRegularExpressionMatchIterator i = datasetExpression.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        setFormat(match.capturedStart(), match.capturedLength(), myClassFormat);
    }
}

PostProDialog::PostProDialog(DataStore* parent) : QDialog(parent),
    mDataStore(parent),
    mSettings(QSettings()),
    mLeftLayout(new QVBoxLayout),
    postProcessorListView(new QListView),
    postProcessorListModel(new QStandardItemModel),
    mHLayout(new QHBoxLayout),
    mVLayout(new QVBoxLayout),
    mButtonBox(new QDialogButtonBox),
    formLayout(new QFormLayout),
    nameEdit(new QLineEdit),
    resampleEdit(new QComboBox(this)),
    expressionEdit(new PostProExpressionEdit(this)),
    mSavePostProcessorButton(new QPushButton),
    mDeletePostProcessorButton(new QPushButton),
    centerButtonBox(new QDialogButtonBox(Qt::Horizontal)),
    mRightLayout(new QVBoxLayout),
    mDatasetList(),
    mDataExplorer(new DataExplorer),
    highlighter(new ExpressionHighlighter)
{    
    // ----- Left column -----
    postProcessorListView->setModel(postProcessorListModel);
    postProcessorListView->setSelectionMode(QAbstractItemView::SingleSelection);
    mLeftLayout->addWidget(postProcessorListView);
    connect(postProcessorListView, &QListView::clicked, this, &PostProDialog::clickedPostProcessor);

    // ----- Edit form -----
    formLayout->addRow("Name", nameEdit);

    resampleEdit->addItem("Oversample");
    resampleEdit->addItem("Fixed");
    formLayout->addRow("Resample mode", resampleEdit);

    formLayout->addRow("Expression", expressionEdit);
    highlighter->setDocument(expressionEdit->document());

    mSavePostProcessorButton->setText("Save");
    centerButtonBox->addButton(mSavePostProcessorButton, QDialogButtonBox::AcceptRole);
    mDeletePostProcessorButton->setText("Delete");
    centerButtonBox->addButton(mDeletePostProcessorButton, QDialogButtonBox::ActionRole);
    formLayout->addRow(centerButtonBox);

    mLeftLayout->addLayout(formLayout);

    // ----- Right column -----
    mDataExplorer->setHideSingleFileRow(true);
    mRightLayout->addWidget(mDataExplorer);

    // ----- Bottom buttons -----
    mButtonBox->setOrientation(Qt::Horizontal);
    mButtonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    connect(mButtonBox, &QDialogButtonBox::accepted, this, &PostProDialog::closeWithSave);
    connect(mButtonBox, &QDialogButtonBox::rejected, this, &PostProDialog::closeWithoutSave);

    // ----- Layouts -----
    mHLayout->addLayout(mLeftLayout);
    mHLayout->addLayout(mRightLayout);

    mVLayout->addLayout(mHLayout);
    mVLayout->addWidget(mButtonBox);

    setLayout(mVLayout);

    // ----- Actions -----
    connect(mDataStore, &DataStore::postProcessorsChanged, this, &PostProDialog::updatePostProcessorList);
    connect(mSavePostProcessorButton, &QPushButton::clicked, this, &PostProDialog::createPostProcessor);
    connect(mDeletePostProcessorButton, &QPushButton::clicked, this, &PostProDialog::deletePostProcessor);

    setWindowTitle("Post Processors");
    resize(1024, 768);

    if (datasetListInSettings())
        loadDatasetListFromSettings();
}

int PostProDialog::exec() {
    nameEdit->setFocus(Qt::PopupFocusReason);
    updatePostProcessorList();
    return QDialog::exec();
}

void PostProDialog::createPostProcessor() {
    auto pp = postpro_ptr(new PostProcessor);
    pp->properties.name = nameEdit->text();
    switch (resampleEdit->currentIndex()) {
    case 0:
        pp->resampling_mode = H2A::ResamplingMode::allDatapoints;
        break;
    case 1:
        pp->resampling_mode = H2A::ResamplingMode::fixed;
        break;
    }
    pp->expression = expressionEdit->toPlainText();

    mDataStore->addPostProcessor(pp);
}

void PostProDialog::deletePostProcessor() {
    auto postProcessor = selectedPostProcessor();
    mDataStore->removePostProcessor(postProcessor);
}

void PostProDialog::updatePostProcessorList() {
    auto postProcessors = mDataStore->postProcessors();
    postProcessorListModel->clear();

    if (postProcessors.size() == 0) {
        auto item = new QStandardItem("No post processors defined yet");
        auto itemFont = item->font();
        itemFont.setItalic(true);
        item->setFont(itemFont);
        item->setEditable(false);
        item->setSelectable(false);
        postProcessorListModel->appendRow(item);
        postProcessorListView->update();
        return;
    }

    for (const auto& postProcessor : postProcessors) {
        auto item = new QStandardItem(postProcessor->properties.name);
        QVariant ppVariant;
        ppVariant.setValue(postProcessor);
        item->setData(ppVariant, H2A::ItemRoles::DatasetPropertyRole);
        item->setEditable(false);
        postProcessorListModel->appendRow(item);
    }
    postProcessorListView->update();
}

postpro_ptr PostProDialog::selectedPostProcessor() {
    auto index = postProcessorListView->selectionModel()->selectedIndexes().front();
    auto item = postProcessorListModel->itemFromIndex(index);
    auto postProcessor = item->data(H2A::ItemRoles::DatasetPropertyRole).value<postpro_ptr>();
    return postProcessor;
}

void PostProDialog::loadDatasetsFromFile() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setDirectory(QDir::home());
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setNameFilter(QString("F8 INTCANLOG (*.mat)"));

    QString filename;
    if (dialog.exec()) {
        filename = dialog.selectedFiles().constFirst();
    } else {
        return;
    }

    mDatasetList.clear();

    auto datafile = H2A::datafile_ptr(new H2A::Datafile);
    DataParser::parseFile(std::filesystem::path(filename.toStdString()), datafile, true);
    for (const auto& dataset : datafile->datasets()) {
        mDatasetList.push_back(dataset->properties());
    }

    qInfo() << mSettings.fileName();
    if (datasetListInSettings()) {
        qInfo() << "Dataset list already in settings file" << mSettings.fileName();
    } else {
        mSettings.beginWriteArray(H2A::SettingsKeys::PostProDatasetList, mDatasetList.size());
        for (auto i = 0; i < mDatasetList.size(); ++i) {
            mSettings.setArrayIndex(i);
            auto propertyMap = mDatasetList.at(i).toMap();
            for (const auto& [key, value] : propertyMap)
                mSettings.setValue(key, value);
        }
        mSettings.endArray();
    }
    updateDatasetList();
}

void PostProDialog::openPostProcessor(const postpro_ptr& pp) {
    nameEdit->setText(pp->properties.name);
    switch (pp->resampling_mode) {
    case H2A::ResamplingMode::allDatapoints:
        resampleEdit->setCurrentIndex(0);
        break;
    case H2A::ResamplingMode::fixed:
        resampleEdit->setCurrentIndex(1);
        break;
    }
    expressionEdit->setText(pp->expression);
}

const postpro_ptr PostProDialog::postProcessorFromIndex(const QModelIndex& index) {
    auto ppItem = postProcessorListModel->itemFromIndex(index);
    return ppItem->data(H2A::DatasetPropertyRole).value<postpro_ptr>();
}

void PostProDialog::clickedPostProcessor(const QModelIndex& index) {
    openPostProcessor(postProcessorFromIndex(index));
}

void PostProDialog::loadDatasetListFromSettings() {
    mDatasetList.clear();
    auto size = mSettings.beginReadArray(H2A::SettingsKeys::PostProDatasetList);
    H2A::DatasetProperties template_properties;
    auto template_map = template_properties.toMap();
    std::map<QString, QString> readMap;
    for (auto i = 0; i < size; ++i) {
        mSettings.setArrayIndex(i);
        for (auto const& [key, _] : template_map) {
            readMap[key] = mSettings.value(key).toString();
        }
        mDatasetList.push_back(H2A::DatasetProperties::fromMap(readMap));
    }
    updateDatasetList();
}

void PostProDialog::updateDatasetList() {
    mDataExplorer->updateTreeView(mDatasetList);
}

void PostProDialog::closeWithSave() {
    mDataStore->savePostProToSettings();
    mDataStore->readPostProFromSettings();
    close();
}

void PostProDialog::closeWithoutSave() {
    mDataStore->readPostProFromSettings();
    close();
}
