#ifndef HEADER_TSP_H
#define HEADER_TSP_H

#include <QString>
#include <QDebug>


//! Определяет отображение редактора изображения
enum IE_ProfileType
{
    None = 0,
    Trichogram,
    Trichoscopy,
    Phototrichogram,
    Simple,
    MeasureIndex,
    Full,
    OnlyImage
};

static QString getIE_ProfileType(IE_ProfileType pt)
{
    switch (pt) {
    case None: return QString("None");
    case Trichogram: return QString("Trichogram");
    case Trichoscopy: return QString("Trichoscopy");
    case Phototrichogram: return QString("Phototrichogram");
    case Simple: return QString("Simple");
    case MeasureIndex: return QString("MeasureIndex");
    case Full: return QString("Full");
    case OnlyImage: return QString("OnlyImage");
    }
    return QString("None");
}

static IE_ProfileType getIE_ProfileType(QString title)
{
    if(title == "None")
        return IE_ProfileType::None;
    if(title == "Trichogram")
        return IE_ProfileType::Trichogram;
    if(title == "Trichoscopy")
        return IE_ProfileType::Trichoscopy;
    if(title == "Phototrichogram")
        return IE_ProfileType::Phototrichogram;
    if(title == "Simple")
        return IE_ProfileType::Simple;
    if(title == "MeasureIndex")
        return IE_ProfileType::MeasureIndex;
    if(title == "Full")
        return IE_ProfileType::Full;
    if(title == "OnlyImage")
        return IE_ProfileType::Full;
    return IE_ProfileType::None;
}

struct TSP_PatientData
{
    QString patient_fullName, doctor_fullName, modelDir, modelFilePath, patientDir, patient_nameAlias;
    uint patient_ID, patient_UID, doctor_ID, model_ID;
    TSP_PatientData()
    {
        patient_fullName = doctor_fullName = modelFilePath = "Empty";
        patient_ID = patient_UID = doctor_ID = model_ID = 0;
        modelDir=patientDir="";
    }
};


#endif // _HEADER_TSP_H
