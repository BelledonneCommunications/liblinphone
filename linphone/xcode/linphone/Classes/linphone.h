//
//  linphone.h
//  linphone
//
//  Created by jehan on 27/09/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

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


@interface linphone : NSObject {

@private
	/*
	 * lib linphone main context
	 */
	LinphoneCore mCore;
	FILE *mylogfile;
	int trace_level ;
	char *logfile_name;
	char configfile_name[PATH_MAX];
	
	LPC_AUTH_STACK auth_stack;
	
}


/*
 * liblinphone initialization method
 */
-(void) init;

/*
 * liblinphone schedulin method;
 */
-(void) iterate;



@end
