//
//  linphone.m
//  linphone
//
//  Created by jehan on 27/09/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "linphone.h"




LinphoneCoreVTable linphonec_vtable = {
.show =(ShowInterfaceCb) NULL,
.inv_recv = NULL,
.bye_recv = NULL, 
.notify_recv = NULL,
.new_unknown_subscriber = NULL,
.auth_info_requested = NULL,
.display_status = NULL,
.display_message=NULL,
.display_warning=NULL,
.display_url=NULL,
.display_question=(DisplayQuestionCb)NULL,
.text_received=NULL,
.general_state=NULL,
.dtmf_received=NULL
};


@implementation linphone

-(void)init {
	
	/*
	 * Set initial values for global variables
	 */
	mylogfile = NULL;
	snprintf(configfile_name, PATH_MAX, "%s/.linphonerc",
			 (CFStringRef)NSHomeDirectory());
	if (trace_level > 0)
	{
		if (logfile_name != NULL)
			mylogfile = fopen (logfile_name, "w+");
		
		if (mylogfile == NULL)
		{
			mylogfile = stdout;
			fprintf (stderr,
					 "INFO: no logfile, logging to stdout\n");
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
	linphone_core_init (&mCore, &linphonec_vtable, configfile_name,
						NULL);
	
	[NSTimer scheduledTimerWithTimeInterval:0.1 
									 target:self 
								   selector:@selector(iterate) 
								   userInfo:nil 
									repeats:YES];
	
	
}

-(void) iterate {
	linphone_core_iterate(&mCore);
}
@end
