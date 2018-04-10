#include "fingerprintreader.h"
#include <QDebug>
#include <iostream>
#include <libfprint/fprint.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct fp_dev *fpdev = NULL;
struct fp_dscv_print **fp_dscv_prints = NULL;

int enroll_stages = 0;
int current_enroll_stage = 1;
static struct fp_img *last_fp_img = NULL;
static struct fp_print_data *enroll_data = NULL;
struct fp_print_data *print_gallery[5000];
char *access_fingerprint_index[5000];

bool enroll_complete = false;
QString access_code = "";
QString access_code2 = "";
CFingerprintReader::CFingerprintReader(QObject *parent) : 
    QObject(parent)
{
}

CFingerprintReader::~CFingerprintReader()
{
    //close up here
}

static void dev_open_cb(struct fp_dev *dev, int status, void *user_data)
{
    struct fp_driver *drv;

    Q_UNUSED(status);
    Q_UNUSED(user_data);

    fpdev = dev;
    drv = fp_dev_get_driver(fpdev);
    qDebug() << "CFingerprintReader::openDeviceHandle(), Device ready to use!";
    qDebug() << "CFingerprintReader::openDeviceHandle(), Driver: '" << fp_driver_get_full_name(drv) << "'";

    if (fp_dev_supports_imaging(fpdev))
    {
        qDebug() << "CFingerprintReader::openDeviceHandle(), IS an imaging device!";
    }
    else
    {
        qDebug() << "CFingerprintReader::openDeviceHandle(), IS NOT an imaging device!";
    }

    enroll_stages = fp_dev_get_nr_enroll_stages(fpdev);
    qDebug() << "CFingerprintReader::openDeviceHandle(), Required enroll stages: " << QString::number(enroll_stages);
}

bool CFingerprintReader::openDeviceHandle()
{
    // Open the device using the VID, PID,
    // and optionally the Serial number.
    // _handle = hid_open(VID, PID, NULL);

    struct fp_dscv_dev **discovered_devs;
    struct fp_dscv_dev *ddev;

    int i;
    int r = 1;

    discovered_devs = fp_discover_devs();
    if( !(sizeof(discovered_devs) / sizeof(discovered_devs[0])) )
    {
        qDebug() << "CFingerprintReader::openDeviceHandle(), No fingerprint devices found..";
        return false;
    }

    int count = 0;
    //struct fp_driver *drv;
    for(i=0; (ddev=discovered_devs[i]); i++)
    {
        //drv = fp_dscv_dev_get_driver(ddev);
        (void)fp_dscv_dev_get_driver(ddev);
        // for now, we'll just stop on one
        count++;
        break;
    }

    qDebug() << "CFingerprintReader::openDeviceHandle(), Opening Device..";

    if( count )
    {
        r = fp_async_dev_open(ddev, dev_open_cb, NULL);
    }

    if( r )
    {
        qDebug() << "CFingerprintReader::openDeviceHandle(), Opening Device FAILED";
        return false;
    }

    return true;
}

//thread start method, may not use this
void CFingerprintReader::start()
{
    qDebug() << "CFingerprintReader::start()";
    while(1) 
    {
        //readCardReaderLoop();
        QCoreApplication::processEvents();
    }
}

bool CFingerprintReader::initFingerprintReader()
{
    int r = fp_init();
    if( r < 0 )
        return false;

    return openDeviceHandle();
}

void CFingerprintReader::handleEvents()
{
    //    fp_handle_events() - blocks for a few moments
    //    fp_handle_events_timeout(struct timeval) - blocks on timeout
    //  * fp_handle_events_timeout(struct timeval), tv_*=0 - nonblocking call
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    fp_handle_events_timeout(&tv);
}

