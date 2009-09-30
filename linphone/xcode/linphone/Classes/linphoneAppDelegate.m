//
//  linphoneAppDelegate.m
//  linphone
//
//  Created by jehan on 25/09/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "PhoneViewController.h"
#import "linphoneAppDelegate.h"



@implementation linphoneAppDelegate

@synthesize window;
@synthesize myViewController;

- (void)applicationDidFinishLaunching:(UIApplication *)application {    
	
	PhoneViewController *aViewController = [[PhoneViewController alloc]
										 initWithNibName:@"PhoneViewController" bundle:[NSBundle mainBundle]];
	
	
	[self  setMyViewController:aViewController];
	[aViewController release];

	[window addSubview:[myViewController view]];

	//init lib linphone
	[aViewController startlibLinphone] ;

    [window makeKeyAndVisible];
	
}


- (void)dealloc {
    [window release];
	[myViewController release];
    [super dealloc];
}


@end
