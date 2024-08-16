#ifndef PLOT_MANAGER_H
#define PLOT_MANAGER_H

#include <QWidget>
#include <QSplitter>
#include <QStackedLayout>

#include "plot_base.h"
#include "plot_time.h"
#include "data_store.h"

class H2Analyst;

class PlotManager : public QWidget
{
    Q_OBJECT

    DataStore* data_store_;

    std::vector<H2A::PlotBase*> getPlots();

    template <typename T>
    T* createPlot();
    void replacePlot(H2A::PlotBase* old_plot, H2A::PlotBase* new_plot);

public:
    QStackedLayout* layout_;

    QSplitter* v_splitter_;
    std::vector<QSplitter*> h_splitters_;
    std::vector<H2A::PlotBase*> plots_;

    explicit PlotManager(QWidget *parent = nullptr);

    void setDataStore(DataStore* data_store) { data_store_ = data_store; }

public slots:
    void setPlotLayout(uint8_t n_rows, uint8_t n_cols);

private slots:
    void plotTime();

signals:
};

#endif // PLOT_MANAGER_H
