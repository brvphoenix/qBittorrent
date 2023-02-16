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

#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QHash>
#include <QTextStream>

#include "base/global.h"
#include "base/logger.h"
#include "base/utils/fs.h"
#include "base/utils/gzip.h"

namespace
{
    const std::chrono::seconds FLUSH_INTERVAL {2};

    bool isObsolete(const QFileInfo &info, int ageType, int age)
    {
        QDateTime modificationDate = info.lastModified();
        switch (static_cast<FileLogger::FileLogAgeType>(ageType))
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

    bool compressWrapper(const Path &before, const Path &after, int level, QString &msg)
    {
        const qsizetype chunkSize = 512 * 1024; // Bytes

        QFileInfo info(before.data());
        const QDateTime atime = info.lastRead();
        const QDateTime mtime = info.lastModified();
        // It seems the created time can't be modified on UNIX.
        const QDateTime ctime = info.birthTime();
        const QDateTime mctime = info.metadataChangeTime();

        QFile source(before.data());
        if (!source.open(QIODevice::ReadOnly))
        {
            msg = QObject::tr("Can't open %1!").arg(before.data());
            return false;
        }
        QDataStream in(&source);

        QFile dest(after.data());
        if (!dest.open(QIODevice::WriteOnly | QIODevice::NewOnly))
        {
            msg = QObject::tr("Can't open %1!").arg(after.data());
            return false;
        }
        QDataStream out(&dest);

        bool ok = true;
        while (in.status() == QDataStream::Ok && ok)
        {
            QByteArray buffer(chunkSize, 0);
            qsizetype bytes = in.readRawData(buffer.data(), chunkSize);
            if (in.atEnd())
                buffer.truncate(bytes);
            const QByteArray data = Utils::Gzip::compress(buffer, level, &ok);

            if (!ok)
                msg = QObject::tr("Can't compress %1!").arg(before.data());

            if (out.writeRawData(data.data(), data.size()) == -1)
            {
                msg = QObject::tr("Can't save to %1!").arg(after.data());
                ok = false;
            }
        }

        source.close();
        dest.close();

        if (ok)
        {
            // Change the file's timestamp.
            dest.open(QIODevice::ReadOnly);
            dest.setFileTime(atime, QFileDevice::FileAccessTime);
            dest.setFileTime(mtime, QFileDevice::FileModificationTime);
            dest.setFileTime(ctime, QFileDevice::FileBirthTime);
            dest.setFileTime(mctime, QFileDevice::FileMetadataChangeTime);
            dest.close();
        }
        else
        {
            Utils::Fs::removeFile(after);
        }
        return ok;
    }
}

FileLogger::FileLogger(IApplication *IApp)
    : ApplicationComponent(IApp)
{
    m_flusher.setInterval(FLUSH_INTERVAL);
    m_flusher.setSingleShot(true);
    connect(&m_flusher, &QTimer::timeout, this, &FileLogger::flushLog);

    changePath(IApp->fileLoggerPath());

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

    if (isObsolete(QFileInfo {m_path.data()}, app()->fileLoggerAgeType(), app()->fileLoggerAge()))
    {
        Utils::Fs::removeFile(m_path);
        sortBackup();
    }
    else
    {
        if (app()->isFileLoggerBackup() && (m_logFile.size() >= app()->fileLoggerMaxSize()))
        {
            makeBackup();
        }
    }

    openLogFile();
}

void FileLogger::makeBackup()
{
    sortBackup(2);
    Path renameFrom = m_path;
    Path renameTo = m_path + u".bak1"_qs;
    if (app()->isFileLoggerCompressed())
    {
        renameFrom = m_path + u".1.gz."_qs + QString::number(QDateTime::currentSecsSinceEpoch(), 36);
        QString err;
        if (!compressWrapper(m_path, renameFrom, 6, err))
        {
            renameFrom = m_path;
            app()->setFileLoggerCompressed(false);
            qWarning() << err;
        }
        else
        {
            renameTo = m_path + u".1.gz"_qs;
            Utils::Fs::removeFile(m_path);
        }
    }

    sortBackup(2);
    Utils::Fs::renameFile(renameFrom, renameTo);
}

void FileLogger::sortBackup(const int startAt) const
{
    const QString extension = app()->isFileLoggerCompressed() ? u".%1.gz"_qs : u".bak%1"_qs;
    const QFileInfoList fileList = QDir(m_path.parentPath().data()).entryInfoList(
                    QStringList { u"qbittorrent.log"_qs + extension.arg(u"*"_qs) },
                    (QDir::Files | QDir::Writable), (QDir::Time | QDir::Reversed));
    const QString tmpSuffix = u'.' + QString::number(QDateTime::currentSecsSinceEpoch(), 36);

    QHash<Path, Path> mappings;
    int fileCount = fileList.size() + startAt;
    bool skipCheck = false;

    for (const QFileInfo &file : fileList)
    {
        if (!skipCheck && app()->isFileLoggerDeleteOld())
        {
            if (!isObsolete(file, app()->fileLoggerAgeType(), app()->fileLoggerAge()))
                skipCheck = true;
            else
            {
                Utils::Fs::removeFile(Path(file.absoluteFilePath()));
                --fileCount;
                continue;
            }
        }

        const Path oldName {file.absoluteFilePath()};
        const Path newName = m_path + extension.arg(--fileCount);
        if (oldName == newName)
            continue;
        else if (!newName.exists())
            Utils::Fs::renameFile(oldName, newName);
        else
        {
            Utils::Fs::renameFile(oldName, newName + tmpSuffix);
            mappings.insert(newName + tmpSuffix, newName);
        }
    }

    for (auto it = mappings.cbegin(); it != mappings.cend(); ++it)
        Utils::Fs::renameFile(it.key(), it.value());
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

    if (app()->isFileLoggerBackup() && (m_logFile.size() >= app()->fileLoggerMaxSize()))
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
