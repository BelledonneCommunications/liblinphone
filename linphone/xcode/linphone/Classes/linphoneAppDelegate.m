//
//  linphoneAppDelegate.m
//  linphone
//
//  Created by jehan on 25/09/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "linphoneAppDelegate.h"
#import "mainView.h"

@implementation linphoneAppDelegate

@synthesize window;


- (void)applicationDidFinishLaunching:(UIApplication *)application {    
	mainView *lView = [[mainView alloc] initWithFrame:[window frame]];
	[window addSubview:lView];
	[lView release];	
    // Override point for customization after application launch
    [window makeKeyAndVisible];
}


- (void)dealloc {
    [window release];
    [super dealloc];
}


@end
