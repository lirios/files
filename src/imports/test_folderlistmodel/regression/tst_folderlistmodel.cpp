#include "filesystemaction.h"
#include "dirmodel.h"
#include "tempfiles.h"
#include "externalfswatcher.h"
#include "dirselection.h"
#include "qtrashdir.h"
#include "location.h"
#include "locationurl.h"
#include "locationsfactory.h"
#include "disklocation.h"
#include "qtrashutilinfo.h"

#if defined(Q_OS_UNIX)
#include <stdio.h>
#include <sys/types.h>
#include <utime.h>
#include <sys/time.h>
#endif

#ifndef DO_NOT_USE_TAG_LIB
#include <taglib/attachedpictureframe.h>
#include <taglib/id3v2tag.h>
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>
#include <taglib/audioproperties.h>
#endif

#include <QApplication>
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QFileInfo>
#include <QDirIterator>
#include <QIcon>
#include <QPixmap>
#include <QFileIconProvider>
#include <QStandardPaths>

#include <QMimeType>
#include <QMimeDatabase>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QFile>
#include <QTemporaryDir>
#include <QTemporaryFile>

//files to generate
#include "testonly_pdf.h"
#include "sound_mp3.h"
#include "media_asx.h"
#include "media_xspf.h"

#define TIME_TO_PROCESS       2300
#define TIME_TO_REFRESH_DIR   90

#if QT_VERSION  >= 0x050000
#define  QSKIP_ALL_TESTS(statement)   QSKIP(statement)
#else
#define  QSKIP_ALL_TESTS(statement)   QSKIP(statement,SkipAll)
#endif


QByteArray md5FromIcon(const QIcon& icon);
QString createFileInTempDir(const QString& name, const char *content, qint64 size);

class TestDirModel : public QObject
{
   Q_OBJECT

public:
       TestDirModel();
      ~TestDirModel();

protected slots:
    void slotFileAdded(const QString& s)     {m_filesAdded.append(s); }
    void slotFileRemoved(const QString& s)   {m_filesRemoved.append(s); }
    void slotFileAdded(const DirItemInfo& f)   {m_filesAdded.append(f.absoluteFilePath()); }
    void slotFileRemoved(const DirItemInfo& f) {m_filesRemoved.append(f.absoluteFilePath()); }
    void slotPathChamged(QString path)       { m_currentPath = path;}
    void progress(int, int, int);
    void slotRemoveFileWhenProgressArrive(int,int,int);
    void cancel(int index, int, int percent);
    void slotclipboardChanged();
    void slotError(QString title, QString message);
    void slotExtFsWatcherPathModified(const QString&)     { ++m_extFSWatcherPathModifiedCounter; }
    void slotSelectionChanged(int counter)  { m_selectedItemsCounter = counter; }
    void slotSelectionModeChanged(int m)    { m_selectionMode = m;}

private Q_SLOTS:
    void initTestCase();       //before all tests
    void cleanupTestCase();    //after all tests
    void init();               //before every test
    void cleanup();            //after every test

private Q_SLOTS: // test cases
    void  fsActionRemoveSingleFile();
    void  fsActionRemoveSingleDir();
    void  fsActionRemoveOneFileOneDir();
    void  fsActionRemoveTwoFilesTwoDirs();
    void  modelRemoveRecursiveDirByIndex();
    void  modelRemoveMultiItemsByFullPathname();
    void  modelRemoveMultiItemsByName();
    void  modelCopyDirPasteIntoAnotherModel();
    void  modelCopyManyItemsPasteIntoAnotherModel();
    void  modelCopyTwoEmptyFiles();
    void  modelCopyFileAndRemoveBeforePaste();
    void  modelCopyPasteFileAndRemoveWhenFirstProgressSignalArrives();
    void  modelCutManyItemsPasteIntoAnotherModel();
    void  fsActionMoveItemsForcingCopyAndThenRemove();
    void  modelCancelRemoveAction();
    void  modelCancelRemoveSelection();
    void  modelTestFileSize();
    void  modelRemoveDirWithHiddenFilesAndLinks();
    void  modelCancelCopyAction();
    void  modelCopyPasteAndPasteAgain();
    void  modelCutPasteIntoExistentItems();
    void  modelCopyFileAndDirectoryLinks();
    void  modelCopyAndPaste3Times();
    void  modelCutAndPaste3Times();
    void  modelCutAndPasteInTheSamePlace();
    void  modelCopyAndPasteToBackupFiles();
    void  fileIconProvider();
    void  getThemeIcons();
#ifndef DO_NOT_USE_TAG_LIB
    void  verifyMP3Metadata();
#endif
    void  openPathAbsouluteAndRelative();
    void  existsDirAnCanReadDir();
    void  existsFileAndCanReadFile();
    void  pathProperties();
    void  watchExternalChanges();
    void  extFsWatcherChangePathManyTimesModifyAllPathsLessLast();             // no notification
    void  extFsWatcherModifySamePathManyTimesWithInInterval();  // just one notification
    void  extFsWatcherSetPathAndModifyManyTimesWithInInterval();// just one notification
    void  extFsWatcherChangePathManyTimesModifyManyTimes();     // many notifications
#if defined(Q_OS_UNIX)
    void  extFsWatcherNoticeChangesWithSameTimestamp();
#endif

    //define TEST_OPENFILES to test QDesktopServices::openUrl() for some files
#if defined(TEST_OPENFILES)
    void  openMP3();
    void  openTXT();
    void  openPDF();
#endif

    void modelSingleSelection();
    void modelMultiSelection();   
    void modelSelectionItemsRange();

    void trashDiretories();

    void locationFactory();
    void moveOneFileToTrashAndRestore();
    void restoreTrashWithMultipleSources();
    void emptyTrash();

private:
    bool createTempHomeTrashDir(const QString& existentDir);
    void initDeepDirs();
    void cleanDeepDirs();
    void initModels();
    void cleanModels();
    bool compareDirectories(const QString& d1,
                            const QString& d2);

    bool createLink(const QString& fullSouce,
                    const QString& link,
                    bool  fullLink = false);

    bool createFileAndCheckIfIconIsExclisive(const QString& termination,
                                             const unsigned char *content,
                                             qint64 len);

private:
    FileSystemAction  fsAction;

    QStringList m_filesAdded;
    QStringList m_filesRemoved;

    DeepDir   *    m_deepDir_01;
    DeepDir   *    m_deepDir_02;
    DeepDir   *    m_deepDir_03;

    DirModel  *    m_dirModel_01;
    DirModel  *    m_dirModel_02;

    int            m_progressCounter;
    int            m_progressTotalItems;
    int            m_progressCurrentItem;
    int            m_progressPercentDone;
    bool           m_receivedClipboardChangesSignal;
    bool           m_receivedErrorSignal;
    int            m_progressNotificationsCounter;
    bool           m_visibleProgressMessages;
    QHash<QByteArray, QString>  m_md5IconsTable;
    QFileIconProvider           m_provider;
    QString        m_currentPath;
    QString        m_fileToRemoveInProgressSignal;
    int            m_extFSWatcherPathModifiedCounter;
    int            m_selectedItemsCounter;
    int            m_selectionMode;
    int            m_minimumProgressToCancelAction;

};

TestDirModel::TestDirModel() : m_deepDir_01(0)
                                    ,m_deepDir_02(0)
                                    ,m_deepDir_03(0)
                                    ,m_dirModel_01(0)
                                    ,m_dirModel_02(0)
{
    connect(&fsAction, SIGNAL(added(QString)),
            this,      SLOT(slotFileAdded(QString)) );
    connect(&fsAction, SIGNAL(removed(QString)),
            this,      SLOT(slotFileRemoved(QString)) );

    connect(&fsAction, SIGNAL(added(DirItemInfo)),
            this,      SLOT(slotFileAdded(DirItemInfo)));
    connect(&fsAction, SIGNAL(removed(DirItemInfo)),
            this,      SLOT(slotFileRemoved(DirItemInfo)));

    connect(&fsAction, SIGNAL(progress(int,int,int)),
            this,      SLOT(progress(int,int,int)));
}

void TestDirModel::progress(int cur, int total, int percent)
{
    m_progressCounter++;
    m_progressCurrentItem = cur;
    m_progressTotalItems  = total;
    m_progressPercentDone = percent;
    if (m_visibleProgressMessages)
    {
        QString p;
        p.sprintf("progress(cur=%d, total=%d, percent=%d)", cur,total,percent);
        qDebug() << p;
    }
}

void TestDirModel::slotRemoveFileWhenProgressArrive(int cur, int total, int percent)
{
    qDebug() << "removing file" << m_fileToRemoveInProgressSignal
                << "ret" << QFile::remove(m_fileToRemoveInProgressSignal);
    progress(cur,total,percent);
}

bool TestDirModel::createLink(const QString &fullSouce, const QString &link, bool fullLink)
{
    bool ret = false;

    QFileInfo source(fullSouce);
    if (source.exists())
    {
        if (fullLink)
        {
            QFileInfo lnk(link);
            ret = QFile::link(source.absoluteFilePath(), lnk.absoluteFilePath());
        }
        else
        {
            QString curDir = QDir::currentPath();
            if (QDir::setCurrent(source.absolutePath()))
            {
                QFileInfo lnk(link);
                if (source.absolutePath() != lnk.absolutePath())
                {
                    if (lnk.isAbsolute())
                    {
                        ret = QFile::link(source.absoluteFilePath(), link);
                    }
                    else
                    {
                        QDir relative(lnk.absolutePath());
                        if (relative.exists() || relative.mkpath(lnk.absolutePath()))
                        {
                           int diff=0;
                           QStringList sourceDirs = source.absolutePath().
                                                    split(QDir::separator(), QString::SkipEmptyParts);
                           QStringList targetDirs = lnk.absolutePath().
                                                    split(QDir::separator(), QString::SkipEmptyParts);
                           while (diff < sourceDirs.count())
                           {
                               if (sourceDirs.at(diff) !=
                                       targetDirs.at(diff))
                               {
                                   break;
                               }
                               diff++;
                           }
                           QString relativePath = sourceDirs.at(diff);
                           QString gap(QLatin1String("..") + QDir::separator());
                           while (diff++ < targetDirs.count())
                           {
                               relativePath.prepend(gap);
                           }
                           ret = QFile::link(relativePath
                                                 + QDir::separator()
                                                 + source.fileName(),
                                             link);
                        }
                    }
                }
                else
                {
                    ret = QFile::link(source.fileName(), link);
                }
                QDir::setCurrent(curDir);
            }
        }
    }
    return ret;
}

bool TestDirModel::compareDirectories(const QString &d1, const QString &d2)
{
    QDirIterator d1Info(d1,
                    QDir::Files | QDir::Hidden | QDir::System,
                    QDirIterator::Subdirectories);

    int len = d1.length();

    while (d1Info.hasNext() &&  !d1Info.next().isEmpty())
    {
        QString target(d2  + d1Info.fileInfo().absoluteFilePath().mid(len));

        QFileInfo d2Info(target);
        if (d1Info.fileName() != d2Info.fileName())
        {
            qDebug() << "false name" << d1Info.fileName() << d2Info.fileName();
            return false;
        }
        if (d1Info.fileInfo().size() != d2Info.size())
        {
            qDebug() << "false size" << d1Info.fileName() << d1Info.fileInfo().size()
                                    << d2Info.fileName() << d2Info.size();
            return false;
        }
        if (d1Info.fileInfo().permissions() != d2Info.permissions())
        {
            qDebug() << "false permissions" << d1Info.fileName() << d2Info.fileName();
            return false;
        }       
    }

    return true;
}

void TestDirModel::cancel(int index, int, int percent)
{   
    Q_UNUSED(index);
    if (percent > m_minimumProgressToCancelAction)
    {
        DirModel * model = static_cast<DirModel*> (sender());
        model->cancelAction();
    }
}

void TestDirModel::slotclipboardChanged()
{
     m_receivedClipboardChangesSignal     = true;
}

void TestDirModel::slotError(QString title, QString message)
{
    qWarning("Received Error: [title: %s] [message: %s]", qPrintable(title), qPrintable(message));
    m_receivedErrorSignal = true;
}

TestDirModel::~TestDirModel()
{

}

void TestDirModel::initDeepDirs()
{
    cleanDeepDirs();
}


void TestDirModel::cleanDeepDirs()
{
    if (m_deepDir_01) delete m_deepDir_01;
    if (m_deepDir_02) delete m_deepDir_02;
    if (m_deepDir_03) delete m_deepDir_03;
    m_deepDir_01 = 0;
    m_deepDir_02 = 0;
    m_deepDir_03 = 0;
}


void TestDirModel::initModels()
{
    cleanModels();
    m_dirModel_01 = new DirModel();
    m_dirModel_02 = new DirModel();

    connect(m_dirModel_01->m_fsAction, SIGNAL(added(QString)),
            this,      SLOT(slotFileAdded(QString)) );
    connect(m_dirModel_01->m_fsAction, SIGNAL(removed(QString)),
            this,      SLOT(slotFileRemoved(QString)) );
    connect(m_dirModel_01->m_fsAction, SIGNAL(added(DirItemInfo)),
            this,      SLOT(slotFileAdded(DirItemInfo)));
    connect(m_dirModel_01->m_fsAction, SIGNAL(removed(DirItemInfo)),
            this,      SLOT(slotFileRemoved(DirItemInfo)));

    connect(m_dirModel_02->m_fsAction, SIGNAL(added(QString)),
            this,      SLOT(slotFileAdded(QString)) );
    connect(m_dirModel_02->m_fsAction, SIGNAL(removed(QString)),
            this,      SLOT(slotFileRemoved(QString)) );
    connect(m_dirModel_02->m_fsAction, SIGNAL(added(DirItemInfo)),
            this,      SLOT(slotFileAdded(DirItemInfo)));
    connect(m_dirModel_02->m_fsAction, SIGNAL(removed(DirItemInfo)),
            this,      SLOT(slotFileRemoved(DirItemInfo)));

    m_dirModel_01->setEnabledExternalFSWatcher(true);
    m_dirModel_02->setEnabledExternalFSWatcher(true);
}


