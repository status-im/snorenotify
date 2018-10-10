/*
    SnoreNotify is a Notification Framework based on Qt
    Copyright (C) 2015  Hannah von Reth <vonreth@kde.org>

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

#ifndef SNORENOTIFIER_H
#define SNORENOTIFIER_H

#include "libsnore/plugins/snorebackend.h"

#include "notifywidget.h"

namespace SnorePlugin {

class Snore : public ::Snore::SnoreBackend
{
    Q_OBJECT
    Q_INTERFACES(Snore::SnoreBackend)
    Q_PLUGIN_METADATA(IID "org.Snore.NotificationBackend/1.0" FILE "snore_plugin.json")
public:
    Snore();
    ~Snore();

    bool canCloseNotification() const override;
    bool canUpdateNotification() const override;
    int maxNumberOfActiveNotifications() const override;

protected:
    void setDefaultSettings() override;
public Q_SLOTS:
    virtual void slotNotify(::Snore::Notification notification) override;
    virtual void slotCloseNotification(::Snore::Notification notification) override;
    virtual void setDockBadgeLabel(const QString label) override;
    void slotRegisterApplication(const ::Snore::Application &application);

private:
    QVector<NotifyWidget *> m_widgets;
};

}
#endif // SNORENOTIFIER_H
