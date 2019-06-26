#include "idcd_editor.h"
#include "ui_idcd_editor.h"
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QTimer>

IDCD_Editor::IDCD_Editor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IDCD_Editor)
{
    ui->setupUi(this);
    ui->ParameterList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->xmlDoc = new QDomDocument();

    QTimer *autoSaveTimer = new QTimer(this);
    connect(autoSaveTimer, SIGNAL(timeout()), this, SLOT(autoSave()));
    //autoSaveTimer->start(1000);   //Autosave disabled from multiple bugs (race conditions)

    this->setWindowTitle("IDCD Editor");
}

IDCD_Editor::~IDCD_Editor()
{
    delete ui;
}

void IDCD_Editor::save()
{
    this->generateXML();

    if(!this->filePath.isEmpty()){
        QFile file(this->filePath);
        if(file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)){
            QTextStream stream(&file);
            stream << this->xmlDoc->toString();
            file.close();
        }
        else{
            qDebug() << "Cannot save IDCD, unable to open file for read write operations" << filePath;
        }
    }
    else{
        qDebug() << "Cannot save IDCD, no file path specified";
    }
}

void IDCD_Editor::load()
{
    if(!this->filePath.isEmpty()){
        QFile file(this->filePath);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
            this->xmlDoc->clear();
            this->xmlDoc->setContent(&file);
            file.close();

            this->setWindowTitle("IDCD Editor - " + filePath.split("/").last());

            this->parseXML();
        }
    }
    else{
        qDebug() << "Cannot load IDCD, no file path specified";
    }
}

void IDCD_Editor::on_OpenFile_released()
{
    QString currentDir = QDir::currentPath();
    this->filePath = QFileDialog::getOpenFileName(this,
                        "Open IDCD File", currentDir, "XML Files (*.xml)");
    this->ui->FilePathInput->setText(this->filePath);

    //Load the XML
    this->load();
}

void IDCD_Editor::on_CreateFileButton_released()
{
    if(this->filePath.isEmpty()){
        QString currentDir = QDir::currentPath();
        this->filePath = QFileDialog::getSaveFileName(this,
                            "Save IDCD File", currentDir, "XML Files (*.xml)");

        this->ui->FilePathInput->setText(this->filePath);
    }
    else{
        //Do nothing
    }

    //Create the file
    delete this->xmlDoc;    //Free memory
    this->xmlDoc = new QDomDocument();
    this->save();

    //Enable autosave
    this->autoSaveEnabled = true;
}

void IDCD_Editor::on_AddMessageButton_released()
{
    Message m;
    this->messages.append(m);

    QList<Parameter> params;
    this->parameters.append(params);

    this->updateMessages();
}

void IDCD_Editor::parseXML()
{
    messages.clear();
    parameters.clear();

    QDomElement root = this->xmlDoc->documentElement();

    QDomElement Component = root.firstChild().toElement();

    int messageNum = 0;

    while(!Component.isNull()){
        //Child should be a message
        if(Component.tagName() == "Message"){
            //Get the ID, Name, and DLC
            QString name = Component.attribute("Name","BLANK_MESSAGE");
            int ID = Component.attribute("ID", "0").toInt();
            int DLC = Component.attribute("DLC", "0").toInt();

            Message m;
            m.Name = name;
            m.ID = static_cast<uint16_t>(ID);
            m.DLC = static_cast<uint8_t>(DLC);

            this->messages.append(m);

            QList<Parameter> params;

            //Grab all of the parameters
            QDomElement parameter = Component.firstChild().toElement();
            while(!parameter.isNull()){
                Parameter p;
                p.name = parameter.attribute("Name", "N/A");
                p.type = parameter.attribute("Type", "N/A");
                p.defaultVal = parameter.attribute("DefaultValue", "0").toInt();
                p.bit = parameter.attribute("Bit", "0").toInt();
                p.byte = parameter.attribute("Byte", "0").toInt();
                p.size = parameter.attribute("Size", "0").toInt();

                params.append(p);

                parameter = parameter.nextSibling().toElement();
            }
            parameters.append(params);
            messageNum++;
        }

        Component = Component.nextSibling().toElement();
    }

    //Enable autosave since a valid file is loaded.
    this->autoSaveEnabled = true;
    this->updateMessages();
}

