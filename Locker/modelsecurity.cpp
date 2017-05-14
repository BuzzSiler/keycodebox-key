#include "modelsecurity.h"
#include <sqlite3.h>
#include <sqlite3ext.h>
#include <unistd.h>
#include "encryption.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QDateTime>

void CModelSecurity::openDatabase()
{
    _DB = QSqlDatabase::addDatabase( "QSQLITE", "SQLITE" );
    // Get the app directory
    QString sPath = "/home/pi/run/Alpha.db";
    qDebug() << "Path:" << sPath;
    _DB.setDatabaseName(sPath);

    if( !_DB.open() )
    {
        qDebug() << _DB.lastError();
        qFatal( "Failed to connect." );
    }
}



CModelSecurity::CModelSecurity(QObject *parent) : QObject(parent)
{
    initialize();

    qDebug( "Connected!" );
}

CModelSecurity::~CModelSecurity()
{
    removeCreatedObjects();
}

void CModelSecurity::OnReadLockSet(int nLockNum, QDateTime start, QDateTime end)
{
    CLockSet    *pLockSet;
    // Build the lock set for this lock num
    _ptblCodes->selectCodeSet(nLockNum, start, end, &pLockSet);

    emit __OnLockSet(pLockSet);
}

void CModelSecurity::OnReadLockHistorySet(int nLockNum, QDateTime start, QDateTime end)
{
    CLockHistorySet *pLockHistorySet;
    // Build the lock history set for this locknum and date range
    _ptblCodeHistory->selectLockCodeHistorySet(nLockNum, start, end, &pLockHistorySet);

    emit __OnLockHistorySet(pLockHistorySet);
}

/**
 * @brief CModelSecurity::OnUpdateCurrentAdmin
 * @param adminInfo
 * Password and AccessCode are required to be clear text here.
 */
void CModelSecurity::OnUpdateCurrentAdmin(CAdminRec *adminInfo)
{
    bool bSuccess = _ptblAdmin->updateAdminClear(adminInfo->getAdminName(), adminInfo->getAdminEmail(), adminInfo->getAdminPhone(),
                                 adminInfo->getEmailReportActive(), adminInfo->getDefaultReportFreq(),
                                 adminInfo->getDefaultReportStart(), adminInfo->getPassword(), adminInfo->getAccessCode(),
                                 adminInfo->getUsePredictiveAccessCode(), adminInfo->getPredictiveKey(), adminInfo->getPredictiveResolution(),
                                 adminInfo->getMaxLocks(),
                                 adminInfo->getSMTPServer(), adminInfo->getSMTPPort(), adminInfo->getSMTPType(),
                                 adminInfo->getSMTPUsername(), adminInfo->getSMTPPassword(),
				 adminInfo->getVNCPort(), adminInfo->getVNCPassword(),
                                 adminInfo->getReportViaEmail(), adminInfo->getReportToFile(), adminInfo->getReportDirectory());
    emit __OnUpdatedCurrentAdmin(bSuccess);
}

void CModelSecurity::OnUpdateCodeState(CLockState *rec)
{
     bool bSuccess = _ptblCodes->updateCode(rec);

     emit __OnUpdatedCodeState(bSuccess);
}

void CModelSecurity::initialize()
{
    _ptblCodes = 0;
    _ptblCodeHistory = 0;
    _ptblAdmin = 0;

    openDatabase();
    setupTimer();
    setupTables();
}

void CModelSecurity::removeCreatedObjects()
{
    if(_ptblCodes) {
        delete _ptblCodes;
    }
    if( _ptblCodeHistory) {
        delete _ptblCodeHistory;
    }
    if( _ptblAdmin) {
        delete _ptblAdmin;
    }
}

void CModelSecurity::OnTimeout()
{
    // Timeout waiting on security code?
    if( _type == "Admin" ) {
        _type = "";
        emit __OnAdminSecurityCheckFailed();
    } else {
        _type = "";
        emit __OnSecurityCheckedFailed();
    }
    emit __OnSecurityCheckTimedOut();
}

void CModelSecurity::setupTimer()
{
    connect(&_timer, SIGNAL(timeout()), this, SLOT(OnTimeout()));
    _timer.setInterval(100000);
    _timer.setSingleShot(true);
}

void CModelSecurity::setupTables()
{
    if(_DB.isOpen())
    {
        _ptblCodes = new CTblCodes(&_DB);
        _ptblCodeHistory = new CTblCodeHistory(&_DB);
        _ptblAdmin = new CTblAdmin(&_DB);
    }
}


