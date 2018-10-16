/*
    SnoreNotify is a Notification Framework based on Qt
    Copyright (C) 2013-2015  Hannah von Reth <vonreth@kde.org>

    SnoreNotify is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SnoreNotify is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with SnoreNotify.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "notification/notification_p.h"
#include "notification/icon.h"
#include "../hint.h"
#include "libsnore/plugins/plugins.h"
#include "libsnore/snore.h"

#include <QDateTime>
#include <QSharedData>

using namespace Snore;

uint NotificationData::notificationCount = 0;

namespace {
uint m_idCount = 1;
}
uint getNewId() {
    const QDateTime& d = QDateTime::currentDateTime();
    QString result = QString(QStringLiteral("%1%2")).arg(d.toString(QStringLiteral("Hmmsszzz"))).arg(m_idCount++);
    return result.toUInt();
}

NotificationData::NotificationData(const Snore::Application &application, const Snore::Alert &alert, const QString &title, const QString &text, const Icon &icon,
                                   int timeout, Notification::Prioritys priority):
    m_id(getNewId()),
    m_timeout(priority == Notification::Emergency ? 0 : timeout),
    m_application(application),
    m_alert(alert),
    m_title(title),
    m_text(text),
    m_icon(icon),
    m_priority(priority),
    m_hints(m_application.constHints())
{
    notificationCount++;
    qCDebug(SNORE) << "Creating Notification: ActiveNotifications" << notificationCount << "id" << m_id;
    qCDebug(SNORE) << title << text;
}

Snore::NotificationData::NotificationData(const Notification &old, const QString &title, const QString &text, const Icon &icon, int timeout, Notification::Prioritys priority):
    m_id(getNewId()),
    m_timeout(priority == Notification::Emergency ? 0 : timeout),
    m_application(old.application()),
    m_alert(old.alert()),
    m_title(title),
    m_text(text),
    m_icon(icon),
    m_priority(priority),
    m_hints(m_application.constHints()),
    m_toReplace(old)
{
    notificationCount++;
    qCDebug(SNORE) << "Creating Notification: ActiveNotifications" << notificationCount << "id" << m_id;
    qCDebug(SNORE) << title << text;
}

NotificationData::~NotificationData()
{
    stopTimeoutTimer();
    notificationCount--;
    qCDebug(SNORE) << "Deleting Notification: ActiveNotifications" << notificationCount << "id" << m_id << "Close Reason:" << m_closeReason;
}

void NotificationData::setActionInvoked(const Snore::Action &action)
{
    m_actionInvoked = action;
}

void NotificationData::setCloseReason(Snore::Notification::CloseReasons r)
{
    m_closeReason = r;
    stopTimeoutTimer();
}

QString NotificationData::resolveMarkup(const QString &string, Utils::MarkupFlags flags)
{
    if (!m_hints.value("use-markup").toBool()) {
        if (flags == Utils::NoMarkup) {
            return string;
        } else {
            return Utils::normalizeMarkup(string.toHtmlEscaped(), flags);
        }
    } else {
        return Utils::normalizeMarkup(string, flags);
    }
}

void NotificationData::setBroadcasted()
{
    m_isBroadcasted = true;
}

bool NotificationData::isBroadcasted() const
{
    return m_isBroadcasted;
}

void NotificationData::setSource(SnorePlugin *soure)
{
    m_source = soure;
}

const SnorePlugin *NotificationData::source() const
{
    return m_source;
}

bool NotificationData::sourceAndTargetAreSimilar(const SnorePlugin *target)
{
    if (source() && source()->name() == target->name()) {
        qCDebug(SNORE) << "Source" << source() << "and Target" << target << "are the same.";
        return true;
    }
    return false;
}

void NotificationData::stopTimeoutTimer()
{
    if (m_timeoutTimer) {
        m_timeoutTimer->deleteLater();
        m_timeoutTimer = nullptr;
    }
}

