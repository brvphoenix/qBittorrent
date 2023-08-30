/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2016  sledgehammer999 <hammered999@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 */

#include "filelogger.h"

#include <chrono>
#include <functional>

#include <QtGlobal>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QIODevice>
#include <QRunnable>
#include <QTextStream>
#include <QThreadPool>

#include "base/global.h"
#include "base/logger.h"
#include "base/utils/fs.h"
#include "base/utils/gzip.h"

namespace
{
    const std::chrono::seconds FLUSH_INTERVAL {2};

    Path handleBackups(const Path &baseName, const Path &renameFrom, const bool compressed)
    {
        Path renameTo = baseName + u".bak"_qs + (compressed ? u".gz"_qs :  u""_qs);
        int counter = 0;

        while(renameTo.exists())
        {
            renameTo = baseName + u".bak"_qs + QString::number(++counter) + (compressed ? u".gz"_qs :  u""_qs);
        }

        Utils::Fs::renameFile(renameFrom, renameTo);
        return renameTo;
    }

    bool isObsolete(const QFileInfo &info, FileLogger::FileLogAgeType ageType, int age)
    {
        QDateTime modificationDate = info.lastModified();
        switch (ageType)
        {
        case FileLogger::DAYS:
            modificationDate = modificationDate.addDays(age);
            break;
        case FileLogger::MONTHS:
            modificationDate = modificationDate.addMonths(age);
            break;
        default:
            modificationDate = modificationDate.addYears(age);
        }
        return modificationDate <= QDateTime::currentDateTime();
    }

    class CmpressTask final : public QRunnable
    {
        Q_DISABLE_COPY_MOVE(CmpressTask)

    public:
        CmpressTask(const Path &p, const std::function<Path (const Path &, const bool)> callback)
            : m_path {p}
            , m_callback {callback}
        {}

        void run() override
        {
            Path destPath {m_path + u"."_qs + QString::number(QDateTime::currentSecsSinceEpoch(), 36) + u".gz"_qs};
            QString err;
            if (compressBackupFile(m_path, destPath, 6, err))
            {
                if (!(err.isEmpty()))
                    qDebug() << u"Error: "_qs << err;

                Utils::Fs::removeFile(m_path);
                m_callback(destPath, true);
            }
        }

    private:
        bool compressBackupFile(const Path &sourcePath, const Path &destPath, int level, QString &msg)
        {
            QFile source {sourcePath.data()};
            const QDateTime atime = source.fileTime(QFileDevice::FileAccessTime);
            const QDateTime mtime = source.fileTime(QFileDevice::FileModificationTime);
            // It seems the created time can't be modified on UNIX.
            const QDateTime ctime = source.fileTime(QFileDevice::FileBirthTime);
            const QDateTime mctime = source.fileTime(QFileDevice::FileMetadataChangeTime);

            if (!source.open(QIODevice::ReadOnly))
            {
                msg = FileLogger::tr("Can't open %1!").arg(sourcePath.data());
                return false;
            }

            QFile dest {destPath.data()};
            if (!dest.open(QIODevice::WriteOnly | QIODevice::NewOnly))
            {
                msg = QObject::tr("Can't open %1!").arg(destPath.data());
                return false;
            }

            const bool ok = Utils::Gzip::compress(source, dest, level);
            source.close();
            dest.close();

            if (ok)
            {
                // Change the file's timestamp.
                dest.open(QIODevice::Append);
                dest.setFileTime(atime, QFileDevice::FileAccessTime);
                dest.setFileTime(mtime, QFileDevice::FileModificationTime);
                dest.setFileTime(ctime, QFileDevice::FileBirthTime);
                dest.setFileTime(mctime, QFileDevice::FileMetadataChangeTime);
                dest.close();
            }
            else
            {
                Utils::Fs::removeFile(destPath);
            }
            return ok;
        }

        Path m_path;
        const std::function<Path (const Path &, const bool)> m_callback;
    };
}

