#include "osxnotificationcenter.h"
#include "libsnore/plugins/snorebackend.h"
#include "libsnore/notification/notification_p.h"
#include "libsnore/utils.h"
#include "libsnore/snore.h"

#include <QDebug.h>
#import <QThread.h>
#import <QMap>
#include <Foundation/Foundation.h>
#import <objc/runtime.h>

using namespace Snore;

QMap<int, Notification> m_IdToNotification;
NSMutableDictionary * m_IdToNSNotification;
OSXNotificationCenter * notificationCenter = nil;


void emitNotificationClicked(Notification notification) {
    emit SnoreCore::instance().actionInvoked(notification);
}

//
// Overcome need for bundle identifier to display notification
//

#pragma mark - Swizzle NSBundle

@implementation NSBundle(swizle)
// Overriding bundleIdentifier works, but overriding NSUserNotificationAlertStyle does not work.
- (NSString *)__bundleIdentifier
{
    if (self == [NSBundle mainBundle] && ![[self __bundleIdentifier] length]) {
        return @"com.apple.terminal";
    } else {
        return [self __bundleIdentifier];
    }
}

@end
BOOL installNSBundleHook()
{
    Class cls = objc_getClass("NSBundle");
    if (cls) {
        method_exchangeImplementations(class_getInstanceMethod(cls, @selector(bundleIdentifier)),
                                       class_getInstanceMethod(cls, @selector(__bundleIdentifier)));
        return YES;
    }
    return NO;
}


//
// Enable reaction when user clicks on NSUserNotification
//

@interface UserNotificationItem : NSObject<NSUserNotificationCenterDelegate> { }

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification;
@end

@implementation UserNotificationItem
- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification {
    Q_UNUSED(center);
    Q_UNUSED(notification);
    return YES;
}
- (void) userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification
{
    qCDebug(SNORE) << "User clicked on notification";
    int notificationId = [notification.userInfo[@"id"] intValue];
    [center removeDeliveredNotification: notification];
    if (!m_IdToNotification.contains(notificationId)) {
        qCWarning(SNORE) << "User clicked on notification that was not recognized. Notification id:" << notificationId;
        return;
    } else {
        auto snoreNotification = m_IdToNotification[notificationId];
        snoreNotification.data()->setCloseReason(Notification::Activated);
        emitNotificationClicked(snoreNotification);
    }
}
- (void) userNotificationCenter:(NSUserNotificationCenter *)center didDeliverNotification:(NSUserNotification *)notification
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
        ^{
            int notificationId = [notification.userInfo[@"id"] intValue];
            BOOL notificationAvailable = YES;
            while(notificationAvailable) {
                notificationAvailable = NO;
                for(NSUserNotification *osxNotification in [[NSUserNotificationCenter defaultUserNotificationCenter] deliveredNotifications]) {
                   int fetchedNotificationID = [osxNotification.userInfo[@"id"] intValue];
                   if(fetchedNotificationID == notificationId) {
                        notificationAvailable = YES;
                        [NSThread sleepForTimeInterval:0.25f];
                        break;
                    }
                }
            }
            dispatch_async(dispatch_get_main_queue(), ^{
                if (!m_IdToNotification.contains(notificationId)) {
                    qCWarning(SNORE) << "Delivered notification is not recognized and will not be remove from active list. Notification id:" << notificationId;
                    return;
                }
            });
            qCWarning(SNORE) << "Notification with following id is delivered or dismissed:" << notificationId;
            auto snoreNotification = m_IdToNotification.take(notificationId);
            snoreNotification.removeActiveIn(notificationCenter);
            snoreNotification.removeActiveIn(&SnoreCore::instance());
        });
}
@end

class UserNotificationItemClass
{
public:
    UserNotificationItemClass() {
        item = [UserNotificationItem alloc];
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_8) {
            [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:item];
        }
    }
    ~UserNotificationItemClass() {
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_8) {
            [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:nil];
        }
        [item release];
    }
    UserNotificationItem *item;
};



static UserNotificationItemClass * delegate = 0;

OSXNotificationCenter::OSXNotificationCenter()
{
    installNSBundleHook();
    m_IdToNSNotification = [[NSMutableDictionary alloc] init];
    if (!delegate) {
        delegate = new UserNotificationItemClass();
    }
    notificationCenter = this;
}

OSXNotificationCenter::~OSXNotificationCenter()
{
    delete delegate;
}

void OSXNotificationCenter::slotNotify(Snore::Notification notification)
{
    qDebug() << "!!! OSXNotificationCenter::slotNotify 2";
    NSUserNotification * osxNotification = [[[NSUserNotification alloc] init] autorelease];
    NSString * notificationId = [NSString stringWithFormat:@"%d",notification.id()];
    osxNotification.title = notification.title().toNSString();
    osxNotification.userInfo = [NSDictionary dictionaryWithObjectsAndKeys:notificationId, @"id", nil];
    osxNotification.informativeText = notification.text().toNSString();
    //osxNotification.identifier = notificationId;
    
    // Add notification to mapper from id to Nofification / NSUserNotification
    m_IdToNotification.insert(notification.id(), notification);
    [m_IdToNSNotification setObject:osxNotification forKey: notificationId];
    [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification: osxNotification];
    
    slotNotificationDisplayed(notification);
}