void IDCD_Editor::generateXML()
{
    this->xmlDoc->clear();

    //Calculate byte and bit offset for each message parameter
    this->calculateParameterValues();

    QDomElement root = xmlDoc->createElement("Messages");
    xmlDoc->appendChild(root);

    for(int i = 0; i < messages.size(); i++){
        Message m = messages[i];
        QDomElement elem = xmlDoc->createElement("Message");
        elem.setAttribute("Name", m.Name);
        elem.setAttribute("ID", m.ID);
        elem.setAttribute("DLC", m.DLC);

        //Generate parameter xml
        for(int j = 0; j < parameters[i].size(); j++){
            Parameter p = parameters[i][j];
            QDomElement paramElem = xmlDoc->createElement("Parameter");
            paramElem.setAttribute("Name", p.name);
            paramElem.setAttribute("Type", p.type);
            paramElem.setAttribute("Size", p.size);
            paramElem.setAttribute("DefaultValue", p.defaultVal);
            paramElem.setAttribute("Byte", p.byte);
            paramElem.setAttribute("Bit", p.bit);
            elem.appendChild(paramElem);
        }

        root.appendChild(elem);
    }
}

void IDCD_Editor::organizeMessages()
{
    for(int j = 0; j < messages.size(); j++){
        for(int i = 0; i < messages.size() - 1; i++){
            if(messages[i].ID > messages[i + 1].ID){
                Message t = messages[i + 1];
                messages[i + 1] = messages[i];
                messages[i] = t;

                QList<Parameter> p = parameters[i + 1];
                parameters[i + 1] = parameters[i];
                parameters[i] = p;
            }
        }
    }
}

void IDCD_Editor::updateMessages()
{
    updatingInProgress = true;
    this->organizeMessages();
    this->clearMessagesFromGUI();
    this->ui->MessageList->setRowCount(messages.size());

    for(int i = 0; i < messages.size(); i++){
        Message *m = &messages[i];

        QTableWidgetItem *nameItem = new QTableWidgetItem();
        nameItem->setText(m->Name);

        QTableWidgetItem *idItem = new QTableWidgetItem();
        idItem->setText(QString::number(m->ID));

        QTableWidgetItem *dlcItem = new QTableWidgetItem();
        dlcItem->setText(QString::number(m->DLC));

        this->ui->MessageList->setItem(i, 0, nameItem);
        this->ui->MessageList->setItem(i, 1, idItem);
        this->ui->MessageList->setItem(i, 2, dlcItem);
    }
    updatingInProgress = false;
}

void IDCD_Editor::updateParameterGUI()
{
    int row = ui->MessageList->currentRow();
    if(row >= 0){
        ui->ParameterList->setRowCount(0);
        ui->ParameterList->setRowCount(this->parameters[row].size());

        for(int i = 0; i < this->parameters[row].size(); i++){
            Parameter p = this->parameters[row][i];

            ui->ParameterList->setItem(i, 0, new QTableWidgetItem());
            ui->ParameterList->setItem(i, 1, new QTableWidgetItem());
            ui->ParameterList->setItem(i, 2, new QTableWidgetItem());
            ui->ParameterList->setItem(i, 3, new QTableWidgetItem());

            ui->ParameterList->item(i, 0)->setText(p.name);
            ui->ParameterList->item(i, 1)->setText(p.type);
            ui->ParameterList->item(i, 2)->setText(QString::number(p.size));
            ui->ParameterList->item(i, 3)->setText(QString::number(p.defaultVal));
        }
    }
    else{
        //Do nothing
    }
}

void IDCD_Editor::clearMessagesFromGUI()
{
    this->ui->MessageList->setRowCount(0);
}

void IDCD_Editor::calculateParameterValues()
{
    for(int i = 0; i < parameters.size(); i++){
        int byte = 0;
        int bit = 0;
        for(int j = 0; j < parameters[i].size(); j++){
            Parameter p = parameters[i][j];

            //If we were finishing off a bit field, and this value isn't a bit
            if(bit > 0 && p.size >= 8){
                bit = 0;
                byte++;
            }

            p.bit = bit;
            p.byte = byte;

            //Increment the bit and byte offset
            int size = bit + p.size;
            while(size >= 8){
                size -= 8;
                byte++;
            }
            bit = size;

            parameters[i].replace(j, p);
        }
    }
}

void IDCD_Editor::saveQtHeader(QString headerText)
{
    QString currentDir = QDir::currentPath();
    QString filePath = QFileDialog::getSaveFileName(this,
                        "Save IDCD Qt Header File", currentDir, "Header Files (*.h)");

    QFile file(filePath);
    if(file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)){
        QTextStream stream(&file);
        stream << headerText;
        file.close();
    }
    else{
        qDebug() << "Cannot save IDCD, unable to open file for read write operations" << filePath;
    }
}

