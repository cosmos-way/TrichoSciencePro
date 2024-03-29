#include "ie_imageBase.h"

#include <QMessageBox>
#include <QDebug>
#include <QJsonDocument>
#include <QVBoxLayout>
#include <QTreeView>
#include <QListView>

const char * IE_ImageBaseCnt::STD_DIR = "data/imageBase";
const char * IE_ImageBaseCnt::FILE_NAME_RULE = "%1%2";
const char * IE_ImageBaseCnt::FILE_NAME = "imageBase";
const char * IE_ImageBaseCnt::FILE_EXTENSION = ".json";
const char * IE_ImageBaseCnt::TSP_docTypeVALUE = "TSP_JSON_imageBase";

IE_ImageBaseCnt::IE_ImageBaseCnt(QDir workDir, QObject *parent) : QObject(parent),
    FULL_FILE_NAME(QString(FILE_NAME_RULE).arg(FILE_NAME).arg(FILE_EXTENSION)),
    m_workDir( workDir )
{
    m_pTreeModel = nullptr;
    m_allImagesDockWidget = nullptr;
    m_pWidget = nullptr;
    open();
}

QDockWidget *IE_ImageBaseCnt::getDockWidgetWithAllImages()
{
    if(!m_allImagesDockWidget)
    {
        m_pWidget = new IE_IB_widget();
        m_pWidget->init();
        m_pWidget->setDataModel(m_pTreeModel);
        m_allImagesDockWidget = new QDockWidget("Справочные изображения");
        m_allImagesDockWidget->setWidget(m_pWidget);
    }
    return m_allImagesDockWidget;
}

bool IE_ImageBaseCnt::containsImageBaseUserChoice(const QJsonObject &json)
{
    return IE_IB_treeModel::containsImageBaseUserChoice(json);
}

int IE_ImageBaseCnt::readUserChoice(const QJsonObject &json, int index)
{
    //! Установка данных выбора пользователя из массива-объекта `imageBaseUserChoiceArray`
    //!

    if(!m_pTreeModel)
        return 1;

    return m_pTreeModel->readUserChoice(json, index);
}

int IE_ImageBaseCnt::setUserChoiceListSize(int size)
{
    if(!m_pTreeModel)
        return 1;
    return m_pTreeModel->setUserChoiseListSize(size);
}

int IE_ImageBaseCnt::writeUserChoice(QJsonObject &json, int index) const
{
    if(!m_pTreeModel)
        return 1;


    return m_pTreeModel->writeUserChoice(json, index);
}

int IE_ImageBaseCnt::setCurrentUserChoiceList(int num)
{
    if(!m_pTreeModel)
        return 1;
     m_pTreeModel->setCurrentUserChoiceList(num);
     return 0;
}

int IE_ImageBaseCnt::read(const QJsonObject &json)
{
    if(m_pTreeModel)
        delete m_pTreeModel;
    m_pTreeModel = new IE_IB_treeModel(m_workDir);
    m_pTreeModel->setImageBaseData(json);
    return 0;
}

int IE_ImageBaseCnt::write(QJsonObject &json) const
{
    if(m_pTreeModel)
        m_pTreeModel->write(json);
    return 0;
}

int IE_ImageBaseCnt::open()
{
    QFile imageBaseFile;
    imageBaseFile.setFileName( m_workDir.filePath( FULL_FILE_NAME ) );

    if(!imageBaseFile.exists())
    {
        save();
        if(imageBaseFile.exists())
        {
            return open();
        }
        return 1;
    }
    if(!imageBaseFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr, "Application", QString("Ошибка. Не удалось открыть файл %1 с пациентами.")
                                                                .arg(imageBaseFile.fileName()));
        qWarning() << "Couldn't open patientIndex file " << imageBaseFile.fileName();
        return 1;
    }

    QByteArray saveData = imageBaseFile.readAll();
    imageBaseFile.close();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));


    QJsonObject jsonDoc = loadDoc.object();

    if(jsonDoc["TSP_docType"] != TSP_docTypeVALUE)
    {
        qWarning() << imageBaseFile.fileName() << "isn't " << TSP_docTypeVALUE << ".";
        QMessageBox::warning(nullptr, "Application", QString("Ошибка. Не удалось прочитать файл %1 с базой изображений.")
                                                                .arg(imageBaseFile.fileName()));
        imageBaseFile.rename( m_workDir.filePath("!!!patientIndex_ERROR.json") );
        return 1;
    }

     return read(jsonDoc);
}

int IE_ImageBaseCnt::save()
{
    if( !m_workDir.exists() )
        m_workDir.mkpath(".");

    QFile imageBaseFile;
    imageBaseFile.setFileName( m_workDir.filePath( FULL_FILE_NAME ) );
    if(!imageBaseFile.open(QIODevice::WriteOnly))
    {
        qDebug() << m_workDir.absolutePath();
        qWarning() << "Couldn't open file " << imageBaseFile.fileName() << ".";
        return 0;
    }
    QJsonObject pimageBaseObj;
    pimageBaseObj["TSP_docType"] = TSP_docTypeVALUE;
    write(pimageBaseObj);
    QJsonDocument saveDoc(pimageBaseObj);
    imageBaseFile.write(saveDoc.toJson());
    imageBaseFile.close();
    return 0;
}