void TestDirModel::cleanModels()
{
    if (m_dirModel_01) delete m_dirModel_01;
    if (m_dirModel_02) delete m_dirModel_02;
    m_dirModel_01 = 0;
    m_dirModel_02 = 0;
}

void TestDirModel::initTestCase()
{
    DirModel::registerMetaTypes();
}


void TestDirModel::cleanupTestCase()
{
    cleanDeepDirs();
    cleanModels();
}


void TestDirModel::init()
{
   m_filesAdded.clear();
   m_filesRemoved.clear();
   initDeepDirs();
   initModels();
   m_progressCounter = 0;
   m_progressTotalItems = 0;
   m_progressCurrentItem = 0;
   m_progressPercentDone = 0;
   m_receivedClipboardChangesSignal     = false;
   m_receivedErrorSignal = false;
   m_progressNotificationsCounter = 0;
   m_visibleProgressMessages      = false;
   m_extFSWatcherPathModifiedCounter = 0;
   m_selectedItemsCounter   = 0;
   m_selectionMode          = -1;
   m_minimumProgressToCancelAction = 1;
}



void TestDirModel::cleanup()
{
    cleanDeepDirs();
    cleanModels();
    m_filesAdded.clear();
    m_filesRemoved.clear();
    m_progressCounter = 0;
    m_progressTotalItems = 0;
    m_progressCurrentItem = 0;
    m_progressPercentDone = 0;
    m_receivedClipboardChangesSignal     = false;
    m_receivedErrorSignal                = false;
    m_fileToRemoveInProgressSignal.clear();
    m_progressNotificationsCounter      = 0;
    m_visibleProgressMessages           = false;
    m_extFSWatcherPathModifiedCounter   = 0;
    m_selectedItemsCounter = 0;
    m_selectionMode          = -1;
    m_minimumProgressToCancelAction = 1;
}


void TestDirModel::fsActionRemoveSingleFile()
{
    TempFiles file;
    QCOMPARE(file.create("fsAtion_removeSingleFile") , true);

    fsAction.remove(file.createdList());
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(m_filesRemoved.count() , 1);

    QCOMPARE(m_filesRemoved.at(0), file.lastFileCreated());
    QFileInfo now(file.lastFileCreated());
    QCOMPARE(now.exists(),  false);
}



void TestDirModel::fsActionRemoveSingleDir()
{
    const int level = 10;
    m_deepDir_01 = new DeepDir("removeSingleDir", level);
    QCOMPARE( QFileInfo(m_deepDir_01->path()).exists(),  true);

    QStringList myDeepDir(m_deepDir_01->path());
    fsAction.remove(myDeepDir);
    QTest::qWait(TIME_TO_PROCESS);

    QCOMPARE(m_filesRemoved.count() , 1);
    QCOMPARE( QFileInfo(m_deepDir_01->path()).exists(),  false);
    QVERIFY(m_progressCounter > 2);
}


void TestDirModel::fsActionRemoveOneFileOneDir()
{
    const int level = 10;
    m_deepDir_01 = new DeepDir("removeSingleDir", level);
    QCOMPARE( QFileInfo(m_deepDir_01->path()).exists(),  true);

    TempFiles file;
    QCOMPARE(file.create("fsActionRemoveOneFileOneDir") , true);

    QStringList twoItems(m_deepDir_01->path());
    twoItems.append(file.lastFileCreated());
    fsAction.remove(twoItems);
    QTest::qWait(TIME_TO_PROCESS);

    QCOMPARE(m_filesRemoved.count() , 2);
    QCOMPARE( QFileInfo(m_deepDir_01->path()).exists(),  false);
    QCOMPARE(file.howManyExist(),  0);
    QVERIFY(m_progressCounter > 2);
}



void TestDirModel::fsActionRemoveTwoFilesTwoDirs()
{
    const int level = 12;
    m_deepDir_01 = new DeepDir("fsActionRemoveTwoFilesTwoDirs_01", level);
    QCOMPARE( QFileInfo(m_deepDir_01->path()).exists(),  true);

    m_deepDir_02 = new DeepDir("fsActionRemoveTwoFilesTwoDirs_02", level);
    QCOMPARE( QFileInfo(m_deepDir_02->path()).exists(),  true);

    TempFiles twoFiles;
    QCOMPARE(twoFiles.create(2) , true);

    QStringList fourItems(m_deepDir_01->path());
    fourItems.append(m_deepDir_02->path());
    fourItems.append(twoFiles.createdList());

    fsAction.remove(fourItems);
    QTest::qWait(TIME_TO_PROCESS);

    QCOMPARE(m_filesRemoved.count() , 4);
    QCOMPARE(QFileInfo(m_deepDir_01->path()).exists(),  false);
    QCOMPARE(QFileInfo(m_deepDir_02->path()).exists(),  false);
    QCOMPARE(twoFiles.howManyExist(),  0);
    QVERIFY(m_progressCounter > 2);
}

void TestDirModel::modelRemoveRecursiveDirByIndex()
{
    const int level = 5;
    m_deepDir_01 = new DeepDir("modelRemoveRecursiveDirByIndex", level);
    QCOMPARE( QFileInfo(m_deepDir_01->path()).exists(),  true);

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);

    QCOMPARE(m_dirModel_01->rowCount(), 1);
    m_dirModel_01->removeIndex(0);
    QTest::qWait(500);

    QCOMPARE(m_filesRemoved.count() , 1);
    QCOMPARE(m_dirModel_01->rowCount(), 0);
}

void TestDirModel::modelRemoveMultiItemsByFullPathname()
{
    QString tmpDir("modelRemoveMultiItemsByFullPathame");
    m_deepDir_01 = new DeepDir(tmpDir, 0);
    QCOMPARE( QFileInfo(m_deepDir_01->path()).exists(),  true);

    const int filesToCreate = 2;
    const int itemsToCreate = filesToCreate + 1;

    TempFiles files;
    files.addSubDirLevel(tmpDir);
    files.create(filesToCreate);

    QStringList items (files.createdList());

    files.addSubDirLevel("subDir"); // + 1 item
    files.create(1);
    items.append(files.lastPath());

    connect(m_dirModel_01, SIGNAL(progress(int,int,int)),
            this,          SLOT(progress(int,int,int)));
    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(), itemsToCreate);

    m_dirModel_01->rm(items);
    int steps = m_dirModel_01->getProgressCounter();
    QTest::qWait(500);

    QCOMPARE(m_filesRemoved.count() , itemsToCreate);
    QCOMPARE(m_dirModel_01->rowCount(), 0);
    QCOMPARE(files.howManyExist(), 0);
    QCOMPARE(steps,        m_progressCounter);
}


void TestDirModel::modelRemoveMultiItemsByName()
{
     QString tmpDir("modelRemoveMultiItemsByName");
     const int filesToCreate = 4;

     TempFiles files;
     files.addSubDirLevel(tmpDir);
     files.create(4);
     QCOMPARE(files.howManyExist(), filesToCreate);

     m_dirModel_01->setPath(files.lastPath());
     QTest::qWait(TIME_TO_REFRESH_DIR);

     QStringList items(files.createdNames());
     m_dirModel_01->removePaths(items);
     QTest::qWait(500);

     QCOMPARE(m_filesRemoved.count() , filesToCreate);
     QCOMPARE(m_dirModel_01->rowCount(),    0);
     QCOMPARE(files.howManyExist(), 0);

     QDir().rmdir(m_dirModel_01->path());
}


void TestDirModel::modelCopyDirPasteIntoAnotherModel()
{
    QString orig("modelCopyDirToAnotherModel_orig");

    m_deepDir_01 = new DeepDir(orig, 1);  
    m_dirModel_01->setPath(m_deepDir_01->path());
    connect(m_dirModel_01,  SIGNAL(clipboardChanged()),
            this,           SLOT(slotclipboardChanged()));
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(),  1);

    QString target("modelCopyDirToAnotherModel_target");
    m_deepDir_02 = new DeepDir(target, 0);
    connect(m_dirModel_02, SIGNAL(progress(int,int,int)),
            this,          SLOT(progress(int,int,int)));
    m_dirModel_02->setPath(m_deepDir_02->path());    

    QTest::qWait(TIME_TO_REFRESH_DIR);

    QCOMPARE( QFileInfo(m_deepDir_02->path()).exists(),  true);
    QCOMPARE(m_dirModel_02->rowCount(),  0);

    m_dirModel_01->copyIndex(0);
    m_visibleProgressMessages = true;
    m_dirModel_02->paste();
    int steps = m_dirModel_02->getProgressCounter();
    QTest::qWait(TIME_TO_PROCESS);

    QCOMPARE(m_dirModel_02->rowCount(),  1);
    QCOMPARE(m_progressPercentDone, 100);   
    QCOMPARE(compareDirectories(m_deepDir_01->path(), m_deepDir_02->path()), true);
    QCOMPARE(m_receivedClipboardChangesSignal,      true);
    QCOMPARE(steps,          m_progressCounter);
}


void TestDirModel::modelCopyManyItemsPasteIntoAnotherModel()
{
    QString orig("modelCopyManyItemstoAnotherModel_orig");

    m_deepDir_01  = new DeepDir(orig, 5);  
    connect(m_dirModel_01,  SIGNAL(clipboardChanged()),
            this,           SLOT(slotclipboardChanged()));
    const int  filesCreated = 10;
    int  itemsCreated = filesCreated + 1;

    //create a big file to test copy loop
    QByteArray buf(4096, 't');
    QFile big(m_deepDir_01->path() + QDir::separator() + "big.txt");
    QCOMPARE(big.open(QFile::WriteOnly),  true);
    for(int i=0; i < 106; i++)
    {
        int wrote = (int) big.write(buf);
        QCOMPARE(wrote, buf.size());
        buf += "sdfsdsedccw121222";
    }
    big.close();
    itemsCreated++;

    // create more temporary files
    TempFiles tempFiles;
    tempFiles.addSubDirLevel(orig);
    tempFiles.create(filesCreated);
    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(),  itemsCreated);

    QString target("modelCopyManyItemstoAnotherModel_target");
    m_deepDir_02 = new DeepDir(target, 0); 
    m_dirModel_02->setPath(m_deepDir_02->path());
    connect(m_dirModel_02, SIGNAL(progress(int,int,int)),
            this,          SLOT(progress(int,int,int)));
    QTest::qWait(TIME_TO_REFRESH_DIR);

    QCOMPARE( QFileInfo(m_deepDir_02->path()).exists(),  true);
    QCOMPARE(m_dirModel_02->rowCount(),  0);

    QStringList allFiles(m_deepDir_01->firstLevel());
    allFiles.append(tempFiles.createdList());
    allFiles.append(big.fileName());

    m_dirModel_01->copyPaths(allFiles);
    m_visibleProgressMessages = true;
    m_dirModel_02->paste();
    int steps = m_dirModel_02->m_fsAction->m_curAction->steps;

    QTest::qWait(TIME_TO_PROCESS);

    QCOMPARE(m_dirModel_02->rowCount(),  itemsCreated);
    QCOMPARE(m_dirModel_01->rowCount(),  itemsCreated);
    QCOMPARE(m_progressPercentDone, 100);
    QCOMPARE(compareDirectories(m_deepDir_01->path(), m_deepDir_02->path()), true);
    QCOMPARE(m_receivedClipboardChangesSignal,   true);

    QCOMPARE(steps,     m_progressCounter);
}


void TestDirModel::modelCopyTwoEmptyFiles()
{
    QString orig("modelCopyTwoEmptyFiles_orig");
    const int itemsCreated = 2;
    m_deepDir_01  = new DeepDir(orig, 0);
    TempFiles empty;
    empty.addSubDirLevel(orig);
    empty.touch(itemsCreated);

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(),  itemsCreated);

    QString target("modelCopyTwoEmptyFiles_target");
    m_deepDir_02 = new DeepDir(target, 0);
    m_dirModel_02->setPath(m_deepDir_02->path());
    connect(m_dirModel_02, SIGNAL(progress(int,int,int)),
            this,          SLOT(progress(int,int,int)));
    QTest::qWait(TIME_TO_REFRESH_DIR);


    m_dirModel_01->copyPaths(empty.createdList());
    m_dirModel_02->paste();
    QTest::qWait(TIME_TO_PROCESS);

    QCOMPARE(m_dirModel_02->rowCount(),  itemsCreated);
    QCOMPARE(m_dirModel_01->rowCount(),  itemsCreated);
    QCOMPARE(m_progressPercentDone, 100);
    QCOMPARE(m_progressCurrentItem, itemsCreated);
    QCOMPARE(compareDirectories(m_deepDir_01->path(), m_deepDir_02->path()), true);
}

/*!
 * \brief TestDirModel::modelCopyFileAndRemoveBeforePaste()
 *
 *  The file is removed before paste, that means at the moment of building the list of files
 *  it does not exist anymore, so the list of items must be empty and no files are going to be performed
 */
void TestDirModel::modelCopyFileAndRemoveBeforePaste()
{
    QString orig("modelCopyFileAndRemoveBeforePaste_orig");
    m_deepDir_01  = new DeepDir(orig, 0);
    TempFiles tempFile;
    tempFile.addSubDirLevel(orig);
    tempFile.create();

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(),  1);

    QString target("modelCopyFileAndRemoveBeforePaste_target");
    m_deepDir_02 = new DeepDir(target, 0); 
    m_dirModel_02->setPath(m_deepDir_02->path());
    connect(m_dirModel_02, SIGNAL(progress(int,int,int)),
            this,          SLOT(progress(int,int,int)));
    connect(m_dirModel_02, SIGNAL(error(QString,QString)),
            this,          SLOT(slotError(QString,QString)));
    QTest::qWait(TIME_TO_REFRESH_DIR);

    m_dirModel_01->copyPaths(tempFile.createdList());
    tempFile.removeAll();
    m_dirModel_02->paste();
    QTest::qWait(TIME_TO_PROCESS);

    QCOMPARE(m_dirModel_02->rowCount(),  0);
    QCOMPARE(m_receivedErrorSignal,      true);
}

