#include "h2analyst.h"

H2Analyst::H2Analyst(QWidget *parent)
    : QMainWindow(parent),
    central_widget_(new QWidget(this)),
    left_column_(new QWidget(this)),
    logo_(QPixmap(":/img/logo-blue-rgb")),
    logo_label_(new QLabel(this)),
    data_store_(new DataStore(this)),
    plot_manager_(new PlotManager(this)),
    layout_(new QVBoxLayout),
    main_splitter_(new QSplitter),
    left_layout_(new QVBoxLayout)
{
    QCoreApplication::setOrganizationName("Forze Hydrogen Racing");
    QCoreApplication::setOrganizationDomain("forzehydrogenracing.com");
    QCoreApplication::setApplicationName("H2Analyst");

    setCentralWidget(central_widget_);
    setWindowTitle(tr("H2Analyst"));
    central_widget_->setLayout(layout_);

    plot_manager_->setDataStore(data_store_);

    logo_ = logo_.scaledToWidth(350, Qt::SmoothTransformation);
    logo_label_->setPixmap(logo_);
    logo_label_->setFixedSize(logo_.size());

    data_store_->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding));
    plot_manager_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    left_layout_->addWidget(logo_label_);
    left_layout_->addWidget(data_store_);
    left_column_->setLayout(left_layout_);

    main_splitter_->addWidget(left_column_);
    main_splitter_->addWidget(plot_manager_);
    main_splitter_->setCollapsible(0, true);
    main_splitter_->setCollapsible(1, false);
    main_splitter_->setHandleWidth(0);

    layout_->addWidget(main_splitter_);

    createActions();
    createMenus();

    resize(1440, 900);
    main_splitter_->setSizes({logo_.width(), width() - logo_.width()});
}

H2Analyst::~H2Analyst() {}

void H2Analyst::createActions() {

    // ----- File menu actions -----
    open_act_ = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen), tr("&Open"), this);
    open_act_->setShortcuts(QKeySequence::Open);
    open_act_->setToolTip(tr("Open a file with data to analyse"));
    connect(open_act_, &QAction::triggered, this, &H2Analyst::open);

    quit_act_ = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ApplicationExit), tr("&Quit"), this);
    quit_act_->setShortcuts(QKeySequence::Quit);
    quit_act_->setToolTip(tr("Quit 2Analyst"));
    connect(quit_act_, &QAction::triggered, this, &H2Analyst::quit);

    // ----- Edit menu actions -----
    post_pro_dialog_act_ = new QAction("Post processing", this);
    connect(post_pro_dialog_act_, &QAction::triggered, data_store_, &DataStore::dialog);

    // ----- Window menu actions -----
    plot_layout_act_ = new LayoutSelector(this);
    //plot_layout_act_->setShortcuts(QKeySequence(tr("Ctrl+L")));
    connect(plot_layout_act_, &LayoutSelector::layoutSelected, plot_manager_, &PlotManager::setPlotLayout);
}

void H2Analyst::createMenus() {
    file_menu_ = menuBar()->addMenu(tr("&File"));
    file_menu_->addAction(open_act_);
    file_menu_->addSeparator();
    file_menu_->addAction(quit_act_);

    edit_menu_ = menuBar()->addMenu(tr("&Edit"));
    edit_menu_->addAction(post_pro_dialog_act_);

    window_menu_ = menuBar()->addMenu(tr("&Window"));
    plot_layout_menu_ = window_menu_->addMenu(tr("Plot &layout"));
    plot_layout_menu_->addAction(plot_layout_act_);
}

void H2Analyst::open() {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setDirectory(QDir::home());
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setNameFilter(QString("F8 INTCANLOG (*.mat)"));

    QStringList files;
    if (dialog.exec())
        files = dialog.selectedFiles();

    for (const auto &s : files)
        data_store_->openFile(s.toStdString());
}

void H2Analyst::quit() {
    QMessageBox dialog;
    dialog.setModal(true);
    dialog.setWindowFlags(Qt::FramelessWindowHint);
    dialog.setText("Are you sure?");
    dialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    dialog.setDefaultButton(QMessageBox::Yes);

    int answer = dialog.exec();

    switch(answer) {
    case QMessageBox::Yes:
        this->close();
        break;
    case QMessageBox::No:
        break;
    }
}
