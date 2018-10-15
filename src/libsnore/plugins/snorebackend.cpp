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

#include "snorebackend.h"
#include "../snore.h"
#include "../snore_p.h"
#include "../application.h"
#include "../notification/notification.h"
#include "../notification/notification_p.h"

#include <QTimer>
#include <QThread>
#include <QMetaMethod>

using namespace Snore;

SnoreBackend::SnoreBackend()
{
    connect(this, &SnoreBackend::enabledChanged, [this](bool enabled) {
        if (enabled) {
            connect(getSnoreCore()->impl(), &SnoreCorePrivate::applicationRegistered, this, &SnoreBackend::slotRegisterApplication, Qt::QueuedConnection);
            connect(getSnoreCore()->impl(), &SnoreCorePrivate::applicationDeregistered, this, &SnoreBackend::slotDeregisterApplication, Qt::QueuedConnection);

            connect(this, &SnoreBackend::notificationClosed, getSnoreCore()->impl(), &SnoreCorePrivate::slotNotificationClosed, Qt::QueuedConnection);
            connect(getSnoreCore()->impl(), &SnoreCorePrivate::notify, this, &SnoreBackend::slotNotify, Qt::QueuedConnection);

            for (const Application &a : getSnoreCore()->aplications()) {
                slotRegisterApplication(a);
            }
        } else {
            for (const Application &a : getSnoreCore()->aplications()) {
                slotDeregisterApplication(a);
            }
            disconnect(getSnoreCore()->impl(), &SnoreCorePrivate::applicationRegistered, this, &SnoreBackend::slotRegisterApplication);
            disconnect(getSnoreCore()->impl(), &SnoreCorePrivate::applicationDeregistered, this, &SnoreBackend::slotDeregisterApplication);

            disconnect(this, &SnoreBackend::notificationClosed, getSnoreCore()->impl(), &SnoreCorePrivate::slotNotificationClosed);
            disconnect(getSnoreCore()->impl(), &SnoreCorePrivate::notify, this, &SnoreBackend::slotNotify);
        }
    });
}

SnoreBackend::~SnoreBackend()
{
    qCDebug(SNORE) << "Deleting" << name();
}

void SnoreBackend::requestCloseNotification(Notification notification, Notification::CloseReasons reason)
{
    if (notification.isValid()) {
        if (canCloseNotification()) {
            slotCloseNotification(notification);
            closeNotification(notification, reason);
        }
    }
}

void SnoreBackend::closeNotification(Notification n, Notification::CloseReasons reason)
{
    if (!n.isValid()) {
        return;
    }
    n.removeActiveIn(this);
    if (n.isUpdate()) {
        n.old().removeActiveIn(this);
    }
    n.data()->setCloseReason(reason);
    qCDebug(SNORE) << n << reason;
    emit notificationClosed(n);
}

void SnoreBackend::slotCloseNotification(Notification notification)
{
    Q_UNUSED(notification)
}

bool SnoreBackend::canCloseNotification() const
{
    return false;
}

bool SnoreBackend::canUpdateNotification() const
{
    return false;
}

int SnoreBackend::maxNumberOfActiveNotifications() const
{
    return 3;
}

SnorePlugin::PluginTypes SnoreBackend::type() const
{
    return Backend;
}

void SnoreBackend::slotRegisterApplication(const Application &application)
{
    Q_UNUSED(application);
}

void SnoreBackend::slotDeregisterApplication(const Application &application)
{
    Q_UNUSED(application);
}

void SnoreBackend::slotNotificationDisplayed(Notification notification)
{
    notification.addActiveIn(this);
    getSnoreCore()->impl()->slotNotificationDisplayed(notification);
}

void SnoreBackend::slotNotificationActionInvoked(Notification notification, const Action &action)
{
    notification.data()->setActionInvoked(action);
    getSnoreCore()->impl()->slotNotificationActionInvoked(notification);
}