void CFingerprintReader::enrollStageCb(struct fp_dev *dev, int result,
                                       struct fp_print_data *print, struct fp_img *img,
                                       void *user_data)
{
    QString tmp = "";
    QString prependPath = "";
    QString fullPath = "";

    Q_UNUSED(dev);

    if (result < 0) {
        //edlg_cancel_enroll(result);
        return;
    }

    if (img) 
    {
        last_fp_img = img;
    } 
    else 
    {
        last_fp_img = NULL;
    }

    if (print)
    {
        enroll_data = print;
    }

    switch (result) 
    {
        case FP_ENROLL_COMPLETE:
            tmp = tr("<b>Enrollment completed!</b>");
            break;
        case FP_ENROLL_PASS:
            tmp = "";
            current_enroll_stage++;
            qDebug() << "CFingerprintReader::enrollStageCB(), Step " << QString::number(current_enroll_stage) << " of " << QString::number(enroll_stages);
            break;
        case FP_ENROLL_FAIL:
            tmp = tr("<b>Enrollment failed!</b>");
            break;
        case FP_ENROLL_RETRY:
            tmp = tr("<b>Bad scan. Please try again.</b>");
            break;
        case FP_ENROLL_RETRY_TOO_SHORT:
            tmp = tr("<b>Bad scan: swipe was too short. Please try again.</b>");
            break;
        case FP_ENROLL_RETRY_CENTER_FINGER:
            tmp = tr("<b>Bad scan: finger was not centered on scanner. Please try again.</b>");
            break;
        case FP_ENROLL_RETRY_REMOVE_FINGER:
            tmp = tr("<b>Bad scan: please remove finger before retrying.</b>");
            break;
        default:
            tmp = tr("Unknown state!");
    }

    if (result == FP_ENROLL_COMPLETE || result == FP_ENROLL_FAIL)
    {
        if( FP_ENROLL_COMPLETE )
        {
            unsigned char *buf;
            size_t len;
            //fp_print_data_save(enroll_data, RIGHT_INDEX);

            prependPath = "/home/pi/run/prints/";

            if( !QDir(prependPath).exists() )
                QDir().mkdir(prependPath);

            prependPath += access_code;

            if( !QDir(prependPath).exists() )
                QDir().mkdir(prependPath);

            fullPath = prependPath + "/" + access_code;
            QByteArray ba = fullPath.toLatin1();
            const char *full_path = ba.data();

            FILE *file = fopen(full_path, "wb");
            if( file != NULL )
            {
                len = fp_print_data_get_data(enroll_data, &buf);

                fwrite(buf, len, 1, file);
                fclose(file);
            }
        }
        // cancel enrollment and reset our current stage count
        ((CFingerprintReader *)user_data)->cancelEnrollment();
    }

    /* FIXME show binarized images? */

    // cast-magic because we're in a static function
    emit ((CFingerprintReader *)user_data)->__onFingerprintStageComplete(current_enroll_stage, enroll_stages, tmp);
}

bool CFingerprintReader::initEnrollment(QString sCode)
{
    int result;
    access_code = sCode;
    result = fp_async_enroll_start(fpdev, enrollStageCb, this);
    if (result < 0 )
        return false;
    return true;
}

void CFingerprintReader::enrollCancelCb(struct fp_dev *dev, void *user_data)
{
    Q_UNUSED(dev);
    Q_UNUSED(user_data);
    qDebug() << "CFingerprintReader::enrollCancelCb()";
}

bool CFingerprintReader::cancelEnrollment()
{
    qDebug() << "CFingerprintReader::cancelEnrollment()";

    current_enroll_stage = 1;

    int result = 0;

    result = fp_async_enroll_stop(fpdev, enrollCancelCb, NULL);
    if( result < 0 )
    {
        return false;
    }
    return true;
}

void CFingerprintReader::resetEnrollmentStage()
{
    current_enroll_stage = 1;
}

void CFingerprintReader::verifyCancelCb(struct fp_dev *dev, void *user_data)
{
    Q_UNUSED(dev);
    Q_UNUSED(user_data);
    qDebug() << "CFingerprintReader::verifyCancelCb()";
}

bool CFingerprintReader::cancelVerify()
{
    qDebug() << "CFingerprintReader::cancelVerify()";

    int result = 0;

    result = fp_async_identify_stop(fpdev, verifyCancelCb, NULL);
    if( result < 0 )
    {
        return false;
    }
    return true;
}

