#include "data_explorer.h"

DatasetTreeView::DatasetTreeView(QWidget *parent) : QTreeView(parent),
    mItemModel(new QStandardItemModel),
    mFilterModel(new QSortFilterProxyModel),
    mTreeSelectionModel(new QItemSelectionModel),
    mOptions()
{
    mFilterModel->setSourceModel(mItemModel);
    mFilterModel->setFilterKeyColumn(-1);
    mFilterModel->setFilterRole(H2A::FilterRole);
    mFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mFilterModel->setRecursiveFilteringEnabled(true);

    mTreeSelectionModel->setModel(mFilterModel);

    setModel(mFilterModel);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionModel(mTreeSelectionModel);
    setHeaderHidden(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setDragEnabled(true);
}

void DatasetTreeView::startDrag(Qt::DropActions supportedActions) {
    auto mimeData = new QMimeData ();

    QString dataset_names;
    auto selectedDatasets = getSelectedDatasets();
    for (const auto& dataset : selectedDatasets) {
        dataset_names = dataset_names + dataset->name();
        if (dataset != selectedDatasets.back()) {
            dataset_names = dataset_names + H2A::listDelimiter;
        }
    }
    mimeData->setText(dataset_names);

    auto drag = new QDrag(this);
    drag->setMimeData (mimeData);
    drag->exec(Qt::CopyAction);
}

const H2A::dataset_vec DatasetTreeView::getSelectedDatasets() {
    H2A::dataset_vec datasets;
    auto rows = mTreeSelectionModel->selectedRows();
    for (const auto& index : rows) {
        auto item = mItemModel->itemFromIndex(mFilterModel->mapToSource(index));
        datasets.push_back(item->data(H2A::DatasetRole).value<QSharedPointer<H2A::Dataset>>());
    }
    return datasets;
}

void DatasetTreeView::update(const H2A::datafile_vec& datafiles) {
    mItemModel->clear();
    auto root = mItemModel->invisibleRootItem();

    for (const auto& datafile : datafiles) {

        auto datafile_item = new QStandardItem;
        if (mOptions.hideSingleFileRow && datafiles.size() == 1) {
            datafile_item = root;
        } else {
            datafile_item = createTreeItem(H2A::TreeItemType::File, datafile->name());
            root->appendRow(datafile_item);
        }

        auto datasets = datafile->datasets();

        // Create set of systems
        std::unordered_set<std::string> systems;
        std::string system_delimiter = " - ";
        for (const auto& dataset : datasets) {
            auto delimiter_position = dataset->name().toStdString().find(system_delimiter);
            if (delimiter_position == std::string::npos) continue;
            systems.insert(dataset->name().toStdString().substr(0, delimiter_position));
        }

        H2A::dataset_vec datasets_in_system;
        std::unordered_map<std::string, H2A::dataset_vec> subsystem_datasets;
        std::vector<std::string> split;
        std::string subsystem;
        QString label;

        // Iterate over systems and fill them with datasets
        for (const auto& system : systems) {
            auto system_item = createTreeItem(H2A::TreeItemType::System, QString(system.c_str()));
            datafile_item->appendRow(system_item);

            // Collect datasets that are part of this system
            datasets_in_system.clear();
            for (const auto& dataset : datasets) {
                if (dataset->name().toStdString().rfind(system, 0) == 0)
                    datasets_in_system.push_back(dataset);
            }

            // Remove datasets from to_be_placed datasets
            for (const auto& dataset : datasets_in_system)
                datasets.erase(std::remove(datasets.begin(), datasets.end(), dataset), datasets.end());

            // Count all n-word subsystems
            for (size_t n_words = 2; n_words > 0; n_words --) {
                subsystem_datasets.clear();
                for (const auto& dataset : datasets_in_system) {
                    boost::split(split, dataset->name().toStdString(), boost::is_any_of(" "));
                    if (split.size() >= 3 + n_words) {
                        subsystem = boost::join(std::vector<std::string>(split.begin() + 2, split.begin() + 2 + n_words), " ");
                        subsystem_datasets.insert({subsystem, H2A::dataset_vec()});
                        subsystem_datasets[subsystem].push_back(dataset);
                    }
                }

                // If a subsystem has 2 or more datasets, add it to the tree
                for (auto const& subsystem_it : subsystem_datasets) {
                    if (subsystem_it.second.size() < 2) continue;

                    auto subsystem_item = createTreeItem(H2A::TreeItemType::Subsystem, QString(subsystem_it.first.c_str()));
                    system_item->appendRow(subsystem_item);

                    for (auto const& dataset : subsystem_it.second) {
                        boost::split(split, dataset->name().toStdString(), boost::is_any_of(" "));
                        label = QString(boost::join(std::vector<std::string>(split.begin() + 2 + n_words, split.end()), " ").c_str());
                        auto dataset_item = createTreeItem(H2A::TreeItemType::Dataset, label, dataset);
                        subsystem_item->appendRow(dataset_item);

                        datasets_in_system.erase(std::remove(datasets_in_system.begin(), datasets_in_system.end(), dataset), datasets_in_system.end());
                    }
                }
            }

            // Add the datasets not part of a subsystem to the system directly
            for (auto const& dataset : datasets_in_system) {
                boost::split(split, dataset->name().toStdString(), boost::is_any_of(" "));
                label = QString(boost::join(std::vector<std::string>(split.begin() + 2, split.end()), " ").c_str());
                auto dataset_item = createTreeItem(H2A::TreeItemType::Dataset, label, dataset);
                system_item->appendRow(dataset_item);
            }
        }

        // If there are any remaining datasets not part of any system, add them to the datafile directly
        for (auto const& dataset : datasets) {
            auto dataset_item = createTreeItem(H2A::TreeItemType::Dataset, dataset->name(), dataset);
            datafile_item->appendRow(dataset_item);
        }
    }

    mItemModel->setSortRole(H2A::SortRole);
    mItemModel->sort(0);

    emit updated();
}

QStandardItem* DatasetTreeView::createTreeItem(H2A::TreeItemType type, QString label, H2A::dataset_ptr dataset) {
    auto item = new QStandardItem(label);
    item->setEditable(false);
    item->setData("", H2A::FilterRole);

    switch(type) {
    case H2A::TreeItemType::File:
        item->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::FolderOpen));
        item->setData("1_" + label, H2A::SortRole);
        break;
    case H2A::TreeItemType::System:
        item->setData("2_" + label, H2A::SortRole);
        break;
    case H2A::TreeItemType::Subsystem:
        item->setData("3_" + label, H2A::SortRole);
        break;
    case H2A::TreeItemType::Dataset:
        item->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditPaste));
        item->setData("4_" + label, H2A::SortRole);
        item->setData(QVariant::fromValue(dataset), H2A::DatasetRole);
        item->setData(QString(dataset->name().toStdString().c_str()), H2A::FilterRole);
        break;
    }

    return item;
}

