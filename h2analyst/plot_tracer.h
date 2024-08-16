#ifndef PLOT_TRACER_H
#define PLOT_TRACER_H

#include "qcustomplot.h"

class PlotTracer : public QCPItemTracer
{

    QCPItemText* x_label_;
    QCPItemText* y_label_;

    void setLabelProperties(QCPItemText* label);
    void update();

public:
    PlotTracer(QCPGraph* graph, QCustomPlot* parent = nullptr);

    void moveRight();
    void moveLeft();

};

#endif // PLOT_TRACER_H