void IDCD_Editor::saveTivaHeader(QString headerText)
{
    QString currentDir = QDir::currentPath();
    QString filePath = QFileDialog::getSaveFileName(this,
                        "Save IDCD Tiva Header File", currentDir, "Header Files (*.h)");

    QFile file(filePath);
    if(file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)){
        QTextStream stream(&file);
        stream << headerText;
        file.close();
    }
    else{
        qDebug() << "Cannot save IDCD, unable to open file for read write operations" << filePath;
    }
}

void IDCD_Editor::dump_params(QList<IDCD_Editor::Parameter> params)
{
    qDebug() << "parameters: ";
    for(int i = 0; i < params.size(); i++){
        qDebug() << "\t" << params[i].name;
        qDebug() << "\t" << params[i].type;
        qDebug() << "\t" << params[i].defaultVal;
        qDebug() << "\t" << params[i].byte;
        qDebug() << "\t" << params[i].bit;
        qDebug() << "\t" << params[i].size;
        qDebug() << "\t";
    }
}

void IDCD_Editor::on_MessageList_itemChanged(QTableWidgetItem *item)
{
    if(!updatingInProgress){
        //Find the item
        for(int row = 0; row < ui->MessageList->rowCount(); row++){
            for(int col = 0; col < ui->MessageList->columnCount(); col++){
                if(ui->MessageList->item(row,col) == item){
                    Message m = messages.at(row);
                    switch(col){
                        case 0:
                            m.Name = item->text();
                            break;
                        case 1:
                            m.ID = static_cast<uint16_t>(item->text().toUInt());
                            break;
                        case 2:
                            m.DLC = static_cast<uint8_t>(item->text().toUInt());
                            break;
                        default:
                            //Do nothing
                            break;
                    }
                    messages.replace(row, m);
                }
            }
        }

        this->updateMessages();
    }
    else{
        //Currently updating GUI, do nothing
    }
}

void IDCD_Editor::on_RemoveMessageButton_released()
{
    int row = ui->MessageList->currentRow();
    ui->MessageList->removeRow(row);
    this->messages.removeAt(row);
    this->parameters.removeAt(row);
}

void IDCD_Editor::on_MessageList_currentCellChanged(int, int, int, int)
{
    this->updateParameterGUI();
}

void IDCD_Editor::on_AddParameterButton_released()
{
    Parameter p;
    int row = this->ui->MessageList->currentRow();

    if(row >= 0){
        this->parameters[this->ui->MessageList->currentRow()].append(p);
        this->updateParameterGUI();
    }
    else{
        //Do nothing
    }
}

void IDCD_Editor::on_ParameterList_itemChanged(QTableWidgetItem *item)
{
    int messageIndex = ui->MessageList->currentRow();

    if(messageIndex >= 0){
        //Find the item
        for(int row = 0; row < ui->ParameterList->rowCount(); row++){
            for(int col = 0; col < ui->ParameterList->columnCount(); col++){
                if(ui->ParameterList->item(row,col) == item){
                    Parameter p = parameters[messageIndex][row];
                    switch(col){
                        case 0://Name
                            p.name          = item->text();
                            break;
                        case 1://Type
                            p.type          = item->text();
                            break;
                        case 2://Size
                            p.size          = item->text().toInt();
                            break;
                        case 3://Default value
                            p.defaultVal    = item->text().toInt();
                            break;
                        default:
                            //Do nothing
                            break;
                    }
                    parameters[messageIndex].replace(row, p);
                }
            }
        }
    }
}

void IDCD_Editor::autoSave()
{
    if(this->autoSaveEnabled){
        this->save();
    }
    else{
        //Do nothing
    }
}

void IDCD_Editor::on_RemoveParameterButton_released()
{
    this->parameters[this->ui->MessageList->currentRow()].removeAt(this->ui->ParameterList->currentRow());
    this->updateParameterGUI();
}

