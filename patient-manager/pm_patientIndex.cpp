#include "pm_patientIndex.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QAction>
#include <QMenu>

PM_PatientIndexCnt::PM_PatientIndexCnt(QDir workDir, QObject *parent): QObject(parent), m_workDir(workDir)
{
    m_pStackedWidget = new QStackedWidget();

    m_pPatientListWidget = nullptr;
    m_pSeacrhWidget = nullptr;
    m_pCurrentPatient = nullptr;
    if(!workDir.exists())
        workDir.mkpath(".");


}

PM_PatientIndexCnt::~PM_PatientIndexCnt()
{
    delete m_pStackedWidget;
}

int PM_PatientIndexCnt::init()
{
    int answer = open();
    if(answer == ReturnCode::Error_needToCheckDirs)
    {
        checkDirs();
        answer = open();
    }
    if(answer)
        return answer;
    emit availableActionsWasChanged();
    return 0;
}

void PM_PatientIndexCnt::checkDirs()
{
    m_patientIndexList.clear();
    if(m_pPatientListWidget)
        m_pPatientListWidget->clear();
    QStringList folderList = m_workDir.entryList(QDir::Dirs);
    foreach(QString subdir, folderList)
    {
        if(subdir == "." || subdir == "..")
            continue;
        TSP_PatientData patientData = PM_Patient::getTSP_PatientData( m_workDir.filePath(subdir) );
        if(patientData.patient_UID != 0)
            m_patientIndexList.append( PM_PatientIndex(patientData) );
    }
    save();
}

void PM_PatientIndexCnt::openPatientWidget(uint patientUID)
{
    if(m_pCurrentPatient)
        delete m_pCurrentPatient;
    m_pCurrentPatient = new PM_Patient(m_workDir);
    if(m_pCurrentPatient->open(patientUID))
        return;

    m_pStackedWidget->addWidget(m_pCurrentPatient->getWidget());
    m_pStackedWidget->setCurrentWidget(m_pCurrentPatient->getWidget());

    connect(m_pCurrentPatient, &PM_Patient::goBack, [this]()
    {
        m_pStackedWidget->removeWidget(m_pCurrentPatient->getWidget());
        delete m_pCurrentPatient;
        m_pCurrentPatient = nullptr;
        emit availableActionsWasChanged();
    });
    connect(m_pCurrentPatient, &PM_Patient::dataWasChanged, this, &PM_PatientIndexCnt::checkDirs);
    connect(m_pCurrentPatient, &PM_Patient::needToRunImageEditor, [this](TSP_PatientData patientData, IE_ProfileType ie_type)
    {
        emit needToRunImageEditor(patientData, ie_type);
    });

    emit availableActionsWasChanged();
}

int PM_PatientIndexCnt::addPatient_Dialog()
{
    PM_Patient patient;
    patient.initAsNew_Dialog();
    checkDirs();
    return 0;
}

int PM_PatientIndexCnt::read(const QJsonObject &json)
{
    m_patientIndexList.clear();
    QJsonArray patientArray = json["patientArray"].toArray();
    for (int patientIndex = 0; patientIndex < patientArray.size(); ++patientIndex) {
            QJsonObject patientObject = patientArray[patientIndex].toObject();
            PM_PatientIndex pi;
            pi.read(patientObject);
            m_patientIndexList.append( pi );
        }
    emit patinetIndexWasChanged();
    return 0;
}

//! Не стоит беспокоиться о том, что база будет большой. Ее размеры не могут привышать более 5 тыс. пациентов. Так как это максимум, то и данные можно каждый раз перезаписывать.
int PM_PatientIndexCnt::write(QJsonObject &json) const
{
    QJsonArray patientArray;
    foreach(PM_PatientIndex patientIndex, m_patientIndexList)
    {
        QJsonObject patientObj;
        patientIndex.write(patientObj);
        patientArray.append(patientObj);
    }
    json["patientArray"] = patientArray;

    return 0;
}



QList<QMenu*> PM_PatientIndexCnt::getAvailableActions()
{
    if(m_pCurrentPatient)
        if(m_pStackedWidget->currentWidget() == m_pCurrentPatient->getWidget())
            return m_pCurrentPatient->getAvailableActions();
    QList<QMenu*> menuList;
    QMenu * pMenu = new QMenu("Файл");
    QAction *pAction = new QAction("Экспорт всех данных");
    pAction->setDisabled(true);
    pMenu->addAction(pAction);
    pAction = new QAction("Импорт всех данных");
    pAction->setDisabled(true);
    pMenu->addAction(pAction);
    menuList.append(pMenu);

    pMenu = new QMenu("Пациент");

    pAction = new QAction("Добавить");
    pMenu->addAction(pAction);
    connect(pAction, &QAction::triggered, this, &PM_PatientIndexCnt::addPatient_Dialog);

    pAction = new QAction("Отобразить всех");
    pMenu->addAction(pAction);
    pAction = new QAction("Найти всех пациентов");
    connect(pAction, &QAction::triggered, this, &PM_PatientIndexCnt::checkDirs);
    pMenu->addAction(pAction);

    pAction = new QAction("Импорт пацента");
    pAction->setDisabled(true);
    pMenu->addAction(pAction);

    menuList.append(pMenu);

    pMenu = new QMenu("Действия");

    pAction = new QAction("Экспресс трихоскопия...");
    pAction->setDisabled(true);
    pMenu->addAction(pAction);

    menuList.append(pMenu);

    menuList.append(pMenu);
    return menuList;
}