void CModelSecurity::CheckPredictiveAccessCode(QString code)
{
    uint32_t nLockNum;
    QDateTime datetime = QDateTime::currentDateTime();
    std::string skey;
    std::string outEncrypt = "";
    QString sTmp;
    QString filename="Data.txt";
    QFile file( filename );
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );

        datetime = CEncryption::roundDateTime(_ptblAdmin->getCurrentAdmin().getPredictiveResolution(), datetime);
        skey = _ptblAdmin->getCurrentAdmin().getPredictiveKey();

        for(nLockNum=1; nLockNum<256; nLockNum++) {
            // Round the current date up to the resolution specified
            sTmp = "Predictive Matching:" + QVariant(nLockNum).toString();
            CEncryption::calculatePredictiveCodeOld(nLockNum, skey, datetime, &outEncrypt, 5 /*max results length*/, &sTmp);

            stream << sTmp << endl;

            if( outEncrypt == code.toStdString()) {
                stream << "MATCH: Predict for Lock# " << QVariant(nLockNum).toString() << " calculated:" << outEncrypt.c_str() << " code to match:" << code << endl;

		// ----let's keep this here in case things go horribly wrong
		// emit __OnSecurityCheckSuccess(nLockNum, false);
                // emit __OnCreateHistoryRecordFromLastPredictiveLogin(nLockNum, code, false);

		OnVerifyCodeOne(code);
                return;
            }
        }
        stream << "NO MATCH: Predict calculated: " << outEncrypt.c_str() << " code to match:" << code << endl;
    }

    emit __OnSecurityCheckedFailed();
}


/**
 * @brief CModelSecurity::OnVerifyCodeOne
 * @param code - encrypted
 */
void CModelSecurity::OnVerifyCodeOne(QString code)
{
    // Check the DB
    _type = "";  // "User" or "Admin" or ""

    qDebug() << "CModelSecurity::OnVerifyCodeOne. Code:" << code;

//    _timer.moveToThread(this->thread());
//    _timer.start();
    int nDoorNum = -1;
        if( _ptblAdmin->isAccessCode(code) ) {
            _type = "Admin";
            emit __OnRequireAdminPassword();
        } else {
                //QString sCodeEnc = QString(CEncryption::encryptDecrypt(code.size(), code.toStdString()).c_str());
                QString sCodeEnc = CEncryption::encryptString(code);

                qDebug() << "CModelSecurity::OnVerifyCode() Check Code One:" << sCodeEnc;
                nDoorNum = _ptblCodes->checkCodeOneElms(sCodeEnc.toStdString(),nDoorNum);

                qDebug() << "CModelSecurity::OnVerifyCode() nDoorNum:" << nDoorNum;
		
                if( nDoorNum > 0 )
                {
		  
		    emit __OnSecurityCheckSuccess(nDoorNum, false);
                    _type = "User";
		    OnCreateHistoryRecordFromLastSuccessfulLogin(nDoorNum, false);

		    // elms requires us to delete the code after successful authentication
		    _ptblCodes->deleteCode(QString::number(nDoorNum), sCodeEnc, "", QDateTime(QDate(1990,1,1), QTime(0,0,0)), QDateTime(QDate(1990,1,1), QTime(0,0,0))); 
		}
		else
		  {
		    // end-of-commented-out-logic

		    // grab the capacity of the elms system
		    // hardcode to 16 for now
		    int capacity = 16;
		    QString strCapacity = "16";
		    QStringList contents;
		    QFile file("/home/pi/run/capacity.elms");
		    if( file.open(QFile::ReadOnly | QFile::Text) )
		      {
			QTextStream in(&file);
			contents = in.readAll().split('\n');
			strCapacity = contents.value(0);
			capacity = strCapacity.toInt();
		      }
		    else
		      std::system("echo '16' > /home/pi/run/capacity.elms");
		    
		    qDebug() << "CModelSecurity::OnVerifyCode(), strCapacity: " << strCapacity;
		    
		    // need to add a new code (if we have available locks on the device)
		    int *fullLockers;
		    int c;
		    bool createdCode = false;
		    fullLockers = _ptblCodes->getFullLockersElms(capacity);
		    for(c=0; c<capacity; c++)
		      {
			qDebug() << "CModelSecurity::OnVerifyCode(), c, fullLockers[c]: " << c << ", " << fullLockers[c];
			if(!fullLockers[c])
			  {
			    QString code1 = "";
			    _ptblCodes->addLockCode(c + 1, code.toStdString(), code1.toStdString(), QDateTime(QDate(1990,1,1), QTime(0,0,0)), QDateTime(QDate(1990,1,1), QTime(0,0,0)), false, false, false, "", "", "", "", "", "", 0, 0, 1);
			    createdCode = true;
			    nDoorNum = c + 1;
			    break;
			  }
		      }
		    free(fullLockers);
		    
		    if( createdCode )
		      emit __OnSecurityCheckSuccess(nDoorNum, true);
		    else
		      emit __OnSecurityCheckedFailed();
		  }
	}
}

