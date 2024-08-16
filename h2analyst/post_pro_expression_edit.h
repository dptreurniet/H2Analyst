#ifndef POST_PRO_EXPRESSION_EDIT_H
#define POST_PRO_EXPRESSION_EDIT_H

#include <QTextEdit>
#include <QDragEnterEvent>
#include <QMimeData>

class PostProExpressionEdit : public QTextEdit
{
public:
    PostProExpressionEdit(QWidget* parent = nullptr);

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

#endif // POST_PRO_EXPRESSION_EDIT_H
