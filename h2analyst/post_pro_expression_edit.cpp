#include "post_pro_expression_edit.h"

PostProExpressionEdit::PostProExpressionEdit(QWidget* parent) : QTextEdit(parent)
{
    setAcceptDrops(true);
}

void PostProExpressionEdit::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasText()) {
        qInfo() << "Accepting drop";
        event->acceptProposedAction();
    }
}

void PostProExpressionEdit::dropEvent(QDropEvent *event) {
    event->acceptProposedAction();
}
