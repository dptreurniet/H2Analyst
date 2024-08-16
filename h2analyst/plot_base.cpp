#include "plot_base.h"

H2A::PlotBase::PlotBase(QWidget *parent) : QCustomPlot(parent),
    plot_selected_time_act_(new QAction),
    type_(H2A::PlotType::Base),
    enforce_own_axes_limits_(true),
    tracing_enabled_(false)
{
    createActions();
    createMenus();

    setAcceptDrops(true);

    addLayer("labels", layer("overlay"), QCustomPlot::limBelow);

    connect(xAxis, qOverload<const QCPRange&>(&QCPAxis::rangeChanged), this, &PlotBase::enforceAxesLimits);
    connect(yAxis, qOverload<const QCPRange&>(&QCPAxis::rangeChanged), this, &PlotBase::enforceAxesLimits);
}

void H2A::PlotBase::createActions() {
    plot_selected_time_act_ = new QAction("Plot", this);
    connect(plot_selected_time_act_, &QAction::triggered, this, &PlotBase::plotSelectedTime);

    reset_zoom_act_ = new QAction("Reset zoom", this);
    connect(reset_zoom_act_, &QAction::triggered, this, &PlotBase::resetZoom);

    enable_tracing_act_ = new QAction("Enable tracing", this);
    connect(enable_tracing_act_, &QAction::triggered, this, &PlotBase::enableTracing);
}

void H2A::PlotBase::createMenus() {
    setContextMenuPolicy(Qt::CustomContextMenu);
    context_menu_ = new QMenu(this);

    context_menu_->addAction(plot_selected_time_act_);
    context_menu_->addSeparator();

    context_menu_->addAction(reset_zoom_act_);
    context_menu_->addSeparator();

    if (tracing_enabled_)
        context_menu_->addAction(disable_tracing_act_);
    else
        context_menu_->addAction(enable_tracing_act_);

    connect(this, &QCustomPlot::customContextMenuRequested, this, &PlotBase::contextMenuEvent);
}

void H2A::PlotBase::contextMenuEvent(const QPoint& pos) {
    context_menu_->exec(mapToGlobal(pos));
}

void H2A::PlotBase::scaleRange(QCPRange& range, float scale) {
    auto span = range.upper - range.lower;
    range.lower -= (span * (scale - 1) / 2);
    range.upper += (span * (scale - 1) / 2);

}

void H2A::PlotBase::resetZoom() {
    xAxis->setRange(bounds_.x);
    yAxis->setRange(bounds_.y);
}

void H2A::PlotBase::enforceAxesLimits(const QCPRange& current_range) {
    if (!enforce_own_axes_limits_) return;
    xAxis->setRange(xAxis->range().bounded(bounds_.x.lower, bounds_.x.upper));
    yAxis->setRange(yAxis->range().bounded(bounds_.y.lower, bounds_.y.upper));
}

void H2A::PlotBase::dragEnterEvent(QDragEnterEvent *event) {
    event->acceptProposedAction();  // Temporarily assume any drags are coming from datastore
}

void H2A::PlotBase::dropEvent(QDropEvent *event) {
    emit plotSelectedTime();
}

void H2A::PlotBase::enableTracing() {
    for (const auto& graph : mGraphs) {
        auto tracer = new PlotTracer(graph, this);
        tracer->setStyle(QCPItemTracer::TracerStyle::tsCrosshair);
        tracer->setInterpolating(false);
        tracers_.push_back(tracer);
    }
    tracing_enabled_ = true;
    replot();
}

void H2A::PlotBase::plot(const H2A::dataset_vec& datasets) {}
void H2A::PlotBase::setLabelY() {}
void H2A::PlotBase::calculateBounds() {};
void H2A::PlotBase::keyPressEvent(QKeyEvent* event) {};
