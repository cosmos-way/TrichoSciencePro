#ifndef TSPIMAGEEDITORMODEL_H
#define TSPIMAGEEDITORMODEL_H

#include <QtWidgets>
#include "ie_report.h"
#include "ie_fieldOfView_controller.h"
#include "ie_modelLayer_controller.h"

QT_CHARTS_USE_NAMESPACE

class IE_ModelData: public QObject
{
    Q_OBJECT
public:
    IE_ModelData();
    void initNew(TSP_PatientData patientData);
    void init(TSP_PatientData patientData);
    void update();

#ifdef QT_DEBUG
    void activateTestData()
    {
        modelDir = "data/patients/0";
        patientID = 0;
        patientFullName = "ФИО тестовые";
    }
#endif
    QString getPath();

    TSP_PatientData to_TSP_patientData();

    int    read(const QJsonObject &json);
    int    write(QJsonObject &json) const;
    void printAllData();

    IE_ProfileType getProfile() const;
    void setProfile(const IE_ProfileType &value);

    QString getPatientFullName() const;

    QDir getModelDir() const;

    QDir getResDir() const;

    QDir getTmpDir() const;
    void setTmpDir(const QDir &value);

    uint getPatientID() const;

    uint getPatientUID() const;

    uint getModel_ID() const;
    void setModel_ID(const uint &value);
    IEM_type getIem_type() const;
    void setIem_type(const IEM_type &iem_type);

    QDateTime getSaveDateTime() const;
    void setSaveDateTime(const QDateTime &saveDateTime);

    QDateTime getCreateDateTime() const;

signals:
    void updated();
private:
    IE_ProfileType          profile;    /** Тип редактора изображения. К примеру, в трихосопии
                                            находится раздел "плотность волос", этот же раздел
                                            может находится в другом редакторе, но при этом, сам
                                            тип модели не меняется. Это нужно только для отображения.
                                        */
    IEM_type                m_iem_type; /** Тип модели. Определяет набор инструментов, и параметры,
                                            которые необходимы.
                                            Точки определения типа: создание объекта (инициалиазция
                                            модели???), чтение модели.
                                        */
    QString patientFullName; //     ФИО

    QDir    modelDir, // директория модели. Новая создается в папке IE_MODEL_RES_DIR_NAME
            resDir, // директория ресурсов модели. В ней хранятся различные изображения, файлы, которые используются.
            tmpDir, // директория временного размещения данных модели,
                    //  в ней хранятся данные до сохранения. Удаляется после закрытия модели.
            patientDir;
    uint    patientID,
            patientUID, // присваивается ЕДИНОЖДЫ !!!
            model_ID;

    QDateTime m_createDateTime, m_saveDateTime;
};

/*!
\brief Класс для работы с моделью изображения.

\todo сохранение данных о связанных моделях.

*/
class IE_Model : public QGraphicsScene
{
    Q_OBJECT
public:
                            IE_Model();
                            ~IE_Model();

    int                     read(const QJsonObject &json);
    int                     write(QJsonObject &json)const;

    QRectF                  getModelRect() const;
    qreal                   getMeasureIndex() const;
    ToolsController*        getPToolCnt() const;
    //! \warning срочно убрать из реализации
//    IE_ModelLayer*          getLayerByListIndex(int listIndex);
    //! \warning срочно убрать из реализации передачу подобного списка
//    QList<IE_ModelLayer*>::iterator
//                            getLayerIteratorByListIndex(int listIndex);
    QDockWidget*            getPDockLayers() const;
    //! \warning срочно убрать из реализации передачу подобного списка
//    QList<IE_ModelLayer * > :: const_iterator
//                            getLayersListIter() const;
    IE_ModelLayer::PublicConstPtrToList
                            getConstModelLayerListConstPtr() const;

    //! \warning срочно убрать из реализации передачу подобного списка
//    QList<IE_ModelLayer * > getLayersList() const;
    TSP_PatientData         get_TSP_patientData();
    IE_ProfileType          getIE_ProfileType()const;
    _global_ie *            getPGlobal_data() const;
    QDockWidget *           getFieldOfViewControllerInfoDock() const;
    QStringList             getRelatedModelList() const;
    QString                 getPath();
    IEM_type                getIEM_type();
    QDockWidget *           getImageBaseDockWidget();
    QWidget *               getFieldOfViewFastManagerWidget();

    void                    addRelatedModel(QString path);

    void                    setMeasureIndex(const qreal &value);
    void                    setPToolCnt(ToolsController *value);

//    void                    showLayer(int listIndex);
//    void                    hideLayer(int listIndex);

    QDockWidget*            initInfoDock();
//    QDockWidget*            initLayersDock();
    QDockWidget*            initToolInfoDock();
    QDockWidget*            initFieldOfViewControllerInfoDock();

//    void                    makeHairDensityComputeWithWidget();
//    void                    makeHairDiameterComputeWithWidget();
    void                    makeReport(IE_ReportType rt);
    void                    setInputArgs();


signals:
    void                    changedModelSize(qreal fx, qreal fy);
    void                    measureIndexChanged(qreal measure);
    //! \todo перенос в контроллер слоев
    void                    layerListWasChanged();
    void                    boundingRectWasChanged(QRectF boundingRect);
    void                    wasSaved();


public slots:
    int                     initAsNewModel (TSP_PatientData patientData,
                                            IEM_type iem_type,
                                            IE_ProfileType ie_type,
                                            bool dialog = false
                                            );
    int                     initWithModel(TSP_PatientData patientData);
    int                     saveModel();
    int                     saveModelAsImage();
    void                    save(QString modelFilePath);
    void                    close(QString modelFilePath);

    //! \todo манипуляции со слоями должны происходить через метод makeActionWithLayer(...)
    void                    addLayer(IE_ModelLayer* layerToAdd);
    void                    addLayerViaToolCnt();

    void                    layerAction(IE_ModelLayer::Action action,
                                        IE_ModelLayer::PublicType
                                        layer);

private:
    QFileInfo               modelDataFileInfo;
    QRectF                  modelRect;
    QTableView              * pDockTableView;
    QDockWidget             * pDockDebugInfo,
                            * pDockLayers,
                            * m_pFVCDockWidget;

    //! \todo убрать список слоев
    IE_ModelLayer_Controller m_modelLayerCnt;
//    QList<IE_ModelLayer*>   layersList;
    QStringList             m_relatedModelList;
    ToolsController         * pToolCnt;
    IE_FieldOfView_Controller
                            * m_pFieldOfViewCnt;
    IE_ModelData            _modelData;

    _global_ie              * m_ieGlobalData;

    QDialog::DialogCode makeDialogForSetupModelAsNew();

private slots:
    void                    selectedLayer(int row);
};



#endif // TSPIMAGEEDITORMODEL_H