int PM_PatientIndexCnt::save()
{
    QFile patientIndexFile;
    patientIndexFile.setFileName( m_workDir.filePath("patientIndex.json") );
    if(!patientIndexFile.open(QIODevice::WriteOnly))
    {
        qWarning("Couldn't open file.");
        return ReturnCode::Ok;
    }
    QJsonObject patientIndexObj;
    patientIndexObj["TSP_docType"] = "TSP_JSON_patientIndex";
    write(patientIndexObj);
    QJsonDocument saveDoc(patientIndexObj);
    patientIndexFile.write(saveDoc.toJson());
    patientIndexFile.close();
    return ReturnCode::Ok;
}

int PM_PatientIndexCnt::open()
{
    QFile patientIndexFile;
    patientIndexFile.setFileName( m_workDir.filePath("patientIndex.json") );

    if(!patientIndexFile.exists())
        return ReturnCode::Error_needToCheckDirs;
    if(!patientIndexFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr, "Application", QString("Ошибка. Не удалось открыть файл %1 с пациентами.")
                                                                .arg(patientIndexFile.fileName()));
        qWarning() << "Couldn't open patientIndex file " << patientIndexFile.fileName();
        return ReturnCode::Error_needToCheckDirs;
    }

    QByteArray saveData = patientIndexFile.readAll();
    patientIndexFile.close();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));


    QJsonObject jsonDoc = loadDoc.object();

    if(jsonDoc["TSP_docType"] != "TSP_JSON_patientIndex")
    {
        qWarning() << patientIndexFile.fileName() << "isn't TSP_JSON_patientIndex.";
        QMessageBox::warning(nullptr, "Application", QString("Ошибка. Не удалось прочитать файл %1 с пациентами.")
                                                                .arg(patientIndexFile.fileName()));
        patientIndexFile.rename( m_workDir.filePath("!!!patientIndex_ERROR.json") );
        return ReturnCode::Error_needToCheckDirs;
    }

     return read(jsonDoc);
}

QWidget *PM_PatientIndexCnt::getMainWidget()
{
    if(!m_pSeacrhWidget)
    {
        initSearchWidget();
        m_pStackedWidget->addWidget(m_pSeacrhWidget);
    }
    return m_pStackedWidget;
}

QList<PM_PatientIndex> PM_PatientIndexCnt::search(QString str)
{
    //! желательно переделать и делать поиску по каждому слову поисковой фразы
    //! Алгоритм поиска:
    //! - проверяется, является ли строка числом. Если является, то происходит поиск среди "id" and "uid".
    //! - далее, проверется существование подстроки в строке "fullName" and "nameAlias". Если в фразе есть
    //! пробел, то берется первое слово.

    open();
    if(str.isEmpty())
        return m_patientIndexList;


    QList<PM_PatientIndex> patientList;
    patientList.clear();
    QString currentStr = str;

    bool ok;
    uint id = str.toUInt(&ok);
    if(ok)
    {
        foreach(PM_PatientIndex patientIndexElem,m_patientIndexList)
            if(patientIndexElem.id == id || patientIndexElem.uid == id)
                patientList.append(patientIndexElem);
    }

    bool answer = true;
    while(answer)
    {
        foreach(PM_PatientIndex patientIndexElem,m_patientIndexList)
        {
            if( patientIndexElem.fullName.contains(str, Qt::CaseInsensitive) || patientIndexElem.alias.contains(str, Qt::CaseInsensitive) )
            {
                bool eq = false;
                foreach(PM_PatientIndex pi, patientList)
                    if(patientIndexElem == pi)
                    {
                        eq = true;
                        break;
                    }
                if(!eq)
                    patientList.append(patientIndexElem);
            }
        }
        if( str.contains(" ") )
            str = str.split(" ").at(0);
        else answer = false;
    }
    return patientList;
}

QWidget * PM_PatientIndexCnt::initSearchWidget()
{
    m_pSeacrhWidget = new QWidget();
    QVBoxLayout * pvbLayout = new QVBoxLayout(m_pSeacrhWidget);
    QLineEdit * pSearchLineEdit = new QLineEdit(m_pSeacrhWidget);
    pvbLayout->addWidget(pSearchLineEdit);
    QPushButton * pButton = new QPushButton("Поиск", m_pSeacrhWidget);
    pvbLayout->addWidget(pButton);

    connect(pButton, &QPushButton::clicked, [this, pSearchLineEdit, pvbLayout]()
    {

        QStringList patientListForWidget;
        patientListForWidget.clear();
        QList<PM_PatientIndex> listResult = search(pSearchLineEdit->text());
        QString tmp;
        foreach(PM_PatientIndex elem, listResult)
            patientListForWidget.append( QString("%1\tid: %2\tuid: %3").arg(elem.alias).arg(elem.id).arg(elem.uid) );

        if(!m_pPatientListWidget)
        {
            m_pPatientListWidget = new QListWidget();
            pvbLayout->addWidget(m_pPatientListWidget);
            connect(m_pPatientListWidget, &QListWidget::doubleClicked, [this, patientListForWidget](const QModelIndex &index)
            {
                if(index.row()==-1)
                    return;
                QString text = m_pPatientListWidget->currentItem()->text();
                int uidPos = text.indexOf("uid: ");
                uint uid = text.mid(uidPos+5).toUInt();

                openPatientWidget(uid);
            });

        }

        m_pPatientListWidget->clear();
        m_pPatientListWidget->addItems(patientListForWidget);
    });
    return m_pSeacrhWidget;
}

bool operator==(const PM_PatientIndex &left, const PM_PatientIndex &right)
{
    if(left.id != right.id)
        return false;
    if(left.uid != right.uid)
        return false;
    if(left.fullName != right.fullName)
        return false;
    if(left.alias != right.alias)
        return false;
    return true;
}
