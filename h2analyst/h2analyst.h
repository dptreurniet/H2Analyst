#ifndef H2ANALYST_H
#define H2ANALYST_H

#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QSplitter>

#include "data_store.h"
#include "plot_manager.h"
#include "layout_selector.h"

class H2Analyst : public QMainWindow
{
    Q_OBJECT

    // ----- Widgets -----
    QWidget *central_widget_;
    QWidget *left_column_;
    QPixmap logo_;
    QLabel *logo_label_;
    DataStore *data_store_;
    PlotManager *plot_manager_;

    // ----- Layouts -----
    QVBoxLayout *left_layout_;
    QSplitter *main_splitter_;

    QVBoxLayout *layout_;

    // ----- Menus and actions -----
    QMenu *file_menu_;
    QAction *open_act_;
    QAction *quit_act_;

    QMenu *edit_menu_;
    QAction *post_pro_dialog_act_;

    QMenu *window_menu_;
    QMenu *plot_layout_menu_;
    LayoutSelector *plot_layout_act_;

    void createActions();
    void createMenus();

public:
    H2Analyst(QWidget *parent = nullptr);
    ~H2Analyst();

private slots:
    void open();
    void quit();
};

#endif // H2ANALYST_H
