/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef SHAREFILEINFO_H
#define SHAREFILEINFO_H

#include <QObject>
#include "abstractfileinfo.h"
#include "app/fmevent.h"

class ShareFileInfo : public AbstractFileInfo
{
public:
    ShareFileInfo();
    ShareFileInfo(const DUrl& url);
    ShareFileInfo(const QString& url);
    ~ShareFileInfo();

    bool isCanRename() const Q_DECL_OVERRIDE;
    bool isReadable() const Q_DECL_OVERRIDE;
    bool isWritable() const Q_DECL_OVERRIDE;

    QString displayName() const Q_DECL_OVERRIDE;
    void setUrl(const DUrl &fileUrl) Q_DECL_OVERRIDE;

    QIcon fileIcon() const Q_DECL_OVERRIDE;
    QMimeType mimeType() const Q_DECL_OVERRIDE;

    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;
    QSet<MenuAction> disableMenuActionList() const Q_DECL_OVERRIDE;

    bool columnDefaultVisibleForRole(int userColumnRole) const Q_DECL_OVERRIDE;

    MenuAction menuActionByColumnRole(int userColumnRole) const Q_DECL_OVERRIDE;

    bool canIteratorDir() const Q_DECL_OVERRIDE;
    bool makeAbsolute() Q_DECL_OVERRIDE;

    DUrl mimeDataUrl() const Q_DECL_OVERRIDE;

    bool isShared() const Q_DECL_OVERRIDE;

    QAbstractItemView::SelectionMode supportSelectionMode() const Q_DECL_OVERRIDE;
    Qt::ItemFlags fileItemDisableFlags() const Q_DECL_OVERRIDE;

private:
    QString m_displayName;

};

#endif // SHAREFILEINFO_H