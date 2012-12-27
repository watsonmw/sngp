#ifndef NAVLISTVIEW_H
#define NAVLISTVIEW_H

#include <QListView>

/*
 * Augment QListView so up/down arrowa selects an item
 * and issue a 'clicked' signal.
 */
class NavListView : public QListView
{
    Q_OBJECT
public:
    explicit NavListView(QWidget *parent = 0);

protected:
    virtual void keyPressEvent(QKeyEvent *event);
};

#endif // NAVLISTVIEW_H
