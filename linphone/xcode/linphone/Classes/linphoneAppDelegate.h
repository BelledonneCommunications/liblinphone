//
//  linphoneAppDelegate.h
//  linphone
//
//  Created by jehan on 25/09/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import <UIKit/UIKit.h>


@class PhoneViewController;

@interface linphoneAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
	PhoneViewController *myViewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet PhoneViewController *myViewController;

@end