/*!
 * \brief TestDirModel::modelCopyPasteFileAndRemoveWhenFirstProgressSignalArrives()
 *
 *  The file is removed after pasting and before the copy itself starts.
 *  That means, the list of files to be copied is built, but at the moment of the open
 *  the file does not exist.
 */
void TestDirModel::modelCopyPasteFileAndRemoveWhenFirstProgressSignalArrives()
{
    QString orig("modelCopyPasteFileAndRemoveWhenFirstProgressSignalArrives_orig");
    m_deepDir_01  = new DeepDir(orig, 0);
    TempFiles tempFile;
    tempFile.addSubDirLevel(orig);
    tempFile.create();

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(),  1);

    QString target("modelCopyPasteFileAndRemoveWhenFirstProgressSignalArrives_target");
    m_deepDir_02 = new DeepDir(target, 0);
    m_dirModel_02->setPath(m_deepDir_02->path());
    connect(m_dirModel_02, SIGNAL(progress(int,int,int)),
            this,          SLOT(slotRemoveFileWhenProgressArrive(int,int,int)));
    connect(m_dirModel_02, SIGNAL(error(QString,QString)),
            this,          SLOT(slotError(QString,QString)));
    QTest::qWait(TIME_TO_REFRESH_DIR);

    m_fileToRemoveInProgressSignal = tempFile.lastFileCreated();
    m_dirModel_01->copyPaths(tempFile.createdList());
    m_dirModel_02->paste();
    QTest::qWait(TIME_TO_PROCESS);

    QCOMPARE(m_dirModel_02->rowCount(),  0);
    QCOMPARE(m_receivedErrorSignal,      true);
    QCOMPARE(m_progressTotalItems,       1);
    QCOMPARE(m_progressCurrentItem,      0);
}


void TestDirModel::modelCutManyItemsPasteIntoAnotherModel()
{
    QString orig("modelCutManyItemsPasteIntoAnotherModel_orig");

    m_deepDir_01 = new DeepDir(orig, 5);  
    connect(m_dirModel_01,  SIGNAL(clipboardChanged()),
            this,           SLOT(slotclipboardChanged()));
    const int  filesCreated = 10;
    const int  itemsCreated = filesCreated + 1;

    TempFiles tempFiles;
    tempFiles.addSubDirLevel(orig);
    tempFiles.create(filesCreated);
    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(),  itemsCreated);

    QString target("modelCutManyItemsPasteIntoAnotherModel_target");
    m_deepDir_02 = new DeepDir(target, 0); 
    connect(m_dirModel_02, SIGNAL(progress(int,int,int)),
            this,          SLOT(progress(int,int,int)));
    connect(m_dirModel_02, SIGNAL(error(QString,QString)),
            this,          SLOT(slotError(QString,QString)));
    m_dirModel_02->setPath(m_deepDir_02->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);

    QCOMPARE( QFileInfo(m_deepDir_02->path()).exists(),  true);
    QCOMPARE(m_dirModel_02->rowCount(),  0);

    QStringList allFiles(m_deepDir_01->firstLevel());
    allFiles.append(tempFiles.createdList());

    m_dirModel_01->cutPaths(allFiles);
    m_visibleProgressMessages = true;
    m_dirModel_02->paste();   
    int steps = m_dirModel_02->getProgressCounter();
    QTest::qWait(TIME_TO_PROCESS);

    QCOMPARE(m_dirModel_02->rowCount(),  itemsCreated); //pasted into
    QCOMPARE(m_dirModel_01->rowCount(),  0);  //cut from
    QCOMPARE(m_receivedClipboardChangesSignal,  true);
    QCOMPARE(steps,        m_progressCounter);
}

void  TestDirModel::fsActionMoveItemsForcingCopyAndThenRemove()
{
     QString orig("fsActionMoveItemsForcingCopyAndThenRemove_orig");

     m_deepDir_01 = new DeepDir(orig, 1);

     const int  filesCreated = 4;
     const int  itemsCreated = filesCreated +1;

     TempFiles tempFiles;
     tempFiles.addSubDirLevel(orig);
     tempFiles.create(filesCreated);
     m_dirModel_01->setPath(m_deepDir_01->path());
     QTest::qWait(TIME_TO_REFRESH_DIR);
     QCOMPARE(m_dirModel_01->rowCount(),  itemsCreated);

     QString target("fsActionMoveItemsForcingCopyAndThenRemove_target");
     m_deepDir_02 = new DeepDir(target, 0);

     m_dirModel_02->setPath(m_deepDir_02->path());
     QTest::qWait(TIME_TO_REFRESH_DIR);

     connect(m_dirModel_02, SIGNAL(progress(int,int,int)),
             this,          SLOT(progress(int,int,int)));

     QCOMPARE( QFileInfo(m_deepDir_02->path()).exists(),  true);
     QCOMPARE(m_dirModel_02->rowCount(),  0);

     QStringList allFiles(m_deepDir_01->firstLevel());
     allFiles.append(tempFiles.createdList());

     m_dirModel_02->m_fsAction->m_forceUsingOtherFS = true;
     m_dirModel_02->m_fsAction->createAndProcessAction(FileSystemAction::ActionMove,
                                                       allFiles);

     QTest::qWait(TIME_TO_PROCESS);

     QCOMPARE(m_dirModel_02->rowCount(),  itemsCreated); //pasted into
     QCOMPARE(m_dirModel_01->rowCount(),  0);  //cut from

     int totalCopied = filesCreated + m_deepDir_01->itemsCreated();
     QCOMPARE(itemsCreated, m_filesAdded.count());
     QCOMPARE(totalCopied, m_progressTotalItems);
}


/*!
 * \brief TestDirModel::modelCancelRemoveAction()
 *
 *  It tests cancel on a remove operation
 *
 *  Two directories with a deep tree of items are created
 *
 *  The first directory is removed and the cancel is triggered after 10 percent of the all files be removed,
 *  then the first phase of the checks are performed
 *
 *  The second phase consists in remove the same directory after the cancel operation
 *  then a new set of checks are performed
 *
 *  The third phase consists in remove the second directory previously created
 *
 */
void TestDirModel::modelCancelRemoveAction()
{
     const int level = 30;
     m_deepDir_01 = new DeepDir("modelCancelRemoveAction", level);
     QCOMPARE( QFileInfo(m_deepDir_01->path()).exists(),  true);

     m_dirModel_01->setPath(m_deepDir_01->path());
     QTest::qWait(TIME_TO_REFRESH_DIR);

     QCOMPARE(m_dirModel_01->rowCount(), 1);
     connect(m_dirModel_01, SIGNAL(progress(int,int,int)),
             this,          SLOT(progress(int,int,int)));

     connect(m_dirModel_01, SIGNAL(error(QString,QString)),
             this,          SLOT(slotError(QString,QString)));

     //cancel only after removing some files
     m_minimumProgressToCancelAction = 10;
     connect(m_dirModel_01, SIGNAL(progress(int,int,int)),
             this,          SLOT(cancel(int,int,int)));


     //create a second directory
     m_deepDir_02 = new DeepDir("modelCancelRemoveAction2", level);
     QCOMPARE( QFileInfo(m_deepDir_02->path()).exists(),  true);

     m_dirModel_02->setPath(m_deepDir_02->path());
     QTest::qWait(TIME_TO_REFRESH_DIR);
     QCOMPARE(m_dirModel_02->rowCount(), 1);


     //remove from first directory
     m_dirModel_01->removeIndex(0);
     QTest::qWait(TIME_TO_PROCESS);
     QTest::qWait(5);

     // sub items do not emit removed()
     QCOMPARE(m_filesRemoved.count(), 0);
     QCOMPARE(m_dirModel_01->rowCount(), 1);
     QVERIFY(m_progressCurrentItem > 0);     // some file were performed
     QVERIFY(m_progressPercentDone < 100);   //
     QCOMPARE(m_receivedErrorSignal, false);
     // high level item still exists
     QDir a(m_deepDir_01->path(), QString(), QDir::NoSort,  QDir::AllEntries | QDir::NoDotAndDotDot);
     QCOMPARE(a.exists(), true);
     QCOMPARE((int)a.count(), 1);

     //First phase OK cancel worked
     //SECOND PHASE  now remove everything
     disconnect(m_dirModel_01, SIGNAL(progress(int,int,int)),
                this,          SLOT(cancel(int,int,int)));

     m_dirModel_01->removeIndex(0);
     QTest::qWait(TIME_TO_PROCESS * 2);
     QTest::qWait(5);
     QCOMPARE(m_progressPercentDone,  100);
     QCOMPARE(m_filesRemoved.count(), 1);
     QCOMPARE(m_dirModel_01->rowCount(), 0);
     QCOMPARE(m_receivedErrorSignal, false);
     // high level item does NOT exist anymore
     QDir b(m_deepDir_01->path(), QString(), QDir::NoSort, QDir::AllEntries | QDir::NoDotAndDotDot);
     QCOMPARE(b.exists(), true);
     QCOMPARE((int)b.count(), 0);

     //THIRD PHASE
     //now remove the second directory
     QDir c(m_deepDir_02->path(), QString(), QDir::NoSort, QDir::AllEntries | QDir::NoDotAndDotDot);
     QCOMPARE(c.exists(), true);
     QCOMPARE((int)c.count(), 1);
     m_dirModel_02->removeIndex(0);
     QTest::qWait(TIME_TO_PROCESS * 2);
     QDir d(m_deepDir_02->path(), QString(), QDir::NoSort, QDir::AllEntries | QDir::NoDotAndDotDot);
     QCOMPARE(d.exists(), true);
     QCOMPARE((int)d.count(), 0);
     QCOMPARE(m_receivedErrorSignal, false);
     QCOMPARE(m_dirModel_02->rowCount(), 0);
}


void TestDirModel::modelCancelRemoveSelection()
{
     QString tmpDir("modelCancelRemoveSelection");
     m_deepDir_01 = new DeepDir(tmpDir, 0);
     QCOMPARE( QFileInfo(m_deepDir_01->path()).exists(),  true);
     TempFiles tempFiles;
     tempFiles.addSubDirLevel(tmpDir);
     const int createdFiles = 250;
     tempFiles.create(createdFiles);
     m_dirModel_01->setPath(m_deepDir_01->path());    
     QTest::qWait(TIME_TO_REFRESH_DIR);

     QCOMPARE(m_dirModel_01->rowCount(), createdFiles);
     connect(m_dirModel_01, SIGNAL(progress(int,int,int)),
             this,          SLOT(progress(int,int,int)));

     connect(m_dirModel_01, SIGNAL(error(QString,QString)),
             this,          SLOT(slotError(QString,QString)));

     //cancel only after removing some files
     m_minimumProgressToCancelAction = 20;
     connect(m_dirModel_01, SIGNAL(progress(int,int,int)),
             this,          SLOT(cancel(int,int,int)));


    DirSelection  *selection = m_dirModel_01->selectionObject();
    QVERIFY(selection != 0);

    //remove all, but cancel will be triggered
    selection->selectAll();
    m_dirModel_01->removeSelection();
    QTest::qWait(TIME_TO_PROCESS*2);

    QVERIFY( m_dirModel_01->rowCount() > 0 );
    QVERIFY( m_dirModel_01->rowCount() <  createdFiles);
    QVERIFY(m_filesRemoved.count() > 0);

    int notRemoved = createdFiles - m_filesRemoved.count();
    QVERIFY(notRemoved > 0);

    QDir c(m_deepDir_01->path(), QString(), QDir::NoSort, QDir::AllEntries | QDir::NoDotAndDotDot);
    QCOMPARE(c.exists(), true);
    QCOMPARE((int)c.count(), notRemoved);

    disconnect(m_dirModel_01, SIGNAL(progress(int,int,int)),
             this,          SLOT(cancel(int,int,int)));

    //now remove everything
    selection->clear();
    selection->selectAll();
    QCOMPARE(selection->counter(),  m_dirModel_01->rowCount());
    m_dirModel_01->removeSelection();
    QTest::qWait(TIME_TO_PROCESS*2);

    QCOMPARE(m_dirModel_01->rowCount() , 0);
    QDir d(m_deepDir_01->path(), QString(), QDir::NoSort, QDir::AllEntries | QDir::NoDotAndDotDot);
    QCOMPARE(d.exists(), true);
    QCOMPARE((int)d.count(), 0);
}

void TestDirModel::modelTestFileSize()
{  
     QCOMPARE(m_dirModel_01->fileSize(0),      QString("0 Bytes"));
     QCOMPARE(m_dirModel_01->fileSize(1023),   QString("1023 Bytes"));
     QCOMPARE(m_dirModel_01->fileSize(1024),   QString("1.0 kB"));
     QCOMPARE(m_dirModel_01->fileSize(1000*1000),
              QString("1.0 MB"));
     QCOMPARE(m_dirModel_01->fileSize(1000*1000*1000),
              QString("1.0 GB"));
}



void TestDirModel::modelRemoveDirWithHiddenFilesAndLinks()
{
     const int level = 3;
     m_deepDir_01 = new DeepDir("modelRemoveDirWithHiddenFilesAndLinks", level);
     QCOMPARE( QFileInfo(m_deepDir_01->path()).exists(),  true);

     m_dirModel_01->setShowHiddenFiles(true);
     m_dirModel_01->setPath(m_deepDir_01->path());
     QTest::qWait(TIME_TO_REFRESH_DIR);

     QCOMPARE(m_dirModel_01->rowCount(), 1);

     connect(m_dirModel_01, SIGNAL(progress(int,int,int)),
             this,          SLOT(progress(int,int,int)));

     QModelIndex filepathIdx = m_dirModel_01->index(0, DirModel::FilePathRole - DirModel::FileNameRole);
     QString firstItemFullPath(m_dirModel_01->data(filepathIdx).toString());

     // hidden files and links are created under first item
     QFile hiddenFile(firstItemFullPath
                      + QDir::separator()
                      + QLatin1String(".hidden.txt"));

     QCOMPARE(hiddenFile.open(QFile::WriteOnly)  ,true);
     hiddenFile.close();
     QString link_to_hiddenFile(firstItemFullPath
                                + QDir::separator()
                                + QLatin1String("link_to_hiddenFile"));
     QCOMPARE(hiddenFile.link(link_to_hiddenFile), true);

     QString hiddenFolder(firstItemFullPath
                          + QDir::separator()
                          + QLatin1String(".hiddenFolder"));

     QCOMPARE(QDir().mkdir(hiddenFolder)         , true);
     QString link_to_hidden_folder(firstItemFullPath
                                   + QDir::separator()
                                   + QLatin1String("link_to_hidden_folder"));
     QCOMPARE(QFile(hiddenFolder).link(link_to_hidden_folder),  true);

     m_dirModel_01->removeIndex(0);
     QTest::qWait(TIME_TO_PROCESS);

     QCOMPARE(m_dirModel_01->rowCount(), 0);
     QCOMPARE(m_progressPercentDone, 100);
}


