#ifndef PLOT_BASE_H
#define PLOT_BASE_H

#include <tuple>

#include <QMenu>
#include <QAction>

#include "qcustomplot.h"
#include "namespace.h"
#include "datafile.h"
#include "plot_tracer.h"

namespace H2A
{

struct PlotBounds {
    QCPRange x;
    QCPRange y;
};

class PlotBase : public QCustomPlot
{
    Q_OBJECT

protected:


    H2A::PlotType type_;
    dataset_vec datasets_;

    const float margin_ = 1.1;
    bool enforce_own_axes_limits_;
    bool tracing_enabled_;

    virtual void setLabelY();

    // Axis functions
    PlotBounds bounds_;
    virtual void calculateBounds();
    static void scaleRange(QCPRange& range, float scale);

    // Data tracers
    std::vector<PlotTracer*> tracers_;

    // Context menu    
    QMenu* context_menu_;
    QAction* plot_selected_time_act_;
    QAction* reset_zoom_act_;
    QAction* enable_tracing_act_;
    QAction* disable_tracing_act_;

    void createActions();
    void createMenus();

    // Drag and drop
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

    // Key event
    virtual void keyPressEvent(QKeyEvent* event) override;


public:
    PlotBase(QWidget *parent = nullptr);

    virtual void plot(const H2A::dataset_vec& datasets);

    const H2A::PlotType& type() { return type_; }
    bool isEmpty() const { return graphCount() == 0; }

signals:
    void plotSelectedTime();

private slots:
    void contextMenuEvent(const QPoint&);
    virtual void enforceAxesLimits(const QCPRange& current_range);
    void enableTracing();

public slots:
    void resetZoom();

};

}

#endif // PLOT_BASE_H