void DatasetTreeView::update(const QList<H2A::DatasetProperties>& properties) {
    auto datafile = QSharedPointer<H2A::Datafile>::create();
    for (const auto& property : properties) {
        auto dataset = QSharedPointer<H2A::Dataset>::create(datafile, property);
        datafile->addDataset(dataset);
    }
    H2A::datafile_vec vec;
    vec.push_back(datafile);
    update(vec);
}

DataExplorer::DataExplorer(QWidget *parent)
    : QWidget{parent},
    mLayout(new QVBoxLayout),
    mFilterInput(new QLineEdit),
    mTreeView(new DatasetTreeView),
    mPlaceholderButton(new QPushButton(mTreeView))
{
    // ----- Filter input -----
    mFilterInput->setPlaceholderText("Search...");
    mFilterInput->setClearButtonEnabled(true);
    connect(mFilterInput, &QLineEdit::textChanged, this, &DataExplorer::filterChanged);
    connect(mFilterInput, &QLineEdit::returnPressed, this, &DataExplorer::expandAll);
    connect(mTreeView, &DatasetTreeView::updated, this, &DataExplorer::updatePlaceholder);

    // ----- Tree view -----
    mTreeView->options().hideSingleFileRow = false;
    mPlaceholderButton->setText("Click here");
    connect(mPlaceholderButton, &QPushButton::clicked, this, &DataExplorer::placeholderClicked);

    // ----- Layout -----
    mLayout->addWidget(mFilterInput);
    mLayout->addWidget(mTreeView);
    setLayout(mLayout);
}

void DataExplorer::setPlaceholderText(const QString& text) {
    mPlaceholderButton->setText(text);
}

void DataExplorer::filterChanged() {
    auto filter_str = mFilterInput->text();
    if (filter_str.size() == 0) mTreeView->filterModel()->setFilterFixedString("");
    if (filter_str.size() < 3) return;
    mTreeView->filterModel()->setFilterRegularExpression(QRegularExpression(filter_str, QRegularExpression::CaseInsensitiveOption));
}

void DataExplorer::expandAll() {
    if (mFilterInput->text().size() < 3) return;
    mTreeView->expandAll();
}

void DataExplorer::updateTreeView(const H2A::datafile_vec& datafile) {
    mTreeView->update(datafile);
}

void DataExplorer::updateTreeView(const QList<H2A::DatasetProperties>& properties) {
    mTreeView->update(properties);
}

void DataExplorer::updatePlaceholder() {
    mPlaceholderButton->setHidden(mTreeView->model()->rowCount() > 0);
}
