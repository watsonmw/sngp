#include "navlistview.h"

NavListView::NavListView(QWidget *parent)
  : QListView(parent)
{
}

void NavListView::keyPressEvent(QKeyEvent *event)
{
    QModelIndex oldIdx = currentIndex();
    QListView::keyPressEvent(event);
    QModelIndex newIdx = currentIndex();
    if (oldIdx.row() != newIdx.row()) {
        emit clicked(newIdx);
    }
}
