#include "data_store.h"

DataStore::DataStore(QWidget *parent)
    : QWidget{parent},
    mSettings(),
    layout_(new QVBoxLayout),
    item_context_menu_(new QMenu),
    data_explorer_(new DataExplorer),
    mDialog(new PostProDialog(this))
{

    // ----- Context menu -----
    item_properties_act_ = new QAction(tr("&Properties"), this);
    item_properties_act_->setToolTip(tr("See dataset properties"));
    item_context_menu_->addAction(item_properties_act_);

    // ----- Layout -----
    setLayout(layout_);
    layout_->addWidget(data_explorer_);

    readPostProFromSettings();
}

void DataStore::resampleSelectedDataset() {
    auto dataset = getSelectedDatasets().front();
    populateDataset(dataset);
    QVector<double> time(10);
    for (auto i = 0; i < time.size(); i ++) {
        time[i] = dataset->getTimeVec().back() / time.size() * i;
    }
    auto resampled_dataset = H2A::resampleDataset(dataset, time);
    dataset->datafile()->addDataset(resampled_dataset);
    data_explorer_->updateTreeView(datafiles_);
}

void DataStore::openFile(const std::string& file) {
    auto datafile = H2A::datafile_ptr(new H2A::Datafile);
    if (DataParser::parseFile(file, datafile)) {
        datafiles_.push_back(datafile);
        emit datafileOpened(datafiles_.back());
    }
    data_explorer_->updateTreeView(datafiles_);
}

void DataStore::populateDataset(H2A::dataset_ptr& dataset) {
    auto datafile = dataset->datafile();
    for (auto i = 0; i < datafile->data().ids.size(); i++) {
        if (datafile->data().ids[i] != dataset->id()) continue;
    }
}

void DataStore::addPostProcessor(const postpro_ptr& pp) {

    bool postProcessorAlreadyExists = false;
    for (const auto& postProcessor : mPostProcessors) {
        if (postProcessor->properties.name == pp->properties.name) {
            postProcessorAlreadyExists = true;
            break;
        }
    }

    if (postProcessorAlreadyExists) {
        QMessageBox dialog;
        dialog.setIcon(QMessageBox::Warning);
        dialog.setWindowFlags(dialog.windowFlags() | Qt::WindowStaysOnTopHint);
        dialog.setWindowTitle("Sure?");
        dialog.setModal(true);
        dialog.setText("Overwrite post processor?");
        dialog.setInformativeText("A post processor with the same name already exists.");
        dialog.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
        dialog.setDefaultButton(QMessageBox::Cancel);
        int ret = dialog.exec();

        if (ret == QMessageBox::Cancel) return;
        mPostProcessors.removeIf([pp](const postpro_ptr postProcessor){ return postProcessor->properties.name == pp->properties.name; });
    }

    mPostProcessors.push_back(pp);
    emit postProcessorsChanged();
}

void DataStore::removePostProcessor(const postpro_ptr& pp) {
    mPostProcessors.removeAll(pp);
    emit postProcessorsChanged();
}

const H2A::dataset_vec DataStore::getSelectedDatasets() {
    return data_explorer_->getSelectedDatasets();
}

void DataStore::dialog() {
    mDialog->exec();
}

void DataStore::savePostProToSettings() {
    mSettings.remove(H2A::SettingsKeys::PostProList);

    mSettings.beginWriteArray(H2A::SettingsKeys::PostProList, mPostProcessors.size());
    for (auto i = 0; i < mPostProcessors.size(); ++i) {
        mSettings.setArrayIndex(i);
        auto map = mPostProcessors[i]->toMap();
        for (const auto& [key, val] : map) {
            mSettings.setValue(key, val);
        }
    }
    mSettings.endArray();
}

void DataStore::readPostProFromSettings() {
    mPostProcessors.clear();

    auto size = mSettings.beginReadArray(H2A::SettingsKeys::PostProList);
    PostProcessor templatePostPro;
    auto templateMap = templatePostPro.toMap();
    std::map<QString, QString> readMap;
    for (auto i = 0; i < size; ++i) {
        mSettings.setArrayIndex(i);
        for (auto const& [key, _] : templateMap) {
            readMap[key] = mSettings.value(key).toString();
        }
        mPostProcessors.push_back(PostProcessor::fromMap(readMap));
        qInfo() << mPostProcessors.back()->properties.name;
    }
    mSettings.endArray();
}

std::map<QString, QVariant> PostProcessor::toMap() const {
    auto map = properties.toMap();

    QVariant resampleVar(resampling_mode);
    map["resampling_mode"] = resampleVar;

    QVariant expressionVar(expression);
    map["expression"] = expressionVar;

    return map;
}

postpro_ptr PostProcessor::fromMap(const std::map<QString, QString>& map) {
    postpro_ptr pp(new PostProcessor);

    pp->properties = H2A::DatasetProperties::fromMap(map);
    pp->resampling_mode = static_cast<H2A::ResamplingMode>(map.at("resampling_mode").toInt());
    pp->expression = map.at("expression");

    return pp;
}