FileLogger::FileLogger(const Path &path, const bool backup
                       , const int maxSize, const bool deleteOld, const int age
                       , const FileLogAgeType ageType, bool compressBackups)
    : m_age(age)
    , m_ageType(ageType)
    , m_backup(backup)
    , m_compressBackups(compressBackups)
    , m_deleteOld(deleteOld)
    , m_maxSize(maxSize)
{
    m_flusher.setInterval(FLUSH_INTERVAL);
    m_flusher.setSingleShot(true);
    connect(&m_flusher, &QTimer::timeout, this, &FileLogger::flushLog);

    changePath(path);

    const Logger *const logger = Logger::instance();
    for (const Log::Msg &msg : asConst(logger->getMessages()))
        addLogMessage(msg);

    connect(logger, &Logger::newLogMessage, this, &FileLogger::addLogMessage);
}

FileLogger::~FileLogger()
{
    closeLogFile();
}

void FileLogger::changePath(const Path &newPath)
{
    // compare paths as strings to perform case sensitive comparison on all the platforms
    if (newPath.data() == m_path.parentPath().data())
        return;

    closeLogFile();

    m_path = newPath / Path(u"qbittorrent.log"_qs);
    m_logFile.setFileName(m_path.data());

    Utils::Fs::mkpath(newPath);

    if (m_deleteOld)
    {
        deleteOld();
    }

    if (isObsolete(QFileInfo(m_path.data()), m_ageType, m_age))
    {
        Utils::Fs::removeFile(m_path);
    }
    else
    {
        if (m_backup && (m_logFile.size() >= m_maxSize))
        {
            makeBackup();
        }
    }

    openLogFile();
}

void FileLogger::makeBackup()
{
    Path renameTo = handleBackups(m_path, m_path, false);

    if (m_compressBackups)
    {
        using namespace std::placeholders;
        CmpressTask *task = new CmpressTask(renameTo, std::bind(handleBackups, m_path, _1, _2));
        QThreadPool::globalInstance()->start(task);
    }
}

void FileLogger::deleteOld()
{
    const QStringList nameFilter { u"qbittorrent.log.bak*"_qs + (m_compressBackups ? u".gz"_qs : u""_qs) };
    const QFileInfoList fileList = QDir(m_path.parentPath().data()).entryInfoList(
            nameFilter, (QDir::Files | QDir::Writable), (QDir::Time | QDir::Reversed));

    for (const QFileInfo &file : fileList)
    {
        if (isObsolete(file, m_ageType, m_age))
        {
            Utils::Fs::removeFile(Path(file.absoluteFilePath()));
        }
        else
        {
            break;
        }
    }
}

void FileLogger::setAge(const int value)
{
    m_age = value;
}

void FileLogger::setAgeType(const FileLogAgeType value)
{
    m_ageType = value;
}

void FileLogger::setBackup(const bool value)
{
    m_backup = value;
}

void FileLogger::setCompressBackups(const bool value)
{
    m_compressBackups = value;
}

void FileLogger::setDeleteOld(const bool value)
{
    m_deleteOld = value;
}

void FileLogger::setMaxSize(const int value)
{
    m_maxSize = value;
}

void FileLogger::addLogMessage(const Log::Msg &msg)
{
    if (!m_logFile.isOpen()) return;

    QTextStream stream(&m_logFile);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    stream.setCodec("UTF-8");
#endif

    switch (msg.type)
    {
    case Log::INFO:
        stream << QStringView(u"(I) ");
        break;
    case Log::WARNING:
        stream << QStringView(u"(W) ");
        break;
    case Log::CRITICAL:
        stream << QStringView(u"(C) ");
        break;
    default:
        stream << QStringView(u"(N) ");
    }

    stream << QDateTime::fromSecsSinceEpoch(msg.timestamp).toString(Qt::ISODate) << QStringView(u" - ") << msg.message << QChar(u'\n');

    if (m_deleteOld)
    {
        deleteOld();
    }

    if (m_backup && (m_logFile.size() >= m_maxSize))
    {
        closeLogFile();
        makeBackup();
        openLogFile();
    }
    else
    {
        if (!m_flusher.isActive())
            m_flusher.start();
    }
}

void FileLogger::flushLog()
{
    if (m_logFile.isOpen())
        m_logFile.flush();
}

void FileLogger::openLogFile()
{
    if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)
        || !m_logFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner))
    {
        m_logFile.close();
        LogMsg(tr("An error occurred while trying to open the log file. Logging to file is disabled."), Log::CRITICAL);
    }
}

void FileLogger::closeLogFile()
{
    m_flusher.stop();
    m_logFile.close();
}

#include "filelogger.moc"