void TestDirModel::modelCancelCopyAction()
{
    QString orig("modelCancelCopyAction_orig");
    m_deepDir_01  = new DeepDir(orig, 0);

    //create a big file to test copy loop
    QByteArray buf(4096, 't');
    QFile big(m_deepDir_01->path() + QDir::separator() + "big.txt");
    QCOMPARE(big.open(QFile::WriteOnly),  true);
    for(int i=0; i < 186; i++)
    {
        int wrote = (int) big.write(buf);
        QCOMPARE(wrote, buf.size());
        buf += "sdfsdsedccw121222";
    }
    big.close();

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(), 1);

    QString target("modelCancelCopyAction_target");
    m_deepDir_02 = new DeepDir(target, 0);

    connect(m_dirModel_02, SIGNAL(progress(int,int,int)),
            this,          SLOT(progress(int,int,int)));
    connect(m_dirModel_02, SIGNAL(progress(int,int,int)),
            this,          SLOT(cancel(int,int,int)));
    m_dirModel_02->setPath(m_deepDir_02->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);

    m_dirModel_01->copyIndex(0);
    m_dirModel_02->paste();
    QTest::qWait(TIME_TO_PROCESS);

    QCOMPARE( QFileInfo(m_deepDir_02->path()).exists(),  true);
    QCOMPARE(m_dirModel_02->rowCount(),  0);
}

void TestDirModel::modelCopyFileAndDirectoryLinks()
{
    QString orig("modelCopyFileAndDirectoryLinks_orig");
    m_deepDir_01  = new DeepDir(orig, 1);

    //create a link in the same directory to the directory created at first level
    QString firstDir         = m_deepDir_01->lastLevel();
    QString link_to_firstDir = m_deepDir_01->path() + QDir::separator()
                               + QLatin1String("link_to_firstDir");
    QCOMPARE(createLink(firstDir, link_to_firstDir),  true);

    QDir d(firstDir);
    QFileInfoList files = d.entryInfoList(QDir::Files);
    QVERIFY(files.count() > 0);
    QCOMPARE(createLink(files.at(0).absoluteFilePath(), QLatin1String("link_to_file")),  true);

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(), 2);


    QString target("modelCopyFileAndDirectoryLinks_target");
    m_deepDir_02 = new DeepDir(target, 0);
    connect(m_dirModel_02, SIGNAL(progress(int,int,int)),
            this,           SLOT(progress(int,int,int)));

    m_dirModel_02->setPath(m_deepDir_02->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);

    QStringList  items(firstDir);
    items.append(link_to_firstDir);

    m_dirModel_01->copyPaths(items);
    QTest::qWait(10);
    m_dirModel_02->paste();
    QTest::qWait(TIME_TO_PROCESS);

    QCOMPARE(m_dirModel_01->rowCount(), 2);
    QCOMPARE(m_dirModel_02->rowCount(), 2);
    QCOMPARE(compareDirectories(m_deepDir_01->path(), m_deepDir_02->path()), true);
    QCOMPARE(m_progressPercentDone, 100);
    QCOMPARE(m_progressCurrentItem, m_progressTotalItems);
}


void TestDirModel::modelCutAndPasteInTheSamePlace()
{
    QString orig("modelCutAndPasteInTheSamePlace_orig");
    m_deepDir_01 = new DeepDir (orig,0);  // just to be removed
    const int files_to_create = 4;

    TempFiles  files;
    files.addSubDirLevel(orig);
    files.create(files_to_create);

    QFileInfo info_before[files_to_create];
    QStringList created_files(files.createdList());

    QCOMPARE(created_files.count(), files_to_create);
    int counter = 0;
    for(counter=0; counter < files_to_create; counter++)
    {
        info_before[counter] = QFileInfo(created_files.at(counter));
    }

    m_dirModel_01->setPath(files.lastPath());
    connect(m_dirModel_01, SIGNAL(error(QString,QString)),
            this,          SLOT(slotError(QString,QString)));

    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(), files_to_create);

    m_dirModel_01->cutPaths(created_files);
    m_dirModel_01->paste();
    QTest::qWait(TIME_TO_PROCESS);
    m_dirModel_01->refresh();
    QTest::qWait(TIME_TO_REFRESH_DIR);


    QCOMPARE(m_dirModel_01->rowCount(), files_to_create);
    for(counter=0; counter < files_to_create; counter++)
    {
        //files exist and did not were touched
        QCOMPARE( QFileInfo(created_files.at(counter)).lastModified(),
                  info_before[counter].lastModified()) ;
    }

    QCOMPARE(m_receivedErrorSignal,  true);
}


void TestDirModel::modelCopyAndPasteToBackupFiles()
{
    QString orig("modelBackupFiles_orig");
    m_deepDir_01 = new DeepDir (orig,5);  // just to be removed
    const int files_to_create = 12;

    TempFiles  files;
    files.addSubDirLevel(orig);
    files.create(files_to_create);

    QStringList created_files(files.createdList());
    QCOMPARE(created_files.count(), files_to_create);

    m_dirModel_01->setPath(files.lastPath());

    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(), files_to_create + 1);
    created_files.append(m_deepDir_01->firstLevel());

    m_dirModel_01->copyPaths(created_files);
    connect(m_dirModel_02, SIGNAL(error(QString,QString)),
            this,          SLOT(slotError(QString,QString)));

    m_dirModel_02->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    m_dirModel_02->paste();
    QTest::qWait(TIME_TO_PROCESS);

    QCOMPARE(m_dirModel_02->rowCount(), (files_to_create + 1) * 2);
    QCOMPARE(m_receivedErrorSignal,  false);

    m_dirModel_02->paste();
    QTest::qWait(TIME_TO_PROCESS);

    QCOMPARE(m_dirModel_02->rowCount(), (files_to_create + 1) * 3);
    QCOMPARE(m_receivedErrorSignal,  false);
 }



