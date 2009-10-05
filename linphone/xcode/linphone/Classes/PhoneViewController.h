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

@interface PhoneViewController : UIViewController <UITextFieldDelegate> {

@private
	//UI definition
	UITextField* address;
	UIButton* call;
	UIButton* cancel;
	UILabel* status;

	//key pad
	UIButton* one;
	UIButton* two;
	UIButton* three;
	UIButton* four;
	UIButton* five;
	UIButton* six;
	UIButton* seven;
	UIButton* eight;
	UIButton* nine;
	UIButton* star;
	UIButton* zero;
	UIButton* hash;

	/*
	 * lib linphone main context
	 */
	LinphoneCore* mCore;
	FILE *mylogfile;
	int traceLevel;
	NSString* confiFileName;
	NSString* logFileName;
	
}
@property (nonatomic, retain) IBOutlet UITextField* address;
@property (nonatomic, retain) IBOutlet UIButton* call;
@property (nonatomic, retain) IBOutlet UIButton* cancel;
@property (nonatomic, retain) IBOutlet UILabel* status;

@property (nonatomic, retain) IBOutlet UIButton* one;
@property (nonatomic, retain) IBOutlet UIButton* two;
@property (nonatomic, retain) IBOutlet UIButton* three;
@property (nonatomic, retain) IBOutlet UIButton* four;
@property (nonatomic, retain) IBOutlet UIButton* five;
@property (nonatomic, retain) IBOutlet UIButton* six;
@property (nonatomic, retain) IBOutlet UIButton* seven;
@property (nonatomic, retain) IBOutlet UIButton* eight;
@property (nonatomic, retain) IBOutlet UIButton* nine;
@property (nonatomic, retain) IBOutlet UIButton* star;
@property (nonatomic, retain) IBOutlet UIButton* zero;
@property (nonatomic, retain) IBOutlet UIButton* hash;

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

//method to handle cal/hangup events
- (IBAction)doAction:(id)sender;

// method to handle keypad event
- (IBAction)doKeyPad:(id)sender;
@end