/**
 * @brief CModelSecurity::OnVerifyCodeOne
 * @param code - encrypted
 */
void CModelSecurity::OnVerifyFingerprintCodeOne(QString code)
{
    // Check the DB
    _type = "";  // "User" or "Admin" or ""

    qDebug() << "CModelSecurity::OnVerifyFingerprintCodeOne. Code:" << code;

//    _timer.moveToThread(this->thread());
//    _timer.start();
    {
        if( _ptblAdmin->isAccessCode(code) ) {
            _type = "Admin";
            emit __OnRequireAdminPassword();
        } else {
            bool bSecondCodeRequired;
	    bool bFingerprintRequired = false;
            int nDoorNum;

            if( _ptblAdmin->getCurrentAdmin().getUsePredictiveAccessCode() )
            {
                //        qDebug() << "Use Predictive";
//                QString sCodeEnc = QString(CEncryption::encryptDecrypt(code.size(), code.toStdString()).c_str());
                qDebug() << "CModelSecurity::OnVerifyCode() Predictive. Code:" << code;
                CheckPredictiveAccessCode(code);

            } else if( 0 /*TBD - _ptblAdmin->getCurrentAdmin().getUseAnyAccessCode() */ )
            {
                // Store AnyAccessCode = code
                // Enter Secondary code - Verify against DB (second access code?)
            }
            else {
                //QString sCodeEnc = QString(CEncryption::encryptDecrypt(code.size(), code.toStdString()).c_str());
                QString sCodeEnc = CEncryption::encryptString(code);

//                qDebug() << "CModelSecurity::OnVerifyCode() Check Code One:" << sCodeEnc;
                nDoorNum = _ptblCodes->checkCodeOne(sCodeEnc.toStdString(), bSecondCodeRequired, bFingerprintRequired, nDoorNum);
                if( nDoorNum > 0 )
                {
                    _type = "User";

		    // need to check if fingerprint security is enabled
		    // if so, check for existence of existing fingerprints
		    // if fingerprints do not exist, open up enroll dialog
		    // (otherwise let user know fingerprints are enrolled?
		    //  or emit __OnSecurityCheckFailed()

		    // this is probably a good time to mention,
		    // fingerprints for a second code are stored like:
		    //   /home/pi/run/prints/<codeOne>/<codeOne>.<codeTwo>
		    // single code are of course stored like:
		    //   /home/pi/run/prints/<codeOne>/<codeOne>
		    
		    // need to do check again for code 2
		    
                    if( bSecondCodeRequired ) {
                        emit __OnRequireCodeTwo(); 
                    } else {
		      emit __OnSecurityCheckSuccess(nDoorNum,true);
                    }
                } else {
                    emit __OnSecurityCheckedFailed();
                }
            }
        }
    }
}

void CModelSecurity::OnVerifyCodeTwo(QString code)
{
//    _timer.stop();
  bool bFingerprintRequired = false;
  bool bQuestionsRequired = false;
  std::string codeOne;
  bool bAskQuestions = false;
  QString question1 = "";
  QString question2 = "";
  QString question3 = "";

  _updateCodeLockboxState = false;
    if( _type == "Admin" ) {
        if( _ptblAdmin->isPassword(code) ) {
            emit __OnAdminSecurityCheckOk();
        } else {
            emit __OnAdminSecurityCheckFailed();
        }
    } else if( _type == "User" ) {
        int nDoorNum = -1;
        if( _ptblCodes->checkCodeTwo(code.toStdString(), bFingerprintRequired, bQuestionsRequired, codeOne, nDoorNum, bAskQuestions, question1, question2, question3) > 0 )
        {
	  //we need to check if a fingerprint directory already exists,
	  // if they do, do not attempt enrollmesnt
	  if( bFingerprintRequired == true)
	    {
	      // this is probably a good time to mention,
	      // fingerprints for a second code are stored like:
	      //   /home/pi/run/prints/<codeOne>/<codeOne>.<codeTwo>
	      // single code are of course stored like:
	      //   /home/pi/run/prints/<codeOne>/<codeOne>
	      
	      if( !QDir(QString::fromStdString("/home/pi/run/prints/" + codeOne + "." + code.toStdString())).exists() )
		{
		  emit __EnrollFingerprintDialog(QString::fromStdString(codeOne + "." + code.toStdString()));
		  emit __EnrollFingerprint(QString::fromStdString(codeOne + "." + code.toStdString()));
		}
	      else
		emit __OnSecurityCheckedFailed();
	      
	      return;
	    }
	  
	  if( bQuestionsRequired == true)
	    {
	      qDebug() << "ASK_QUESTIONS: " << QString::number(bAskQuestions);
	      qDebug() << "QUESTION1: " << question1;
	      qDebug() << "QUESTION2: " << question2;
	      qDebug() << "QUESTION3: " << question3;

	      emit __QuestionUserDialog(nDoorNum,question1,question2,question3);
	    }
	  else
	    emit __OnSecurityCheckSuccess(nDoorNum,false);
	  
        } else {
            emit __OnSecurityCheckedFailed();
        }
    } else {
        // Might have timed out and cleared the _type
    }
}