void TestDirModel::modelCopyAndPaste3Times()
{
    QString orig("modelCopyAndPaste3Times_orig");
    m_deepDir_01  = new DeepDir(orig, 5);

    TempFiles moreFiles;
    QCOMPARE(moreFiles.addSubDirLevel(orig),   true);
    QCOMPARE(moreFiles.create(10),             true);

    QStringList items(m_deepDir_01->firstLevel());
    items.append(moreFiles.createdList());

    QString target1("modelCopyAndPaste3Times_target1");
    QString target2("modelCopyAndPaste3Times_target2");
    QString target3("modelCopyAndPaste3Times_target3");

    DeepDir t1(target1,0);
    DeepDir t2(target2,0);
    DeepDir t3(target3,0);

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(items.count(), m_dirModel_01->rowCount());
    m_dirModel_01->copyPaths(items);

    DirModel  model1;
    model1.setPath(t1.path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(model1.rowCount() , 0);
    model1.paste();
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(model1.rowCount(),  items.count());
    QCOMPARE(compareDirectories(m_deepDir_01->path(), t1.path()),   true);

    DirModel  model2;
    model2.setPath(t2.path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(model2.rowCount() , 0);
    model2.paste();
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(model2.rowCount(),  items.count());
    QCOMPARE(compareDirectories(m_deepDir_01->path(), t2.path()),   true);

    DirModel  model3;
    model3.setPath(t3.path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(model3.rowCount() , 0);
    model3.paste();
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(model3.rowCount(),  items.count());
    QCOMPARE(compareDirectories(m_deepDir_01->path(), t3.path()),   true);
}


void TestDirModel::modelCutAndPaste3Times()
{
    QString orig("modelCutAndPaste3Times_orig");
    m_deepDir_01  = new DeepDir(orig, 5);

    TempFiles moreFiles;
    QCOMPARE(moreFiles.addSubDirLevel(orig),   true);
    QCOMPARE(moreFiles.create(10),             true);

    QStringList items(m_deepDir_01->firstLevel());
    items.append(moreFiles.createdList());

    QString target1("modelCutAndPaste3Times_target1");
    QString target2("modelCutAndPaste3Times_target2");
    QString target3("modelCutAndPaste3Times_target3");

    DeepDir t1(target1,0);
    DeepDir t2(target2,0);
    DeepDir t3(target3,0);

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(items.count(), m_dirModel_01->rowCount());  
    m_dirModel_01->cutPaths(items);

    DirModel  model1;
    model1.setPath(t1.path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(model1.rowCount() , 0);
    model1.paste();
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(model1.rowCount(),  items.count());
    QCOMPARE(m_dirModel_01->rowCount()  , 0 );

    DirModel  model2;
    model2.setPath(t2.path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(model2.rowCount() , 0);
    model2.paste();
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(model2.rowCount(),  items.count());

    DirModel  model3;
    model3.setPath(t3.path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(model3.rowCount() , 0);
    model3.paste();
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(model3.rowCount(),  items.count());
}


void TestDirModel::modelCopyPasteAndPasteAgain()
{
    QString orig("modelCopyPasteAndPasteAgain_orig");
    m_deepDir_01  = new DeepDir(orig, 1);
    connect(m_dirModel_01,  SIGNAL(error(QString,QString)),
            this,           SLOT(slotError(QString,QString)));

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(), 1);


    QString target("modelCopyPasteAndPasteAgain_target");
    m_deepDir_02  = new DeepDir(target, 0);
    connect(m_dirModel_02,  SIGNAL(error(QString,QString)),
            this,           SLOT(slotError(QString,QString)));

    m_dirModel_02->setPath(m_deepDir_02->path());  
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_02->rowCount(), 0);

    m_dirModel_01->copyIndex(0);

    //first time
    m_dirModel_02->paste();
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(m_filesRemoved.count(),  0);
    QCOMPARE(m_filesAdded.count(),    1);

    //second time
    m_dirModel_02->paste();
    QTest::qWait(TIME_TO_PROCESS);

    QCOMPARE(compareDirectories(m_deepDir_01->path(), m_deepDir_02->path()), true);
    QCOMPARE(m_receivedErrorSignal,   false);   

    //when items already exist, changed signal is emitted
    QCOMPARE(m_filesRemoved.count(),  0);
    QCOMPARE(m_filesAdded.count(),    1);
}


void TestDirModel::modelCutPasteIntoExistentItems()
{
    QString orig("modelCutPasteIntoExistentItems_orig");
    m_deepDir_01  = new DeepDir(orig, 0);
    QString moreOneLevel("MoreOneLevel");
    TempFiles tempFiles_01;
    TempFiles tempDir_01;

    const int createCounterTopLevel = 8;
    const int createCounterSubLevel = 2;

    tempFiles_01.addSubDirLevel(orig);
    tempFiles_01.create(createCounterTopLevel);
    tempDir_01.addSubDirLevel(orig);
    tempDir_01.addSubDirLevel(moreOneLevel);
    tempDir_01.create(2);

    connect(m_dirModel_01,  SIGNAL(error(QString,QString)),
            this,           SLOT(slotError(QString,QString)));
    m_dirModel_01->setPath(tempFiles_01.lastPath());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(), createCounterTopLevel+1); //those created top level plus the subfolder

    QString target("modelCutPasteIntoExistentItems_target");
    m_deepDir_02  = new DeepDir(target, 0);
    TempFiles tempFiles_02;
    TempFiles tempDir_02;

    tempFiles_02.addSubDirLevel(target);
    tempFiles_02.create(createCounterTopLevel);
    tempDir_02.addSubDirLevel(target);
    tempDir_02.addSubDirLevel(moreOneLevel);
    tempDir_02.create(createCounterSubLevel);

    connect(m_dirModel_02,  SIGNAL(error(QString,QString)),
            this,           SLOT(slotError(QString,QString)));
    connect(m_dirModel_02, SIGNAL(progress(int,int,int)),
            this,          SLOT(progress(int,int,int)));
    m_dirModel_02->setPath(tempFiles_02.lastPath());
    QTest::qWait(TIME_TO_REFRESH_DIR);

    QCOMPARE(m_dirModel_02->rowCount(), createCounterTopLevel+1); //those created top level plus the subfolder

   //both directories have the same content
    QCOMPARE(compareDirectories(tempFiles_01.lastPath(), tempFiles_02.lastPath()), true);

    //cut from first Model
    QStringList items(tempFiles_01.createdList());
    items.append(tempDir_01.lastPath());
    m_dirModel_01->cutPaths(items);
    //paste into the second model
    m_dirModel_02->paste();
    QTest::qWait(TIME_TO_PROCESS *3);

    QCOMPARE(m_receivedErrorSignal,   false);

    //only one directory that already exists will be removed using another Action
    QCOMPARE(m_filesRemoved.count(),  1);
    //when items being copied from COPY or renamed from CUT already exist, they are not added
    QCOMPARE(m_filesAdded.count(),  0);
}


void TestDirModel::openPathAbsouluteAndRelative()
{
    QString orig("openPathAbsouluteAndRelative");
    m_deepDir_01  = new DeepDir(orig, 1);

    connect(m_dirModel_01,  SIGNAL(error(QString,QString)),
            this,           SLOT(slotError(QString,QString)));

    connect(m_dirModel_01, SIGNAL(pathChanged(QString)),
            this,          SLOT(slotPathChamged(QString)));

    m_currentPath.clear();
    bool ret = m_dirModel_01->openPath(QLatin1String("_ItDoesNotExist_"));
    QTest::qWait(TIME_TO_REFRESH_DIR);

    QCOMPARE(ret,                        false);
    QCOMPARE(m_currentPath.isEmpty(),    true);

    ret = m_dirModel_01->openPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(ret,                        true);
    QCOMPARE(m_currentPath,              m_deepDir_01->path());
    QCOMPARE(m_dirModel_01->rowCount(), 1);
    QCOMPARE(m_receivedErrorSignal,      false);

    ret = m_dirModel_01->openPath(QLatin1String(".."));
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(ret,                        true);   
    QCOMPARE(m_currentPath,              QDir::tempPath());
    QCOMPARE(m_receivedErrorSignal,      false);

    ret = m_dirModel_01->openPath(orig);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(ret,                        true);
    QCOMPARE(m_currentPath,              m_deepDir_01->path());
    QCOMPARE(m_receivedErrorSignal,      false);

    // trash --------------------------------------
    TempFiles files;
    QCOMPARE(files.addSubDirLevel(m_deepDir_01->path()),  true);

    createTempHomeTrashDir(m_deepDir_01->path());

    QTrashDir tempTrash;
    QCOMPARE(files.addSubDirLevel(tempTrash.homeTrash()),  true);
    QCOMPARE(files.addSubDirLevel(QTrashUtilInfo::filesTrashDir(tempTrash.homeTrash())),  true);

    QString level1("Level1");
    QCOMPARE(files.addSubDirLevel(level1),  true);

    QTrashUtilInfo trashInfo;
    trashInfo.setInfo(tempTrash.homeTrash(), level1);
    QCOMPARE(trashInfo.existsFile()   , true);
    QFile infoFile(trashInfo.absInfo);
    QCOMPARE(infoFile.open(QFile::WriteOnly),  true);
    infoFile.close();

    //check if "Level1" is valid item under trash
    QCOMPARE(trashInfo.existsInfoFile()   , true);

    QString level2("level2");
    QString level3("level3");

    QCOMPARE(files.addSubDirLevel(level2),  true);
    QCOMPARE(files.addSubDirLevel(level3),  true);

    files.create(1);

    // using trash:///
    ret = m_dirModel_01->openPath(LocationUrl::TrashRootURL);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(ret,                        true);
    QCOMPARE(m_currentPath,              LocationUrl::TrashRootURL);
    QCOMPARE(m_receivedErrorSignal,      false);

    // using relative "Level1"
    ret = m_dirModel_01->openPath(level1);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(ret,                        true);
    QCOMPARE(m_currentPath,              QString(LocationUrl::TrashRootURL + level1) );
    QCOMPARE(m_receivedErrorSignal,      false);

   //using trash:///Level1/Level2/Level3
   QString deep(LocationUrl::TrashRootURL + level1 + QDir::separator() + level2 + QDir::separator() + level3);
   ret = m_dirModel_01->openPath(deep);
   QTest::qWait(TIME_TO_REFRESH_DIR);
   QCOMPARE(ret,                        true);
   QCOMPARE(m_receivedErrorSignal,      false);

   //using ../ to go up into Level2
   ret = m_dirModel_01->openPath("../");
   QTest::qWait(TIME_TO_REFRESH_DIR);
   QCOMPARE(ret,                        true);
   QCOMPARE(m_receivedErrorSignal,      false);

   //using .. to go up into Level1
   ret = m_dirModel_01->openPath("..");
   QTest::qWait(TIME_TO_REFRESH_DIR);
   QCOMPARE(ret,                        true);
   QCOMPARE(m_receivedErrorSignal,      false);

   //back to trash:///
   ret = m_dirModel_01->openPath("..");
   QTest::qWait(TIME_TO_REFRESH_DIR);
   QCOMPARE(ret,                        true);   
   QCOMPARE(m_currentPath,              LocationUrl::TrashRootURL);
   QCOMPARE(m_receivedErrorSignal,      false);

   //now it must fail
   ret = m_dirModel_01->openPath("..");
   QTest::qWait(TIME_TO_REFRESH_DIR);
   QCOMPARE(ret,                        false);
   QCOMPARE(m_receivedErrorSignal,      false);

   ret = m_dirModel_01->openPath("file:///");
   QTest::qWait(TIME_TO_REFRESH_DIR);
   QCOMPARE(ret,                        true);
   QCOMPARE(m_currentPath,              QDir::rootPath());
   QCOMPARE(m_receivedErrorSignal,      false);

   ret = m_dirModel_01->openPath("file://");
   QTest::qWait(TIME_TO_REFRESH_DIR);
   QCOMPARE(ret,                        true);
   QCOMPARE(m_currentPath,              QDir::rootPath());
   QCOMPARE(m_receivedErrorSignal,      false);

   ret = m_dirModel_01->openPath("file:/");
   QTest::qWait(TIME_TO_REFRESH_DIR);
   QCOMPARE(ret,                        true);
   QCOMPARE(m_currentPath,              QDir::rootPath());
   QCOMPARE(m_receivedErrorSignal,      false);
}


void TestDirModel::existsDirAnCanReadDir()
{
    QString orig("existsDirAnCanReadDir");
    m_deepDir_01  = new DeepDir(orig, 1);

    connect(m_dirModel_01,  SIGNAL(error(QString,QString)),
            this,           SLOT(slotError(QString,QString)));

    connect(m_dirModel_01, SIGNAL(pathChanged(QString)),
            this,          SLOT(slotPathChamged(QString)));

    m_currentPath.clear();
    m_dirModel_01->goHome();
    QTest::qWait(TIME_TO_REFRESH_DIR);

    //test absoulute path
    QCOMPARE(m_dirModel_01->existsDir(QDir::tempPath()),  true);
    QCOMPARE(m_dirModel_01->canReadDir(QDir::tempPath()), true);

    //relative does not exist
    QString relativeDoesNoEist("__IT_May_Not_Exist_in_Home__HAHA");
    QCOMPARE(m_dirModel_01->existsDir(relativeDoesNoEist), false);
    QCOMPARE(m_dirModel_01->canReadDir(relativeDoesNoEist), false);

    //relative exists
    m_dirModel_01->setPath(QDir::tempPath());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->existsDir(orig),  true);
    QCOMPARE(m_dirModel_01->canReadDir(orig), true);

    //check permissions from canReadDir()
    bool ok = QFile::setPermissions(m_deepDir_01->path(),
                                    QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    QCOMPARE(ok,                             true);
    QCOMPARE(m_dirModel_01->existsDir(orig),  true);
    QCOMPARE(m_dirModel_01->canReadDir(orig), false);

    ok = QFile::setPermissions(m_deepDir_01->path(),
                               QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner );
    QCOMPARE(ok,                             true);
    QCOMPARE(m_dirModel_01->existsDir(orig),  true);
    QCOMPARE(m_dirModel_01->canReadDir(orig), true);
    QCOMPARE(m_dirModel_01->existsFile(orig), false);
}

void TestDirModel::existsFileAndCanReadFile()
{
    QString orig("existsFileAndCanReadFile");
    m_deepDir_01  = new DeepDir(orig, 1);

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);

    QString fileName("myFile.txt");
    QCOMPARE(m_dirModel_01->rowCount(),   1) ;
    QCOMPARE(m_dirModel_01->existsFile(fileName),  false);
    QCOMPARE(m_dirModel_01->canReadFile(fileName),  false);

    TempFiles   temp;
    temp.addSubDirLevel(orig);
    temp.create(fileName,1);
    m_dirModel_01->refresh();
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(),   2) ;

    // it has a counter in the name like "myFile.txt_00
    fileName  = temp.lastNameCreated();

    //relative
    QCOMPARE(m_dirModel_01->existsFile(fileName),   true);
    QCOMPARE(m_dirModel_01->canReadFile(fileName),  true);
    QCOMPARE(m_dirModel_01->existsDir(fileName),    false);

    //absolute
    QCOMPARE(m_dirModel_01->existsFile(temp.lastFileCreated()),   true);
    QCOMPARE(m_dirModel_01->canReadFile(temp.lastFileCreated()),  true);

    QCOMPARE(m_dirModel_01->existsDir(temp.lastFileCreated()),   false);
    QCOMPARE(m_dirModel_01->canReadDir(temp.lastFileCreated()),  false);

    bool ok = m_dirModel_01->cdIntoPath( m_deepDir_01->lastLevel());
    QCOMPARE(ok,              true);

    //relative
    QString fileIsUp = QLatin1String("..") + QDir::separator() + fileName;
    QCOMPARE(m_dirModel_01->existsFile(fileIsUp),   true);
    QCOMPARE(m_dirModel_01->canReadFile(fileIsUp),  true);

    ok = QFile::setPermissions(temp.lastFileCreated(), QFileDevice::WriteOwner);
    QCOMPARE(ok,              true);
    QCOMPARE(m_dirModel_01->existsFile(fileIsUp),   true);
    QCOMPARE(m_dirModel_01->canReadFile(fileIsUp),  false);
}


void TestDirModel::pathProperties()
{
    QString orig("pathProperties");
    m_deepDir_01  = new DeepDir(orig, 0);

    QTest::qWait(1000); // wait one second

    TempFiles temp;
    temp.addSubDirLevel(orig);
    temp.create(1);

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);

    QCOMPARE(m_dirModel_01->curPathIsWritable(), true);
    bool ok =  QFile::setPermissions(m_deepDir_01->path(),
                                      QFileDevice::ReadOwner | QFileDevice::ExeOwner );

    QCOMPARE(ok,   true);
    QCOMPARE(m_dirModel_01->curPathIsWritable(), false);

    ok =  QFile::setPermissions(m_deepDir_01->path(),
                                QFileDevice::ReadOwner | QFileDevice::ExeOwner | QFileDevice::WriteOwner);

    QCOMPARE(ok,   true);
    qWarning("created  %s", m_dirModel_01->curPathCreatedDateLocaleShort().toLatin1().constData());
    qWarning("modified %s", m_dirModel_01->curPathModifiedDateLocaleShort().toLatin1().constData());

}


/** description:
 *
 *  1.  2 file managers in /tmp/watchExternalChanges, m_dirModel_01 and m_dirModel_02
 *  2.  level_01 will be created under  /tmp/watchExternalChanges,
 *       so both m_dirModel_01 and m_dirModel_02 start with 1 item
 *  3. Under /tmp/watchExternalChanges/level_01 10 items "from_level_01.cut_nn" are created
 *  4  A third File manager is created pointing to /tmp/watchExternalChanges/level_01
 *  5  A loop is created to move all files under /tmp/watchExternalChanges/level_01
 *     to /tmp/watchExternalChanges where there are the 2 File Manager instances /tmp/watchExternalChanges/level_01
 *  inside the loop:
 *     5.1  files are created using createdOutsideName which is a TemFiles object
 *     5.2  from m_dirModel_01 a file can be removed depending on the current time
 */
void TestDirModel::watchExternalChanges()
{
    QString orig("watchExternalChanges");
    m_deepDir_01  = new DeepDir(orig, 0);
    TempFiles       tempUnderFirstLevel;
    tempUnderFirstLevel.addSubDirLevel(orig);
    tempUnderFirstLevel.addSubDirLevel(QLatin1String("level_01"));
    const int cut_items = 10;
    tempUnderFirstLevel.create("from_level_01.cut", cut_items);

    DirModel  thirdFM;
    thirdFM.setPath(tempUnderFirstLevel.lastPath());


    connect(m_dirModel_01, SIGNAL(error(QString,QString)),
            this,          SLOT(slotError(QString,QString)));
    m_dirModel_01->setPath(m_deepDir_01->path());

    QTest::qWait(TIME_TO_REFRESH_DIR);

    connect(m_dirModel_02, SIGNAL(error(QString,QString)),
            this,          SLOT(slotError(QString,QString)));
    m_dirModel_02->setPath(m_deepDir_01->path());

    QTest::qWait(TIME_TO_REFRESH_DIR);

    QCOMPARE(thirdFM.rowCount(),  cut_items);
    QCOMPARE(m_dirModel_01->rowCount(), 1);
    QCOMPARE(m_dirModel_02->rowCount(), 1);

    qDebug() << "dirModelObjs:" << m_dirModel_01 << m_dirModel_02 << &thirdFM;

    QString    createdOutsideName;
    TempFiles  createdOutsideFiles;
    createdOutsideFiles.addSubDirLevel(orig);
    int        total_removed = 0;
    int        odd=0;  

    thirdFM.setEnabledExternalFSWatcher(true);

    for ( int counter = 0; counter < cut_items; ++counter )
    {
        thirdFM.cutIndex(0);
        if ( (odd^=1) )
        {
            m_dirModel_01->paste();
        }
        else
        {
            m_dirModel_02->paste();
        }
        QTest::qWait(TIME_TO_PROCESS);
        //at least one file is removed
        if (m_dirModel_02->rowCount() > 1 &&
            (!total_removed || (QDateTime::currentDateTime().time().msec() % 100) == 0))
        {
            //using index 1 because 0 is a directory and the items are being moved up
           m_dirModel_02->removeIndex(1);
           QTest::qWait(TIME_TO_PROCESS);
            ++total_removed;
        }       
        createdOutsideName.sprintf("created_%d", counter);
        createdOutsideFiles.create(createdOutsideName);
    }
    int total_created = 1 + cut_items + createdOutsideFiles.created() - total_removed;
    QTest::qWait(EX_FS_WATCHER_TIMER_INTERVAL * 2);
    qWarning("using 2 instances [cut_items]=%d [created outside]=%d, [removed outside]=%d", cut_items, createdOutsideFiles.created(),  total_removed);

    QCOMPARE(m_dirModel_01->rowCount(),    total_created);
    if (m_dirModel_02->getEnabledExternalFSWatcher())
    {
        QCOMPARE(m_dirModel_02->rowCount(),    total_created);
    }
}


