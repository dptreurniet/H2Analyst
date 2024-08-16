#include "plot_tracer.h"

PlotTracer::PlotTracer(QCPGraph* graph, QCustomPlot* parent) : QCPItemTracer(parent),
    x_label_(new QCPItemText(parent)),
    y_label_(new QCPItemText(parent))
{
    setGraph(graph);

    setLabelProperties(x_label_);
    x_label_->setPositionAlignment(Qt::AlignCenter);
    x_label_->position->setCoords(1, 1);
    x_label_->setText("X-label");

    setLabelProperties(y_label_);
    y_label_->setPositionAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    y_label_->position->setCoords(0, 0);
    y_label_->setText("Y-label");

    update();
}

void PlotTracer::setLabelProperties(QCPItemText* label) {
    label->position->setType(QCPItemPosition::PositionType::ptAxisRectRatio);
    label->position->setParentAnchorY(position, false);
    label->setBrush(QBrush(QColor(Qt::white)));
    label->setPen(QPen(label->color()));
    label->setLayer("labels");
}

void PlotTracer::moveRight() {
    setGraphKey(graphKey() + 1);
    update();
}

void PlotTracer::moveLeft() {
    setGraphKey(graphKey() - 1);
    update();
}

void PlotTracer::update() {
    x_label_->setText(QString::number(graphKey()));
    auto value = graph()->data()->findBegin(graphKey(), true)->value;
    y_label_->setText(QString::number(value));
    updatePosition();
    graph()->parentPlot()->replot();
}