void CModelSecurity::OnSuccessfulQuestionUsersAnswers(int doorNum, QString answer1, QString answer2, QString answer3)
{
  qDebug() << "CModelSecurity::OnSuccessfulQuestionUsersAnswers()";
  emit __OnSecurityCheckSuccessWithAnswers(doorNum, answer1, answer2, answer3);
}

void CModelSecurity::OnQuestionUserCancelled()
{
  qDebug() << "CModelSecurity::OnQuestionUserCancelled()";
  //emit __OnSecurityCheckedFailed();
  _ptblCodes->updateLockboxState(_ptblCodes->_lastIDS, true);
}

void CModelSecurity::OnVerifyFingerprintCodeTwo(QString code)
{
//    _timer.stop();
  bool bFingerprintRequired = false;
  bool bQuestionsRequired = false;
  std::string codeOne;
  bool bAskQuestions = false;
  QString question1 = "";
  QString question2 = "";
  QString question3 = "";
  
    if( _type == "Admin" ) {
        if( _ptblAdmin->isPassword(code) ) {
            emit __OnAdminSecurityCheckOk();
        } else {
            emit __OnAdminSecurityCheckFailed();
        }
    } else if( _type == "User" ) {
        int nDoorNum = -1;
        if( _ptblCodes->checkCodeTwo(code.toStdString(), bFingerprintRequired, bQuestionsRequired, codeOne, nDoorNum, bAskQuestions, question1, question2, question3) > 0 )
        {
	  emit __OnSecurityCheckSuccess(nDoorNum,false);
        } else {
            emit __OnSecurityCheckedFailed();
        }
    } else {
        // Might have timed out and cleared the _type
    }
}

void CModelSecurity::OnCreateHistoryRecordFromLastPredictiveLogin(int nLockNum, QString code, bool lockboxState)
{
    qDebug() << " ----------------------OnCreateHistoryRecordFromLastSuccessfulLogin()";

    CLockHistoryRec lockHistoryRec;
    CLockState  lockState;

    // Build the lock state for this lock num
    lockState.setLockNum(nLockNum);
    lockState.setCode1(code.toStdString());
    lockHistoryRec.setFromLockState(lockState);
    _ptblCodeHistory->addLockCodeHistory(lockHistoryRec, lockboxState);
}

void CModelSecurity::OnCreateHistoryRecordFromLastSuccessfulLogin(int doorNum, bool lockboxState)
{
    qDebug() << " ----------------------OnCreateHistoryRecordFromLastSuccessfulLogin(int)";
    
    int         nVal;
    CLockSet    *pLockSet;
    // Build the lock set for this lock num
    _ptblCodes->selectCodeSet(doorNum, &pLockSet);
    
    CLockHistoryRec lockHistoryRec;
    CLockState      *pState;
    nVal = 0;
    for(CLockSet::Iterator itor = pLockSet->begin(); itor != pLockSet->end(); itor++) {
      nVal++;
      pState = itor.value();
      lockHistoryRec.setFromLockState(*pState);
      _ptblCodeHistory->addLockCodeHistory(lockHistoryRec, lockboxState);
    }
    if(nVal > 1) {
      qDebug() << "Error: CModelSecurity::OnCreateHistoryRecordFromLastSuccessfulLogin() has more than 1 record";
    }
}

