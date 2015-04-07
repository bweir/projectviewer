#include "projectview.h"

#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QStandardItem>
#include <QStandardItemModel>

#include <QRegExp>
#include <QKeyEvent>
#include <QMap>

#include <QTextDocument>

ProjectView::ProjectView(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    QFile file(":/icons/projectviewer/style.qss");
    file.open(QFile::ReadOnly);
    QString style = file.readAll();

    ui.view->header()->hide();
    ui.view->setStyleSheet(style);

    connect(ui.search,SIGNAL(textChanged(const QString &)),
            &proxy, SLOT(setFilterRegExp(const QString &)));
    connect(ui.search,SIGNAL(textChanged(const QString &)),
            this, SLOT(changeView()));
    connect(ui.view,SIGNAL(clicked(QModelIndex)),this,SLOT(clicked(QModelIndex)));

    installEventFilter(this);
}

ProjectView::~ProjectView()
{
}

void ProjectView::clicked(QModelIndex index)
{
    const QStandardItemModel * model = (QStandardItemModel *) proxy.sourceModel();
    QStandardItem * item = model->itemFromIndex(proxy.mapToSource(index));
    QMap<QString, QVariant> data = item->data().toMap();
    QString filename = data["file"].toString();
    QString exact = data["exact"].toString();

    QFileInfo fi(filename);
    if (!fi.exists() || !fi.isFile())
        return;

    QFile f(filename);

    f.open(QFile::ReadWrite);
    QString text = f.readAll();

    QTextDocument doc(text);
    int line = doc.find(exact).blockNumber();

    qDebug() << filename << line << exact;

    emit showFileLine(filename, line);
}

void ProjectView::changeView()
{
    if (!ui.search->text().isEmpty())
    {
        ui.view->expandAll();
    }
    else
    {
        ui.view->collapseAll();
        ui.view->setExpanded(ui.view->model()->index(0,0), true);
    }
}

void ProjectView::selectionChanged(
        const QItemSelection & selected,
        const QItemSelection & deselected)
{
    QModelIndex index = selected.indexes()[0];
    QStandardItem * item = ((QStandardItemModel * )index.model())->itemFromIndex(index);

    QList<QStandardItem *> items = ((QStandardItemModel *) index.model())->findItems(item->text());
}

void ProjectView::expandChildren(const QModelIndex &index, bool expandOrCollapse)
{
    if (!index.isValid()) {
        return;
    }

    int childCount = index.model()->rowCount(index);
    for (int i = 0; i < childCount; i++) {
        const QModelIndex &child = index.child(i, 0);
        expandChildren(child, expandOrCollapse);
    }


    ui.view->setExpanded(index, expandOrCollapse);
}

void ProjectView::setModel(QStandardItemModel * model)
{
    proxy.setSourceModel(model);

    ui.view->setModel(&proxy);

//    ui.view->setExpanded(ui.view->model()->index(0,0), true);

//    expandChildren(ui.view->model()->index(0,0), true);
    ui.view->setExpanded(ui.view->model()->index(0,0), true);
}

bool ProjectView::eventFilter(QObject *target, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *e = static_cast<QKeyEvent *>(event);

        switch (e->key())
        {
        case (Qt::Key_Escape):
            ui.search->clear();
            return true;
        }
    }
    return QWidget::eventFilter(target, event);
}
