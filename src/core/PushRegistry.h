#include <PushKit/PushKit.h>
#include <PushKit/PKPushRegistry.h>
#include <UserNotifications/UserNotifications.h>
#include <UIKit/UIKit.h>

// TODO: Remove me
#include "private.h"
@interface RegistryDelegate : NSObject <PKPushRegistryDelegate> {
	std::shared_ptr<LinphonePrivate::Core> pcore;
	NSData *pkToken;
	NSData *rmToken;
}
- (void)setCore:(std::shared_ptr<LinphonePrivate::Core> )core;
@end