void TestDirModel::getThemeIcons()
{   
    QStringList mimesToTest = QStringList()
                             << "text/plain"
                             << "text/x-c++src"
                             << "text/x-csrc"
                             << "inode/directory"
                             << "application/msword"
                             << "application/octet-stream"
                             << "application/pdf"
                             << "application/postscript"
                             << "application/x-bzip-compressed-tar"
                             << "application/x-executable"
                             << "application/x-gzip"
                             << "application/x-shellscript"                                                         
                             ;
    QMimeDatabase mimeBase;
    QString msg;
    QHash<QByteArray, QString>     md5IconsTable;

    qDebug() << "QIcon::themeSearchPaths()" << QIcon::themeSearchPaths();
    qDebug() << "QIcon::themeName()"        << QIcon::themeName();


    for (int counter=0; counter < mimesToTest.count(); counter++)
    {
        QMimeType mimetype = mimeBase.mimeTypeForName(mimesToTest.at(counter));

        msg = QLatin1String("invalid mimetype ") + mimesToTest.at(counter);        
        if (!mimetype.isValid())
        {
              QSKIP_ALL_TESTS(qPrintable(msg));
        }
        QString iconName = mimetype.iconName();
        if (!QIcon::hasThemeIcon(iconName) && QIcon::hasThemeIcon(mimetype.genericIconName()))
        {
            iconName = mimetype.genericIconName();
        }
        QIcon   icon = QIcon::fromTheme(iconName);
        msg = QLatin1String("invalid QIcon::fromTheme ") + iconName;
        if (icon.isNull())
        {
              QSKIP_ALL_TESTS(qPrintable(msg));
        }

        QPixmap pix = icon.pixmap(QSize(48,48));
        msg = QLatin1String("invalid QPixmap from icon ") + iconName;
        if (pix.isNull())
        {
              QSKIP_ALL_TESTS(qPrintable(msg));
        }

        QImage image = pix.toImage();
        msg = QLatin1String("invalid QImage from QPixmap/QIcon ") + iconName;
        if (image.isNull())
        {
              QSKIP_ALL_TESTS(qPrintable(msg));
        }

        const uchar *bits = image.bits();
        const char *bytes = reinterpret_cast<const char*> (bits);
        QByteArray bytesArray(bytes, image.byteCount());
        QByteArray md5 = QCryptographicHash::hash(bytesArray, QCryptographicHash::Md5);
        bool ret = !md5IconsTable.contains(md5);
        qWarning("%s icon using QIcon::fromTheme() for mime type %s",
                  ret ? "GOOD" : "BAD ",
                   mimetype.name().toLatin1().constData());
        md5IconsTable.insert(md5, mimesToTest.at(counter));
    }
}


bool TestDirModel::createFileAndCheckIfIconIsExclisive(const QString& termination,
                                                       const unsigned char *content,
                                                       qint64 len)
{
    QString myFile = createFileInTempDir(QString("tst_folderlistmodel_test.") + termination,
                                         reinterpret_cast<const char*> (content),
                                         len);
    bool ret = false;
    bool triedIcon = false;
    if (!myFile.isEmpty())
    {
        QFileInfo myFileInfo(myFile);
        if (myFileInfo.exists())
        {
            triedIcon = true;
            QIcon icon = m_provider.icon(myFileInfo);
            QFile::remove(myFile);
            QByteArray  md5 = md5FromIcon(icon);
            ret = !m_md5IconsTable.contains(md5);
            qWarning("%s icon from QFileIconProvider::icon() for  %s",  ret ? "GOOD" : "QFileIconProvider::File or a BAD", qPrintable(myFile));
        }
    }
    if(!triedIcon)
    {
        qDebug() << "ERROR: could not get icon for" << myFile;
    }
    return ret;
}


//generate some files and test if there is a icon available in
void TestDirModel::fileIconProvider()
{    
    QIcon commonIcon  = m_provider.icon(QFileIconProvider::File);
    if (commonIcon.isNull())
    {
        QFAIL("No QFileIconProvider::File available");
    }

    QByteArray  commonIconMd5 = md5FromIcon(commonIcon);
    m_md5IconsTable.insert(commonIconMd5 , "commonIcon");

    //generate PDF file
    createFileAndCheckIfIconIsExclisive("mp3",  sound_44100_mp3_data, sound_44100_mp3_data_len);
    createFileAndCheckIfIconIsExclisive("pdf",  testonly_pdf_data, testonly_pdf_len);
    createFileAndCheckIfIconIsExclisive("asx",  media_asx, media_asx_len);
    createFileAndCheckIfIconIsExclisive("xspf", media_xspf, media_xspf_len);
}

#ifndef DO_NOT_USE_TAG_LIB
//generate mp3 and verify its metadata
void TestDirModel::verifyMP3Metadata()
{
    QString mp3File = createFileInTempDir("tst_folderlistmodel_for_media_read.mp3",
                                          reinterpret_cast<const char*>(sound_44100_mp3_data),
                                          sound_44100_mp3_data_len);
    QCOMPARE(mp3File.isEmpty(),   false);

    m_dirModel_01->setReadsMediaMetadata(true);
    QFileInfo fi(mp3File);

    QString title  = m_dirModel_01->getAudioMetaData(fi, DirModel::TrackTitleRole).toString();
    QString artist = m_dirModel_01->getAudioMetaData(fi, DirModel::TrackArtistRole).toString();
    QString album  = m_dirModel_01->getAudioMetaData(fi, DirModel::TrackAlbumRole).toString();
    QString year   = m_dirModel_01->getAudioMetaData(fi, DirModel::TrackYearRole).toString();
    QString track  = m_dirModel_01->getAudioMetaData(fi, DirModel::TrackNumberRole).toString();
    QString genre  = m_dirModel_01->getAudioMetaData(fi, DirModel::TrackGenreRole).toString();

    QFile::remove(mp3File);

    QCOMPARE(title,     QString("TitleTest"));
    QCOMPARE(artist,    QString("ArtistTest"));
    QCOMPARE(album,     QString("AlbumTest"));
    QCOMPARE(year,      QString::number(2013));
    QCOMPARE(track,     QString::number(99));
    QCOMPARE(genre,     QString("GenreTest"));
}
#endif


#if defined(TEST_OPENFILES)
void TestDirModel::TestDirModel::openMP3()
{
    QString mp3File = createFileInTempDir("tst_folderlistmodel_for_open.mp3",
                                          reinterpret_cast<const char*>(sound_44100_mp3_data),
                                          sound_44100_mp3_data_len);
    QCOMPARE(mp3File.isEmpty(),   false);
    QUrl mp3Url = QUrl::fromLocalFile(mp3File);
    bool ret = QDesktopServices::openUrl(mp3Url);
    if (ret)
    {
         QTest::qWait(TIME_TO_PROCESS *5);
         qWarning("GOOD: QDesktopServices::openUrl() works for MP3 files");
    }
    else {
        qWarning("BAD: QDesktopServices::openUrl() does NOT work for MP3 files");
    }
    QCOMPARE(QFile::remove(mp3File),  true);
}

void  TestDirModel::openTXT()
{
    QByteArray text("This is a test only\n");
    QString txtFile = createFileInTempDir("tst_folderlistmodel_for_open.txt",
                                          text.constData(),
                                          text.size());
    QCOMPARE(txtFile.isEmpty(),   false);
    QUrl txtUrl = QUrl::fromLocalFile(txtFile);
    bool ret = QDesktopServices::openUrl(txtUrl);
    if (ret)
    {
         QTest::qWait(TIME_TO_PROCESS *5);
         qWarning("GOOD: QDesktopServices::openUrl() works for TEXT files");
    }
    else {
         qWarning("BAD: QDesktopServices::openUrl() does NOT work for TEXT files");
    }
    QCOMPARE(QFile::remove(txtFile),  true);
}

void  TestDirModel::openPDF()
{
    QString pdfFile = createFileInTempDir("tst_folderlistmodel_for_open.pdf",
                                          reinterpret_cast<const char*>(testonly_pdf_data),
                                          testonly_pdf_len);
    QCOMPARE(pdfFile.isEmpty(),   false);
    QUrl pdfUrl = QUrl::fromLocalFile(pdfFile);
    bool ret = QDesktopServices::openUrl(pdfUrl);
    if (ret)
    {
         QTest::qWait(TIME_TO_PROCESS *5);
         ("GOOD: QDesktopServices::openUrl() works for PDF files");
    }
    else {
        qWarning("BAD: QDesktopServices::openUrl() does NOT work for PDF files");
    }
    QCOMPARE(QFile::remove(pdfFile),  true);
}
#endif


/*!
 * \brief TestDirModel::extFsWatcherChangePathManyTimesModifyAllPathsLessLast() expects no Notification
 *
 *  As the current path changes in a small interval of the time and the last path set
 *  is not modified, the signal pathModified() MUST NOT be fired.
 */
void TestDirModel::extFsWatcherChangePathManyTimesModifyAllPathsLessLast()
{
    ExternalFSWatcher  watcher;
    connect(&watcher, SIGNAL(pathModified(QString)),
            this,     SLOT(slotExtFsWatcherPathModified(QString)));

    const int items = 150;
    QVector<DeepDir *>  deepDirs;
    deepDirs.reserve(items);

    for (int counter=1; counter <= items ; ++counter)
    {
        QString dirName(QString("extModifyAllLessLast") + QString::number(counter));
        DeepDir *d =  new DeepDir(dirName,0);
        QTest::qWait(1);
        watcher.setCurrentPath(d->path());
        if (counter < items) // last dir does not receive a file
        {
            TempFiles  file;
            file.addSubDirLevel(dirName);
            file.create();
            QTest::qWait(1);
        }
        deepDirs.append(d);
    }
    QTest::qWait(TIME_TO_PROCESS);
    qDeleteAll(deepDirs);

    QCOMPARE(m_extFSWatcherPathModifiedCounter,    0);
}


/*!
 * \brief TestDirModel::extFsWatcherChangePathManyTimesModifyManyTimes
 *
 *  Change path many times, force one notification at each path
 */
void TestDirModel::extFsWatcherChangePathManyTimesModifyManyTimes()
{
    ExternalFSWatcher  watcher;
    connect(&watcher, SIGNAL(pathModified(QString)),
            this,     SLOT(slotExtFsWatcherPathModified(QString)));

    const int items = 50;
    QVector<DeepDir *>  deepDirs;
    deepDirs.reserve(items);

    for (int counter=0; counter < items ; ++counter)
    {
        QString dirName(QString("extFsWatcherChangePathManyTimesModifyManyTimes")
                        + QString::number(counter));
        DeepDir *d =  new DeepDir(dirName,0);
        watcher.setCurrentPath(d->path());
        TempFiles  file;
        file.addSubDirLevel(dirName);
        file.create(20);
        QTest::qWait(watcher.getIntervalToNotifyChanges() + 20);
        deepDirs.append(d);
    }
    QTest::qWait(TIME_TO_PROCESS);
    qDeleteAll(deepDirs);

    QCOMPARE(m_extFSWatcherPathModifiedCounter,    items);
}

/*!
 * \brief TestDirModel::extFsWatcherModifySamePathManyTimesWithInInterval() expects just one Notification
 *
 * A path is modified many times in a small time interval than ExternalFSWatcher class notifies changes,
 * so only one Notification is expected.
 */
void TestDirModel::extFsWatcherModifySamePathManyTimesWithInInterval()
{
    ExternalFSWatcher  watcher;
    connect(&watcher, SIGNAL(pathModified(QString)),
            this,     SLOT(slotExtFsWatcherPathModified(QString)));

    QString dirName("extFsWatcher_expects_just_one_signal");
    m_deepDir_01 = new DeepDir(dirName,0);
    watcher.setCurrentPath(m_deepDir_01->path());
    int  loop = 10;
    int   waitTime  = watcher.getIntervalToNotifyChanges() / loop  - 10;
    while (loop--)
    {
        TempFiles file;
        file.addSubDirLevel(dirName);
        file.create(QString("file_") + QString::number(loop), 1);
        QTest::qWait(waitTime);
    }

    QTest::qWait(TIME_TO_PROCESS);    
    QCOMPARE(m_extFSWatcherPathModifiedCounter,  1 );
}


/*!
 * \brief TestDirModel::extFsWatcherSetPathAndModifyManyTimesWithInInterval() expects just one Notification
 */
void TestDirModel::extFsWatcherSetPathAndModifyManyTimesWithInInterval()
{
    ExternalFSWatcher  watcher;
    connect(&watcher, SIGNAL(pathModified(QString)),
            this,     SLOT(slotExtFsWatcherPathModified(QString)));

    QList<DeepDir *>  deepDirs;

    int  loop = 5;
    int   waitTime  = watcher.getIntervalToNotifyChanges() / loop  - 5;

    for (int counter=1; counter <= loop ; ++counter)
    {
        QString dirName(QString("extModifyAll") + QString::number(counter));
        DeepDir *d =  new DeepDir(dirName,0);
        watcher.setCurrentPath(d->path());
        TempFiles  file;
        file.addSubDirLevel(dirName);
        file.create();
        QTest::qWait(waitTime);
        deepDirs.append(d);
    }
    QTest::qWait(TIME_TO_PROCESS);
    qDeleteAll(deepDirs);

    QCOMPARE(m_extFSWatcherPathModifiedCounter,    1);
}

#if defined(Q_OS_UNIX)
/*!
 * \brief TestDirModel::extFsWatcherNoticeChangesWithSameTimestamp()
 *
 *  Test if it gets notification from \ref ExternalFSWatcher for a file changed/created that generates the same
 *  timestamp as the last modification which had generated previous notification.
 *
 *  \note To do this the "last modification time" is forced with utimes(2) system call.
 */
