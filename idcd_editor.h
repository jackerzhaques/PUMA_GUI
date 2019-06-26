#ifndef IDCD_EDITOR_H
#define IDCD_EDITOR_H

#include <QDialog>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QXmlStreamReader>
#include <QTableWidgetItem>

#include "message.h"

namespace Ui {
class IDCD_Editor;
}

class IDCD_Editor : public QDialog
{
    Q_OBJECT

public:
    explicit IDCD_Editor(QWidget *parent = nullptr);
    ~IDCD_Editor();

    void save();
    void load();

public slots:
    void autoSave();

private slots:
    void on_OpenFile_released();

    void on_CreateFileButton_released();

    void on_AddMessageButton_released();

    void on_MessageList_itemChanged(QTableWidgetItem *item);

    void on_RemoveMessageButton_released();

    void on_MessageList_currentCellChanged(int, int, int, int);

    void on_AddParameterButton_released();

    void on_ParameterList_itemChanged(QTableWidgetItem *item);

    void on_RemoveParameterButton_released();

    void on_GenerateQtHeaderButton_released();

    void on_GenerateTivaHeaderButton_released();

private:
    struct Parameter{
        QString name    = "BLANK_PARAMETER";
        QString type    = "uint8_t";
        int defaultVal  = 0;
        int byte        = 0;
        int bit         = 0;
        int size        = 8;
    };

    Ui::IDCD_Editor *ui;
    QDomDocument *xmlDoc;
    QString filePath    = "";
    bool autoSaveEnabled       = false;
    bool updatingInProgress    = false;

    QList<Message> messages;
    QList<QList<Parameter>> parameters;

    void parseXML();
    void generateXML();
    void organizeMessages();
    void updateMessages();
    void updateParameterGUI();
    void clearMessagesFromGUI();
    void calculateParameterValues();

    void saveQtHeader(QString headerText);
    void saveTivaHeader(QString headerText);

    void dump_params(QList<Parameter> params);
};

#endif // IDCD_EDITOR_H
