/* linphoneAppDelegate.m
 *
 * Copyright (C) 2009  Belledonne Comunications, Grenoble, France
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or   
 *  (at your option) any later version.                                 
 *                                                                      
 *  This program is distributed in the hope that it will be useful,     
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of      
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       
 *  GNU Library General Public License for more details.                
 *                                                                      
 *  You should have received a copy of the GNU General Public License   
 *  along with this program; if not, write to the Free Software         
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */                                                                           

#import "PhoneViewController.h"
#import "linphoneAppDelegate.h"
#import "ContactPickerDelegate.h"
#import "IncallViewController.h"


@implementation linphoneAppDelegate

@synthesize window;
@synthesize myTabBarController;
@synthesize myPeoplePickerController;



- (void)applicationDidFinishLaunching:(UIApplication *)application {    
	
#define DIALER_TAB_INDEX 0
#define CONTACTS_TAB_INDEX 1  
	
	
	
	myPeoplePickerController = [[[ABPeoplePickerNavigationController alloc] init] autorelease];
	myContactPickerDelegate = [[ContactPickerDelegate alloc] init];
	
	myContactPickerDelegate.phoneControllerDelegate=(PhoneViewController*)[myTabBarController.viewControllers objectAtIndex: DIALER_TAB_INDEX];
	myContactPickerDelegate.linphoneDelegate=self;
	
	[myPeoplePickerController setPeoplePickerDelegate:myContactPickerDelegate];
	//copy tab bar item
	myPeoplePickerController.tabBarItem = [(UIViewController*)[myTabBarController.viewControllers objectAtIndex:CONTACTS_TAB_INDEX] tabBarItem]; 
	//insert contact controller
	NSMutableArray* newArray = [NSMutableArray arrayWithArray:self.myTabBarController.viewControllers];
	[newArray replaceObjectAtIndex:1 withObject:myPeoplePickerController];
	
	[myTabBarController setViewControllers:newArray animated:NO];
	
	[window addSubview:myTabBarController.view];

    [window makeKeyAndVisible];

	
}
-(void)selectDialerTab {
	[myTabBarController setSelectedIndex:0];
}


- (void)dealloc {
    [window release];
	[myPeoplePickerController release];
    [super dealloc];
}


@end