void TestDirModel::extFsWatcherNoticeChangesWithSameTimestamp()
{
    bool updateAndSetModificationTime(const QString& filename, QDateTime& desiredTime);

    connect( m_dirModel_01->getExternalFSWatcher(), SIGNAL(pathModified(QString)),
            this,     SLOT(slotExtFsWatcherPathModified(QString)));

    QString dirName("extFsWatcher_generate_fileswithsameTimeStamp");
    m_deepDir_01 = new DeepDir(dirName,0);
    //create 2 files
    TempFiles  tmp1, tmp2;
    tmp1.addSubDirLevel(dirName);
    tmp2.addSubDirLevel(dirName);
    tmp1.create(QLatin1String("first"), 1);
    tmp2.create(QLatin1String("second"), 1);
    QString   firstPathName  = tmp1.createdList().at(0);
    QString   secondPathName = tmp2.createdList().at(0);

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    //make sure model has 2 files
    QCOMPARE(m_dirModel_01->rowCount(),  2) ;

    QDateTime timeStamp     = QDateTime::currentDateTime();
    QTime     time          = timeStamp.time();
    //set msecs to 0
    time.setHMS(time.hour(), time.minute(), time.second());
    timeStamp.setTime(time);

    //wait some time to have a different timestamp
    QTest::qWait(2200);

    //update first time and set its modification time to timeStamp
    bool ret = updateAndSetModificationTime(firstPathName, timeStamp);
    QCOMPARE(ret,    true);
    QFileInfo firstFile(firstPathName);
    QCOMPARE(firstFile.lastModified(),  timeStamp);

    const int maxWait   = m_dirModel_01->getExternalFSWatcher()->getIntervalToNotifyChanges() + 10;
    int counter = 0;

    //make sure ExternalFSWatcher has not notified any change
    QCOMPARE(m_extFSWatcherPathModifiedCounter,  0);
    //wait for notification on first file
    for (counter = 0; m_extFSWatcherPathModifiedCounter == 0 && counter < maxWait; ++counter)
    {
       QTest::qWait(1);
    }   
    //make sure as notification was sent, it must be one
    QCOMPARE(m_extFSWatcherPathModifiedCounter,   1);

    //now update the second file and set the mofication time the same as the first file
    //this second file is one what matters because the first change has just been notified
    ret = updateAndSetModificationTime(secondPathName, timeStamp);
    QCOMPARE(ret,    true);
    QFileInfo secondFile(secondPathName);
    QCOMPARE(secondFile.lastModified(),  timeStamp);
    for (counter = 0; m_extFSWatcherPathModifiedCounter == 1 && counter < maxWait; ++counter)
    {
       QTest::qWait(1);
    }
    //make sure as notification was sent, it must be 2
    QCOMPARE(m_extFSWatcherPathModifiedCounter,   2);

    //this comparation is not necessary since both last modification files were compared to timeStamp
    QCOMPARE(secondFile.lastModified(),  firstFile.lastModified());
}
#endif //Q_OS_UNIX


void TestDirModel::modelSingleSelection()
{
    DirSelection  *selection = m_dirModel_01->selectionObject();
    QVERIFY(selection != 0);

    connect(selection, SIGNAL(selectionChanged(int)),
            this,      SLOT(slotSelectionChanged(int)));

    connect(selection, SIGNAL(modeChanged(int)),
            this,      SLOT(slotSelectionModeChanged(int)));

    QString dirName("modelSingleSelection");
    m_deepDir_01 = new DeepDir(dirName,0);
    //create 2 files
    TempFiles  tmp1, tmp2, tmp3;
    tmp1.addSubDirLevel(dirName);
    tmp2.addSubDirLevel(dirName);
    tmp3.addSubDirLevel(dirName);
    tmp1.create(QLatin1String("01first"), 1);
    tmp2.create(QLatin1String("02second"), 1);
    tmp3.create(QLatin1String("03third"), 1);
    QString   firstFile  = tmp1.createdNames().at(0);
    QString   secondFile = tmp2.createdList().at(0);
    QString   thirdFile  = tmp3.createdNames().at(0);

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(),  3) ;

    //relative path = name in the file manager
    QCOMPARE(QFileInfo(firstFile).isAbsolute() , false);
    //using full path
    QCOMPARE(QFileInfo(secondFile).isAbsolute(), true);

    selection->setMode(DirSelection::Multi);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QVERIFY(m_selectionMode != DirSelection::Single);
    selection->setMode(DirSelection::Single);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_selectionMode, (int)DirSelection::Single);

    QModelIndex firstIdx  = m_dirModel_01->index(0, DirModel::IsSelectedRole - DirModel::FileNameRole);
    QModelIndex secondIdx = m_dirModel_01->index(1, DirModel::IsSelectedRole - DirModel::FileNameRole);
    QModelIndex thirdIdx  = m_dirModel_01->index(2, DirModel::IsSelectedRole - DirModel::FileNameRole);

    //toggle first item selection using index
    selection->toggleIndex(0);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->data(firstIdx).toBool(),   true);
    QCOMPARE(m_dirModel_01->data(secondIdx).toBool(),  false);
    QCOMPARE(m_dirModel_01->data(thirdIdx).toBool(),   false);
    QCOMPARE(m_selectedItemsCounter,                   1);

    //toggle second item selection
    selection->toggleIndex(1);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->data(firstIdx).toBool(),    false);
    QCOMPARE(m_dirModel_01->data(secondIdx).toBool(),   true);
    QCOMPARE(m_dirModel_01->data(thirdIdx).toBool(),    false);
    QCOMPARE(m_selectedItemsCounter,                    1);

    //toggle third item selection
    selection->toggleIndex(2);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->data(firstIdx).toBool(),    false);
    QCOMPARE(m_dirModel_01->data(secondIdx).toBool(),   false);
    QCOMPARE(m_dirModel_01->data(thirdIdx).toBool(),    true);
    QCOMPARE(m_selectedItemsCounter,                    1);

    selection->selectAll();
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_selectedItemsCounter,                    3);

    //all items are selected, if one item is unselected, other 2 items remain selected
    selection->toggleIndex(0);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->data(firstIdx).toBool(),   false);
    QCOMPARE(m_dirModel_01->data(secondIdx).toBool(),  true);
    QCOMPARE(m_dirModel_01->data(thirdIdx).toBool(),   true);
    QCOMPARE(m_selectedItemsCounter,                   2);

    //compare results
    QList<int>  selectedIndexes = selection->selectedIndexes();
    QCOMPARE(selectedIndexes.count(),   2);
    QCOMPARE(selectedIndexes.at(0),     1);
    QCOMPARE(selectedIndexes.at(1),     2);
    QStringList selectedNames = selection->selectedNames();
    QCOMPARE(selectedNames.count(),     2);
      // as secondFile is absolute path
    QCOMPARE(selectedNames.at(0),       QFileInfo(secondFile).fileName());
    QCOMPARE(selectedNames.at(1),       thirdFile);
    QStringList pathNames  = selection->selectedAbsFilePaths();
    QCOMPARE(pathNames.count(),         2);
    QCOMPARE(pathNames.at(0),           tmp2.createdList().first());
    QCOMPARE(pathNames.at(1),           tmp3.createdList().first());

    //now as the first item is unselected, toggle
    selection->toggleIndex(0);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->data(firstIdx).toBool(),   true);
    QCOMPARE(m_dirModel_01->data(secondIdx).toBool(),  false);
    QCOMPARE(m_dirModel_01->data(thirdIdx).toBool(),   false);
    QCOMPARE(m_selectedItemsCounter,                   1);
}


void TestDirModel::modelMultiSelection()
{
    DirSelection  *selection = m_dirModel_01->selectionObject();
    QVERIFY(selection != 0);

    connect(selection, SIGNAL(selectionChanged(int)),
            this,      SLOT(slotSelectionChanged(int)));

    connect(selection, SIGNAL(modeChanged(int)),
            this,      SLOT(slotSelectionModeChanged(int)));

    QString dirName("modelMultiSelection");
    const int createdFiles = 10;

    m_deepDir_01 = new DeepDir(dirName,0);
    TempFiles  tmpFiles;
    tmpFiles.addSubDirLevel(dirName);
    tmpFiles.create(createdFiles);

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(),  createdFiles) ;

    selection->setMode(DirSelection::Multi);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(selection->mode(), DirSelection::Multi);

    QStringList createdNames   = tmpFiles.createdNames();
    QList<int> handledIndexes;
    handledIndexes << 0 << 3 << 5 << 8 << 4;
    int counter = handledIndexes.count();

    QCOMPARE(m_selectedItemsCounter,         0);
    while (counter--)
    {
        int item = handledIndexes.at(counter);
        selection->setIndex(item, true);
        QTest::qWait(10);
    }
    QCOMPARE(m_selectedItemsCounter,     handledIndexes.count());

    counter = handledIndexes.count();
    while (counter--)
    {
         int item = handledIndexes.at(counter);
         QModelIndex index =  m_dirModel_01->index(item,
                                 DirModel::IsSelectedRole - DirModel::FileNameRole);
         QCOMPARE(m_dirModel_01->data(index).toBool(),   true);
    }

    //remove selected Items
    m_dirModel_01->removeIndex(handledIndexes.takeAt(2));
    QTest::qWait(TIME_TO_PROCESS);
    m_dirModel_01->removeIndex(handledIndexes.takeLast());
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(m_selectedItemsCounter,     handledIndexes.count());

    selection->selectAll();
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_selectedItemsCounter,                    m_dirModel_01->rowCount());

    selection->clear();
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_selectedItemsCounter,                    0);
}


void TestDirModel::modelSelectionItemsRange()
{
    QString dirName("modelSelectionItemsRange");
    m_deepDir_01 = new DeepDir(dirName,0);
    TempFiles  tmpFiles;
    const int createdFiles = 11;
    tmpFiles.addSubDirLevel(dirName);
    tmpFiles.create(createdFiles);

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(),  createdFiles);

    DirSelection  *selection = m_dirModel_01->selectionObject();
    QVERIFY(selection != 0);

    connect(selection, SIGNAL(selectionChanged(int)),
            this,      SLOT(slotSelectionChanged(int)));


    selection->setIndex(3, true);
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(m_selectedItemsCounter,     1);

    //try to do range with same index (expected nothing to happen)
    selection->selectRange(3);
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(m_selectedItemsCounter,     1);

    //try item under selected item
    selection->selectRange(8);
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(m_selectedItemsCounter,     6);

    //try item above selected item
    selection->selectRange(0);
    QTest::qWait(TIME_TO_PROCESS);
    QCOMPARE(m_selectedItemsCounter,     9);
}

void TestDirModel::trashDiretories()
{
    QTrashDir  trash;
    QString dirName("trashDiretories");
    m_deepDir_01 = new DeepDir(dirName,0);

   // there is not .Trash dir
   QCOMPARE(trash.isMountPointSharedWithStickBit(dirName),  false);

   //create .Trash in it
   QString trashDir(m_deepDir_01->path() + QDir::separator() + ".Trash");
   QCOMPARE(QDir().mkpath(trashDir),           true);


   //still faling
   QCOMPARE(trash.isMountPointSharedWithStickBit(dirName),  false);

   //turn stick bit on
   QString cmd("chmod +t " + trashDir);
   QCOMPARE(system(cmd.toLatin1().constData()),   0);
   QCOMPARE(trash.isMountPointSharedWithStickBit((m_deepDir_01->path())),  true);

   //test validate Trash Dir()
   QCOMPARE(trash.validate(trashDir, false),  false);

   //test   permissions
   QFile f(trashDir);
   bool permOk = f.setPermissions(QFile::ReadOwner  |
                                  QFile::WriteOwner |
                                  QFile::ExeOwner);
   QCOMPARE(permOk,    true);
   QCOMPARE(trash.checkUserDirPermissions(trashDir),  true);

   //create files in Trash
   QCOMPARE(trash.createUserDir(QTrashUtilInfo::filesTrashDir(trashDir)),  true);
   //create info in Trash
   QCOMPARE(trash.createUserDir(QTrashUtilInfo::infoTrashDir(trashDir)),  true);
   //test validate Trash Dir(), now it MUST pass
   QCOMPARE(trash.validate(trashDir, false),  true);

   //invalidate Trash dir
   QFile *f1 = new QFile(trashDir) ;
   permOk  =  f1->setPermissions(QFile::ReadOwner  | QFile::ReadOther |
                               QFile::WriteOwner   | QFile::WriteGroup|
                               QFile::ExeOwner);
   QCOMPARE(permOk,    true);
   f1->flush();
   delete f1;
   QCOMPARE(trash.validate(trashDir, false),  false);

   QString xdgTrash("xdg_Trash");
   m_deepDir_02 = new DeepDir(xdgTrash,0);

   if (::qgetenv("XDG_DATA_HOME").size() == 0)
   {
       QString myTrash(QDir::homePath() + "/.local/share/Trash");
       QCOMPARE(trash.homeTrash(), myTrash);
   }

   //test XDG Home Trash
   ::setenv("XDG_DATA_HOME", m_deepDir_02->path().toLatin1().constData(), true );
   QString xdgTrashDir(trash.homeTrash());
   QCOMPARE(trash.validate(xdgTrashDir, false),  true);
   QCOMPARE(trash.homeTrash() , xdgTrashDir);

   QCOMPARE(trash.getMountPoint(QDir::rootPath()), QDir::rootPath());  

   QStringList mountedPoints = trash.mountedPoints();
   foreach (const QString& mp, mountedPoints)
   {
        //attemp to create a temporary folder inside the mount point
        QTemporaryDir  dir(mp + QDir::separator() );
        if (dir.isValid())
        {
           QFileInfo fiDir(dir.path());
           if (fiDir.exists())
           {
              qDebug() << "temp dir" << dir.path();
              QCOMPARE(trash.getMountPoint(fiDir.canonicalFilePath()), mp);
              //creates a temporary file inside temporary dir
              QTemporaryFile file (fiDir.canonicalFilePath() + QDir::separator());
              QCOMPARE(file.open(),   true);
              file.close();
              QCOMPARE(trash.getMountPoint(file.fileName()),   mp);
          }
        }
   }

   //file under home
   QTemporaryFile homeFile (QDir::homePath() + QDir::separator());
   QCOMPARE(homeFile.open(),     true);
   homeFile.close();
   QString        homeTrashDir = trash.suitableTrash(homeFile.fileName());
   QCOMPARE(homeTrashDir,          trash.homeTrash());

   //home
   homeTrashDir = trash.suitableTrash(QDir::homePath());
   QVERIFY(homeTrashDir != trash.homeTrash());

   //file under home Trash
   QTemporaryFile cannotMoveTotrash(trash.homeTrash() + QDir::separator());
   QCOMPARE(cannotMoveTotrash.open(),   true);
   cannotMoveTotrash.close();
   QCOMPARE(trash.suitableTrash(cannotMoveTotrash.fileName()).isEmpty(),   true);

   QStringList trashes =  trash.allTrashes();
   qWarning() << "return from allTrashes():" << trashes;

   QVERIFY(trashes.count() > 0);

   qDebug() << "QStandardPaths::CacheLocation"  << QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
   qDebug() << "QStandardPaths::DataLocation" << QStandardPaths::standardLocations(QStandardPaths::DataLocation);
   qDebug() << "QStandardPaths::GenericCacheLocation" << QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation);
   qDebug() << "QStandardPaths::GenericDataLocation" << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
}