void CModelSecurity::OnCreateHistoryRecordFromLastSuccessfulLoginWithAnswers(QString answer1, QString answer2, QString answer3)
{
    qDebug() << " ----------------------OnCreateHistoryRecordFromLastSuccessfulLoginWithAnswers(QString, QString, QString)";
    int ids = _ptblCodes->getLastSuccessfulIDS();
    if(ids != -1)
    {
        int         nVal;
        CLockSet    *pLockSet;
        // Build the lock set for this lock num
        _ptblCodes->selectCodeSet(ids, &pLockSet);

        CLockHistoryRec lockHistoryRec;
        CLockState      *pState;
        nVal = 0;
        for(CLockSet::Iterator itor = pLockSet->begin(); itor != pLockSet->end(); itor++) {
            nVal++;
            pState = itor.value();
            lockHistoryRec.setFromLockState(*pState);
            _ptblCodeHistory->addLockCodeHistoryWithAnswers(lockHistoryRec, answer1, answer2, answer3);
        }
        if(nVal > 1) {
            qDebug() << "Error: CModelSecurity::OnCreateHistoryRecordFromLastSuccessfulLoginWithAnswers() has more than 1 record";
        }
    }
}

void CModelSecurity::RequestLastSuccessfulLogin()
{
    qDebug() << " ----------------------RequestLastSuccessfulLogin()";
    if( !_ptblAdmin->getCurrentAdmin().getUsePredictiveAccessCode() )
    {
        int ids = _ptblCodes->getLastSuccessfulIDS();
        if(ids != -1){
            qDebug() << "--------lastSuccessfulIDS ids != -1";
            int         nVal;
            CLockSet    *pLockSet;
            // Build the lock set for this lock num
            _ptblCodes->selectCodeSet(ids, &pLockSet);

            CLockHistoryRec *plockHistoryRec;
            CLockState      *pState;
            nVal = 0;
            for(CLockSet::Iterator itor = pLockSet->begin(); itor != pLockSet->end(); itor++) {
                nVal++;
                pState = itor.value();
                plockHistoryRec = new CLockHistoryRec();
                plockHistoryRec->setFromLockState(*pState);
                //
                emit __OnLastSuccessfulLogin(plockHistoryRec);
            }
            if(nVal > 1) {
                qDebug() << "Error: CModelSecurity::RequestLastSuccessfulLogin() has more than 1 record";
            }
        } else {
            qDebug() << "ids == -1)";
        }
    }
    else {
        qDebug() << "RequestLastSuccessfulLogin( Predictive )";
        CLockHistorySet *_pHistorySet = NULL;

        QDateTime   time, now;
        time = time.currentDateTime().addSecs(-5);  // 5 Seconds ago
        now = now.currentDateTime();
        int nLockNum = -1;

        qDebug() << ">>SENDING >>> Start:" << time.toString("yyyy-MM-dd HH:mm:ss") << "  End:" << now.toString("yyyy-MM-dd HH:mm:ss");

        _ptblCodeHistory->selectLastLockCodeHistorySet(nLockNum, time, now, &_pHistorySet);

        // Predictive - so history rec
        CLockHistoryRec *plockHistoryRec;

        for(CLockHistorySet::Iterator itor = _pHistorySet->begin(); itor != _pHistorySet->end(); itor++) {
            plockHistoryRec = itor.value();
            //
            qDebug() << "  Found last successful login";
            emit __OnLastSuccessfulLogin(plockHistoryRec);
        }
    }
}

void CModelSecurity::OnRequestCodeHistoryForDateRange(QDateTime dtStart, QDateTime dtEnd)
{
    CLockHistorySet     *pLockHistorySet = new CLockHistorySet();
    // Get the data
    int nLockNum = -1;
    _ptblCodeHistory->selectLockCodeHistorySet(nLockNum, dtStart, dtEnd, &pLockHistorySet);
    // Send it back
    emit __OnCodeHistoryForDateRange(dtStart, dtEnd, pLockHistorySet);
}

void CModelSecurity::OnVerifyAdminPassword(QString code)
{
//    _timer.stop();

    if( _type == "Admin" ) {
        if( _ptblAdmin->isPassword(code) ) {
            emit __OnAdminSecurityCheckOk();
        } else {
            emit __OnAdminSecurityCheckFailed();
        }
    }
    else {
        // Might have timed out and cleared the _type
    }
}

void CModelSecurity::OnRequestCurrentAdmin()
{
    qDebug() << "CModelSecurity::OnRequestCurrentAdmin() : retrieved current admin -> emit __OnRequestedCurrentAdmin(CAdminRec*)";

    emit __OnRequestedCurrentAdmin(&_ptblAdmin->getCurrentAdmin());
}
