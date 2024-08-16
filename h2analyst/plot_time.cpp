#include "plot_time.h"

H2A::PlotTime::PlotTime(QWidget *parent) : H2A::PlotBase(parent),
    ticker_x_(QSharedPointer<QCPAxisTickerTime>::create())
{
    this->type_ = H2A::PlotType::Time;

    ticker_x_->setTimeFormat("%s");

    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    this->xAxis->setLabel(QString("time [sec]"));
    this->xAxis->setTicker(ticker_x_);
}

void H2A::PlotTime::plot(const H2A::dataset_vec& datasets) {
    // Make sure all datasets to plot are populated with data
    for (const auto& dataset : datasets) {
        dataset->datafile()->populateDataset(dataset);
    }

    this->clearGraphs();
    datasets_.clear();
    for (const auto& dataset : datasets) {
        auto graph = addGraph();
        graph->setPen(QPen(H2A::getLineColor(graphCount() - 1)));
        graph->setData(dataset->getTimeVec(), dataset->getDataVec());
        graph->setAdaptiveSampling(true);
        graph->setLineStyle(QCPGraph::lsStepLeft);
        datasets_.push_back(dataset);
    }

    calculateBounds();
    resetZoom();
    setLabelY();
    replot();
}

void H2A::PlotTime::setLabelY() {
    std::unordered_set<QString> quantities, units;
    for (const auto& dataset : datasets_) {
        quantities.insert(dataset->quantity());
        units.insert(dataset->unit());
    }

    QString label;
    if (quantities.size() == 1) {
        label = datasets_.front()->quantity();
    } else {
        label = "Mixed signals";
    }

    if (units.size() == 1) {
        label += " [" + datasets_.front()->unit() +"]";
    } else {
        label += " [-]";
    }

    yAxis->setLabel(label);
}

void H2A::PlotTime::calculateBounds() {
    if (mGraphs.size() == 0) return;
    bool found_range;

    PlotBounds bounds;
    bounds.x = mGraphs.front()->getKeyRange(found_range);
    bounds.y = mGraphs.front()->getValueRange(found_range);

    for (const auto& graph : mGraphs) {
        bounds.x.expand(graph->getKeyRange(found_range));
        bounds.y.expand(graph->getValueRange(found_range));
    }
    scaleRange(bounds.x, margin_);
    scaleRange(bounds.y, margin_);
    bounds_ = bounds;
}

void H2A::PlotTime::keyPressEvent(QKeyEvent* event) {
    switch(event->key()) {
    case Qt::Key_Right:
        for (auto tracer : tracers_)
            tracer->moveRight();
        break;
    case Qt::Key_Left:
        for (auto tracer : tracers_)
            tracer->moveLeft();
        break;
}

}

