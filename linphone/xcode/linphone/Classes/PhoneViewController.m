//
//  UIViewController.m
//  linphone
//
//  Created by jehan on 28/09/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "PhoneViewController.h"


void linphone_iphone_log(struct _LinphoneCore * lc, const char * message) {
	NSLog([NSString stringWithCString:message length:strlen(message)]);
}

void linphone_iphone_display_status(struct _LinphoneCore * lc, const char * message) {
	PhoneViewController* lPhone = linphone_core_get_user_data(lc);
	[lPhone.status setText:[NSString stringWithCString:message length:strlen(message)]];
}

void linphone_iphone_show(struct _LinphoneCore * lc) {

}
LinphoneCoreVTable linphonec_vtable = {
.show =(ShowInterfaceCb) linphone_iphone_show,
.inv_recv = NULL,
.bye_recv = NULL, 
.notify_recv = NULL,
.new_unknown_subscriber = NULL,
.auth_info_requested = NULL,
.display_status = linphone_iphone_display_status,
.display_message=linphone_iphone_log,
.display_warning=linphone_iphone_log,
.display_url=NULL,
.display_question=(DisplayQuestionCb)NULL,
.text_received=NULL,
.general_state=NULL,
.dtmf_received=NULL
};



@implementation PhoneViewController
@synthesize  address ;
@synthesize  call;
@synthesize  cancel;
@synthesize status;

-(IBAction) doAction:(id)sender {
	
	if (sender == call) {
		const char* lCallee = [[address text]  cStringUsingEncoding:[NSString defaultCStringEncoding]];
		int err = linphone_core_invite(mCore,lCallee ) ;		
	} else if (sender == cancel) {
		linphone_core_terminate_call(mCore,NULL);
	}

}


/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        // init lib linphone
    }
    
	return self;
}
*/

/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/

/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


- (BOOL)textFieldShouldReturn:(UITextField *)theTextField {
    if (theTextField == address) {
        [address resignFirstResponder];
    }
    return YES;
}


/*************
 *lib linphone methods
 */
-(void)startlibLinphone  {
	
	traceLevel = 9;
	//copy linphonerc from bundle
	NSBundle* myBundle = [NSBundle mainBundle];
	NSString* defaultConfigFile = [myBundle pathForResource:@"linphonerc"ofType:nil] ;
	
	confiFileName = [NSHomeDirectory() stringByAppendingString:@"/.linphonerc"];
	logFileName = [NSHomeDirectory() stringByAppendingString:@"/linphone.log"]; 	
	//remove existing config file
	[[NSFileManager defaultManager] removeItemAtPath:confiFileName error:nil ]; 
	//copy config file from bundle
	[[NSFileManager defaultManager] copyItemAtPath:defaultConfigFile toPath:confiFileName error:nil ]; 
	
	/* Set initial values for global variables
	 */
	mylogfile = NULL;
	if (traceLevel > 0) {
		mylogfile = fopen ([logFileName cStringUsingEncoding:[NSString defaultCStringEncoding]], "w+");
		
		if (mylogfile == NULL)
		{
			mylogfile = stdout;
			fprintf (stderr,"INFO: no logfile, logging to stdout\n");
		}
		linphone_core_enable_logs(mylogfile);
	}
	else
	{
		linphone_core_disable_logs();
	}
	/*
	 * Initialize auth stack
	 */
	auth_stack.nitems=0;

	/*
	 * Initialize linphone core
	 */
	
	mCore = linphone_core_new (&linphonec_vtable, [confiFileName cStringUsingEncoding:[NSString defaultCStringEncoding]],self);
	
	const char*  lRing = [[myBundle pathForResource:@"oldphone"ofType:@"wav"] cStringUsingEncoding:[NSString defaultCStringEncoding]];
	linphone_core_set_ring(mCore, lRing );
	const char*  lRingBack = [[myBundle pathForResource:@"ringback"ofType:@"wav"] cStringUsingEncoding:[NSString defaultCStringEncoding]];
	linphone_core_set_ringback(mCore, lRingBack);
	
	//start liblinphone scheduler
	[NSTimer scheduledTimerWithTimeInterval:0.1 
									 target:self 
								   selector:@selector(iterate) 
								   userInfo:nil 
									repeats:YES];
	
	
}

-(void) iterate {
	linphone_core_iterate(mCore);
}


- (void)dealloc {
    [address dealloc];
	[call dealloc];
	[cancel dealloc];
	[status dealloc];
	[super dealloc];
}


@end
