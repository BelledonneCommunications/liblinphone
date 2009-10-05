//
//  UIViewController.m
//  linphone
//
//  Created by jehan on 28/09/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "PhoneViewController.h"
#import "osip2/osip.h"

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

@synthesize one;
@synthesize two;
@synthesize three;
@synthesize four;
@synthesize five;
@synthesize six;
@synthesize seven;
@synthesize eight;
@synthesize nine;
@synthesize star;
@synthesize zero;
@synthesize hash;

-(IBAction) doAction:(id)sender {
	
	if (sender == call) {
	if (!linphone_core_in_call(mCore)) {
		const char* lCallee = [[address text]  cStringUsingEncoding:[NSString defaultCStringEncoding]];
		linphone_core_invite(mCore,lCallee ) ;		
	}
	} else if (sender == cancel) {
		linphone_core_terminate_call(mCore,NULL);
	} 

}

-(IBAction) doKeyPad:(id)sender {
	if (linphone_core_in_call(mCore)) {
	//incall behavior
		if (sender == one) {
			linphone_core_send_dtmf(mCore,"1");	
		} else if (sender == two) {
			linphone_core_send_dtmf(mCore,"2");	
		} else if (sender == three) {
			linphone_core_send_dtmf(mCore,"3");	
		} else if (sender == four) {
			linphone_core_send_dtmf(mCore,"4");	
		} else if (sender == five) {
			linphone_core_send_dtmf(mCore,"5");	
		} else if (sender == six) {
			linphone_core_send_dtmf(mCore,"6");	
		} else if (sender == seven) {
			linphone_core_send_dtmf(mCore,"7");	
		} else if (sender == eight) {
			linphone_core_send_dtmf(mCore,"8");	
		} else if (sender == nine) {
			linphone_core_send_dtmf(mCore,"9");	
		} else if (sender == star) {
			linphone_core_send_dtmf(mCore,"*");	
		} else if (sender == zero) {
			linphone_core_send_dtmf(mCore,"0");	
		} else if (sender == hash) {
			linphone_core_send_dtmf(mCore,"#");	
		} else  {
			NSLog(@"unknown event from dial pad");	
		}
	} else {
		//outcall behavior	
		NSString* newAddress = nil;
		if (sender == one) {
			newAddress = [address.text stringByAppendingString:@"1"];
		} else if (sender == two) {
			newAddress = [address.text stringByAppendingString:@"2"];
		} else if (sender == three) {
			newAddress = [address.text stringByAppendingString:@"3"];
		} else if (sender == four) {
			newAddress = [address.text stringByAppendingString:@"4"];
		} else if (sender == five) {
			newAddress = [address.text stringByAppendingString:@"5"];
		} else if (sender == six) {
			newAddress = [address.text stringByAppendingString:@"6"];
		} else if (sender == seven) {
			newAddress = [address.text stringByAppendingString:@"7"];
		} else if (sender == eight) {
			newAddress = [address.text stringByAppendingString:@"8"];
		} else if (sender == nine) {
			newAddress = [address.text stringByAppendingString:@"9"];
		} else if (sender == star) {
			newAddress = [address.text stringByAppendingString:@"*"];
		} else if (sender == zero) {
			newAddress = [address.text stringByAppendingString:@"0"];
		} else if (sender == hash) {
			newAddress = [address.text stringByAppendingString:@"#"];
		} else  {
			NSLog(@"unknown event from diad pad");	
		}
		[address setText:newAddress];	
		if (newAddress != nil) [newAddress release];
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
	else {
		linphone_core_disable_logs();
	}
	
	/*
	 * Initialize linphone core
	 */
	
	mCore = linphone_core_new (&linphonec_vtable, [confiFileName cStringUsingEncoding:[NSString defaultCStringEncoding]],self);
	
	// Set audio assets
	const char*  lRing = [[myBundle pathForResource:@"oldphone"ofType:@"wav"] cStringUsingEncoding:[NSString defaultCStringEncoding]];
	linphone_core_set_ring(mCore, lRing );
	const char*  lRingBack = [[myBundle pathForResource:@"ringback"ofType:@"wav"] cStringUsingEncoding:[NSString defaultCStringEncoding]];
	linphone_core_set_ringback(mCore, lRingBack);
	
	
	//configure sip proxy
	//get data from Settings bundle
	NSString* accountNameUri = [[NSUserDefaults standardUserDefaults] stringForKey:@"account_preference"];
	const char* identity = [accountNameUri cStringUsingEncoding:[NSString defaultCStringEncoding]];
	
	NSString* accountPassword = [[NSUserDefaults standardUserDefaults] stringForKey:@"password_preference"];
	const char* password = [accountPassword cStringUsingEncoding:[NSString defaultCStringEncoding]];
	
	NSString* proxyUri = [[NSUserDefaults standardUserDefaults] stringForKey:@"proxy_preference"];
	const char* proxy = [proxyUri cStringUsingEncoding:[NSString defaultCStringEncoding]];
	
	if (([accountNameUri length] + [proxyUri length]) >6 ) {
		//possible valid config detected
		LinphoneProxyConfig* proxyCfg;	
		//clear auth info list
		linphone_core_clear_all_auth_info(mCore);
		//get default proxy
		linphone_core_get_default_proxy(mCore,&proxyCfg);
		if (proxyCfg == NULL) {
			//create new proxy	
			proxyCfg = linphone_proxy_config_new();
			linphone_core_add_proxy_config(mCore,proxyCfg);
        		//set to default proxy
			linphone_core_set_default_proxy(mCore,proxyCfg);
		}
		
		// add username password
		osip_from_t *from;
                LinphoneAuthInfo *info;
                osip_from_init(&from);
                if (osip_from_parse(from,identity)==0){
                        char realm[128];
                        snprintf(realm,sizeof(realm)-1,"\"%s\"",from->url->host);
                        info=linphone_auth_info_new(from->url->username,NULL,password,NULL,realm);
                        linphone_core_add_auth_info(mCore,info);
                }
                osip_from_free(from);

	        // configure proxy entries
		linphone_proxy_config_edit(proxyCfg);
		linphone_proxy_config_set_identity(proxyCfg,identity);
        	linphone_proxy_config_set_server_addr(proxyCfg,proxy);
        	linphone_proxy_config_enable_register(proxyCfg,TRUE);
		linphone_proxy_config_done(proxyCfg);
	}
	
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
