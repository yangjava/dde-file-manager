/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dmimedatabase.h"
#include "shutil/fileutils.h"

#include <QFileInfo>

DFM_BEGIN_NAMESPACE

DMimeDatabase::DMimeDatabase()
{

}

QMimeType DMimeDatabase::mimeTypeForFile(const QString &fileName, QMimeDatabase::MatchMode mode) const
{
    QFileInfo fileInfo(fileName);

    // Ignore the mode argument if file is retome file
    if (!fileInfo.isDir() && FileUtils::isGvfsMountFile(fileInfo.absoluteFilePath()))
        return QMimeDatabase::mimeTypeForFile(fileName, QMimeDatabase::MatchExtension);

    return QMimeDatabase::mimeTypeForFile(fileName, mode);
}

QMimeType DMimeDatabase::mimeTypeForFile(const QFileInfo &fileInfo, QMimeDatabase::MatchMode mode) const
{
    // Ignore the mode argument if file is retome file
    if (!fileInfo.isDir() && FileUtils::isGvfsMountFile(fileInfo.absoluteFilePath()))
        return QMimeDatabase::mimeTypeForFile(fileInfo, QMimeDatabase::MatchExtension);

    return QMimeDatabase::mimeTypeForFile(fileInfo, mode);
}

QMimeType DMimeDatabase::mimeTypeForUrl(const QUrl &url) const
{
    if (url.isLocalFile())
        return mimeTypeForFile(url.toLocalFile());

    return QMimeDatabase::mimeTypeForUrl(url);
}

DFM_END_NAMESPACE
