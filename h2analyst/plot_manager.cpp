#include "plot_manager.h"

PlotManager::PlotManager(QWidget* parent)
    : QWidget{parent},
    data_store_(nullptr),  // Set from H2Analyst
    layout_(new QStackedLayout),
    v_splitter_(new QSplitter),
    h_splitters_()
{
    v_splitter_->setOrientation(Qt::Vertical);
    v_splitter_->setChildrenCollapsible(false);

    layout_->addWidget(v_splitter_);
    setLayout(layout_);

    setPlotLayout(2, 1);
}

std::vector<H2A::PlotBase*> PlotManager::getPlots() {
    std::vector<H2A::PlotBase*> plots;
    for (auto i = 0; i < v_splitter_->count(); i ++) {
        auto h_splitter = dynamic_cast<QSplitter*>(v_splitter_->widget(i));
        for (auto j = 0; j < h_splitter->count(); j ++) {
            plots.push_back(dynamic_cast<H2A::PlotBase*>(h_splitter->widget(j)));
        }
    }
    return plots;
}

template <typename T>
T* PlotManager::createPlot() {
    auto plot = new T(this);
    connect(plot, &H2A::PlotBase::plotSelectedTime, this, &PlotManager::plotTime);
    return plot;
}

void PlotManager::plotTime() {
    // Replace plot that triggered this slot by a new time-based plot
    auto plot = static_cast<H2A::PlotBase*>(sender());
    auto time_plot = new H2A::PlotTime(this);
    for (auto splitter_i = 0; splitter_i < v_splitter_->count(); splitter_i ++) {
        auto splitter = static_cast<QSplitter*>(v_splitter_->widget(splitter_i));
        for (auto plot_i = 0; plot_i < splitter->count(); plot_i ++) {
            if (splitter->widget(plot_i) == plot) {
                splitter->replaceWidget(plot_i, time_plot);
            }
        }
    }
    plot->deleteLater();

    auto datasets = data_store_->getSelectedDatasets();
    time_plot->plot(datasets);
}

void PlotManager::replacePlot(H2A::PlotBase* old_plot, H2A::PlotBase* new_plot) {

}

void PlotManager::setPlotLayout(uint8_t n_rows, uint8_t n_cols) {
    // Collect all plots for potential re-use and delete empty ones
    auto plots = getPlots();
    plots.erase(std::remove_if(plots.begin(), plots.end(),
                               [](const auto& plot) { return plot->isEmpty(); }), plots.end());
    std::reverse(plots.begin(), plots.end());

    // Remove plots from old splitters so they can be removed
    for (auto plot : plots)
        plot->setParent(this);

    // Remove all horizontal splitters
    for (auto i = 0; i < v_splitter_->count(); i ++) {
        v_splitter_->widget(i)->deleteLater();
    }

    // Make new splitters and insert the plots. Re-use if available, otherwise make new plots.
    for (auto row_i = 0; row_i < n_rows; row_i ++) {
        auto h_splitter = new QSplitter;
        h_splitter->setChildrenCollapsible(false);
        v_splitter_->addWidget(h_splitter);
        for (auto col_i = 0; col_i < n_cols; col_i ++) {
            H2A::PlotBase* plot_;
            if (!plots.empty()) {
                plot_ = plots.back();
                plots.pop_back();
            } else plot_ = createPlot<H2A::PlotBase>();
            h_splitter->addWidget(plot_);
        }
    }

    // Delete remaining plots
    for (auto plot : plots)
        plot->deleteLater();
}

