#ifndef TCONTEXTHELPWIDGET_H
#define TCONTEXTHELPWIDGET_H

#include <QTextEdit>

class ContextItem;

class TContextHelpWidget : public QTextEdit
{
        Q_OBJECT
public:
        TContextHelpWidget(QWidget* parent=0);

private:
        QString create_html_for_object(QObject* obj);

        QHash<QString, QString> m_help;

private slots:
        void context_changed();
        void jog_started();
};

#endif // TCONTEXTHELPWIDGET_H