void TestDirModel::locationFactory()
{
    LocationsFactory factoryLocations(this);
    const Location *location = 0;

    QString validTrashURL(LocationUrl::TrashRootURL);

   //Due to current File Manager UI typing method both: "file:" and "trash:" are supported
   // location = factoryLocations.setNewPath("trash:");
   // QVERIFY(location == 0);

    location = factoryLocations.setNewPath("trash:/");
    QVERIFY(location);
    QVERIFY(location->type() == LocationsFactory::TrashDisk);
    QCOMPARE(location->info()->absoluteFilePath(),   validTrashURL);
    QCOMPARE(location->urlPath(), validTrashURL);
    QCOMPARE(location->isRoot(), true);

    location = factoryLocations.setNewPath("trash://");
    QVERIFY(location);
    QVERIFY(location->type() == LocationsFactory::TrashDisk);
    QCOMPARE(location->info()->absoluteFilePath(),   validTrashURL);
    QCOMPARE(location->urlPath(), validTrashURL);
    QCOMPARE(location->isRoot(), true);

    location = factoryLocations.setNewPath("trash:///");
    QVERIFY(location);
    QVERIFY(location->type() == LocationsFactory::TrashDisk);
    QCOMPARE(location->info()->absoluteFilePath(),   validTrashURL);
    QCOMPARE(location->urlPath(), validTrashURL);
    QCOMPARE(location->isRoot(), true);

    location = factoryLocations.setNewPath("trash://////");
    QVERIFY(location);
    QVERIFY(location->type() == LocationsFactory::TrashDisk);
    QCOMPARE(location->info()->absoluteFilePath(),   validTrashURL);
    QCOMPARE(location->urlPath(), validTrashURL);
    QCOMPARE(location->isRoot(), true);

    QString myDir("___myDir_must_NOT_EXIST___");
    location = factoryLocations.setNewPath(QString("trash:/") + myDir);
    QVERIFY(location == 0);

    location = factoryLocations.setNewPath("file://////");
    QVERIFY(location);
    QVERIFY(location->type() == LocationsFactory::LocalDisk);
    QCOMPARE(location->info()->absoluteFilePath(),   QDir::rootPath());
    QCOMPARE(location->urlPath(), QDir::rootPath());
    QCOMPARE(location->isRoot(), true);

    location = factoryLocations.setNewPath("/");
    QVERIFY(location);
    QVERIFY(location->type() == LocationsFactory::LocalDisk);
    QCOMPARE(location->info()->absoluteFilePath(),   QDir::rootPath());
    QCOMPARE(location->urlPath(), QDir::rootPath());
    QCOMPARE(location->isRoot(), true);

    location = factoryLocations.setNewPath("//");
    QVERIFY(location);
    QVERIFY(location->type() == LocationsFactory::LocalDisk);
    QCOMPARE(location->info()->absoluteFilePath(),   QDir::rootPath());
    QCOMPARE(location->urlPath(), QDir::rootPath());
    QCOMPARE(location->isRoot(), true);

    location = factoryLocations.setNewPath("//bin");
    QVERIFY(location);
    QVERIFY(location->type() == LocationsFactory::LocalDisk);
    QCOMPARE(location->info()->absoluteFilePath(),   QLatin1String("/bin"));
    QCOMPARE(location->urlPath(), QLatin1String("/bin"));
    QCOMPARE(location->isRoot(), false);

    QTrashDir  trash;
    QString dirName("trashDirectory");
    m_deepDir_01 = new DeepDir(dirName,0);

    //create a Trash to have

    ::setenv("XDG_DATA_HOME", m_deepDir_01->path().toLatin1().constData(), true );
    QString xdgTrashDir(trash.homeTrash());
    QCOMPARE(trash.validate(xdgTrashDir, true),  true);
    QCOMPARE(trash.homeTrash() , xdgTrashDir);

    QString trash3("trash:///Dir1/Dir2/Dir3");
    QString trash2("trash:///Dir1/Dir2");
    QString trash1("trash:///Dir1");

    QCOMPARE(QDir().mkpath(QTrashUtilInfo::filesTrashDir(xdgTrashDir) + "/Dir1/Dir2/Dir3"), true);


    //create a empty .trashinfo file to validate the Trash
    QFile trashinfo (QTrashUtilInfo::infoTrashDir(xdgTrashDir) + "/Dir1.trashinfo");
    QCOMPARE(trashinfo.open(QFile::WriteOnly),  true);
    trashinfo.close();

    location =  factoryLocations.setNewPath(trash3);
    Location * myLocation = const_cast<Location*> (location);
    QCOMPARE(myLocation->becomeParent(),   true);
    QCOMPARE(location->urlPath(),  trash2);
    QCOMPARE(location->isRoot(), false);

    location =  factoryLocations.setNewPath(trash2);
    myLocation = const_cast<Location*> (location);
    QCOMPARE(myLocation->becomeParent(),   true);
    QCOMPARE(location->urlPath(),  trash1);
    QCOMPARE(location->isRoot(), false);

    myLocation = const_cast<Location*> (location);
    QCOMPARE(myLocation->becomeParent(),  true);
    QCOMPARE(location->urlPath(),  LocationUrl::TrashRootURL);
    QCOMPARE(location->isRoot(), true);
}



void TestDirModel::moveOneFileToTrashAndRestore()
{
    QString orig("moveFilesTrash");
    m_deepDir_01  = new DeepDir(orig, 0);
    const int createdFiles = 4;
    TempFiles tempfiles;
    tempfiles.addSubDirLevel(orig);
    tempfiles.create(createdFiles);

    m_dirModel_01->setPath(m_deepDir_01->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(), createdFiles);

    QString tempTrash("tempTrashDir");
    m_deepDir_02  = new DeepDir(tempTrash, 0);
    createTempHomeTrashDir(m_deepDir_02->path());

    // move item to Trash
    m_dirModel_01->moveIndexToTrash(0);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_01->rowCount(), createdFiles -1);

    //use another DirModel instance and point it to Trash
    m_dirModel_02->goTrash();
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_02->rowCount(), 1);

    // now restore from Trash
    m_dirModel_02->restoreIndexFromTrash(0);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_02->rowCount(), 0);

    //using refresh to not depend from External File System Watcher
    if (m_dirModel_01->rowCount() != createdFiles)
    {
        qWarning("using refresh() external File System Watcher did not get it right");
        m_dirModel_01->refresh();
        QTest::qWait(TIME_TO_REFRESH_DIR);
    }
    QCOMPARE(m_dirModel_01->rowCount(), createdFiles);
}


void TestDirModel::restoreTrashWithMultipleSources()
{
    DeepDir d_01("folder_01", 0);
    DeepDir d_02("folder_02", 0);
    DeepDir d_03("folder_03", 0);

    //create a temp trash
    QString tempTrash("tempTrashDir");
    m_deepDir_02  = new DeepDir(tempTrash, 0);
    createTempHomeTrashDir(m_deepDir_02->path());

    const int dirsCounter = 3;
    DeepDir * dirs [dirsCounter]  = {&d_01, &d_02, &d_03};
    int counter = 0;

    //move items from different sources to trash
    for(counter = 0; counter < dirsCounter; counter++)
    {
        TempFiles  files;
        files.addSubDirLevel(dirs[counter]->path());
        QString name = QString("file_from_dir%1.txt").arg(counter);
        files.create(name, 1);
        DirModel model;
        model.setPath(dirs[counter]->path());
        QTest::qWait(TIME_TO_REFRESH_DIR);
        QCOMPARE(model.rowCount(), 1);
        model.selectionObject()->selectAll();
        model.moveSelectionToTrash();
        QTest::qWait(TIME_TO_REFRESH_DIR);
        QCOMPARE(model.rowCount(), 0);
    }

    //another model points to temporary Trash
    connect(m_dirModel_02, SIGNAL(error(QString,QString)),
            this,          SLOT(slotError(QString,QString)));
    m_dirModel_02->goTrash();
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_02->rowCount(), dirsCounter);
    m_dirModel_02->restoreTrash();
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_02->rowCount(), 0);

    //now look into info directories from trash, check if it is empty
    m_dirModel_02->setPath(m_deepDir_02->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_02->rowCount(), 1);
    QCOMPARE(m_dirModel_02->openPath("Trash"),    true);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_02->openPath("info"),    true);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_02->rowCount(),  0);

    //test it items were moved back to their sources
    for(counter = 0; counter < dirsCounter; counter++)
    {
        DirModel model;
        model.setPath(dirs[counter]->path());
        QTest::qWait(TIME_TO_REFRESH_DIR);
        QCOMPARE(model.rowCount(), 1);
    }

    QCOMPARE(m_receivedErrorSignal,   false);
}


void TestDirModel::emptyTrash()
{
    DeepDir d_01("folder_01", 0);
    DeepDir d_02("folder_02", 0);
    DeepDir d_03("folder_03", 0);

    //create a temp trash
    QString tempTrash("tempTrashDir");
    m_deepDir_02  = new DeepDir(tempTrash, 0);
    createTempHomeTrashDir(m_deepDir_02->path());

    const int dirsCounter = 3;
    DeepDir * dirs [dirsCounter]  = {&d_01, &d_02, &d_03};
    int counter = 0;

    //move items from different sources to trash
    for(counter = 0; counter < dirsCounter; counter++)
    {
        TempFiles  files;
        files.addSubDirLevel(dirs[counter]->path());
        QString name = QString("file_from_dir%1.txt").arg(counter);
        files.create(name, 1);
        DirModel model;
        model.setPath(dirs[counter]->path());
        QTest::qWait(TIME_TO_REFRESH_DIR);
        QCOMPARE(model.rowCount(), 1);
        model.selectionObject()->selectAll();
        model.moveSelectionToTrash();
        QTest::qWait(TIME_TO_REFRESH_DIR);
        QCOMPARE(model.rowCount(), 0);
    }

    //another model points to temporary Trash
    connect(m_dirModel_02, SIGNAL(error(QString,QString)),
            this,          SLOT(slotError(QString,QString)));
    m_dirModel_02->goTrash();
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_02->rowCount(), dirsCounter);
    m_dirModel_02->emptyTrash();
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_02->rowCount(), 0);

    //now look into info directories from trash, check if it is empty
    m_dirModel_02->setPath(m_deepDir_02->path());
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_02->rowCount(), 1);
    QCOMPARE(m_dirModel_02->openPath("Trash"),    true);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_02->openPath("info"),    true);
    QTest::qWait(TIME_TO_REFRESH_DIR);
    QCOMPARE(m_dirModel_02->rowCount(),  0);

    //test if original items folder is still empty
    for(counter = 0; counter < dirsCounter; counter++)
    {
        DirModel model;
        model.setPath(dirs[counter]->path());
        QTest::qWait(TIME_TO_REFRESH_DIR);
        QCOMPARE(model.rowCount(), 0);
    }

    QCOMPARE(m_receivedErrorSignal,   false);
}



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TestDirModel tc;
    QStringList args = QCoreApplication::arguments();
    int count = args.count();
    while (count--)
    {
       //Ubuntu Touch parameter
       if (args.at(count).startsWith(QLatin1String("--desktop_file_hint")))
       {
          args.removeAt(count);
          break;
       }
    }
    return  QTest::qExec(&tc, args);
}




QByteArray md5FromIcon(const QIcon& icon)
{
    QByteArray ret;
    if (!icon.isNull())
    {
       QPixmap pix = icon.pixmap(QSize(48,48));
       QImage image = pix.toImage();
       if (!image.isNull())
       {
           const uchar *bits = image.bits();
           const char *bytes = reinterpret_cast<const char*> (bits);
           QByteArray bytesArray(bytes, image.byteCount());
           ret = QCryptographicHash::hash(bytesArray, QCryptographicHash::Md5);
       }
    }
    return ret;
}

QString createFileInTempDir(const QString& name, const char *content, qint64 size)
{
    QString ret;
    QString fullName(QDir::tempPath() + QDir::separator() + name);
    QFile file(fullName);
    if (file.open(QFile::WriteOnly))
    {
        if (file.write(content, size) == size)
        {
            ret = fullName;
        }
        file.close();
    }
    return ret;
}


bool TestDirModel::createTempHomeTrashDir(const QString& existentDir)
{
    QDir d(existentDir);
    bool ret = false;
    if (existentDir.startsWith(QDir::tempPath()) && (d.exists() || d.mkpath(existentDir)))
    {
        QTrashDir trash;
        ::setenv("XDG_DATA_HOME", existentDir.toLatin1().constData(), true );
        QString xdgTrashDir(trash.homeTrash());
        ret = trash.validate(xdgTrashDir, true);
    }
    return ret;
}

#if defined(Q_OS_UNIX)
/*!
 * \brief updateAndSetModificationTime()
 *        updates the file content and sets its last modification time to another time
 * \param filename
 * \param desiredTime   some time less than current time
 * \return true if it could be performed, false otherwise
 */
bool updateAndSetModificationTime(const QString& filename, QDateTime& desiredTime)
{
    bool ret = false;
    QFile f(filename);
    if (f.open(QFile::Append))
    {
        QByteArray data("1234");
        if (f.write(data) == (qint64)data.size())
        {
            f.close();
            struct timeval times[2] =
            {
              {(long)desiredTime.toTime_t(), (long)desiredTime.time().msec()},
              {(long)desiredTime.toTime_t(), (long)desiredTime.time().msec()}
            };
            QFileInfo info(filename);
            qDebug() << "last modification of" << info.fileName() << info.lastModified()
                     << "forcing it to" << desiredTime;
            if (::utimes( QFile::encodeName(filename).constData(), times) == 0)
            {
                ret = true;
            }
        }
    }
    return ret;
}
#endif //Q_OS_UNIX

#include "tst_folderlistmodel.moc"