void IDCD_Editor::on_GenerateQtHeaderButton_released()
{
    QString HeaderText;

    //Add headers 'header'
    HeaderText += "#ifndef IDCD_H\n"
                  "#define IDCD_H\n"
                  "\n"
                  "#include \"message.h\"\n"
                  "\n";

    for(int i = 0; i < messages.size(); i++){
        Message m = messages[i];

        //Replace spaces in the name with underscores
        QString structName = m.Name.replace(" ", "_");

        QString structText = ""
                             "struct %1 : Message{\n"   //Message name
                             "  uint16_t id = %2;\n"    //ID
                             "  uint8_t dlc = %3;\n"    //DLC
                             "\n"
                             "  struct %4_data{\n"      //Message name
                             "%5\n"//Parameters line
                             "  };"
                             "\n"
                             "  %4_data data;\n"
                             "\n"
                             "  %6(){\n"                  //Message name
                             "      this->ID = id;\n"
                             "      this->DLC = dlc;\n"
                             "      this->pData = reinterpret_cast<uint8_t*>(&data);\n"   //Message name
                             "  }\n"
                             "};\n";

        //Create parameter text
        QString parameterText;
        for(int j = 0; j < parameters[i].size(); j++){
            Parameter p = parameters[i][j];

            parameterText += "      %1 %2%3 = %4;";    //type, name, default value, bit packing

            //Type
            parameterText = parameterText.arg(p.type);

            //Name
            parameterText = parameterText.arg(p.name);

            //Bit packing if necessary
            QString bitPackingText = "";
            if(p.size % 8 != 0){
                //Bit packing is present
                bitPackingText = " : %1";
                bitPackingText = bitPackingText.arg(p.size);
            }
            else{
                //Do nothing
            }
            parameterText = parameterText.arg(bitPackingText);

            //Default value
            parameterText = parameterText.arg(p.defaultVal);

            //Add newline if it isn't the last line
            if(j < parameters[i].size() - 1){
                parameterText += "\n";
            }
        }

        //Message name
        structText = structText.arg(structName);

        //Message ID
        structText = structText.arg(m.ID);

        //Message DLC
        structText = structText.arg(m.DLC);

        //Message name
        structText = structText.arg(structName);

        //Parameters
        structText = structText.arg(parameterText);

        //Message name
        structText = structText.arg(structName);

        //Message name
        structText = structText.arg(structName);

        HeaderText += structText + "\n";
    }

    //Append footer
    HeaderText += "#endif // IDCD_H\n";

    this->saveQtHeader(HeaderText);
    QFile file("P:\\QtProjects\\PumaGUI\\build-PUMA_GUI-Desktop_Qt_5_11_2_MSVC2015_64bit-Debug\\test.txt");
    file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate);
    QTextStream stream(&file);
    stream << HeaderText;
    file.close();
}

void IDCD_Editor::on_GenerateTivaHeaderButton_released()
{
    QString HeaderText;

    //Add headers 'header'
    HeaderText += "#ifndef IDCD_H\n"
                  "#define IDCD_H\n"
                  "\n";

    for(int i = 0; i < messages.size(); i++){
        Message m = messages[i];

        //Replace spaces in the name with underscores
        QString defineName = m.Name.replace(" ", "_").toUpper();
        QString structName = m.Name.replace(" ", "");

        QString messageInit = "{%1_ID, %1_DLC, %1_DATA_INIT}";
        messageInit = messageInit.arg(defineName);

        //Create parameter text
        QString parameterText;
        QString dataInit = "{";
        for(int j = 0; j < parameters[i].size(); j++){
            Parameter p = parameters[i][j];

            dataInit += "0";

            if(j < parameters[i].size() - 1){
                dataInit += ",";
            }
            else{
                //Do nothing
            }

            parameterText += "      %1 %2%3;";    //type, name, default value, bit packing

            //Type
            parameterText = parameterText.arg(p.type);

            //Name
            parameterText = parameterText.arg(p.name);

            //Bit packing if necessary
            QString bitPackingText = "";
            if(p.size % 8 != 0){
                //Bit packing is present
                bitPackingText = " : %1";
                bitPackingText = bitPackingText.arg(p.size);
            }
            else{
                //Do nothing
            }
            parameterText = parameterText.arg(bitPackingText);

            //Add newline if it isn't the last line
            if(j < parameters[i].size() - 1){
                parameterText += "\n";
            }
        }

        dataInit += "}";

        QString structText = ""
                             "#define %1_ID                 %2\n"
                             "#define %1_DLC                %3\n"
                             "#define %1_DATA_INIT          %4\n"
                             "#define %1_INIT               %5\n"
                             "\n"
                             "typedef struct %7_data_tag{\n"
                             "%6\n"
                             "} %7_data;\n"
                             "\n"
                             "typedef struct %7_tag{\n"
                             "  uint16_t ID;\n"
                             "  uint8_t DLC;\n"
                             "  %7_data data;\n"
                             "} %7;\n"
                             "\n";

        structText = structText.arg(defineName);
        structText = structText.arg(m.ID);
        structText = structText.arg(m.DLC);
        structText = structText.arg(dataInit);
        structText = structText.arg(messageInit);
        structText = structText.arg(parameterText);
        structText = structText.arg(structName);

        HeaderText += structText + "\n";
    }

    //Append footer
    HeaderText += "#endif // IDCD_H\n";

    this->saveTivaHeader(HeaderText);
    QFile file("P:\\QtProjects\\PumaGUI\\build-PUMA_GUI-Desktop_Qt_5_11_2_MSVC2015_64bit-Debug\\test.txt");
    file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate);
    QTextStream stream(&file);
    stream << HeaderText;
    file.close();
}
