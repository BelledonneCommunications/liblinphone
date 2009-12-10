//
//  IncallViewController.h
//  linphone
//
//  Created by jehan on 09/12/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "linphonecore.h"
#import "PhoneViewController.h"

@interface IncallViewController : UIViewController {
	LinphoneCore* myLinphoneCore;
	
	UILabel* peerNumber;
	UIButton* end;
	id<PhoneViewControllerDelegate> phoneviewDelegate;

}

-(void) setLinphoneCore:(LinphoneCore*) lc;
-(void) startCall;

- (IBAction)doAction:(id)sender;

@property (nonatomic, retain) IBOutlet UILabel* peerNumber;
@property (nonatomic, retain) IBOutlet UIButton* end;
@property (nonatomic, retain) id<PhoneViewControllerDelegate> phoneviewDelegate;
@end
