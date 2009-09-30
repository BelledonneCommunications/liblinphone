//
//  UIViewController.h
//  linphone
//
//  Created by jehan on 28/09/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#import "linphonecore.h"


/*
 * Maximum number of pending authentications
 */
#define MAX_PENDING_AUTH 8
typedef struct {
	LinphoneAuthInfo *elem[MAX_PENDING_AUTH];
	int nitems;
} LPC_AUTH_STACK;


@interface PhoneViewController : UIViewController <UITextFieldDelegate> {

@private
	//UI definition
	UITextField* address;
	UIButton* call;
	UIButton* cancel;
	UILabel* status;

	/*
	 * lib linphone main context
	 */
	LinphoneCore* mCore;
	FILE *mylogfile;
	int traceLevel;
	NSString* confiFileName;
	NSString* logFileName;
	LPC_AUTH_STACK auth_stack;	
	
}
@property (nonatomic, retain) IBOutlet UITextField* address;
@property (nonatomic, retain) IBOutlet UIButton* call;
@property (nonatomic, retain) IBOutlet UIButton* cancel;
@property (nonatomic, retain) IBOutlet UILabel* status;

/**********************************
 * liblinphone initialization method
 **********************************/
-(void) startlibLinphone;

/*
 * liblinphone scheduling method;
 */
-(void) iterate;

/********************
 * UI method handlers
 ********************/
- (IBAction)doAction:(id)sender;
@end
