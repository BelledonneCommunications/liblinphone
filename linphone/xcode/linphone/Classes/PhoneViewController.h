//
//  UIViewController.h
//  linphone
//
//  Created by jehan on 28/09/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface PhoneViewController : UIViewController <UITextFieldDelegate> {
	//UI definition
	UITextField* address;
	UIButton* call;
	UIButton* cancel;
	UILabel* status;
	
}
@property (nonatomic, retain) IBOutlet UITextField* address;
@property (nonatomic, retain) IBOutlet UIButton* call;
@property (nonatomic, retain) IBOutlet UIButton* cancel;
@property (nonatomic, retain) IBOutlet UILabel* status;

- (IBAction)doAction:(id)sender;
@end