void CFingerprintReader::identifyCb(struct fp_dev *dev, int result, size_t match_offset, struct fp_img *img, void *user_data)
{
    qDebug() << "CFingerprintReader::verifyCb()";

    Q_UNUSED(dev);
    Q_UNUSED(img);

    QString tmp = "";
    bool retResult = false;
    bool retryScan = true;

    switch(result) 
    {
        case FP_VERIFY_NO_MATCH:
            tmp = tr("<b>Finger does not match.</b>");
            break;
        case FP_VERIFY_MATCH:
            tmp = tr("<b>Finger matches!</b>");
            retResult = true;
            retryScan = false;
            break;
        case FP_VERIFY_RETRY:
            tmp = tr("<b>Bad scan.</b>");
            break;
        case FP_VERIFY_RETRY_TOO_SHORT:
            tmp = tr("<b>Swipe was too short.</b>");
            break;
        case FP_VERIFY_RETRY_CENTER_FINGER:
            tmp = tr("<b>Finger was not centered on sensor.</b>");
            break;
        case FP_VERIFY_RETRY_REMOVE_FINGER:
            tmp = tr("<b>Bad scan, remove finger.</b>");
            break;
        default:
            tmp = tr("<b>Unknown state! Click 'Close' and try again!</b>");
            break;
    }

    fp_async_identify_stop(fpdev, verifyCancelCb, NULL);
    emit ((CFingerprintReader *)user_data)->__onVerifyFingerprintComplete(retResult, tmp);

    if( retryScan )
    {
        qDebug() << "retying";
        //fp_print_data_load(fpdev, RIGHT_INDEX, &enroll_data);
        result = fp_async_identify_start(fpdev, print_gallery, identifyCb, (CFingerprintReader *)user_data);
    }

    if( retResult )
    {
        const char *access_codes = access_fingerprint_index[match_offset];

        qDebug() << QString::fromStdString(std::string(access_codes));

        std::string codes = std::string(access_codes);

        if( codes.find('.') != std::string::npos )
        {
            access_code = QString::fromStdString(codes.substr(codes.find("/")).substr(codes.find("/")+1,codes.find(".")-1));
            access_code2 = QString::fromStdString(codes.substr(codes.find("/")).substr(codes.find(".")+1));
        }
        else
        {
            access_code = QString::fromStdString(codes.substr(codes.find("/")+1));
            access_code2 = "";
        }

        emit ((CFingerprintReader *)user_data)->__onIdentifiedFingerprint(access_code, access_code2);

        access_code = "";
        access_code2 = "";
    }
}

const char *CFingerprintReader::buildSavePath(QString sCode)
{
    QString prependPath = "/home/pi/run/prints/";

    access_code = sCode;

    prependPath += access_code;

    if( !QDir(prependPath).exists() )
        QDir().mkdir(prependPath);

    QString fullPath = prependPath + "/" + access_code;
    QByteArray ba = fullPath.toLatin1();
    const char *full_path = ba.data();

    return full_path;
}

bool CFingerprintReader::loadEnrollData(QString sCode)
{
    qDebug() << "CFingerprintReader::loadEnrollData(), sCode: " << sCode;

    QByteArray ba = sCode.toLatin1();
    const char *full_path = ba.data();
    unsigned char *buf;

    FILE *file = fopen(full_path, "rb");
    if( file != NULL )
    {
        fseek(file, 0, SEEK_END);
        long fsize = ftell(file);
        fseek(file, 0, SEEK_SET);

        buf = (unsigned char *)malloc(sizeof(unsigned char) *fsize);
        fread(buf, fsize, 1, file);
        enroll_data = fp_print_data_from_data(buf,fsize);

        fclose(file);
    }

    return true;
}

bool CFingerprintReader::initVerify()
{
    qDebug() << "CFingerprintReader::InitVerify()";

    //build list of all enrolldata we have

    QDirIterator it("/home/pi/run/prints/", QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);
    int count = 0;
    QString currentDir;
    QString loadPath;


    for(count=0; count < 5000; count++)
    {
        print_gallery[count] = NULL;
    }

    count = 0;
    while (it.hasNext())
    {
        currentDir = it.next();
        loadPath = currentDir.mid(20,100);
        qDebug() << "PRINT INDEX: " << QString::number(count) << " " << loadPath;
        this->loadEnrollData(currentDir);

        print_gallery[count] = enroll_data;

        QByteArray ba = loadPath.toLatin1();
        const char *load_path = ba.data();
        qDebug() << QString::fromStdString(std::string(load_path));
        access_fingerprint_index[count] = strdup(load_path);
        count++;
    }

    (void)fp_async_identify_start(fpdev, print_gallery, identifyCb, this);

    return true;
}
